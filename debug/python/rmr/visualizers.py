"""
A set of visualizers from Armor associative containers for use in LLDB's script mode.

Example:
  (lldb) b assoc_common.cc:936
  Breakpoint 1: 8 locations.
  (lldb) r
  ...
  Process 17750 stopped
  * thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 1.2
      frame #0: 0x00000001000c00c8 test_assoc_common`assoc_common_bug$value_when_reverse_climbing_tree_Test<trie_map>::TestBody(this=0x0000000101001000) at assoc_common.cc:936
     933      t.emplace(TestFixture::key_to_value("foobar"));
     934      t.emplace(TestFixture::key_to_value("foobarbaz"));
     935
  -> 936      EXPECT_EQ(3u, t.size());
     937      EXPECT_EQ(3u, std::distance(t.rbegin(), t.rend()));
  (lldb) script
  >>> from visualizers import TrieVisualizer
  >>> tvis = TrieVisualizer('t')
  >>> tvis.quicklook()

For more advanced visualization, you can use automatic breakpoint scripts and
the mark functionality. Consider a file named `assoc_common.cc' which iterates
through a trie t at line 936 like so:
  932      t.emplace(TestFixture::key_to_value("foobar"));
  933      t.emplace(TestFixture::key_to_value("foobarbaz"));
  934
  935      for (auto it = t.begin(); it != t.end(); ++it)
  936          std::cout << *it << std::endl;
  937
Configure LLDB to visualize the trie and mark the current node like this:
  (lldb) b assoc_common.cc:936
  Breakpoint 1: 8 locations.
  (lldb) b command add -s python 1
  def function (frame, bp_loc, internal_dict):
      from visualizers import TrieVisualizer
      t = TrieVisualizer(frame.FindVariable('t'))
      t.mark(frame.FindVariable('it'))
      t.quicklook()
      return False
  (lldb) r
"""

import collections
import lldb
import tempfile
import subprocess


class Digraph(object):
    Edge = collections.namedtuple("Edge", ["value", "to"])
    Vertex = collections.namedtuple("Vertex", ["name", "value"])

    def __init__(self):
        self._data = {}

    def add_vertex(self, vertex):
        if vertex not in self._data.keys():
            self._data[vertex] = []

    def add_edge(self, from_, to, value):
        self.add_vertex(from_)
        self.add_vertex(to)
        self._data[from_].append(Digraph.Edge(value=value, to=to))


class Visualizer(object):
    def __init__(self, root):
        self._digraph = Digraph()
        self.radix = len(root.GetChildMemberWithName("children"))
        self._build_digraph(root)

    def _build_digraph(self, root):
        value = root.GetChildMemberWithName("value")
        vertex = Digraph.Vertex(name=root, value=value)

        children = root.GetChildMemberWithName("children")
        for i, child in enumerate(children):
            if int(child.GetValue(), 16) != 0:
                self._digraph.add_edge(vertex, self._build_digraph(child), i)
        return vertex

    def write_dot(self, dot, key_mapper_inverse=None):
        if key_mapper_inverse is None:
            key_mapper_inverse = chr
        dot.write("digraph _ {")

        dot.write("\n// nodes\n")
        self._write_dot_nodes(dot, key_mapper_inverse)
        dot.write("\n// edges\n")
        self._write_dot_edges(dot, key_mapper_inverse)

        dot.write("}\n")

    def quicklook(self, key_mapper_inverse=None):
        dot = tempfile.NamedTemporaryFile(suffix=".dot")
        self.write_dot(dot, key_mapper_inverse)
        dot.flush()

        png = tempfile.NamedTemporaryFile(suffix=".png")
        subprocess.Popen(["dot", "-Tpng", dot.name, "-o", png.name],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE).wait()
        subprocess.Popen(["qlmanage", "-p", png.name],
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE).wait()


