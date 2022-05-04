#
# tiledb/sm/label_schema/CMakeLists.txt
#
#
# The MIT License
#
# Copyright (c) 2022 TileDB, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

include(common NO_POLICY_SCOPE)

#
# `label` object library
#
add_library(label_schema OBJECT labels.cc)
target_link_libraries(label_schema PUBLIC baseline $<TARGET_OBJECTS:baseline>)
target_link_libraries(label_schema PUBLIC vfs $<TARGET_OBJECTS:vfs>)
#
# Test-compile of object library ensures link-completeness
#
add_executable(compile_label_schema EXCLUDE_FROM_ALL)
target_link_libraries(compile_label_schema PRIVATE label_schema)
target_sources(compile_label_schema PRIVATE test/compile_labels_main.cc)

if (TILEDB_TESTS)
    add_subdirectory(test)
endif()
