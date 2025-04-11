cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

# Set and enforce C++-11 flags
#set( CMAKE_CXX_STANDARD_REQUIRED TRUE )
#set( CMAKE_CXX_STANDARD 11 )

enable_testing()


project (OpenMesh 
         VERSION 11.0.0
         LANGUAGES C CXX ) 

# Set AUTO UIC/MOC Policy to new for CMAKE 3.17 or higher
if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.17")
    cmake_policy(SET CMP0100 NEW)
endif()


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.0" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "4.9" OR CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "4.9")
		message(WARNING "Your version of GCC contains an optimizer bug. Please verify that you do not use -O3!")
		string(REPLACE "-O3" "-O2" CMAKE_CXX_FLAGS_RELEASE_NEW "${CMAKE_CXX_FLAGS_RELEASE}")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE_NEW}" CACHE STRING "" FORCE)
	endif()
endif()

if ( WIN32 )
  # This is the base directory for windows library search used in the finders we ship.
  set(CMAKE_WINDOWS_LIBS_DIR "c:/libs" CACHE STRING "Default Library search dir on windows." )
endif()

if (NOT WIN32 AND NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# add our macro directory to cmake search path
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set (CMAKE_DEBUG_POSTFIX "d")

# include our cmake files
include (VCICommon)

# Disable package building when built as an external library
if(${CMAKE_PROJECT_NAME} MATCHES "OpenMesh")
    include(OpenMeshPackage)
endif()

include (VCIOutput)
include(VCIQt)

# ========================================================================
# Definitions
# ========================================================================

if (WIN32)
  add_definitions(
      -D_USE_MATH_DEFINES -DNOMINMAX
      -D_CRT_SECURE_NO_WARNINGS
  )
endif ()

set(NO_DECREMENT_DEPRECATED_WARNINGS OFF CACHE BOOL "Disables all deprecated warnings warning about decrement operations on normal circulators.")
if(NO_DECREMENT_DEPRECATED_WARNINGS)
  add_definitions( -DNO_DECREMENT_DEPRECATED_WARNINGS )
endif()

# ========================================================================
# Windows build style control
# ========================================================================

if ( WIN32 )
  if ( NOT DEFINED OPENMESH_BUILD_SHARED )
    set( OPENMESH_BUILD_SHARED false CACHE BOOL "Build as shared library(DLL)?" )
  endif()
endif()

# ========================================================================
# Call the subdirectories with there projects
# ========================================================================

add_subdirectory (src/OpenMesh/Core)
add_subdirectory (src/OpenMesh/Tools)
add_subdirectory (src/OpenMesh/Apps)

# If built as a dependent project simulate effects of
# successful finder run:
set (OPENMESH_FOUND true PARENT_SCOPE)
set (OPENMESH_LIBRARIES OpenMeshCore OpenMeshTools PARENT_SCOPE)
set (OPENMESH_INCLUDE_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/src" PARENT_SCOPE)
set (OPENMESH_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src" PARENT_SCOPE)

# Also define variables provided by the old legacy finder.
set (OPENMESH_CORE_LIBRARY OpenMeshCore PARENT_SCOPE)
set (OPENMESH_TOOLS_LIBRARY OpenMeshTools PARENT_SCOPE)
set (OPENMESH_TOOLS_LIBRARY OpenMeshTools PARENT_SCOPE)
set (OPENMESH_LIBRARY OpenMeshCore OpenMeshTools PARENT_SCOPE)
get_target_property(_OPENMESH_LIBRARY_DIR OpenMeshCore LIBRARY_OUTPUT_DIRECTORY)
set (OPENMESH_LIBRARY_DIR "${_OPENMESH_LIBRARY_DIR}" CACHE PATH "The directory where the OpenMesh libraries can be found.")

# ========================================================================

# Generate openmesh.pc file

set(DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set(PRIVATE_LIBS "-lOpenMeshCore -lOpenMeshTools")

configure_file("openmesh.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/openmesh.pc" @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/openmesh.pc DESTINATION libdata/pkgconfig)

# generate target file

install(EXPORT OpenMeshConfig DESTINATION share/OpenMesh/cmake)

export(TARGETS OpenMeshCore OpenMeshTools FILE OpenMeshConfig.cmake)

# display results
vci_print_configure_header (OPENMESH "OpenMesh")
