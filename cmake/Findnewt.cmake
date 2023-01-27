find_path(NEWT_INCLUDE_DIR newt.h)
find_library(NEWT_LIBRARY NAMES newt)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(newt
	DEFAULT_MSG
	NEWT_LIBRARY
	NEWT_INCLUDE_DIR)

mark_as_advanced(NEWT_INCLUDE_DIR NEWT_LIBRARY)
include_directories(${NEWT_INCLUDE_DIR})
