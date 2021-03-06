# CMake build script for the libgit2 project
#
# Building (out of source build):
# > mkdir build && cd build
# > cmake .. [-DSETTINGS=VALUE]
# > cmake --build .
#
# Testing:
# > ctest -V
#
# Install:
# > cmake --build . --target install

cmake_minimum_required(VERSION 3.5.1)

project(libgit2 VERSION "1.1.0" LANGUAGES C)

# Add find modules to the path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${libgit2_SOURCE_DIR}/cmake/")

include(CheckLibraryExists)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckStructHasMember)
include(CheckPrototypeDefinition) # Added in CMake 3.0
include(AddCFlagIfSupported)
include(FindPkgLibraries)
include(FindThreads)
include(FindStatNsec)
include(GNUInstallDirs)
include(IdeSplitSources)
include(FeatureSummary)
include(EnableWarnings)

# Build options
#
option(ZERO_NSEC "Hint that index produced by the 'git' binary may have zero nsec components in all timestamps" OFF)
option(SONAME "Set the (SO)VERSION of the target" ON)
option(BUILD_SHARED_LIBS "Build Shared Library (OFF for Static)" ON)
option(THREADSAFE "Build libgit2 as threadsafe" ON)
option(BUILD_CLAR "Build Tests using the Clar suite" ON)
option(BUILD_EXAMPLES "Build library usage example apps" OFF)
option(BUILD_FUZZERS "Build the fuzz targets" OFF)
option(ENABLE_TRACE "Enables tracing support" ON)
option(LIBGIT2_FILENAME "Name of the produced binary" OFF)
option(USE_SSH "Link with libssh2 to enable SSH support" ON)
option(USE_HTTPS "Enable HTTPS support. Can be set to a specific backend" ON)
option(USE_SHA1 "Enable SHA1. Can be set to CollisionDetection(ON)/HTTPS/Generic" ON)
option(USE_GSSAPI "Link with libgssapi for SPNEGO auth" OFF)
option(USE_STANDALONE_FUZZERS "Enable standalone fuzzers (compatible with gcc)" OFF)
option(USE_LEAK_CHECKER "Run tests with leak checker" OFF)
option(DEBUG_POOL "Enable debug pool allocator" OFF)
option(ENABLE_WERROR "Enable compilation with -Werror" OFF)
option(USE_BUNDLED_ZLIB "Use the bundled version of zlib" OFF)
set(USE_HTTP_PARSER "" CACHE STRING "Specifies the HTTP Parser implementation; either system or builtin.")
option(DEPRECATE_HARD "Do not include deprecated functions in the library" OFF)
set(REGEX_BACKEND "" CACHE STRING "Regular expression implementation. One of regcomp_l, pcre2, pcre, regcomp, or builtin.")

if (UNIX)
  if (NOT USE_HTTPS)
    option(USE_NTLMCLIENT "Enable NTLM support on Unix." OFF)
  else ()
    option(USE_NTLMCLIENT "Enable NTLM support on Unix." ON)
  endif ()
endif ()

if (UNIX AND NOT APPLE)
  option(ENABLE_REPRODUCIBLE_BUILDS "Enable reproducible builds" OFF)
endif ()

if (APPLE)
  option(USE_ICONV "Link with and use iconv library" ON)
endif ()

if (MSVC)
  # This option must match the settings used in your program, in particular if you
  # are linking statically
  option(STATIC_CRT "Link the static CRT libraries" ON)

  # If you want to embed a copy of libssh2 into libgit2, pass a
  # path to libssh2
  option(EMBED_SSH_PATH "Path to libssh2 to embed (Windows)" OFF)
endif ()


if (WIN32)
  # By default, libgit2 is built with WinHTTP.  To use the built-in
  # HTTP transport, invoke CMake with the "-DWINHTTP=OFF" argument.
  option(WINHTTP "Use Win32 WinHTTP routines" ON)
endif ()

if (MSVC)
  # Enable MSVC CRTDBG memory leak reporting when in debug mode.
  option(MSVC_CRTDBG "Enable CRTDBG memory leak reporting" OFF)
