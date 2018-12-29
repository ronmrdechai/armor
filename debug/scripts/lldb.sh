#!/bin/bash
# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

GIT_ROOT=$(git rev-parse --show-toplevel)
export PYTHONPATH="${GIT_ROOT}/debug/python"
lldb -o "command script import ${PYTHONPATH}/lldb_commands.py" $@
