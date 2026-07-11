#include "shader_module.h"

#include <sstream>

#include "../../common/logger.h"
#include "base.h"

// WebGL only supports ES3.0 shaders.
#if defined(__EMSCRIPTEN__) || defined(__ANDROID__) || (defined(__linux__) && defined(__ARM_ARCH)) || \
    (defined(_WIN32) && defined(_M_ARM64))
    #define PATHFINDER_MINIMUM_SHADER_VERSION_SUPPORT
#endif

namespace Pathfinder {

bool replaceFirst(std::string &str, const std::string &from, const std::string &to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos) {
        return false;
    }
    str.replace(start_pos, from.length(), to);
    return true;
}

void replaceAll(std::string &str, const std::string &from, const std::string &to) {
    if (from.empty()) return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

ShaderModuleGl::ShaderModuleGl(const std::vector<char> &source_code,
                               ShaderStage shader_stage,
                               const std::vector<std::pair<uint32_t, std::string>> &texture_binding_map,
                               const std::vector<std::pair<uint32_t, std::string>> &uniform_buffer_binding_map,
                               const std::string &label) {
    label_ = label;
    texture_binding_map_ = texture_binding_map;
    uniform_buffer_binding_map_ = uniform_buffer_binding_map;

    switch (shader_stage) {
        case ShaderStage::Vertex: {
            id_ = glCreateShader(GL_VERTEX_SHADER);
        } break;
        case ShaderStage::Fragment: {
            id_ = glCreateShader(GL_FRAGMENT_SHADER);
        } break;
#ifdef PATHFINDER_ENABLE_COMPUTE
        case ShaderStage::Compute: {
            id_ = glCreateShader(GL_COMPUTE_SHADER);
        } break;
#endif
        default:
            abort();
    }

    /// Has to pass string.c_str(), as vector<char>.data() doesn't work.
    std::string code_string = {source_code.begin(), source_code.end()};

#ifdef PATHFINDER_MINIMUM_SHADER_VERSION_SUPPORT
    if (shader_stage == ShaderStage::Compute) {
        replaceFirst(code_string, "#version 430", "#version 310 es");
    } else {
        replaceFirst(code_string, "#version 310 es", "#version 300 es");
    }

    // Deprecated, should be handled by SPIRV now.
    // if (shader_stage == ShaderStage::Vertex) {
    //     // 1. Find where "#version" is
    //     size_t ver_pos = code_string.find("#version");
    //     if (ver_pos != std::string::npos) {
    //         // 2. Find the following line break
    //         size_t nl_pos = code_string.find('\n', ver_pos);
    //         if (nl_pos != std::string::npos) {
    //             // 3. Add a new line
    //             code_string.insert(nl_pos + 1, "#define gl_VertexIndex gl_VertexID\n");
    //         }
    //     }
    // }
#endif

    auto code_cstr = code_string.c_str();

    glShaderSource(id_, 1, &code_cstr, nullptr);
    glCompileShader(id_);

    check_compile_errors();
}

ShaderModuleGl::~ShaderModuleGl() {
    glDeleteShader(id_);
}

unsigned int ShaderModuleGl::get_handle() const {
    return id_;
}

void ShaderModuleGl::check_compile_errors() const {
    GLint success;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &success);

    if (!success) {
        GLchar info_log[1024];
        glGetShaderInfoLog(id_, 1024, nullptr, info_log);

        std::ostringstream string_stream;
        string_stream << "SHADER_COMPILATION_ERROR of : " << label_ << "\n" << info_log;
        Logger::error(string_stream.str());
    }
}

} // namespace Pathfinder
