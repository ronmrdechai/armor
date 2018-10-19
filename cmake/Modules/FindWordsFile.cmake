# Armor
#
# Copyright Ron Mordechai, 2018
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

find_file(WORDS_FILE_PATH words PATHS "/usr/share/dict")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WordsFile REQUIRED_VARS WORDS_FILE_PATH)
