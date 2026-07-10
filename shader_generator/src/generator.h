#pragma once

#include "../../pathfinder/gpu/shader.h"

void generate_shader(const char* input_shader_file, const char* output_binary_file, Pathfinder::ShaderKind stage);
