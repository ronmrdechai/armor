"""
Various python utilities for use with Armor debugger functions.
"""

import lldb
import rmr


def lldb_find_variable(frame, name):
    """
    Find a variable. If the variable is not found, try looking for it in parent
    frames.
    """
    var = frame.FindVariable(name)
    if var.IsValid():
        return var
    thread = frame.GetThread()
    frame_id = frame.GetFrameID()
    if frame_id == thread.GetNumFrames():
        raise ValueError("Variable not found (name=%s)" % name)
    return lldb_find_variable(thread.GetFrameAtIndex(frame_id + 1), name)


def lldb_visualizer_for(arg):
    """
    Apply simple heuristics to attempt to select the correct visualizer for
    `arg'.
    """
    if isinstance(arg, str):
        container = lldb.frame.FindVariable(arg)
    elif isinstance(arg, lldb.SBValue):
        container = arg
    else:
        raise NotImplementedError("Invalid type for arg (type=%s)" % type(arg))
    typename = arg.GetChildMemberWithName("trie_").GetType().GetName()
    digraph = rmr.parsers.lldb_parse_container(container)

    if typename.startswith("trie"):
        return rmr.visualizers.TrieVisualizer(digraph)
    if typename.startswith("ternary_search_tree"):
        return rmr.visualizers.TSTVisualizer(digraph)
    else:
        raise ValueError("Don't know how to parse type (name=%s)" % typename)


def lldb_iterator_to_pointer(arg):
    """
    Return this iterator's node pointer.
    """
    if isinstance(arg, str):
        it = lldb.frame.FindVariable(arg)
    elif isinstance(arg, lldb.SBValue):
        it = arg
    else:
        raise NotImplementedError("Invalid type for arg (type=%s)" % type(arg))
    if it.GetName() != "node":  # Is an iterator
        it = it.GetChildMemberWithName("node")
    return rmr.parsers.Pointer(str(it.Dereference().GetAddress()))
