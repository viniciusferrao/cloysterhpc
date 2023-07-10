# Add stdc++fs to EL8 systems with gcc-toolset-12
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if(${os_version_suffix} MATCHES "el8")
        if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
            link_directories(/opt/rh/gcc-toolset-12/root/usr/lib/gcc/x86_64-redhat-linux/12)
        endif()
        set(STDC++FS "stdc++fs")
    endif()
endif()