endif ()

if (DEPRECATE_HARD)
  add_definitions(-DGIT_DEPRECATE_HARD)
endif ()

# Platform specific compilation flags
if (MSVC)
  if (STDCALL)
    message(FATAL_ERROR "The STDCALL option is no longer supported; libgit2 is now always built as a cdecl library.  If you're using PInvoke, please add the CallingConventions.Cdecl attribute for support.")
  endif ()

  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)

  string(REPLACE "/Zm1000" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

  # /GF - String pooling
  # /MP - Parallel build
  set(CMAKE_C_FLAGS "/GF /MP /nologo ${CMAKE_C_FLAGS}")

  # /Gd - explicitly set cdecl calling convention
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Gd")

  if (NOT (MSVC_VERSION LESS 1900))
    # /guard:cf - Enable Control Flow Guard
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /guard:cf")
  endif ()

  if (STATIC_CRT)
    set(CRT_FLAG_DEBUG "/MTd")
    set(CRT_FLAG_RELEASE "/MT")
  else ()
    set(CRT_FLAG_DEBUG "/MDd")
    set(CRT_FLAG_RELEASE "/MD")
  endif ()

  if (MSVC_CRTDBG)
    set(GIT_MSVC_CRTDBG 1)
    set(CRT_FLAG_DEBUG "${CRT_FLAG_DEBUG}")
    set(CMAKE_C_STANDARD_LIBRARIES "${CMAKE_C_STANDARD_LIBRARIES} Dbghelp.lib")
  endif ()

  # /Zi - Create debugging information
  # /Od - Disable optimization
  # /D_DEBUG - #define _DEBUG
  # /MTd - Statically link the multithreaded debug version of the CRT
  # /MDd - Dynamically link the multithreaded debug version of the CRT
  # /RTC1 - Run time checks
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi /Od /D_DEBUG /RTC1 ${CRT_FLAG_DEBUG}")

  # /DNDEBUG - Disables asserts
  # /MT - Statically link the multithreaded release version of the CRT
  # /MD - Dynamically link the multithreaded release version of the CRT
  # /O2 - Optimize for speed
  # /Oy - Enable frame pointer omission (FPO) (otherwise CMake will automatically turn it off)
  # /GL - Link time code generation (whole program optimization)
  # /Gy - Function-level linking
  set(CMAKE_C_FLAGS_RELEASE "/DNDEBUG /O2 /Oy /GL /Gy ${CRT_FLAG_RELEASE}")

  # /Oy- - Disable frame pointer omission (FPO)
  set(CMAKE_C_FLAGS_RELWITHDEBINFO "/DNDEBUG /Zi /O2 /Oy- /GL /Gy ${CRT_FLAG_RELEASE}")

  # /O1 - Optimize for size
  set(CMAKE_C_FLAGS_MINSIZEREL "/DNDEBUG /O1 /Oy /GL /Gy ${CRT_FLAG_RELEASE}")

  # /IGNORE:4221 - Ignore empty compilation units
  set(CMAKE_STATIC_LINKER_FLAGS "/IGNORE:4221")

  # /DYNAMICBASE - Address space load randomization (ASLR)
  # /NXCOMPAT - Data execution prevention (DEP)
  # /LARGEADDRESSAWARE - >2GB user address space on x86
  # /VERSION - Embed version information in PE header
  set(CMAKE_EXE_LINKER_FLAGS "/DYNAMICBASE /NXCOMPAT /LARGEADDRESSAWARE /VERSION:${libgit2_VERSION_MAJOR}.${libgit2_VERSION_MINOR}")

  if (NOT (MSVC_VERSION LESS 1900))
    # /GUARD:CF - Enable Control Flow Guard
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /GUARD:CF")
  endif ()

  # /DEBUG - Create a PDB
  # /LTCG - Link time code generation (whole program optimization)
  # /OPT:REF /OPT:ICF - Fold out duplicate code at link step
  # /INCREMENTAL:NO - Required to use /LTCG
  # /DEBUGTYPE:cv,fixup - Additional data embedded in the PDB (requires /INCREMENTAL:NO, so not on for Debug)
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/RELEASE /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO")
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/DEBUG /RELEASE /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO /DEBUGTYPE:cv,fixup")
  set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "/RELEASE /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO")

  # Same linker settings for DLL as EXE
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
  set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
  set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
  set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL}")
