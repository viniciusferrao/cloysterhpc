# Set package details
set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${CMAKE_PROJECT_VERSION})
set(CPACK_PACKAGE_CONTACT "vinicius@ferrao.net.br")
set(CPACK_PACKAGE_VENDOR "CloysterHPC")

# Specify RPM-specific configurations
set(CPACK_GENERATOR "RPM")
set(CPACK_RPM_PACKAGE_RELEASE "1")
set(CPACK_RPM_PACKAGE_LICENSE "Apache-2.0")
set(CPACK_RPM_PACKAGE_URL "https://github.com/viniciusferrao/cloysterhpc")
set(CPACK_RPM_PACKAGE_DESCRIPTION "Cloyster HPC is a turnkey HPC cluster solution with an user-friendly installer.")
set(CPACK_RPM_PACKAGE_REQUIRES "newt")

# Optionally set the RPM build root directory
set(CPACK_RPM_PACKAGE_BUILD_ROOT "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}")

# Generate a Source RPM (SRPM) as well
set(CPACK_RPM_CREATE_SOURCE_RPM ON)

# Use components to install only the binary
set(CPACK_COMPONENTS_GROUPING ONE_PER_GROUP)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(COMPONENT_BINARY bin)
set(CPACK_COMPONENTS_ALL ${COMPONENT_BINARY})

# Experience shows that explicit package naming can help make it easier to sort
# out potential ABI related issues before they start, while helping you
# track a build to a specific GIT SHA
set(CPACK_RPM_${COMPONENT_BINARY}_FILE_NAME
        "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${GIT_SHORT_SHA}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_TYPE}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}.rpm"
)

## Override the binary component package name to remove the '-bin' suffix
set(CPACK_RPM_${COMPONENT_BINARY}_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")

# Include CPack
include(CPack)
