# Find TCLAP - http://tclap.sourceforge.net/


find_path(
    TCLAP_INCLUDE_DIR
    tclap/CmdLine.h
	PATHS
    /usr/local/include/
    /usr/include/
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    TCLAP
    "TCLAP (http://tclap.sourceforge.net/) could not be found after searching /usr/local/include and /usr/include"
    TCLAP_INCLUDE_DIR
)
