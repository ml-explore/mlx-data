# Try to find libsamplerate
#
# Provides the cmake config target - SampleRate::samplerate
#
# Inputs: SampleRate_INC_DIR: include directory for samplerate headers
# SampleRate_LIB_DIR: directory containing samplerate libraries SampleRate_ROOT_DIR:
# directory containing samplerate installation
#
# Defines: SampleRate_FOUND - system has libsamplerate SampleRate_INCLUDE_DIRS - the
# libsamplerate include directory SampleRate_LIBRARIES - Link these to use libsamplerate
#

find_package(SampleRate CONFIG)

if(NOT TARGET SampleRate::samplerate)
  find_path(
    SampleRate_INCLUDE_DIR samplerate.h
    PATHS ${SampleRate_INC_DIR} ${SampleRate_ROOT_DIR}/include
    PATH_SUFFIXES include)

  find_library(
    SampleRate_LIBRARY samplerate
    PATHS ${SampleRate_LIB_DIR} ${SampleRate_ROOT_DIR}
    PATH_SUFFIXES lib
    HINTS SAMPLERATE)

  set(SampleRate_INCLUDE_DIRS ${SampleRate_INCLUDE_DIR})
  set(SampleRate_LIBRARIES ${SampleRate_LIBRARY})

  mark_as_advanced(SampleRate_INCLUDE_DIRS SampleRate_LIBRARIES)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(SampleRate DEFAULT_MSG SampleRate_INCLUDE_DIRS
                                    SampleRate_LIBRARIES)

  if(SampleRate_FOUND)
    add_library(SampleRate::samplerate UNKNOWN IMPORTED)
    set_target_properties(
      SampleRate::samplerate
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SampleRate_INCLUDE_DIRS}"
                 IMPORTED_LOCATION "${SampleRate_LIBRARIES}"
                 INTERFACE_LINK_LIBRARIES "${SAMPLERATE_DEP_LIBRARIES}")
    message(
      STATUS
        "Found libsamplerate: (lib: ${SampleRate_LIBRARIES} include: ${SampleRate_INCLUDE_DIRS})"
    )
  else()
    message(STATUS "libsamplerate not found.")
  endif()
endif() # NOT TARGET SampleRate::samplerate
