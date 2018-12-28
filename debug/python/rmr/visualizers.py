"""
A set of visualizers for Armor associative containers for use in LLDB's script
mode.
"""

import tempfile
import subprocess


class Visualizer(object):
    """
    An abstract class to visualize Armor containers using graphviz.
    """
    def __init__(self, digraph):
        self._digraph = digraph

    def _write_dot_nodes(self, key_mapper_inverse):
        raise NotImplementedError()

    def _write_dot_edges(self, key_mapper_inverse):
        raise NotImplementedError()

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
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE).wait()
        subprocess.Popen(["qlmanage", "-p", png.name],
                         stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE).wait()


class TrieVisualizerBase(Visualizer):
    def __init__(self, *args):
        super(TrieVisualizerBase, self).__init__(*args)
        self._marks = []

    def _write_dot_shape_and_color(self, dot, vertex):
        shape = "circle"
        if vertex.fields["value"]:
            shape = "doublecircle"
        color = "black"
        for mark, color_ in self._marks:
            if mark == vertex.name:
                color = color_
        dot.write("  node [shape = %s, color = %s];\n" % (shape, color))

    def mark(self, pointer, color="red"):
        self._marks.append((pointer, color))
        return self


class TrieVisualizer(TrieVisualizerBase):
    """
    A visualizer for Armor trie containers.
    """
    def _write_dot_nodes(self, dot, key_mapper_inverse):
        for vertex, _ in self._digraph:
            self._write_dot_shape_and_color(dot, vertex)
            dot.write("  \"%s\" [label = \"\"];\n" % vertex.name)

    def _write_dot_edges(self, dot, key_mapper_inverse):
        for vertex, edges in self._digraph:
            for edge in edges:
                dot.write("  \"%s\" -> \"%s\" [label = \"%s\" ]\n" %
                          (vertex.name,
                           edge.to.name,
                           key_mapper_inverse(edge.fields["value"])))


class TSTVisualizer(TrieVisualizerBase):
    """
    A visualizer for Armor ternary search tree containers.
    """
    def _write_dot_nodes(self, dot, key_mapper_inverse):
        for vertex, _ in self._digraph:
            self._write_dot_shape_and_color(dot, vertex)
            dot.write("  \"%s\" [label = \"%s\"];\n" %
                      (vertex.name,
                       key_mapper_inverse(ord(vertex.fields["c"][1]))))

    def _write_dot_edges(self, dot, _):
        for vertex, edges in self._digraph:
            for edge in edges:
                edge_value = ("l", "m", "r")[int(edge.fields["value"])]
                dot.write("  \"%s\" -> \"%s\" [label = \"%s\" ]\n" %
                          (vertex.name, edge.to.name, edge_value))
