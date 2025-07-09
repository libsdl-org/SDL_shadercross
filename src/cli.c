/*
  Simple DirectMedia Layer Shader Cross Compiler
  Copyright (C) 2024 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <SDL3_shadercross/SDL_shadercross.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_iostream.h>
#ifdef LEAKCHECK
#include <SDL3/SDL_test_memory.h>
#endif

// We can emit HLSL and JSON as a destination, so let's redefine the shader format enum.
typedef enum ShaderCross_DestinationFormat {
    SHADERFORMAT_INVALID,
    SHADERFORMAT_SPIRV,
    SHADERFORMAT_DXBC,
    SHADERFORMAT_DXIL,
    SHADERFORMAT_MSL,
    SHADERFORMAT_HLSL,
    SHADERFORMAT_JSON
} ShaderCross_ShaderFormat;

void print_help(void)
{
    int column_width = 32;
    SDL_Log("Usage: shadercross <input> [options]");
    SDL_Log("Required options:\n");
    SDL_Log("  %-*s %s", column_width, "-s | --source <value>", "Source language format. May be inferred from the filename. Values: [SPIRV, HLSL]");
    SDL_Log("  %-*s %s", column_width, "-d | --dest <value>", "Destination format. May be inferred from the filename. Values: [DXBC, DXIL, MSL, SPIRV, HLSL, JSON]");
    SDL_Log("  %-*s %s", column_width, "-t | --stage <value>", "Shader stage. May be inferred from the filename. Values: [vertex, fragment, compute]");
    SDL_Log("  %-*s %s", column_width, "-e | --entrypoint <value>", "Entrypoint function name. Default: \"main\".");
    SDL_Log("  %-*s %s", column_width, "-o | --output <value>", "Output file.");
    SDL_Log("\n");
    SDL_Log("Optional options:\n");
    SDL_Log("  %-*s %s", column_width, "-I | --include <value>", "HLSL include directory. Only used with HLSL source.");
    SDL_Log("  %-*s %s", column_width, "-D<name>[=<value>]", "HLSL define. Only used with HLSL source. Can be repeated.");
    SDL_Log("  %-*s %s", column_width, "", "If =<value> is omitted the define will be treated as equal to 1.");
    SDL_Log("  %-*s %s", column_width, "--msl-version <value>", "Target MSL version. Only used when transpiling to MSL. The default is 1.2.0.");
    SDL_Log("  %-*s %s", column_width, "-g | --debug", "Generate debug information when possible. Shaders are valid only when graphics debuggers are attached.");
    SDL_Log("  %-*s %s", column_width, "-p | --pssl", "Generate PSSL-compatible shader. Destination format should be HLSL.");
}

static const char* io_var_type_to_string(SDL_ShaderCross_IOVarType io_var_type, Uint32 vector_size)
{
    switch (io_var_type) {
        case SDL_SHADERCROSS_IOVAR_TYPE_INT8:
            switch (vector_size) {
                case 1: return "byte";
                case 2: return "byte2";
                case 3: return "byte3";
                case 4: return "byte4";
                default: break;
            }
            break;
        case SDL_SHADERCROSS_IOVAR_TYPE_UINT8:
            switch (vector_size) {
                case 1: return "ubyte";
                case 2: return "ubyte2";
                case 3: return "ubyte3";
                case 4: return "ubyte4";
                default: break;
            }
            break;
        case SDL_SHADERCROSS_IOVAR_TYPE_INT16:
            switch (vector_size) {
                case 1: return "short";
                case 2: return "short2";
                case 3: return "short3";
                case 4: return "short4";
                default: break;
            }
            break;
        case SDL_SHADERCROSS_IOVAR_TYPE_UINT16:
            switch (vector_size) {
                case 1: return "ushort";
                case 2: return "ushort2";
                case 3: return "ushort3";
                case 4: return "ushort4";
                default: break;
            }
            break;
        case SDL_SHADERCROSS_IOVAR_TYPE_INT32:
            switch (vector_size) {
                case 1: return "int";
                case 2: return "int2";
                case 3: return "int3";
                case 4: return "int4";
                default: break;
            }
            break;
        case SDL_SHADERCROSS_IOVAR_TYPE_UINT32:
            switch (vector_size) {
                case 1: return "uint";
                case 2: return "uint2";
                case 3: return "uint3";
                case 4: return "uint4";
                default: break;
            }
            break;
        case SDL_SHADERCROSS_IOVAR_TYPE_INT64:
            switch (vector_size) {
                case 1: return "long";
                case 2: return "long2";
                case 3: return "long3";
                case 4: return "long4";
                default: break;
            }
            break;
        case SDL_SHADERCROSS_IOVAR_TYPE_UINT64:
            switch (vector_size) {
                case 1: return "ulong";
                case 2: return "ulong2";
                case 3: return "ulong3";
                case 4: return "ulong4";
                default: break;
            }
            break;
        case SDL_SHADERCROSS_IOVAR_TYPE_FLOAT16:
            switch (vector_size) {
                case 1: return "half";
                case 2: return "half2";
                case 3: return "half3";
                case 4: return "half4";
                default: break;
            }
            break;
        case SDL_SHADERCROSS_IOVAR_TYPE_FLOAT32:
            switch (vector_size) {
                case 1: return "float";
                case 2: return "float2";
                case 3: return "float3";
                case 4: return "float4";
                default: break;
            }
            break;
        case SDL_SHADERCROSS_IOVAR_TYPE_FLOAT64:
            switch (vector_size) {
                case 1: return "double";
                case 2: return "double2";
                case 3: return "double3";
                case 4: return "double4";
                default: break;
            }
            break;
        case SDL_SHADERCROSS_IOVAR_TYPE_UNKNOWN:
        default: break;
    }

    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unknown IO variable type: vector_type=%u vector_size=%u", io_var_type, vector_size);
    return "unknown";
}

void write_graphics_reflect_json(SDL_IOStream *outputIO, SDL_ShaderCross_GraphicsShaderMetadata *info)
{
    SDL_IOprintf(
        outputIO,
        "{ \"samplers\": %u, \"storage_textures\": %u, \"storage_buffers\": %u, \"uniform_buffers\": %u, ",
        info->num_samplers,
        info->num_storage_textures,
        info->num_storage_buffers,
        info->num_uniform_buffers
    );

    SDL_IOprintf(outputIO, "\"inputs\": [");
    for (Uint32 i = 0; i < info->num_inputs; i++) {
        const SDL_ShaderCross_IOVarMetadata* input = &info->inputs[i];
        SDL_IOprintf(outputIO, "{ \"name\": \"%s\", \"type\": \"%s\", \"location\": %u }%s",
            input->name,
            io_var_type_to_string(input->vector_type, input->vector_size),
            input->location,
            i + 1 < info->num_inputs ? ", " : ""
        );
    }
    SDL_IOprintf(outputIO, "], ");

    SDL_IOprintf(outputIO, "\"outputs\": [");
    for (Uint32 i = 0; i < info->num_outputs; i++) {
        const SDL_ShaderCross_IOVarMetadata* output = &info->outputs[i];
        SDL_IOprintf(outputIO, "{ \"name\": \"%s\", \"type\": \"%s\", \"location\": %u }%s",
            output->name,
            io_var_type_to_string(output->vector_type, output->vector_size),
            output->location,
            i + 1 < info->num_outputs ? ", " : ""
        );
    }
    SDL_IOprintf(outputIO, "] }\n");
}

void write_compute_reflect_json(SDL_IOStream *outputIO, SDL_ShaderCross_ComputePipelineMetadata *info)
{
    SDL_IOprintf(
        outputIO,
        "{ \"samplers\": %u, \"readonly_storage_textures\": %u, \"readonly_storage_buffers\": %u, \"readwrite_storage_textures\": %u, \"readwrite_storage_buffers\": %u, \"uniform_buffers\": %u, \"threadcount_x\": %u, \"threadcount_y\": %u, \"threadcount_z\": %u }\n",
        info->num_samplers,
        info->num_readonly_storage_textures,
        info->num_readonly_storage_buffers,
        info->num_readwrite_storage_textures,
        info->num_readwrite_storage_buffers,
        info->num_uniform_buffers,
        info->threadcount_x,
        info->threadcount_y,
        info->threadcount_z
    );
}

int main(int argc, char *argv[])
{
    bool sourceValid = false;
    bool destinationValid = false;
    bool stageValid = false;

    bool spirvSource = false;
    ShaderCross_ShaderFormat destinationFormat = SHADERFORMAT_INVALID;
    SDL_ShaderCross_ShaderStage shaderStage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
    char *outputFilename = NULL;
    char *entrypointName = "main";
    char *includeDir = NULL;

    char *filename = NULL;
    size_t fileSize = 0;
    void *fileData = NULL;
    bool accept_optionals = true;

    SDL_ShaderCross_HLSL_Define *defines = NULL;
    size_t numDefines = 0;

    bool enableDebug = false;
    char *mslVersion = NULL;

    bool psslCompat = false;

#ifdef LEAKCHECK
    SDLTest_TrackAllocations();
#endif

    for (int i = 1; i < argc; i += 1) {
        char *arg = argv[i];

        if (accept_optionals && arg[0] == '-') {
            if (SDL_strcmp(arg, "-h") == 0 || SDL_strcmp(arg, "--help") == 0) {
                print_help();
                return 0;
            } else if (SDL_strcmp(arg, "-s") == 0 || SDL_strcmp(arg, "--source") == 0) {
                if (i + 1 >= argc) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s requires an argument", arg);
                    print_help();
                    return 1;
                }
                i += 1;
                if (SDL_strcasecmp(argv[i], "spirv") == 0) {
                    spirvSource = true;
                    sourceValid = true;
                } else if (SDL_strcasecmp(argv[i], "hlsl") == 0) {
                    spirvSource = false;
                    sourceValid = true;
                } else {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unrecognized source input %s, source must be SPIRV or HLSL!", argv[i]);
                    print_help();
                    return 1;
                }
            } else if (SDL_strcmp(arg, "-d") == 0 || SDL_strcmp(arg, "--dest") == 0) {
                if (i + 1 >= argc) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s requires an argument", arg);
                    print_help();
                    return 1;
                }
                i += 1;
                if (SDL_strcasecmp(argv[i], "DXBC") == 0) {
                    destinationFormat = SHADERFORMAT_DXBC;
                    destinationValid = true;
                } else if (SDL_strcasecmp(argv[i], "DXIL") == 0) {
                    destinationFormat = SHADERFORMAT_DXIL;
                    destinationValid = true;
                } else if (SDL_strcasecmp(argv[i], "MSL") == 0) {
                    destinationFormat = SHADERFORMAT_MSL;
                    destinationValid = true;
                } else if (SDL_strcasecmp(argv[i], "SPIRV") == 0) {
                    destinationFormat = SHADERFORMAT_SPIRV;
                    destinationValid = true;
                } else if (SDL_strcasecmp(argv[i], "HLSL") == 0) {
                    destinationFormat = SHADERFORMAT_HLSL;
                    destinationValid = true;
                } else if (SDL_strcasecmp(argv[i], "JSON") == 0) {
                    destinationFormat = SHADERFORMAT_JSON;
                    destinationValid = true;
                } else {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unrecognized destination input %s, destination must be DXBC, DXIL, MSL or SPIRV!", argv[i]);
                    print_help();
                    return 1;
                }
            } else if (SDL_strcmp(arg, "-t") == 0 || SDL_strcmp(arg, "--stage") == 0) {
                if (i + 1 >= argc) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s requires an argument", arg);
                    print_help();
                    return 1;
                }
                i += 1;
                if (SDL_strcasecmp(argv[i], "vertex") == 0) {
                    shaderStage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
                    stageValid = true;
                } else if (SDL_strcasecmp(argv[i], "fragment") == 0) {
                    shaderStage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
                    stageValid = true;
                } else if (SDL_strcasecmp(argv[i], "compute") == 0) {
                    shaderStage = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE;
                    stageValid = true;
                } else {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unrecognized shader stage input %s, must be vertex, fragment, or compute.", argv[i]);
                    print_help();
                    return 1;
                }
            } else if (SDL_strcmp(arg, "-e") == 0 || SDL_strcmp(arg, "--entrypoint") == 0) {
                if (i + 1 >= argc) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s requires an argument", arg);
                    print_help();
                    return 1;
                }
                i += 1;
                entrypointName = argv[i];
            } else if (SDL_strcmp(arg, "-I") == 0 || SDL_strcmp(arg, "--include") == 0) {
                if (includeDir) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "'%s' can only be used once", arg);
                    print_help();
                    return 1;
                }
                if (i + 1 >= argc) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s requires an argument", arg);
                    print_help();
                    return 1;
                }
                i += 1;
                includeDir = argv[i];
            } else if (SDL_strcmp(arg, "-o") == 0 || SDL_strcmp(arg, "--output") == 0) {
                if (i + 1 >= argc) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s requires an argument", arg);
                    print_help();
                    return 1;
                }
                i += 1;
                outputFilename = argv[i];
            } else if (SDL_strncmp(argv[i], "-D", SDL_strlen("-D")) == 0) {
                numDefines += 1;
                defines = SDL_realloc(defines, sizeof(SDL_ShaderCross_HLSL_Define) * numDefines);
                char *equalSign = SDL_strchr(argv[i], '=');
                if (equalSign != NULL) {
                    defines[numDefines - 1].value = equalSign + 1;
                    size_t len = defines[numDefines - 1].value - argv[i] - 2;
                    defines[numDefines - 1].name = SDL_malloc(len);
                    SDL_utf8strlcpy(defines[numDefines - 1].name, (const char *)argv[i] + 2, len);
                } else { // no '=' was found
                    defines[numDefines - 1].value = NULL;
                    size_t len = SDL_utf8strlen(argv[i]) + 1 - 2;
                    defines[numDefines - 1].name = SDL_malloc(len);
                    SDL_utf8strlcpy(defines[numDefines - 1].name, (const char *)argv[i] + 2, len);
                }
            } else if (SDL_strcmp(arg, "--msl-version") == 0) {
                if (i + 1 >= argc) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s requires an argument", arg);
                    print_help();
                    return 1;
                }
                i += 1;
                mslVersion = argv[i];
            } else if (SDL_strcmp(argv[i], "-g") == 0 || SDL_strcmp(arg, "--debug") == 0) {
                enableDebug = true;
            } else if (SDL_strcmp(arg, "-p") == 0 || SDL_strcmp(arg, "--pssl") == 0) {
                psslCompat = true;
            } else if (SDL_strcmp(arg, "--") == 0) {
                accept_optionals = false;
            } else {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Unknown argument: %s", argv[0], arg);
                print_help();
                return 1;
            }
        } else if (!filename) {
            filename = arg;
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Unknown argument: %s", argv[0], arg);
            print_help();
            return 1;
        }
    }
    if (!filename) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: missing input path", argv[0]);
        print_help();
        return 1;
    }
    if (!outputFilename) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: missing output path", argv[0]);
        print_help();
        return 1;
    }
    fileData = SDL_LoadFile(filename, &fileSize);
    if (fileData == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Invalid file (%s)", SDL_GetError());
        return 1;
    }

    if (!SDL_ShaderCross_Init())
    {
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "%s", "Failed to initialize shadercross!");
        return 1;
    }

    if (!sourceValid) {
        if (SDL_strstr(filename, ".spv")) {
            spirvSource = true;
        } else if (SDL_strstr(filename, ".hlsl")) {
            spirvSource = false;
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", "Could not infer source format!");
            print_help();
            return 1;
        }
    }

    if (!destinationValid) {
        if (SDL_strstr(outputFilename, ".dxbc")) {
            destinationFormat = SHADERFORMAT_DXBC;
        } else if (SDL_strstr(outputFilename, ".dxil")) {
            destinationFormat = SHADERFORMAT_DXIL;
        } else if (SDL_strstr(outputFilename, ".msl")) {
            destinationFormat = SHADERFORMAT_MSL;
        } else if (SDL_strstr(outputFilename, ".spv")) {
            destinationFormat = SHADERFORMAT_SPIRV;
        } else if (SDL_strstr(outputFilename, ".hlsl")) {
            destinationFormat = SHADERFORMAT_HLSL;
        } else if (SDL_strstr(outputFilename, ".json")) {
            destinationFormat = SHADERFORMAT_JSON;
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", "Could not infer destination format!");
            print_help();
            return 1;
        }
    }

    if (!stageValid) {
        if (SDL_strcasestr(filename, ".vert")) {
            shaderStage = SDL_SHADERCROSS_SHADERSTAGE_VERTEX;
        } else if (SDL_strcasestr(filename, ".frag")) {
            shaderStage = SDL_SHADERCROSS_SHADERSTAGE_FRAGMENT;
        } else if (SDL_strcasestr(filename, ".comp")) {
            shaderStage = SDL_SHADERCROSS_SHADERSTAGE_COMPUTE;
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not infer shader stage from filename!");
            print_help();
            return 1;
        }
    }

    SDL_IOStream *outputIO = SDL_IOFromFile(outputFilename, "w");

    if (outputIO == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
        return 1;
    }

    size_t bytecodeSize;
    int result = 0;

    // null-terminate the defines array
    if (defines != NULL) {
        defines = SDL_realloc(defines, sizeof(SDL_ShaderCross_HLSL_Define) * (numDefines + 1));
        defines[numDefines].name = NULL;
        defines[numDefines].value = NULL;
    }

    if (spirvSource) {
        SDL_ShaderCross_SPIRV_Info spirvInfo;
        spirvInfo.bytecode = fileData;
        spirvInfo.bytecode_size = fileSize;
        spirvInfo.entrypoint = entrypointName;
        spirvInfo.shader_stage = shaderStage;
        spirvInfo.enable_debug = enableDebug;
        spirvInfo.name = filename;
        spirvInfo.props = SDL_CreateProperties();
        if (mslVersion) {
            SDL_SetStringProperty(spirvInfo.props, SDL_SHADERCROSS_PROP_SPIRV_MSL_VERSION, mslVersion);
        }

        if (psslCompat) {
            SDL_SetBooleanProperty(spirvInfo.props, SDL_SHADERCROSS_PROP_SPIRV_PSSL_COMPATIBILITY, true);
        }

        switch (destinationFormat) {
            case SHADERFORMAT_DXBC: {
                Uint8 *buffer = SDL_ShaderCross_CompileDXBCFromSPIRV(
                    &spirvInfo,
                    &bytecodeSize);
                if (buffer == NULL) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile DXBC from SPIR-V: %s", SDL_GetError());
                    result = 1;
                } else {
                    SDL_WriteIO(outputIO, buffer, bytecodeSize);
                    SDL_free(buffer);
                }
                break;
            }

            case SHADERFORMAT_DXIL: {
                Uint8 *buffer = SDL_ShaderCross_CompileDXILFromSPIRV(
                    &spirvInfo,
                    &bytecodeSize);
                if (buffer == NULL) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile DXIL from SPIR-V: %s", SDL_GetError());
                    result = 1;
                } else {
                    SDL_WriteIO(outputIO, buffer, bytecodeSize);
                    SDL_free(buffer);
                }
                break;
            }

            case SHADERFORMAT_MSL: {
                char *buffer = SDL_ShaderCross_TranspileMSLFromSPIRV(
                    &spirvInfo);
                if (buffer == NULL) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to transpile MSL from SPIR-V: %s", SDL_GetError());
                    result = 1;
                } else {
                    SDL_IOprintf(outputIO, "%s", buffer);
                    SDL_free(buffer);
                }
                break;
            }

            case SHADERFORMAT_HLSL: {
                char *buffer = SDL_ShaderCross_TranspileHLSLFromSPIRV(
                    &spirvInfo);
                if (buffer == NULL) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to transpile HLSL from SPIRV: %s", SDL_GetError());
                    result = 1;
                } else {
                    SDL_IOprintf(outputIO, "%s", buffer);
                    SDL_free(buffer);
                }
                break;
            }

            case SHADERFORMAT_SPIRV: {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Input and output are both SPIRV. Did you mean to do that?");
                result = 1;
                break;
            }

            case SHADERFORMAT_JSON: {
                if (shaderStage == SDL_SHADERCROSS_SHADERSTAGE_COMPUTE) {
                    SDL_ShaderCross_ComputePipelineMetadata *info = SDL_ShaderCross_ReflectComputeSPIRV(
                        fileData,
                        fileSize,
                        0);
                    if (info) {
                        write_compute_reflect_json(outputIO, info);
                        SDL_free(info);
                    } else {
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to reflect SPIRV: %s", SDL_GetError());
                        result = 1;
                    }
                } else {
                    SDL_ShaderCross_GraphicsShaderMetadata *info = SDL_ShaderCross_ReflectGraphicsSPIRV(
                        fileData,
                        fileSize,
                        0);
                    if (info) {
                        write_graphics_reflect_json(outputIO, info);
                        SDL_free(info);
                    } else {
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to reflect SPIRV: %s", SDL_GetError());
                        result = 1;
                    }
                }
                break;
            }

            case SHADERFORMAT_INVALID: {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Destination format not provided!");
                result = 1;
                break;
            }
        }

        SDL_DestroyProperties(spirvInfo.props);
    } else {
        SDL_ShaderCross_HLSL_Info hlslInfo;
        hlslInfo.source = fileData;
        hlslInfo.entrypoint = entrypointName;
        hlslInfo.include_dir = includeDir;
        hlslInfo.defines = defines;
        hlslInfo.shader_stage = shaderStage;
        hlslInfo.enable_debug = enableDebug;
        hlslInfo.name = filename;
        hlslInfo.props = 0;

        switch (destinationFormat) {
            case SHADERFORMAT_DXBC: {
                Uint8 *buffer = SDL_ShaderCross_CompileDXBCFromHLSL(
                    &hlslInfo,
                    &bytecodeSize);
                if (buffer == NULL) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile DXBC from HLSL: %s", SDL_GetError());
                    result = 1;
                } else {
                    SDL_WriteIO(outputIO, buffer, bytecodeSize);
                    SDL_free(buffer);
                }
                break;
            }

            case SHADERFORMAT_DXIL: {
                Uint8 *buffer = SDL_ShaderCross_CompileDXILFromHLSL(
                    &hlslInfo,
                    &bytecodeSize);
                if (buffer == NULL) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile DXIL from HLSL: %s", SDL_GetError());
                    result = 1;
                } else {
                    SDL_WriteIO(outputIO, buffer, bytecodeSize);
                    SDL_free(buffer);
                }
                break;
            }

            // TODO: Should we have TranspileMSLFromHLSL?
            case SHADERFORMAT_MSL: {
                void *spirv = SDL_ShaderCross_CompileSPIRVFromHLSL(
                    &hlslInfo,
                    &bytecodeSize);
                if (spirv == NULL) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to transpile MSL from HLSL: %s", SDL_GetError());
                    result = 1;
                } else {
                    SDL_ShaderCross_SPIRV_Info spirvInfo;
                    spirvInfo.bytecode = spirv;
                    spirvInfo.bytecode_size = bytecodeSize;
                    spirvInfo.entrypoint = entrypointName;
                    spirvInfo.shader_stage = shaderStage;
                    spirvInfo.enable_debug = enableDebug;
                    spirvInfo.props = SDL_CreateProperties();
                    if (mslVersion) {
                        SDL_SetStringProperty(spirvInfo.props, SDL_SHADERCROSS_PROP_SPIRV_MSL_VERSION, mslVersion);
                    }
                    char *buffer = SDL_ShaderCross_TranspileMSLFromSPIRV(
                        &spirvInfo);
                    if (buffer == NULL) {
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to transpile MSL from HLSL: %s", SDL_GetError());
                        result = 1;
                    } else {
                        SDL_IOprintf(outputIO, "%s", buffer);
                        SDL_free(spirv);
                        SDL_free(buffer);
                    }
                    SDL_DestroyProperties(spirvInfo.props);
                }
                break;
            }

            case SHADERFORMAT_SPIRV: {
                Uint8 *buffer = SDL_ShaderCross_CompileSPIRVFromHLSL(
                    &hlslInfo,
                    &bytecodeSize);
                if (buffer == NULL) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile SPIR-V From HLSL: %s", SDL_GetError());
                    result = 1;
                } else {
                    SDL_WriteIO(outputIO, buffer, bytecodeSize);
                    SDL_free(buffer);
                }
                break;
            }

            case SHADERFORMAT_HLSL: {
                void *spirv = SDL_ShaderCross_CompileSPIRVFromHLSL(
                    &hlslInfo,
                    &bytecodeSize);

                if (spirv == NULL) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile HLSL to SPIRV: %s", SDL_GetError());
                    result = 1;
                    break;
                }

                SDL_ShaderCross_SPIRV_Info spirvInfo;
                spirvInfo.bytecode = spirv;
                spirvInfo.bytecode_size = bytecodeSize;
                spirvInfo.entrypoint = entrypointName;
                spirvInfo.shader_stage = shaderStage;
                spirvInfo.enable_debug = enableDebug;
                spirvInfo.props = SDL_CreateProperties();

                if (psslCompat) {
                    SDL_SetBooleanProperty(spirvInfo.props, SDL_SHADERCROSS_PROP_SPIRV_PSSL_COMPATIBILITY, true);
                }

                char *buffer = SDL_ShaderCross_TranspileHLSLFromSPIRV(
                    &spirvInfo);

                if (buffer == NULL) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to transpile HLSL from SPIRV: %s", SDL_GetError());
                    result = 1;
                    break;
                }

                SDL_IOprintf(outputIO, "%s", buffer);
                SDL_free(spirv);
                SDL_free(buffer);
                SDL_DestroyProperties(spirvInfo.props);
                break;
            }

            case SHADERFORMAT_JSON: {
                void *spirv = SDL_ShaderCross_CompileSPIRVFromHLSL(
                    &hlslInfo,
                    &bytecodeSize);

                if (spirv == NULL) {
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to compile HLSL to SPIRV: %s", SDL_GetError());
                    result = 1;
                    break;
                }

                if (shaderStage == SDL_SHADERCROSS_SHADERSTAGE_COMPUTE) {
                    SDL_ShaderCross_ComputePipelineMetadata *info = SDL_ShaderCross_ReflectComputeSPIRV(
                        spirv,
                        bytecodeSize,
                        0);
                    SDL_free(spirv);

                    if (info) {
                        write_compute_reflect_json(outputIO, info);
                        SDL_free(info);
                    } else {
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to reflect SPIRV: %s", SDL_GetError());
                        result = 1;
                    }
                } else {
                    SDL_ShaderCross_GraphicsShaderMetadata *info = SDL_ShaderCross_ReflectGraphicsSPIRV(
                        spirv,
                        bytecodeSize,
                        0);
                    SDL_free(spirv);

                    if (info) {
                        write_graphics_reflect_json(outputIO, info);
                        SDL_free(info);
                    } else {
                        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to reflect SPIRV: %s", SDL_GetError());
                        result = 1;
                    }
                }

                break;
            }

            case SHADERFORMAT_INVALID: {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Destination format not provided!");
                result = 1;
                break;
            }
        }
    }

    SDL_CloseIO(outputIO);
    SDL_free(fileData);
    for (Uint32 i = 0; i < numDefines; i += 1) {
        SDL_free(defines[i].name);
    }
    SDL_free(defines);
    SDL_ShaderCross_Quit();
    SDL_Quit();

#ifdef LEAKCHECK
    SDLTest_LogAllocations();
#endif

    return result;
}
