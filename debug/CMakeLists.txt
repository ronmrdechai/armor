# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

find_package(PyCodeStyle)

if(Armor_LINT_TESTS AND Armor_INCLUDE_TESTS)
    if(PyCodeStyle_FOUND)
        pycodestyle_add_tests("python/*.py"
            RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/python"
        )
    endif()
endif()
