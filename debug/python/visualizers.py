"""
A set of visualizers from Armor associative containers for use in LLDB's script mode.

Example:
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
        if not vertex in self._data.keys():
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
        dot.write("digraph trie {")

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
        for mark in self._marks:
            vertex_addr = str(vertex.name.Dereference().GetAddress())
            mark_addr = str(mark.Dereference().GetAddress())
            if mark_addr == vertex_addr:
                color = "red"
        dot.write("  node [shape = %s, color = %s];\n" % (shape, color))


    def mark(self, arg):
        if isinstance(arg, str):
            it = lldb.frame.FindVariable(arg)
        elif isinstance(arg, lldb.SBValue):
            it = arg
        else:
            raise NotImplementedError("Invalid type for arg (type=%s)" % type(arg))
        self._marks.append(it.GetChildMemberWithName("node"))
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
