cmake_minimum_required (VERSION 3.4)
project (crypt-ta C ASM)

include(GNUInstallDirs)

# FIXME: Just copied from the output from a regular QEMU v7 build.
add_compile_options (-fPIC
	-fdiagnostics-show-option
	-fno-common
	-fno-short-enums
	-fpie
	-funsafe-math-optimizations
	-funwind-tables
	-g3
	-mcpu=cortex-a15
	#-MD
	#-MF 
	-mfloat-abi=hard
	-mno-unaligned-access
	-mthumb
	-mthumb-interwork
	-O0
	-pedantic
	-std=gnu99
	-Waggregate-return
	-Wall
	-Wcast-align
	-Wdeclaration-after-statement
	-Werror
	-Werror-implicit-function-declaration
	-Wextra
	-Wfloat-equal
	-Wformat=2
	-Wformat-nonliteral
	-Wformat-security
	-Winit-self
	-Wmissing-declarations
	-Wmissing-format-attribute
	-Wmissing-include-dirs
	-Wmissing-noreturn
	-Wmissing-prototypes
	-Wnested-externs
	-Wno-format-zero-length
	-Wno-missing-field-initializers
	-Wold-style-definition
	-Wpointer-arith
	-Wredundant-decls
	-Wshadow
	-Wstrict-aliasing=2
	-Wstrict-prototypes
	-Wswitch-default
	-Wundef
	-Wwrite-strings
	#./.hotp_ta.o.d -MT hotp_ta.o -nostdinc 
)

if ($ENV{OPTEE_OS_EXPORT})
	set(OPTEE_OS_EXPORT $ENV{OPTEE_OS_EXPORT})
else()
	set(OPTEE_OS_EXPORT ${CMAKE_SOURCE_DIR}/optee_os/out/arm-plat-vexpress/export-ta_arm32)
endif ($ENV{OPTEE_OS_EXPORT})
message(STATUS "Looking for OP-TEE libs at: " ${OPTEE_OS_EXPORT})

find_library(LIBUTEE
	NAMES "utee"
	PATHS ${OPTEE_OS_EXPORT}/lib
)

find_library(LIBUTILS
	NAMES "utils"
	PATHS ${OPTEE_OS_EXPORT}/lib
)

find_library(LIBMPA
	NAMES "mpa"
	PATHS ${OPTEE_OS_EXPORT}/lib
)

#message(STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})

set (SRC
	${CMAKE_CURRENT_SOURCE_DIR}/aes_impl.c
	${CMAKE_CURRENT_SOURCE_DIR}/aes_taf.c
	${CMAKE_CURRENT_SOURCE_DIR}/cryp_taf.c
	${CMAKE_CURRENT_SOURCE_DIR}/sha2_impl.c
	${CMAKE_CURRENT_SOURCE_DIR}/sha2_taf.c
	${CMAKE_CURRENT_SOURCE_DIR}/ta_entry.c
	# FIXME: These two should be copied, since different TA's cannot(?)
	# compile this from the same location.
	${OPTEE_OS_EXPORT}/src/user_ta_header.c
	${OPTEE_OS_EXPORT}/src/ta.ld.S
)

add_executable (${PROJECT_NAME} ${SRC})

target_compile_definitions (${PROJECT_NAME}
	PRIVATE -D__ILP32__=1
	PRIVATE -DARM32=1
	PRIVATE -DCFG_TEE_TA_LOG_LEVEL=3
	PRIVATE -DTRACE_LEVEL=3
)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE optee-os-headers
	PRIVATE ${LIBUTEE}
	PRIVATE ${LIBUTILS}
	PRIVATE ${LIBMPA}
)
