enable_testing()

set(shader_sources
    "shaders/simple.vert.hlsl"
)

find_package(Python3 COMPONENTS Interpreter)

set(includeable_shader_sources)
foreach(src IN LISTS shader_sources)
    get_filename_component(src_name "${src}" NAME)
    set(src_path "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
    set(hdr_path "${CMAKE_CURRENT_SOURCE_DIR}/shaders/hinc/${src_name}.h")
    list(APPEND includeable_shader_sources "${hdr_path}")
    if(Python3_VERSION VERSION_GREATER_EQUAL "3.11")
        add_custom_command(OUTPUT "${hdr_path}"
            COMMAND Python3::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/embed.py"
                -i "${src_path}" -o "${hdr_path}" -z
            DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/embed.py" "${src_path}"
        )
    endif()
endforeach()

find_package(SDL3 REQUIRED COMPONENTS SDL3_test)

add_executable(testshadercross
    main.c
    ${includeable_shader_sources}
)
sdl_add_warning_options(testshadercross)
target_link_libraries(testshadercross PRIVATE SDL3_shadercross::SDL3_shadercross)
target_link_libraries(testshadercross PRIVATE SDL3::SDL3_test)
target_link_libraries(testshadercross PRIVATE SDL3::SDL3)

set(test_args)
if(SDLSHADERCROSS_TESTS_TRACKMEM)
    list(APPEND test_args "--trackmem")
endif()
add_test(NAME testshadercross COMMAND testshadercross ${test_args})
set_property(TEST testshadercross PROPERTY TIMEOUT 10)

if(UNIX AND NOT APPLE)
    set_property(TEST testshadercross APPEND PROPERTY ENVIRONMENT_MODIFICATION "LD_LIBRARY_PATH=path_list_prepend:${SDL3_shadercross_BINARY_DIR}/vkd3d-prefix/lib")
    if(SDLSHADERCROSS_INSTALL_RUNTIME)
        set_property(TEST testshadercross APPEND PROPERTY ENVIRONMENT_MODIFICATION "LD_LIBRARY_PATH=path_list_prepend:$<TARGET_FILE_DIR:DirectXShaderCompiler::dxcompiler>")
    endif()
elseif(WIN32)
    set_property(TEST testshadercross APPEND PROPERTY ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:DirectXShaderCompiler::dxcompiler>")
    set_property(TEST testshadercross APPEND PROPERTY ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:SDL3::SDL3>")
    set_property(TEST testshadercross APPEND PROPERTY ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:SDL3_shadercross::SDL3_shadercross>")
    if(TARGET spirv-cross-c-shared)
        set_property(TEST testshadercross APPEND PROPERTY ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:spirv-cross-c-shared>")
    endif()
endif()
if(SDLSHADERCROSS_TESTS_TRACKMEM)
    set_property(TEST testshadercross PROPERTY FAIL_REGULAR_EXPRESSION "Total: [0-9]+\\.[0-9]+ Kb in [1-9][0-9]* allocations")
endif()
