#include <SDL3/SDL.h>
#include <SDL3_shadercross/SDL_shadercross.h>
#include <SDL3/SDL_test.h>

#include "shaders/hinc/simple.vert.hlsl.h"

static int SDLCALL shadercross_testInitQuit(void *args)
{
    SDL_GPUShaderFormat hlsl_formats;
    SDL_GPUShaderFormat spirv_formats;
    (void) args;

    hlsl_formats = SDL_ShaderCross_GetHLSLShaderFormats();
    spirv_formats = SDL_ShaderCross_GetSPIRVShaderFormats();
    SDLTest_AssertPass("                                         input");
    SDLTest_AssertPass("                                      HLSL  SPIRV");
    SDLTest_AssertPass("       SDL_GPU_SHADERFORMAT_PRIVATE      %d      %d", !!(hlsl_formats & SDL_GPU_SHADERFORMAT_PRIVATE), !!(spirv_formats & SDL_GPU_SHADERFORMAT_PRIVATE));
    SDLTest_AssertPass("       SDL_GPU_SHADERFORMAT_DXBC         %d      %d", !!(hlsl_formats & SDL_GPU_SHADERFORMAT_DXBC), !!(spirv_formats & SDL_GPU_SHADERFORMAT_DXBC));
    SDLTest_AssertPass("output SDL_GPU_SHADERFORMAT_DXIL         %d      %d", !!(hlsl_formats & SDL_GPU_SHADERFORMAT_DXIL), !!(spirv_formats & SDL_GPU_SHADERFORMAT_DXIL));
    SDLTest_AssertPass("       SDL_GPU_SHADERFORMAT_MSL          %d      %d", !!(hlsl_formats & SDL_GPU_SHADERFORMAT_MSL), !!(spirv_formats & SDL_GPU_SHADERFORMAT_MSL));
    SDLTest_AssertPass("       SDL_GPU_SHADERFORMAT_METALLIB     %d      %d", !!(hlsl_formats & SDL_GPU_SHADERFORMAT_METALLIB), !!(spirv_formats & SDL_GPU_SHADERFORMAT_METALLIB));
    return TEST_COMPLETED;
}

static int SDLCALL shadercross_CompileHLSL_to_XXX(void *args)
{
    size_t i;
    struct {
        const char *formatname;
        const char *funcname;
        void * (SDLCALL * compile_XXXFromHLSL)(const SDL_ShaderCross_HLSL_Info *, size_t *);
        SDL_GPUShaderFormat format;
    } cases[] = {
        { "DXBC", "SDL_ShaderCross_CompileDXBCFromHLSL", SDL_ShaderCross_CompileDXBCFromHLSL, SDL_GPU_SHADERFORMAT_DXBC },
        { "DXIL", "SDL_ShaderCross_CompileDXILFromHLSL", SDL_ShaderCross_CompileDXILFromHLSL, SDL_GPU_SHADERFORMAT_DXIL },
        { "SPIRV", "SDL_ShaderCross_CompileSPIRVFromHLSL", SDL_ShaderCross_CompileSPIRVFromHLSL, SDL_GPU_SHADERFORMAT_SPIRV },
    };

    (void)args;
    for (i = 0; i < SDL_arraysize(cases); i++) {
        SDL_ShaderCross_HLSL_Info hlsl_info;
        SDL_ShaderCross_HLSL_Define hlsl_defines[2];
        void *shader;
        size_t shader_size;

        SDLTest_AssertPass("Compiling HLSL -> %s", cases[i].formatname);

        if (!(SDL_ShaderCross_GetHLSLShaderFormats() & cases[i].format)) {
            SDLTest_Log("SDL_shadercross does not support HLSL -> %s", cases[i].formatname);
            continue;
        }

        SDLTest_AssertPass("Compile a valid HLSL vertex shader to %s", cases[i].formatname);
        SDL_zero(hlsl_info);
        hlsl_info.source = (const char *)simple_vert_hlsl;
        hlsl_info.entrypoint = "main";
        hlsl_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        shader_size = 0;
        shader = cases[i].compile_XXXFromHLSL(&hlsl_info, &shader_size);
        SDLTest_AssertCheck(shader != NULL, "%s should return a valid compiled DXBC shader (%s)", cases[i].funcname, SDL_GetError());
        SDLTest_AssertCheck(shader_size != 0, "Size of shader returned by %s should be size > 0", cases[i].funcname);
        SDL_free(shader);
        SDL_ClearError();

        SDLTest_AssertPass("Compile a valid HLSL vertex shader to %s with Debug enabled", cases[i].formatname);
        SDL_zero(hlsl_info);
        hlsl_info.source = (const char *)simple_vert_hlsl;
        hlsl_info.entrypoint = "main";
        hlsl_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        hlsl_info.props = SDL_CreateProperties();
        SDL_SetBooleanProperty(hlsl_info.props, SDL_SHADERCROSS_PROP_SHADER_DEBUG_ENABLE_BOOLEAN, true);
        SDL_SetStringProperty(hlsl_info.props, SDL_SHADERCROSS_PROP_SHADER_DEBUG_NAME_STRING, "Simple shader");
        shader_size = 0;
        shader = cases[i].compile_XXXFromHLSL(&hlsl_info, &shader_size);
        SDLTest_AssertCheck(shader != NULL, "%s should return a valid compiled DXBC shader (%s)", cases[i].funcname, SDL_GetError());
        SDLTest_AssertCheck(shader_size != 0, "Size of shader returned by %s should be size > 0", cases[i].funcname);
        SDL_free(shader);
        SDL_DestroyProperties(hlsl_info.props);
        SDL_ClearError();

        SDLTest_AssertPass("Break a HLSL vertex shader by defining a macro");
        SDL_zero(hlsl_info);
        SDL_zero(hlsl_defines);
        hlsl_defines[0].name = "BREAK_SHADER";
        hlsl_info.source = (const char *)simple_vert_hlsl;
        hlsl_info.entrypoint = "main";
        hlsl_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        hlsl_info.defines = hlsl_defines;
        shader_size = 0;
        shader = cases[i].compile_XXXFromHLSL(&hlsl_info, &shader_size);
        SDLTest_AssertCheck(shader == NULL, "%s should fail when getting an invalid shader (%s)", cases[i].funcname, SDL_GetError());
        SDLTest_AssertCheck(shader_size == 0, "Size of shader returned by %s should be == 0", cases[i].funcname);
        SDL_free(shader);
        SDL_ClearError();
    }
    return TEST_COMPLETED;
}

