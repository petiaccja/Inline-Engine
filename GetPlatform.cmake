# Create defines based on platform
if (CMAKE_SYSTEM_NAME MATCHES "Windows") 
	set(TARGET_PLATFORM_WINDOWS 1) 
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(TARGET_PLATFORM_LINUX 1)
elseif (CMAKE_SYSTEM_NAME MATCHES "Xbone")
	set(TARGET_PLATFORM_XBONE 1)
endif ()


# Determine target architecture
if (CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64")
	set(TARGET_ARCH_X64 1)
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "i386")
	set(TARGET_ARCH_X86 1)
endif()


# Determine compiler
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set(TARGET_COMPILER_MSVC 1)
endif()
