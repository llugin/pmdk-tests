# Copyright (c) 2018, Intel Corporation
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.
#
# * Neither the name of the copyright holder nor the names of its
# contributors may be used to endorse or promote products derived
# from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# UNSAFE_SHUTDOWN_LOCAL
set(DIR ${CMAKE_CURRENT_LIST_DIR})
set(PREFIX_FILTER "")

file(GLOB_RECURSE us_SRC
	"${DIR}/*.h"
	"${DIR}/*.cc")

set (inject_utils_DIR "${DIR}/../../inject_utils")
file (GLOB_RECURSE inject_utils_SRC
  "${inject_utils_DIR}/*.h"
  "${inject_utils_DIR}/*.cc")

set (us_base_DIR "${DIR}/..")
file (GLOB us_base_SRC
  "${us_base_DIR}/*h"
  "${us_base_DIR}/*cc")

add_executable(UNSAFE_SHUTDOWN_LOCAL
	${us_SRC} ${us_base_SRC} ${inject_utils_SRC})

set_source_groups("${PREFIX_FILTER}" ${us_SRC} ${inject_utils_SRC}
 ${us_base_SRC})
include_directories(src/tests/ras/utils ${inject_utils_DIR}
 ${us_base_DIR})

target_link_libraries(UNSAFE_SHUTDOWN_LOCAL Utils RasUtils libgtest ${Libpmemblk_LIBRARIES}
${Libpmemlog_LIBRARIES} ${Libpmemobj_LIBRARIES} ${Libpmempool_LIBRARIES})
add_dependencies(UNSAFE_SHUTDOWN_LOCAL Utils RasUtils libgtest)