static int SDLCALL shadercross_CompileSPIRV_to_XXX(void *args)
{
    size_t i;
    void *spirv_shader;
    size_t spirv_shader_size;
    struct {
        const char *formatname;
        const char *funcname;
        void * (SDLCALL * compile_XXXFromSPIRV)(const SDL_ShaderCross_SPIRV_Info *, size_t *);
        SDL_GPUShaderFormat format;
    } cases[] = {
        { "DXBC", "SDL_ShaderCross_CompileDXBCFromSPIRV", SDL_ShaderCross_CompileDXBCFromSPIRV, SDL_GPU_SHADERFORMAT_DXBC },
        { "DXIL", "SDL_ShaderCross_CompileDXBCFromSPIRV", SDL_ShaderCross_CompileDXILFromSPIRV, SDL_GPU_SHADERFORMAT_DXIL },
    };

    (void)args;

    {
        SDL_ShaderCross_HLSL_Info hlsl_info;

        SDLTest_AssertPass("Prepare SPIRV vertex shader (HLSL -> SPIRV)");

        if (!(SDL_ShaderCross_GetHLSLShaderFormats() & SDL_GPU_SHADERFORMAT_SPIRV)) {
            SDLTest_AssertPass("SDL_ShaderCross does not support HLSL -> SPIRV");
            return TEST_SKIPPED;
        }

        SDL_zero(hlsl_info);
        hlsl_info.source = (const char *)simple_vert_hlsl;
        hlsl_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        hlsl_info.entrypoint = "main";
        spirv_shader = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlsl_info, &spirv_shader_size);
        SDLTest_AssertCheck(spirv_shader != NULL, "SDL_ShaderCross_CompileSPIRVFromHLSL must return a non-NULL shader (%s)", SDL_GetError());
        if (spirv_shader == NULL) {
            return TEST_ABORTED;
        }
    }

    for (i = 0; i < SDL_arraysize(cases); i++) {
        SDL_ShaderCross_SPIRV_Info spirv_info;
        void *shader;
        size_t shader_size;

        SDLTest_AssertPass("Compiling SPIRV -> %s", cases[i].formatname);

        if (!(SDL_ShaderCross_GetSPIRVShaderFormats() & cases[i].format)) {
            SDLTest_Log("SDL_shadercross does not support SPIRV -> %s", cases[i].formatname);
            continue;
        }

        SDLTest_AssertPass("Compile a valid HLSL vertex shader to %s", cases[i].formatname);
        SDL_zero(spirv_info);
        spirv_info.bytecode = spirv_shader;
        spirv_info.bytecode_size = spirv_shader_size;
        spirv_info.entrypoint = "main";
        spirv_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        shader_size = 0;
        shader = cases[i].compile_XXXFromSPIRV(&spirv_info, &shader_size);
        SDLTest_AssertCheck(shader != NULL, "%s should return a valid compiled DXBC shader (%s)", cases[i].funcname, SDL_GetError());
        SDLTest_AssertCheck(shader_size != 0, "Size of shader returned by %s should be size > 0", cases[i].funcname);
        SDL_free(shader);
        SDL_ClearError();

        SDLTest_AssertPass("Compile a valid HLSL vertex shader to %s with Debug enabled", cases[i].formatname);
        SDL_zero(spirv_info);
        spirv_info.bytecode = spirv_shader;
        spirv_info.bytecode_size = spirv_shader_size;
        spirv_info.entrypoint = "main";
        spirv_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        spirv_info.props = SDL_CreateProperties();
        SDL_SetBooleanProperty(spirv_info.props, SDL_SHADERCROSS_PROP_SHADER_DEBUG_ENABLE_BOOLEAN, true);
        SDL_SetStringProperty(spirv_info.props, SDL_SHADERCROSS_PROP_SHADER_DEBUG_NAME_STRING, "Simple shader");
        shader_size = 0;
        shader = cases[i].compile_XXXFromSPIRV(&spirv_info, &shader_size);
        SDLTest_AssertCheck(shader != NULL, "%s should return a valid compiled DXBC shader (%s)", cases[i].funcname, SDL_GetError());
        SDLTest_AssertCheck(shader_size != 0, "Size of shader returned by %s should be size > 0", cases[i].funcname);
        SDL_free(shader);
        SDL_DestroyProperties(spirv_info.props);
        SDL_ClearError();
    }
    SDL_free(spirv_shader);
    return TEST_COMPLETED;
}

