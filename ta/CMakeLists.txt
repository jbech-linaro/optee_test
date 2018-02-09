cmake_minimum_required (VERSION 3.4)
project (xtest-ta)

################################################################################
# Packages
################################################################################
find_package(Threads REQUIRED)
if(NOT THREADS_FOUND)
	message(FATAL_ERROR "Threads not found")
endif()

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

include(GNUInstallDirs)

add_subdirectory(crypt)

project (xtest-ta-headers)

################################################################################
# Built library
################################################################################
add_library(${PROJECT_NAME} INTERFACE)

################################################################################
# Public and private header and library dependencies
################################################################################
target_include_directories(${PROJECT_NAME}
	INTERFACE aes_perf/include
	INTERFACE concurrent/include
	INTERFACE concurrent_large/include
	INTERFACE create_fail_test/include
	INTERFACE crypt/include
	INTERFACE enc_fs/include
	INTERFACE include
	INTERFACE os_test
	INTERFACE os_test/include
	INTERFACE rpc_test/include
	INTERFACE sdp_basic/include
	INTERFACE sha_perf/include
	INTERFACE sims/include
	INTERFACE socket/include
	INTERFACE storage2/include
	INTERFACE storage_benchmark/include
	INTERFACE storage/include
)

################################################################################
# Install targets
################################################################################
#install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
#DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
