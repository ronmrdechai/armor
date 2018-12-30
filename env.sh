#!/bin/bash

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    echo "Source this file to add debugging scripts to PATH."
    exit 1
fi

GIT_ROOT=$(git rev-parse --show-toplevel)
export PATH="${GIT_ROOT}/debug/scripts:${PATH}"
export GTEST_COLOR=1

unset GIT_ROOT