static int SDLCALL shadercross_TranspileSPIRV_to_MSL(void *args)
{
    void *spirv_shader;
    size_t spirv_shader_size;
    void *msl_shader;
    SDL_ShaderCross_SPIRV_Info spirv_info;

    (void)args;
    if (!(SDL_ShaderCross_GetSPIRVShaderFormats() & SDL_GPU_SHADERFORMAT_MSL)) {
        SDLTest_AssertPass("SDL_ShaderCross does not support SPIRV -> MSL");
        return TEST_SKIPPED;
    }

    {
        SDL_ShaderCross_HLSL_Info hlsl_info;

        SDLTest_AssertPass("Prepare SPIRV vertex shader (HLSL -> SPIRV)");
        if (!(SDL_ShaderCross_GetHLSLShaderFormats() & SDL_GPU_SHADERFORMAT_SPIRV)) {
            SDLTest_AssertPass("SDL_ShaderCross does not support HLSL -> SPIRV");
            return TEST_SKIPPED;
        }

        SDL_zero(hlsl_info);
        hlsl_info.source = (const char *)simple_vert_hlsl;
        hlsl_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        hlsl_info.entrypoint = "main";
        spirv_shader = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlsl_info, &spirv_shader_size);
        SDLTest_AssertCheck(spirv_shader != NULL, "SDL_ShaderCross_CompileSPIRVFromHLSL must return a non-NULL shader (%s)", SDL_GetError());
        if (spirv_shader == NULL) {
            return TEST_ABORTED;
        }
    }

    SDLTest_AssertPass("Transpile SPIRV -> MSL");
    SDL_zero(spirv_info);
    spirv_info.bytecode = spirv_shader;
    spirv_info.bytecode_size = spirv_shader_size;
    spirv_info.entrypoint = "main";
    msl_shader = SDL_ShaderCross_TranspileMSLFromSPIRV(&spirv_info);
    SDLTest_AssertCheck(msl_shader != NULL, "SDL_ShaderCross_TranspileMSLFromSPIRV returns non-NULL shader (%s)", SDL_GetError());
    SDL_free(msl_shader);

    SDL_free(spirv_shader);
    return TEST_COMPLETED;
}

