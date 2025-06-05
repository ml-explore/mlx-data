# Try to find libsndfile
#
# Provides the cmake config target - SndFile::sndfile
#
# Inputs: SndFile_INC_DIR: include directory for sndfile headers
# SndFile_LIB_DIR: directory containing sndfile libraries SndFile_ROOT_DIR:
# directory containing sndfile installation
#
# Defines: SndFile_FOUND - system has libsndfile SndFile_INCLUDE_DIRS - the
# libsndfile include directory SndFile_LIBRARIES - Link these to use libsndfile
#

find_package(SndFile CONFIG QUIET)

# this function runtime-checks if SndFile supports the given format
function(sndfile_supports_format format_name format var)
  set(src_${format_name}
      "
#include <sndfile.h>
#include <memory.h>
sf_count_t noop_vio_get_filelen(void* user_data) {
  return 0;
}
sf_count_t noop_vio_seek(sf_count_t offset, int whence, void* user_data) {
  return 0;
}
sf_count_t noop_vio_read(void* ptr, sf_count_t count, void* user_data) {
  return count;
}
sf_count_t noop_vio_write(const void* ptr, sf_count_t count, void* user_data) {
  return count;
}
sf_count_t noop_vio_tell(void* user_data) {
  return 0;
}
int main() {
  SF_VIRTUAL_IO noop_io = {
      noop_vio_get_filelen,
      noop_vio_seek,
      noop_vio_read,
      noop_vio_write,
      noop_vio_tell};
  SF_INFO info;
  info.samplerate = 16000;
  info.channels = 2;
  info.format = ${format};
  return sf_open_virtual(&noop_io, SFM_WRITE, &info, NULL) == NULL;
}
     ")
  set(src_${format_name}_filename
      "${CMAKE_CURRENT_BINARY_DIR}/test_sndfile_${format_name}.c")
  file(WRITE ${src_${format_name}_filename} "${src_${format_name}}")
  try_run(
    ${format_name}_EXITCODE ${format_name}_COMPILES ${CMAKE_CURRENT_BINARY_DIR}
    ${src_${format_name}_filename}
    CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${SndFile_INCLUDE_DIRS}" LINK_LIBRARIES
                ${SndFile_LIBRARIES} ${CODEC_LIBRARIES}
    # these are not use at this time but could be useful for debug
    COMPILE_OUTPUT_VARIABLE ${format_name}_COMPILE_OUTPUT
    RUN_OUTPUT_VARIABLE ${format_name}_RUN_OUTPUT)
  if(${format_name}_EXITCODE STREQUAL "0")
    set(${var}
        TRUE
        PARENT_SCOPE)
    message(STATUS "SndFile: support for ${format_name} detected")
  else()
    set(${var}
        FALSE
        PARENT_SCOPE)
    message(STATUS "SndFile: does not support ${format_name}")
  endif()
endfunction()

if(TARGET SndFile::sndfile)
  sndfile_supports_format(flac "SF_FORMAT_FLAC  | SF_FORMAT_PCM_16"
                          SNDFILE_SUPPORTS_FLAC "")
  sndfile_supports_format(vorbis "SF_FORMAT_OGG  | SF_FORMAT_VORBIS"
                          SNDFILE_SUPPORTS_VORBIS "")
  sndfile_supports_format(opus "SF_FORMAT_OGG  | SF_FORMAT_OPUS"
                          SNDFILE_SUPPORTS_OPUS "")
  sndfile_supports_format(mpeg "SF_FORMAT_MPEG  | SF_FORMAT_MPEG_LAYER_III"
                          SNDFILE_SUPPORTS_MPEG "")
  # message(STATUS"Found libsndfile: (lib: ${SndFile_LIBRARIES} include:
  # ${SndFile_INCLUDE_DIRS})")
else()
  find_path(
    SndFile_INCLUDE_DIR sndfile.h
    PATHS ${SndFile_INC_DIR} ${SndFile_ROOT_DIR}/include
    PATH_SUFFIXES include)

  find_library(
    SndFile_LIBRARY sndfile
    PATHS ${SndFile_LIB_DIR} ${SndFile_ROOT_DIR}
    PATH_SUFFIXES lib
    HINTS SNDFILE)

  set(SndFile_INCLUDE_DIRS ${SndFile_INCLUDE_DIR})
  set(SndFile_LIBRARIES ${SndFile_LIBRARY})

  mark_as_advanced(SndFile_INCLUDE_DIRS SndFile_LIBRARIES)
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(SndFile DEFAULT_MSG SndFile_INCLUDE_DIRS
                                    SndFile_LIBRARIES)

  if(SndFile_FOUND)
    # We will run-time check if SndFile supports libraries. We want to link only
    # againsts the necessary ones.

    # SndFile must be built with encoder libs. These transitive dependencies
    # will be taken care of if a SndFile config is found via find_dependency() -
    # we need them for creating a new IMPORTED target
    find_package(Ogg)
    find_package(Vorbis)
    find_package(FLAC)
    # recent SndFile may also include the following
    find_package(Opus)
    find_package(mpg123)
    find_package(mp3lame)

    set(CODEC_LIBRARIES)
    if(OGG_FOUND)
      list(APPEND CODEC_LIBRARIES ${OGG_LIBRARIES})
    endif()
    if(VORBIS_FOUND)
      list(APPEND CODEC_LIBRARIES ${VORBIS_LIBRARIES})
    endif()
    if(FLAC_FOUND)
      list(APPEND CODEC_LIBRARIES ${FLAC_LIBRARIES})
    endif()
    if(OPUS_FOUND)
      list(APPEND CODEC_LIBRARIES ${OPUS_LIBRARIES})
    endif()
    if(MPG123_FOUND)
      list(APPEND CODEC_LIBRARIES ${MPG123_LIBRARIES})
    endif()
    if(MP3LAME_FOUND)
      list(APPEND CODEC_LIBRARIES ${MP3LAME_LIBRARIES})
    endif()
    sndfile_supports_format(flac "SF_FORMAT_FLAC  | SF_FORMAT_PCM_16"
                            SNDFILE_SUPPORTS_FLAC)
    sndfile_supports_format(vorbis "SF_FORMAT_OGG  | SF_FORMAT_VORBIS"
                            SNDFILE_SUPPORTS_VORBIS)
    sndfile_supports_format(opus "SF_FORMAT_OGG  | SF_FORMAT_OPUS"
                            SNDFILE_SUPPORTS_OPUS)
    sndfile_supports_format(mpeg "SF_FORMAT_MPEG  | SF_FORMAT_MPEG_LAYER_III"
                            SNDFILE_SUPPORTS_MPEG)

    # add dependencies on found libraries
    if(SNDFILE_SUPPORTS_FLAC)
      if(FLAC_FOUND)
        get_target_property(FLAC_LIB FLAC::FLAC IMPORTED_LOCATION)
        list(APPEND SNDFILE_DEP_LIBRARIES ${FLAC_LIB})
      else()
        message(FATAL_ERROR "SndFile supports FLAC but FLAC not found")
      endif()
    endif()
    if(SNDFILE_SUPPORTS_VORBIS OR SNDFILE_SUPPORTS_OPUS)
      if(OGG_FOUND)
        get_target_property(OGG_LIB Ogg::ogg IMPORTED_LOCATION)
        list(APPEND SNDFILE_DEP_LIBRARIES ${OGG_LIB})
      else()
        message(FATAL_ERROR "SndFile supports Ogg but Ogg not found")
      endif()
      if(SNDFILE_SUPPORTS_VORBIS)
        if(VORBIS_FOUND)
          get_target_property(VORBIS_LIB Vorbis::vorbis IMPORTED_LOCATION)
          get_target_property(VORBIS_ENC_LIB Vorbis::vorbisenc
                              IMPORTED_LOCATION)
          list(APPEND SNDFILE_DEP_LIBRARIES ${VORBIS_LIB} ${VORBIS_ENC_LIB})
        else()
          message(FATAL_ERROR "SndFile supports Vorbis but Vorbis not found")
        endif()
      endif()
      if(SNDFILE_SUPPORTS_OPUS)
        if(OPUS_FOUND)
          get_target_property(OPUS_LIB Opus::opus IMPORTED_LOCATION)
          list(APPEND SNDFILE_DEP_LIBRARIES ${OPUS_LIB})
        else()
          message(FATAL_ERROR "SndFile supports Opus but Opus not found")
        endif()
      endif()
    endif()
    if(SNDFILE_SUPPORTS_MPEG)
      if(MPG123_FOUND AND MP3LAME_FOUND)
        get_target_property(MPG123_LIB MPG123::libmpg123 IMPORTED_LOCATION)
        get_target_property(MP3LAME_LIB mp3lame::mp3lame IMPORTED_LOCATION)
        list(APPEND SNDFILE_DEP_LIBRARIES ${MPG123_LIB} ${MP3LAME_LIB})
      else()
        message(
          FATAL_ERROR "SndFile supports MPEG but mpg123 and mp3lame not found")
      endif()
    endif()
    add_library(SndFile::sndfile UNKNOWN IMPORTED)
    set_target_properties(
      SndFile::sndfile
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SndFile_INCLUDE_DIRS}"
                 IMPORTED_LOCATION "${SndFile_LIBRARIES}"
                 INTERFACE_LINK_LIBRARIES "${SNDFILE_DEP_LIBRARIES}")
    message(
      STATUS
        "Found libsndfile: (lib: ${SndFile_LIBRARIES} include: ${SndFile_INCLUDE_DIRS})"
    )
  else()
    message(STATUS "libsndfile not found.")
  endif()
endif()
