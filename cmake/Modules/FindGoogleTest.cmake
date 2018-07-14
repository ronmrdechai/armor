include(externalproject)

if(NOT PACKAGE_FIND_VERSION)
    set(GTEST_TAG master)
else()
    set(GTEST_TAG "release-${PACKAGE_FIND_VERSION}")
endif()

ExternalProject_Add(googletest
      GIT_REPOSITORY  "https://github.com/google/googletest.git"
      UPDATE_COMMAND  ""
      GIT_TAG         ${GTEST_TAG}
      INSTALL_COMMAND ""
      CMAKE_ARGS      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      LOG_DOWNLOAD    ON
      LOG_CONFIGURE   ON
      LOG_BUILD       ON
)
set(GTEST_VERSION_STRING ${PACKAGE_FIND_VERSION})

ExternalProject_Get_Property(googletest source_dir)
set(GTEST_INCLUDE_DIRS ${source_dir}/googletest/include)
set(GMOCK_INCLUDE_DIRS ${source_dir}/googlemock/include)

ExternalProject_Get_Property(googletest binary_dir)
set(GTEST_LIBRARY_PATH ${binary_dir}/googlemock/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest.a)
set(GTEST_LIBRARY gtest)
add_library(${GTEST_LIBRARY} UNKNOWN IMPORTED)
set_target_properties(${GTEST_LIBRARY}
    PROPERTIES IMPORTED_LOCATION ${GTEST_LIBRARY_PATH}
)
add_dependencies(${GTEST_LIBRARY} googletest)

set(GTEST_MAIN_LIBRARY_PATH ${binary_dir}/googlemock/gtest/${CMAKE_FIND_LIBRARY_PREFIXES}gtest_main.a)
set(GTEST_MAIN_LIBRARY gtest_main)
add_library(${GTEST_MAIN_LIBRARY} UNKNOWN IMPORTED)
set_target_properties(${GTEST_MAIN_LIBRARY} PROPERTIES
                                            IMPORTED_LOCATION ${GTEST_MAIN_LIBRARY_PATH})
add_dependencies(${GTEST_MAIN_LIBRARY} googletest)
set(GTEST_BOTH_LIBRARIES ${GTEST_LIBRARY} ${GTEST_MAIN_LIBRARY})

set(GMOCK_LIBRARY_PATH ${binary_dir}/googlemock/${CMAKE_FIND_LIBRARY_PREFIXES}gmock.a)
set(GMOCK_LIBRARY gmock)
add_library(${GMOCK_LIBRARY} UNKNOWN IMPORTED)
set_target_properties(${GMOCK_LIBRARY} PROPERTIES
                                       IMPORTED_LOCATION ${GMOCK_LIBRARY_PATH})
add_dependencies(${GMOCK_LIBRARY} googletest)

set(GMOCK_MAIN_LIBRARY_PATH ${binary_dir}/googlemock/${CMAKE_FIND_LIBRARY_PREFIXES}gmock_main.a)
set(GMOCK_MAIN_LIBRARY gmock_main)
add_library(${GMOCK_MAIN_LIBRARY} UNKNOWN IMPORTED)
set_target_properties(${GMOCK_MAIN_LIBRARY}
    PROPERTIES IMPORTED_LOCATION ${GMOCK_MAIN_LIBRARY_PATH}
)
add_dependencies(${GMOCK_MAIN_LIBRARY} ${GTEST_LIBRARY})
set(GMOCK_BOTH_LIBRARIES ${GMOCK_LIBRARY} ${GMOCK_MAIN_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GoogleTest
    REQUIRED_VARS
        GTEST_INCLUDE_DIRS GMOCK_INCLUDE_DIRS GTEST_LIBRARY_PATH GTEST_LIBRARY
        GTEST_MAIN_LIBRARY_PATH GTEST_MAIN_LIBRARY_PATH GTEST_BOTH_LIBRARIES
        GMOCK_LIBRARY_PATH GMOCK_LIBRARY_PATH GMOCK_MAIN_LIBRARY_PATH
        GMOCK_MAIN_LIBRARY GMOCK_BOTH_LIBRARIES
    VERSION_VAR GTEST_VERSION_STRING
)
mark_as_advanced(
    GTEST_INCLUDE_DIRS GMOCK_INCLUDE_DIRS GTEST_LIBRARY_PATH GTEST_LIBRARY
    GTEST_MAIN_LIBRARY_PATH GTEST_MAIN_LIBRARY_PATH GTEST_BOTH_LIBRARIES
    GMOCK_LIBRARY_PATH GMOCK_LIBRARY_PATH GMOCK_MAIN_LIBRARY_PATH
    GMOCK_MAIN_LIBRARY GMOCK_BOTH_LIBRARIES
)