else ()
  if (ENABLE_REPRODUCIBLE_BUILDS)
    set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> Dqc <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_APPEND "<CMAKE_AR> Dq  <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> -D <TARGET>")
  endif ()

  set(CMAKE_C_FLAGS "-D_GNU_SOURCE ${CMAKE_C_FLAGS}")

  enable_warnings(all)
  enable_warnings(extra)

  if (CMAKE_SYSTEM_NAME MATCHES "(Solaris|SunOS)")
    set(CMAKE_C_FLAGS "-D_POSIX_C_SOURCE=200112L -D__EXTENSIONS__ -D_POSIX_PTHREAD_SEMANTICS ${CMAKE_C_FLAGS}")
  endif ()

  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG -O0")

  if (MINGW OR MSYS) # MinGW and MSYS always do PIC and complain if we tell them to
    string(REGEX REPLACE "-fPIC" "" CMAKE_SHARED_LIBRARY_C_FLAGS "${CMAKE_SHARED_LIBRARY_C_FLAGS}")
  elseif (BUILD_SHARED_LIBS)
    add_c_flag_if_supported(-fvisibility=hidden)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  endif ()

  if (MINGW)
    # MinGW >= 3.14 uses the C99-style stdio functions
    # automatically, but forks like mingw-w64 still want
    # us to define this in order to use them
    add_definitions(-D__USE_MINGW_ANSI_STDIO=1)
  endif ()

  enable_warnings(documentation)
  disable_warnings(documentation-deprecated-sync)
  disable_warnings(missing-field-initializers)
  enable_warnings(strict-aliasing)
  enable_warnings(strict-prototypes)
  enable_warnings(declaration-after-statement)
  enable_warnings(shift-count-overflow)
  enable_warnings(unused-const-variable)
  enable_warnings(unused-function)
  enable_warnings(int-conversion)

  # MinGW uses gcc, which expects POSIX formatting for printf, but
  # uses the Windows C library, which uses its own format specifiers.
  # Disable format specifier warnings.
  if (MINGW)
    disable_warnings(format)
    disable_warnings(format-security)
  else ()
    enable_warnings(format)
    enable_warnings(format-security)
  endif ()
endif ()

# Ensure that MinGW provides the correct header files.
if (WIN32 AND NOT CYGWIN)
  add_definitions(-DWIN32 -D_WIN32_WINNT=0x0600)
endif ()

if (NOT CMAKE_CONFIGURATION_TYPES)
  # Build Debug by default
  if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
  endif ()
else ()
  # Using a multi-configuration generator eg MSVC or Xcode
  # that uses CMAKE_CONFIGURATION_TYPES and not CMAKE_BUILD_TYPE
endif ()

if (BUILD_FUZZERS AND NOT USE_STANDALONE_FUZZERS)
  # The actual sanitizer link target will be added when linking the fuzz
  # targets.
  set(CMAKE_REQUIRED_FLAGS "-fsanitize=fuzzer-no-link")
  add_c_flag(-fsanitize=fuzzer-no-link)
  unset(CMAKE_REQUIRED_FLAGS)
endif ()

add_subdirectory(src)

# Tests
if (NOT MSVC)
  if (NOT BUILD_SHARED_LIBS)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  endif ()
endif ()

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif ()

if (BUILD_FUZZERS)
  if (NOT USE_STANDALONE_FUZZERS)
    if (BUILD_EXAMPLES)
      message(FATAL_ERROR "Cannot build the fuzzer targets and the examples together")
    endif ()
  endif ()
  add_subdirectory(fuzzers)
endif ()

feature_summary(WHAT ENABLED_FEATURES DESCRIPTION "Enabled features:")
feature_summary(WHAT DISABLED_FEATURES DESCRIPTION "Disabled features:")
