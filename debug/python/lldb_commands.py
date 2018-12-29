"""
This file is sourced by the `lldb.sh' after starting LLDB. It installs a few
new commands to LLDB to allow it to debug Armor types.
"""


import lldb
import rmr


def get_frame(debugger):
    target = debugger.GetSelectedTarget()
    process = target.GetProcess()
    thread = process.GetSelectedThread()
    return thread.GetSelectedFrame()


def quicklook(debugger, varname):
    """
        View Armor data structures graphically.

    Syntax: armor quicklook <variable>

        This command will construct a graphviz drawing of a trie and open it in
        quicklook. Very useful for debugging trie structure and trie related
        bugs.
    """
    frame = get_frame(debugger)
    rmr.util.lldb_visualizer_for(
        rmr.util.lldb_find_variable(frame, varname)).quicklook()


def trace_iteration(debugger, bp_index, t, it):
    """
        Trace Armor data structure iteration.

    Syntax: armor trace-iteration <breakpoint-index> <variable> <iterator>

        Assuming <breakpoint-index> is a breakpoint inside of a loop iterating
        over a trie.  This command will modify breakpoint number
        <breakpoint-index> to display a graphviz diagram and with the iterator
        location marked in it.
    """
    bp = debugger.GetSelectedTarget().GetBreakpointAtIndex(int(bp_index) - 1)
    bp.SetScriptCallbackBody("""
import rmr
t = rmr.util.lldb_visualizer_for(rmr.util.lldb_find_variable(frame, "%s"))
it = rmr.util.lldb_find_variable(frame, "%s")
t.mark(rmr.util.lldb_iterator_to_pointer(it))
t.quicklook()
return False
    """ % (t, it))


def run_command(f, *args):
    """
    Run a command or print its __doc__ if used incorrectly.
    """
    try:
        f(*args)
    except TypeError:
        for line in f.__doc__.split("\n")[1:-1]:
            print line[4:]


def armor_main(debugger, cmdline, result, internal_dict):
    args = filter(lambda x: len(x) != 0, cmdline.strip().split(" "))
    try:
        command, args = args[0], args[1:]
    except IndexError:
        print """Armor debugging utilities.

Syntax: armor <subcommand> [<subcommand-options>]

The following subcommands are supported:
    quicklook       -- View Armor data structures graphically.
    trace-iteration -- Trace Armor data structure iteration.
"""
        return
    if command == "quicklook":
        run_command(quicklook, debugger, *args)
    elif command == "trace-iteration":
        run_command(trace_iteration, debugger, *args)
    else:
        print "invalid command 'armor %s'." % command


def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand(
        "command script add -f lldb_commands.armor_main armor")
