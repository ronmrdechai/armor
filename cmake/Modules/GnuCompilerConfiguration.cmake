# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

if(CMAKE_COMPILER_IS_GNUC)
    set(_compiler ${CMAKE_C_COMPILER})
elseif(CMAKE_COMPILER_IS_GNUCXX)
    set(_compiler ${CMAKE_CXX_COMPILER})
else()
    message(WARNING "Compiler is not GNU C/C++, GnuConguration disabled")
    return()
endif()

execute_process(
    COMMAND ${_compiler} -v
    ERROR_VARIABLE _gnuc_verbose
    ERROR_STRIP_TRAILING_WHITESPACE
)
string(REGEX MATCH "Configured with: ([^\n]*)" _ "${_gnuc_verbose}")
set(_gnuc_configure ${CMAKE_MATCH_1})


string(REGEX MATCHALL "--[^= ]* " _args "${_gnuc_configure}")
foreach(arg IN LISTS _args)
    string(REPLACE "--" "" varname ${arg})
    string(TOUPPER ${varname} varname)
    string(REPLACE "-" "_" varname "${varname}")
    string(STRIP "${varname}" varname)
    set(varname "GNUC_${varname}")

    set(${varname} TRUE)
    mark_as_advanced(${varname})
endforeach()

string(REGEX MATCHALL "--[^= ]*=[^ ]* " _args "${_gnuc_configure}")
foreach(arg IN LISTS _args)
    string(REGEX REPLACE "[^= ]*=" "" value "${arg}")
    string(STRIP "${value}" value)
    string(REGEX REPLACE "=.*" "" varname "${arg}")
    string(REPLACE "--" "" varname ${varname})
    string(TOUPPER ${varname} varname)
    string(REPLACE "-" "_" varname "${varname}")
    set(varname "GNUC_${varname}")

    set(${varname} ${value})
    mark_as_advanced(${varname})
endforeach()

unset(_compiler)
unset(_gnuc_verbose)
unset(_gnuc_configure)
unset(_args)