class TrieVisualizerBase(Visualizer):
    def __init__(self, arg):
        if isinstance(arg, str):
            trie = lldb.frame.FindVariable(arg)
            self._build_from_variable(trie)
        elif isinstance(arg, lldb.SBValue):
            self._build_from_variable(arg)
        else:
            raise NotImplementedError("Invalid type for arg (type=%s)" % type(arg))
        self._marks = []

    def _build_from_variable(self, trie):
        root = trie.GetChildMemberWithName('trie_') \
            .GetChildMemberWithName('impl_') \
            .GetChildMemberWithName('root')
        super(TrieVisualizerBase, self).__init__(root)

    def _write_dot_shape_and_color(self, dot, vertex):
        shape = "circle"
        if int(vertex.value.GetValue(), 16) != 0:
            shape = "doublecircle"
        color = "black"
        for mark, color_ in self._marks:
            vertex_addr = str(vertex.name.Dereference().GetAddress())
            mark_addr = str(mark.Dereference().GetAddress())
            if mark_addr == vertex_addr:
                color = color_
        dot.write("  node [shape = %s, color = %s];\n" % (shape, color))

    def mark(self, arg, color="red"):
        if isinstance(arg, str):
            arg = lldb.frame.FindVariable(arg)
        elif isinstance(arg, lldb.SBValue):
            pass
        else:
            raise NotImplementedError("Invalid type for arg (type=%s)" % type(arg))
        if arg.GetName() != "node":  # Is an iterator
            arg = arg.GetChildMemberWithName("node")
        self._marks.append((arg, color))
        return self


class TrieVisualizer(TrieVisualizerBase):
    def _write_dot_nodes(self, dot, key_mapper_inverse):
        for vertex in self._digraph._data.keys():
            self._write_dot_shape_and_color(dot, vertex)

            vertex_name = str(vertex.name.GetAddress())
            dot.write("  \"%s\" [label = \"\"];\n" % vertex_name)

    def _write_dot_edges(self, dot, key_mapper_inverse):
        for vertex, edges in self._digraph._data.items():
            vertex_name = str(vertex.name.GetAddress())
            for edge in edges:
                edge_to_name = str(edge.to.name.GetAddress())
                dot.write("  \"%s\" -> \"%s\" [label = \"%s\" ]\n" %
                          (vertex_name, edge_to_name, key_mapper_inverse(edge.value)))


class TSTVisualizer(TrieVisualizerBase):
    def _write_dot_nodes(self, dot, key_mapper_inverse):
        for vertex in self._digraph._data.keys():
            self._write_dot_shape_and_color(dot, vertex)

            vertex_name = str(vertex.name.GetAddress())
            vertex_label = ord(vertex.name.GetChildMemberWithName("c").GetValue()[1])
            dot.write("  \"%s\" [label = \"%s\"];\n" %
                      (vertex_name, key_mapper_inverse(vertex_label)))

    def _write_dot_edges(self, dot, _):
        for vertex, edges in self._digraph._data.items():
            vertex_name = str(vertex.name.GetAddress())
            for edge in edges:
                edge_to_name = str(edge.to.name.GetAddress())
                edge_value = ("l", "m", "r")[edge.value]
                dot.write("  \"%s\" -> \"%s\" [label = \"%s\" ]\n" %
                          (vertex_name, edge_to_name, edge_value))


def visualizer_for(arg):
    """
    Apply simple heuristics to attempt to select the correct visualizer for `arg'
    """
    if isinstance(arg, str):
        arg = lldb.frame.FindVariable(arg)
    elif isinstance(arg, lldb.SBValue):
        pass
    else:
        raise NotImplementedError("Invalid type for arg (type=%s)" % type(arg))
    typename = arg.GetChildMemberWithName("trie_").GetType().GetName()
    if typename.startswith("trie"):
        return TrieVisualizer(arg)
    if typename.startswith("ternary_search_tree"):
        return TSTVisualizer(arg)
    else:
        raise ValueError("Don't know how to parse type (name=%s)" % typename)