static const char *iovartype_to_string(SDL_ShaderCross_IOVarType type)
{
    switch (type) {
    case SDL_SHADERCROSS_IOVAR_TYPE_UNKNOWN:
        return "SDL_SHADERCROSS_IOVAR_TYPE_UNKNOWN";
    case SDL_SHADERCROSS_IOVAR_TYPE_INT8:
        return "SDL_SHADERCROSS_IOVAR_TYPE_INT8";
    case SDL_SHADERCROSS_IOVAR_TYPE_UINT8:
        return "SDL_SHADERCROSS_IOVAR_TYPE_UINT8";
    case SDL_SHADERCROSS_IOVAR_TYPE_INT16:
        return "SDL_SHADERCROSS_IOVAR_TYPE_INT16";
    case SDL_SHADERCROSS_IOVAR_TYPE_UINT16:
        return "SDL_SHADERCROSS_IOVAR_TYPE_UINT16";
    case SDL_SHADERCROSS_IOVAR_TYPE_INT32:
        return "SDL_SHADERCROSS_IOVAR_TYPE_INT32";
    case SDL_SHADERCROSS_IOVAR_TYPE_UINT32:
        return "SDL_SHADERCROSS_IOVAR_TYPE_UINT32";
    case SDL_SHADERCROSS_IOVAR_TYPE_INT64:
        return "SDL_SHADERCROSS_IOVAR_TYPE_INT64";
    case SDL_SHADERCROSS_IOVAR_TYPE_UINT64:
        return "SDL_SHADERCROSS_IOVAR_TYPE_UINT64";
    case SDL_SHADERCROSS_IOVAR_TYPE_FLOAT16:
        return "SDL_SHADERCROSS_IOVAR_TYPE_FLOAT16";
    case SDL_SHADERCROSS_IOVAR_TYPE_FLOAT32:
        return "SDL_SHADERCROSS_IOVAR_TYPE_FLOAT32";
    case SDL_SHADERCROSS_IOVAR_TYPE_FLOAT64:
        return "SDL_SHADERCROSS_IOVAR_TYPE_FLOAT64";
    }
    SDLTest_AssertCheck(false, "Unknown SDL_ShaderCross_IOVarType");
    return NULL;
}

static void log_SDL_ShaderCross_IOVarMetadata(Uint32 index, const SDL_ShaderCross_IOVarMetadata *iovar_metadata)
{
    SDLTest_AssertPass("%" SDL_PRIu32 ": { name: \"%s\", location: %" SDL_PRIu32 ", vector_type: %s, vector_size: %" SDL_PRIu32 " }",
        index, iovar_metadata->name, iovar_metadata->location, iovartype_to_string(iovar_metadata->vector_type), iovar_metadata->vector_size);
}

static int SDLCALL shadercross_ReflectSPIRV(void *args)
{
    Uint32 i;
    void *spirv_shader;
    size_t spirv_shader_size;
    SDL_ShaderCross_GraphicsShaderMetadata *shader_gfx_metadata;

    (void)args;
    if (!(SDL_ShaderCross_GetSPIRVShaderFormats() & SDL_GPU_SHADERFORMAT_MSL)) {
        SDLTest_AssertPass("SDL_ShaderCross does not support SPIRV -> MSL");
        return TEST_SKIPPED;
    }

    {
        SDL_ShaderCross_HLSL_Info hlsl_info;

        SDLTest_AssertPass("Prepare SPIRV vertex shader (HLSL -> SPIRV)");
        if (!(SDL_ShaderCross_GetHLSLShaderFormats() & SDL_GPU_SHADERFORMAT_SPIRV)) {
            SDLTest_AssertPass("SDL_ShaderCross does not support HLSL -> SPIRV");
            return TEST_SKIPPED;
        }

        SDL_zero(hlsl_info);
        hlsl_info.source = (const char *)simple_vert_hlsl;
        hlsl_info.shader_stage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        hlsl_info.entrypoint = "main";
        spirv_shader = SDL_ShaderCross_CompileSPIRVFromHLSL(&hlsl_info, &spirv_shader_size);
        SDLTest_AssertCheck(spirv_shader != NULL, "SDL_ShaderCross_CompileSPIRVFromHLSL must return a non-NULL shader (%s)", SDL_GetError());
        if (spirv_shader == NULL) {
            return TEST_ABORTED;
        }
    }

    SDLTest_AssertPass("Reflect SPIRV");

    shader_gfx_metadata = SDL_ShaderCross_ReflectGraphicsSPIRV(spirv_shader, spirv_shader_size, 0);
    SDLTest_AssertCheck(shader_gfx_metadata != NULL, "SDL_ShaderCross_ReflectGraphicsSPIRV returns non-NULL shader (%s)", SDL_GetError());
    SDLTest_AssertCheck(shader_gfx_metadata->resource_info.num_samplers == 0, "num_samplers is %d, should be 0", shader_gfx_metadata->resource_info.num_samplers);
    SDLTest_AssertCheck(shader_gfx_metadata->resource_info.num_storage_textures == 0, "num_storage_textures is %d, should be 0", shader_gfx_metadata->resource_info.num_storage_textures);
    SDLTest_AssertCheck(shader_gfx_metadata->resource_info.num_storage_buffers == 0, "num_storage_buffers is %d, should be 0", shader_gfx_metadata->resource_info.num_storage_buffers);
    SDLTest_AssertCheck(shader_gfx_metadata->resource_info.num_uniform_buffers == 1, "num_uniform_buffers is %d, should be 1", shader_gfx_metadata->resource_info.num_uniform_buffers);
    SDLTest_AssertCheck(shader_gfx_metadata->num_inputs == 1, "num_inputs is %d, should be 1", shader_gfx_metadata->num_inputs);
    SDLTest_AssertCheck(shader_gfx_metadata->num_outputs == 1, "num_outputs is %d, should be 1", shader_gfx_metadata->num_outputs);

    SDLTest_AssertPass("inputs:");
    for (i = 0; i < shader_gfx_metadata->num_inputs; i++) {
        log_SDL_ShaderCross_IOVarMetadata(i, &shader_gfx_metadata->inputs[i]);
    }
    SDLTest_AssertPass("outputs:");
    for (i = 0; i < shader_gfx_metadata->num_outputs; i++) {
        log_SDL_ShaderCross_IOVarMetadata(i, &shader_gfx_metadata->outputs[i]);
    }

    SDL_free(shader_gfx_metadata);

    SDL_free(spirv_shader);
    return TEST_COMPLETED;
}

