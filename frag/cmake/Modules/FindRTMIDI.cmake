find_path(
    RTMIDI_INCLUDE_DIR RtMidi.h
    PATH_SUFFIXES rtmidi
)
find_library(
    RTMIDI_LIBRARIES NAMES rtmidi
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    RTMIDI DEFAULT_MSG RTMIDI_LIBRARIES RTMIDI_INCLUDE_DIR
)


mark_as_advanced (RTMIDI_LIBRARIES RTMIDI_INCLUDE_DIR)
