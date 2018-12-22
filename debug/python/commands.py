import lldb
import visualizers

def get_frame(debugger):
    target = debugger.GetSelectedTarget()
    process = target.GetProcess()
    thread = process.GetSelectedThread()
    return thread.GetSelectedFrame()

class Commands(object):
    @classmethod
    def quicklook(cls, debugger, command, result, internal_dict):
        frame = get_frame(debugger)
        cls.VISUALIZER(frame.FindVariable(command)).quicklook()

    @classmethod
    def trace_iteration(cls, debugger, command, result, internal_dict):
        name, t, it = command.split(" ")
        debugger.HandleCommand("b %s" % name)

        target = debugger.GetSelectedTarget()
        bp = target.GetBreakpointAtIndex(target.GetNumBreakpoints() - 1)
        bp.SetScriptCallbackBody("""
    import visualizers
    t = visualizers.%s(frame.FindVariable("%s"))
    t.mark(frame.FindVariable("%s"))
    t.quicklook()
    return False
    """ % (cls.VISUALIZER_NAME, t, it))


class TrieCommands(Commands):
    VISUALIZER = visualizers.TrieVisualizer
    VISUALIZER_NAME = "TrieVisualizer"


def quicklook_trie(debugger, command, result, internal_dict):
    TrieCommands.quicklook(debugger, command, result, internal_dict)

def trace_iteration_trie(debugger, command, result, internal_dict):
    TrieCommands.trace_iteration(debugger, command, result, internal_dict)

class TSTCommands(Commands):
    VISUALIZER = visualizers.TrieVisualizer
    VISUALIZER_NAME = "TrieVisualizer"


def quicklook_tst(debugger, command, result, internal_dict):
    TSTCommands.quicklook(debugger, command, result, internal_dict)

def trace_iteration_tst(debugger, command, result, internal_dict):
    TSTCommands.trace_iteration(debugger, command, result, internal_dict)

def __lldb_init_module(debugger, internal_dict):
    for typ in ("trie", "tst"):
        debugger.HandleCommand(
            'command script add -f commands.quicklook_%s quicklook-%s' % (typ, typ))
        debugger.HandleCommand(
            'command script add -f commands.trace_iteration_%s trace-iteration-%s' % (typ, typ))
