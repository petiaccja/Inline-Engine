# Set header include dir
set(EXTERNALS_INCLUDE "${CMAKE_SOURCE_DIR}/Externals/include")
include_directories(${EXTERNALS_INCLUDE})


# Set lib dir
if (TARGET_PLATFORM_WINDOWS AND (TARGET_COMPILER_MSVC OR TARGET_COMPILER_CLANG))
	set(EXTERNALS_LIB_1 "")
elseif (TARGET_PLATFORM_LINUX AND TARGET_COMPILER_CLANG)
	set(EXTERNALS_LIB_1 "linux_")
else()
	set(EXTERNALS_LIB_1 "unknown_")
	message(WARNING "Unhandled compiler.")
endif()

if (TARGET_ARCH_X64)
	set(EXTERNALS_LIB_2 "x64_")
else()
	set(EXTERNALS_LIB_2 "unknown_")
	message(WARNING "Unhandled architecture.")
endif()

set(EXTERNALS_LIB_DEBUG "${CMAKE_SOURCE_DIR}/Externals/lib_${EXTERNALS_LIB_1}${EXTERNALS_LIB_2}Debug")
set(EXTERNALS_LIB_RELEASE "${CMAKE_SOURCE_DIR}/Externals/lib_${EXTERNALS_LIB_1}${EXTERNALS_LIB_2}Release")
set(EXTERNALS_BIN_DEBUG "${CMAKE_SOURCE_DIR}/Externals/bin_${EXTERNALS_LIB_1}${EXTERNALS_LIB_2}Debug")
set(EXTERNALS_BIN_RELEASE "${CMAKE_SOURCE_DIR}/Externals/bin_${EXTERNALS_LIB_1}${EXTERNALS_LIB_2}Release")

if (NOT EXISTS ${EXTERNALS_LIB_DEBUG})
	message("External libraries not found at ${EXTERNALS_LIB_DEBUG}!")
	message(FATAL_ERROR "Please compile external libraries for ${CMAKE_CXX_COMPILER_ID} on ${TARGET_ARCH_NAME}.")
endif()
if (NOT EXISTS ${EXTERNALS_LIB_RELEASE})
	message("External libraries not found at ${EXTERNALS_LIB_RELEASE}!")
	message(FATAL_ERROR "Please compile external libraries for ${CMAKE_CXX_COMPILER_ID} on ${TARGET_ARCH_NAME}.")
endif()


# Set binary output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

