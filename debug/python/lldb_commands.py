"""
This file is sourced by the `lldb.sh' after starting LLDB. It installs a few
new commands to LLDB to allow it to debug Armor types.

1. armor-quicklook <variable>
    This command will construct a graphviz drawing of a trie and open it in
    quicklook. Very useful for debugging trie structure and trie related bugs.

2. armor-trace-iteration <bpid> <variable> <iterator>
    Assuming <bpid> is a breakpoint inside of a loop iterating over a trie.
    This command will modify breakpoint number <bpid> to display a graphviz
    diagram and with the iterator location marked in it.
"""


import lldb
import rmr


def get_frame(debugger):
    target = debugger.GetSelectedTarget()
    process = target.GetProcess()
    thread = process.GetSelectedThread()
    return thread.GetSelectedFrame()


def quicklook(debugger, command, result, internal_dict):
    frame = get_frame(debugger)
    rmr.util.lldb_visualizer_for(
        rmr.util.lldb_find_variable(frame, command)).quicklook()


def trace_iteration(debugger, command, result, internal_dict):
    args = command.strip().split(" ")
    if len(args) < 3:
        print "Usage: armor-trace-iteration <bpid> <variable> <iterator>"
        return
    bp_index, t, it = command.split(" ")[0:3]
    bp = debugger.GetSelectedTarget().GetBreakpointAtIndex(int(bp_index) - 1)
    bp.SetScriptCallbackBody("""
import rmr
t = rmr.util.lldb_visualizer_for(rmr.util.lldb_find_variable(frame, "%s"))
it = rmr.util.lldb_find_variable(frame, "%s")
t.mark(rmr.util.lldb_iterator_to_pointer(it))
t.quicklook()
return False
    """ % (t, it))


def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand(
        "command script add -f lldb_commands.quicklook armor-quicklook")
    debugger.HandleCommand(
        "command script add -f "
        "lldb_commands.trace_iteration armor-trace-iteration")
