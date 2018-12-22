#!/bin/bash
export PYTHONPATH="$(git rev-parse --show-toplevel)/debug/python"
lldb $@
