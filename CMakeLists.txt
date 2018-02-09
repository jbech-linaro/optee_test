cmake_minimum_required (VERSION 3.4)
project (xtest C)

set (CMAKE_TOOLCHAIN_FILE CMakeToolchain.txt)

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

################################################################################
# Compiler flags:
#   We want to use the same flags in the entire optee_client git
################################################################################
add_compile_options (
	-Wall -Wbad-function-cast -Wcast-align
	-Werror-implicit-function-declaration -Wextra
	-Wfloat-equal -Wformat-nonliteral -Wformat-security
	-Wformat=2 -Winit-self -Wmissing-declarations
	-Wmissing-format-attribute -Wmissing-include-dirs
	-Wmissing-noreturn -Wmissing-prototypes -Wnested-externs
	-Wpointer-arith -Wshadow -Wstrict-prototypes
	-Wswitch-default -Wunsafe-loop-optimizations
	-Wwrite-strings -Werror -fPIC
 	-Wno-missing-field-initializers
)

################################################################################
# Add sub directories
################################################################################
add_subdirectory(ta)

set (SRC
	host/xtest/adbg/src/adbg_case.c
	host/xtest/adbg/src/adbg_enum.c
	host/xtest/adbg/src/adbg_expect.c
	host/xtest/adbg/src/adbg_log.c
	host/xtest/adbg/src/adbg_run.c
	host/xtest/adbg/src/security_utils_hex.c
	host/xtest/aes_perf.c
	host/xtest/benchmark_1000.c
	host/xtest/benchmark_2000.c
	host/xtest/regression_4000.c
	host/xtest/regression_5000.c
	host/xtest/regression_6000.c
	host/xtest/regression_7000.c
	host/xtest/regression_8000.c
	host/xtest/sha_perf.c
	host/xtest/xtest_helpers.c
	host/xtest/xtest_main.c
	host/xtest/xtest_test.c
)

################################################################################
# Built binary
################################################################################
add_executable (${PROJECT_NAME} ${SRC})

################################################################################
# Flags always set
################################################################################
#make -C /home/jbech/devel/optee_projects/qemu/optee_test
# CROSS_COMPILE_HOST="/usr/bin/ccache /home/jbech/devel/optee_projects/qemu/toolchains/aarch32/bin/arm-linux-gnueabihf-"
# CROSS_COMPILE_TA="/usr/bin/ccache /home/jbech/devel/optee_projects/qemu/toolchains/aarch32/bin/arm-linux-gnueabihf-"
# TA_DEV_KIT_DIR=/home/jbech/devel/optee_projects/qemu/optee_os/out/arm/export-ta_arm32
# OPTEE_CLIENT_EXPORT=/home/jbech/devel/optee_projects/qemu/optee_client/out/export
# COMPILE_NS_USER=32
# O=/home/jbech/devel/optee_projects/qemu/optee_test/out
target_compile_definitions (${PROJECT_NAME}
	PRIVATE -DCOMPILE_NS_USER=32
	PRIVATE -D_GNU_SOURCE
	PRIVATE -DDEBUGLEVEL_1
	PRIVATE -DBINARY_PREFIX="XTEST"
	PRIVATE -DCFG_REE_FS
)

################################################################################
# Optional flags
################################################################################
#if (CFG_GP_SOCKETS)
#	target_compile_definitions (${PROJECT_NAME}
#		PRIVATE -DCFG_GP_SOCKETS=${CFG_GP_SOCKETS})
#endif()

#if (CFG_TA_GPROF_SUPPORT)
#	target_compile_definitions (${PROJECT_NAME}
#		PRIVATE -DCFG_TA_GPROF_SUPPORT)
#endif()

################################################################################
# Public and private header and library dependencies
################################################################################
target_include_directories(${PROJECT_NAME}
	PRIVATE host/xtest
	PRIVATE host/xtest/adbg/include
	PRIVATE host/xtest/xml/include
)

target_link_libraries (${PROJECT_NAME}
	PRIVATE m
	PRIVATE teec
	PRIVATE xtest-ta-headers
	PRIVATE optee-client-headers
	PRIVATE optee-os-headers
)

################################################################################
# Install targets
################################################################################
install (TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
