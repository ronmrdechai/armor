#!/bin/bash

GIT_ROOT=$(git rev-parse --show-toplevel)
export PYTHONPATH="${GIT_ROOT}/debug/python"
lldb -o "command script import ${PYTHONPATH}/commands.py" $@
