"""
Various python utilities for use with Armor LLDB functions.
"""


def find_variable(frame, name):
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
    return find_variable(thread.GetFrameAtIndex(frame_id + 1), name)
