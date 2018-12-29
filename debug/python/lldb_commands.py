# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

"""
This file is sourced by the `lldb.sh' after starting LLDB. It installs a new
command to allow LLDB to debug Armor types.
"""


import lldb
import rmr
import shlex


class RegisterSubclasses(type):
    def __init__(cls, name, bases, nmspc):
        super(RegisterSubclasses, cls).__init__(name, bases, nmspc)
        if not hasattr(cls, "registry"):
            cls.registry = set()
        cls.registry.add(cls)
        cls.registry -= set(bases)

    def __iter__(cls):
        return iter(cls.registry)


class ArmorCommand(object):
    __metaclass__ = RegisterSubclasses

    @classmethod
    def get_frame(cls, debugger):
        target = debugger.GetSelectedTarget()
        process = target.GetProcess()
        thread = process.GetSelectedThread()
        return thread.GetSelectedFrame()


class WriteDot(ArmorCommand):
    command_name = "write-dot"
    command_desc = "Write a GraphViz dot file describing an Armor type."
    command_help = """
    Syntax: armor write-dot <variable> <file-name>

This command will write a GraphViz dot file describing an Armor type to a file
which can later be rendered into an image using the `dot' utility."""

    @classmethod
    def run(cls, debugger, variable, filename):
        frame = cls.get_frame(debugger)
        rmr.util.lldb_visualizer_for(
            rmr.util.lldb_find_variable(frame, variable)).write_dot(
                open(filename, "w"))


class Quicklook(ArmorCommand):
    command_name = "quicklook"
    command_desc = "View Armor data structures graphically."
    command_help = """
    Syntax: armor quicklook <variable>

This command will construct a GraphViz drawing of an armor type and open it in
quicklook. Very useful for debugging trie structure and trie related
bugs."""

    @classmethod
    def run(cls, debugger, variable):
        frame = cls.get_frame(debugger)
        rmr.util.lldb_visualizer_for(
            rmr.util.lldb_find_variable(frame, variable)).quicklook()


class TraceIteration(ArmorCommand):
    command_name = "trace-iteration"
    command_desc = "Trace Armor data structure iteration."
    command_help = """
    Syntax: armor trace-iteration <breakpoint-index> <variable> <iterator>

Assuming <breakpoint-index> is a breakpoint inside of a loop iterating
over a trie.  This command will modify breakpoint number
<breakpoint-index> to display a GraphViz diagram and with the iterator
location marked in it."""

    @classmethod
    def run(cls, debugger, bp_index, t, it):
        bp = debugger.GetSelectedTarget().GetBreakpointAtIndex(
            int(bp_index) - 1)
        bp.SetScriptCallbackBody("""
    import rmr
    t = rmr.util.lldb_visualizer_for(rmr.util.lldb_find_variable(frame, "%s"))
    it = rmr.util.lldb_find_variable(frame, "%s")
    t.mark(rmr.util.lldb_iterator_to_pointer(it))
    t.quicklook()
    return False
        """ % (t, it))


def armor_main_help():
    text = """    Armor debugging utilities.

Syntax: armor <subcommand> [<subcommand-options>]

The following subcommands are supported:

"""
    width = max(map(lambda cls: len(cls.command_name), ArmorCommand))
    for cls in sorted(ArmorCommand, key=lambda cls: cls.command_name):
        text += "    %% -%ds -- %%s\n" % width % (cls.command_name,
                                                  cls.command_desc)
    return text


def armor_main(debugger, cmdline, result, _):
    args = shlex.split(cmdline)
    try:
        command, args = args[0], args[1:]
    except IndexError:
        result.write(armor_main_help())
        return
    for cls in ArmorCommand:
        if cls.command_name == command:
            try:
                cls.run(debugger, *args)
                return
            except TypeError:
                result.write(cls.command_desc + "\n")
                result.write(cls.command_help + "\n")
                return
    result.write("invalid command 'armor %s'.\n" % command)


def __lldb_init_module(debugger, _):
    debugger.HandleCommand(
        "command script add -f lldb_commands.armor_main armor")
