"""
Functions for parsing Armor associative containers into directional graphs
within LLDB and GDB.
"""

import lldb
import collections


class Pointer(int):
    """
    A pointer class. Can only be constructed from hexadecimal strings.
    """
    def __new__(cls, value):
        if isinstance(value, str):
            return super(Pointer, cls).__new__(cls, value, 16)
        else:
            return NotImplemented

    def __repr__(self):
        return "Pointer(0x%016x)" % self


class Edge(collections.namedtuple("Edge", ["to", "fields"])):
    """
    A digraph edge.
    """
    def __hash__(self):
        return hash(self.to)


class Vertex(collections.namedtuple("Vertex", ["name", "fields"])):
    """
    A digraph vertex.
    """
    def __hash__(self):
        return hash(self.name)


class Digraph(object):
    """
    A simple directional graph.
    """
    def __init__(self):
        self._data = {}

    def __iter__(self):
        return self._data.iteritems()

    def add_vertex(self, vertex):
        if vertex not in self._data.keys():
            self._data[vertex] = []

    def add_edge(self, from_, to, fields):
        self.add_vertex(from_)
        self.add_vertex(to)
        self._data[from_].append(Edge(to=to, fields=fields))


def lldb_build_digraph(root):
    """
    Constuct a digraph from a container root node using LLDB's introspection
    functionality.
    """
    class DigraphBuilder(object):
        def __init__(self, root):
            self.digraph = Digraph()
            self._build_digraph(root)

        def _build_digraph(self, root):
            name = Pointer(str(root.GetAddress()))
            fields = {str(x.GetName()): str(x.GetValue())
                      for x in list(root)[1:] if x.GetValue() is not None}
            fields["value"] = \
                Pointer(str(root.GetChildMemberWithName("value").GetValue()))
            vertex = Vertex(name=name, fields=fields)

            children = root.GetChildMemberWithName("children")
            for i, child in enumerate(children):
                if Pointer(str(child.GetValue())):
                    self.digraph.add_edge(vertex,
                                          self._build_digraph(child),
                                          {"value": i})
            return vertex

    return DigraphBuilder(root).digraph

def lldb_container_root(container):
    """
    Find a container's root node using LLDB's introspection functionality.
    """
    return container.GetChildMemberWithName('trie_') \
        .GetChildMemberWithName('impl_') \
        .GetChildMemberWithName('root')


def lldb_parse_container(container):
    """
    Parse a container using LLDB's introspection functionality.
    """
    return lldb_build_digraph(lldb_container_root(container))
