set(DXC_LINUX_X64_URL "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2505/linux_dxc_2025_05_24.x86_64.tar.gz")
set(DXC_LINUX_X64_HASH "SHA256=b99655f65215287825fcdd49102b17e2a1608eff79ffaf9457514c2676892aa5")
set(DXC_WINDOWS_X86_X64_ARM64_URL "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2505/dxc_2025_05_24.zip")
set(DXC_WINDOWS_X86_X64_ARM64_HASH "SHA256=81380f3eca156d902d6404fd6df9f4b0886f576ff3e18b2cc10d3075ffc9d119")

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
