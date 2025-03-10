set(DXC_LINUX_X64_URL "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2502/linux_dxc_2025_02_20.x86_64.tar.gz")
set(DXC_LINUX_X64_HASH "SHA256=e0580d90dbf6053a783ddd8d5153285f0606e5deaad17a7a6452f03acdf88c71")
set(DXC_WINDOWS_X86_X64_ARM64_URL "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2502/dxc_2025_02_20.zip")
set(DXC_WINDOWS_X86_X64_ARM64_HASH "SHA256=70b1913a1bfce4a3e1a5311d16246f4ecdf3a3e613abec8aa529e57668426f85")

get_filename_component(EXTERNAL_PATH "${CMAKE_CURRENT_LIST_DIR}/../external" ABSOLUTE)
if(NOT DEFINED DXC_ROOT)
    set(DXC_ROOT "${EXTERNAL_PATH}/DirectXShaderCompiler-binaries")
endif()

set(DOWNLOAD_LINUX ON)
set(DOWNLOAD_WINDOWS ON)
if(DEFINED CMAKE_SYSTEM_NAME)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(DOWNLOAD_LINUX OFF)
    endif()
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(DOWNLOAD_WINDOWS OFF)
    endif()
endif()

if(DOWNLOAD_LINUX)
    include(FetchContent)
    FetchContent_Populate(
        dxc_linux
        URL  "${DXC_LINUX_X64_URL}"
        URL_HASH  "${DXC_LINUX_X64_HASH}"
        SOURCE_DIR "${DXC_ROOT}/linux"
    )
endif()

if(DOWNLOAD_WINDOWS)
    include(FetchContent)
    FetchContent_Populate(
        dxc_windows
        URL  "${DXC_WINDOWS_X86_X64_ARM64_URL}"
        URL_HASH  "${DXC_WINDOWS_X86_X64_ARM64_HASH}"
        SOURCE_DIR "${DXC_ROOT}/windows"
    )
endif()

message("To make use of the prebuilt DirectXShaderCompiler libraries, configure with:")
message("")
message("  -DSDLSHADERCROSS_VENDORED=OFF")
message("")
message("and")
message("")
message("  -DDirectXShaderCompiler_ROOT=\"${DXC_ROOT}\"")
message("")
