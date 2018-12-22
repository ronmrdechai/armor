#!/bin/bash

GIT_ROOT=$(git rev-parse --show-toplevel)
export PYTHONPATH="${GIT_ROOT}/debug/python"
lldb --source "${GIT_ROOT}/debug/scripts/lldbrc" $@