static const SDLTest_TestCaseReference shadercrossInitQuit = {
    shadercross_testInitQuit, "shadercrossInitQuit", "Test SDL_ShaderCross_Init and SDL_ShaderCross_Quit", TEST_ENABLED
};

static const SDLTest_TestCaseReference shadercrossCompileHLSL = {
    shadercross_CompileHLSL_to_XXX, "shadercross_CompileHLSL", "Compile HLSL -> {DXBC, DXIL, SPIRV}", TEST_ENABLED
};

static const SDLTest_TestCaseReference shadercrossCompileSPIRV = {
    shadercross_CompileSPIRV_to_XXX, "shadercross_CompileSPIRV", "Compile SPIRV -> {DXBC, DXIL}", TEST_ENABLED
};

static const SDLTest_TestCaseReference shadercrossReflectSPIRV = {
    shadercross_TranspileSPIRV_to_MSL, "shadercross_TranspileSPIRVtoMSL", "Compile SPIRV -> {DXBC, DXIL}", TEST_ENABLED
};

static const SDLTest_TestCaseReference shadercrossTranspileSPIRVToMSL = {
    shadercross_ReflectSPIRV, "shadercross_ReflectSPIRV", "Reflect SPIRV", TEST_ENABLED
};

static const SDLTest_TestCaseReference *shadercrossTests[] = {
    &shadercrossInitQuit,
    &shadercrossCompileHLSL,
    &shadercrossCompileSPIRV,
    &shadercrossTranspileSPIRVToMSL,
    &shadercrossReflectSPIRV,
    NULL
};

static void SDLCALL setup_shadercross(void **args)
{
    bool result;
    (void) args;
    SDL_ClearError();
    result = SDL_ShaderCross_Init();
    SDLTest_AssertCheck(result, "SDL_ShaderCross_Init() succeeded (%s)", SDL_GetError());
    SDL_ClearError();
}

static void SDLCALL teardown_shadercross(void *args)
{
    (void) args;
    SDL_ShaderCross_Quit();
}

static SDLTest_TestSuiteReference shadercrossTestSuite = {
    "SDL_shadercross",
    setup_shadercross,
    shadercrossTests,
    teardown_shadercross
};

static SDLTest_TestSuiteReference *testSuites[] = {
    &shadercrossTestSuite,
    NULL
};

int main(int argc, char *argv[])
{
    int i;
    int result;
    SDLTest_CommonState *state;
    SDLTest_TestSuiteRunner *runner;

    /* Initialize test framework */
    state = SDLTest_CommonCreateState(argv, 0);
    if (!state) {
        return 1;
    }

    runner = SDLTest_CreateTestSuiteRunner(state, testSuites);

    /* Parse commandline */
    for (i = 1; i < argc;) {
        int consumed;

        consumed = SDLTest_CommonArg(state, i);
        if (!consumed) {
            /* Parse custom arguments here */
        }
        if (consumed <= 0) {
            const char *options[] = {
                NULL
            };
            SDLTest_CommonLogUsage(state, argv[0], options);
            return 1;
        }

        i += consumed;
    }

    /* Verify custom options here */

    result = SDLTest_ExecuteTestSuiteRunner(runner);

    SDL_Quit();
    SDLTest_DestroyTestSuiteRunner(runner);
    SDLTest_CommonDestroyState(state);
    return result;
}
