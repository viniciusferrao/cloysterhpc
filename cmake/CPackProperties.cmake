# Set package details
set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "vinicius@ferrao.net.br")
set(CPACK_PACKAGE_VENDOR "ferrao.net.br")

# Specify RPM-specific configurations
set(CPACK_GENERATOR "RPM")
set(CPACK_RPM_PACKAGE_RELEASE "1")
set(CPACK_RPM_PACKAGE_LICENSE "Apache-2.0")
set(CPACK_RPM_PACKAGE_GROUP "System Environment/Daemons")
set(CPACK_RPM_PACKAGE_URL "https://github.com/viniciusferrao/cloysterhpc")
set(CPACK_RPM_PACKAGE_DESCRIPTION "Cloyster HPC is a turnkey HPC cluster solution with an user-friendly installer.")
# set(CPACK_RPM_PACKAGE_REQUIRES "") # We need to figure out what dependencies Cloyster requires

# Experience shows that explicit package naming can help make it easier to sort
# out potential ABI related issues before they start, while helping you
# track a build to a specific GIT SHA
set(CPACK_PACKAGE_FILE_NAME
        "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_TYPE}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}"
)

# Optionally set the RPM build root directory
set(CPACK_RPM_PACKAGE_BUILD_ROOT "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}")

# Generate a Source RPM (SRPM) as well
set(CPACK_RPM_CREATE_SOURCE_RPM ON)

# Include CPack
include(CPack)
