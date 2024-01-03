#[=======================================================================[.rst:
FindJPEGTURBO
--------

Find the Joint Photographic Experts Group (JPEGTURBO) library (``turbojpeg``)

Imported targets
^^^^^^^^^^^^^^^^

.. versionadded:: 3.12

This module defines the following :prop_tgt:`IMPORTED` targets:

``JPEGTURBO::JPEGTURBO``
  The JPEGTURBO library, if found.

Result variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``JPEGTURBO_FOUND``
  If false, do not try to use JPEGTURBO.
``JPEGTURBO_INCLUDE_DIRS``
  where to find jpeglib.h, etc.
``JPEGTURBO_LIBRARIES``
  the libraries needed to use JPEGTURBO.
``JPEGTURBO_VERSION``
  .. versionadded:: 3.12
    the version of the JPEGTURBO library found

Cache variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``JPEGTURBO_INCLUDE_DIRS``
  where to find jpeglib.h, etc.
``JPEGTURBO_LIBRARY_RELEASE``
  where to find the JPEGTURBO library (optimized).
``JPEGTURBO_LIBRARY_DEBUG``
  where to find the JPEGTURBO library (debug).

.. versionadded:: 3.12
  Debug and Release variand are found separately.

Obsolete variables
^^^^^^^^^^^^^^^^^^

``JPEGTURBO_INCLUDE_DIR``
  where to find jpeglib.h, etc. (same as JPEGTURBO_INCLUDE_DIRS)
``JPEGTURBO_LIBRARY``
  where to find the JPEGTURBO library.
#]=======================================================================]

find_path(JPEGTURBO_INCLUDE_DIR jpeglib.h)

set(jpeg_names ${JPEGTURBO_NAMES} turbojpeg turbojpeg-static)
foreach(name ${jpeg_names})
  list(APPEND jpeg_names_debug "${name}d")
endforeach()

if(NOT JPEGTURBO_LIBRARY)
  find_library(JPEGTURBO_LIBRARY_RELEASE NAMES ${jpeg_names} NAMES_PER_DIR)
  find_library(JPEGTURBO_LIBRARY_DEBUG NAMES ${jpeg_names_debug} NAMES_PER_DIR)
  include(SelectLibraryConfigurations)
  select_library_configurations(JPEGTURBO)
  mark_as_advanced(JPEGTURBO_LIBRARY_RELEASE JPEGTURBO_LIBRARY_DEBUG)
endif()
unset(jpeg_names)
unset(jpeg_names_debug)

if(JPEGTURBO_INCLUDE_DIR)
  file(GLOB _JPEGTURBO_CONFIG_HEADERS_FEDORA
       "${JPEGTURBO_INCLUDE_DIR}/jconfig*.h")
  file(GLOB _JPEGTURBO_CONFIG_HEADERS_DEBIAN
       "${JPEGTURBO_INCLUDE_DIR}/*/jconfig.h")
  set(_JPEGTURBO_CONFIG_HEADERS
      "${JPEGTURBO_INCLUDE_DIR}/jpeglib.h" ${_JPEGTURBO_CONFIG_HEADERS_FEDORA}
      ${_JPEGTURBO_CONFIG_HEADERS_DEBIAN})
  foreach(_JPEGTURBO_CONFIG_HEADER IN LISTS _JPEGTURBO_CONFIG_HEADERS)
    if(NOT EXISTS "${_JPEGTURBO_CONFIG_HEADER}")
      continue()
    endif()
    file(STRINGS "${_JPEGTURBO_CONFIG_HEADER}" jpeg_lib_version
         REGEX "^#define[\t ]+JPEGTURBO_LIB_VERSION[\t ]+.*")

    if(NOT jpeg_lib_version)
      continue()
    endif()

    string(REGEX REPLACE "^#define[\t ]+JPEGTURBO_LIB_VERSION[\t ]+([0-9]+).*"
                         "\\1" JPEGTURBO_VERSION "${jpeg_lib_version}")
    break()
  endforeach()
  unset(jpeg_lib_version)
  unset(_JPEGTURBO_CONFIG_HEADER)
  unset(_JPEGTURBO_CONFIG_HEADERS)
  unset(_JPEGTURBO_CONFIG_HEADERS_FEDORA)
  unset(_JPEGTURBO_CONFIG_HEADERS_DEBIAN)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  JPEGTURBO
  REQUIRED_VARS JPEGTURBO_LIBRARY JPEGTURBO_INCLUDE_DIR
  VERSION_VAR JPEGTURBO_VERSION)

if(JPEGTURBO_FOUND)
  set(JPEGTURBO_LIBRARIES ${JPEGTURBO_LIBRARY})
  set(JPEGTURBO_INCLUDE_DIRS "${JPEGTURBO_INCLUDE_DIR}")

  if(NOT TARGET JPEGTURBO::JPEGTURBO)
    add_library(JPEGTURBO::JPEGTURBO UNKNOWN IMPORTED)
    if(JPEGTURBO_INCLUDE_DIRS)
      set_target_properties(
        JPEGTURBO::JPEGTURBO PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
                                        "${JPEGTURBO_INCLUDE_DIRS}")
    endif()
    if(EXISTS "${JPEGTURBO_LIBRARY}")
      set_target_properties(
        JPEGTURBO::JPEGTURBO
        PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "C" IMPORTED_LOCATION
                                                         "${JPEGTURBO_LIBRARY}")
    endif()
    if(EXISTS "${JPEGTURBO_LIBRARY_RELEASE}")
      set_property(
        TARGET JPEGTURBO::JPEGTURBO
        APPEND
        PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(
        JPEGTURBO::JPEGTURBO
        PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "C"
                   IMPORTED_LOCATION_RELEASE "${JPEGTURBO_LIBRARY_RELEASE}")
    endif()
    if(EXISTS "${JPEGTURBO_LIBRARY_DEBUG}")
      set_property(
        TARGET JPEGTURBO::JPEGTURBO
        APPEND
        PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(
        JPEGTURBO::JPEGTURBO
        PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
                   IMPORTED_LOCATION_DEBUG "${JPEGTURBO_LIBRARY_DEBUG}")
    endif()
  endif()
endif()

mark_as_advanced(JPEGTURBO_LIBRARY JPEGTURBO_INCLUDE_DIR)
