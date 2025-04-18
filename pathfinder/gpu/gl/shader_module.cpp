#include "shader_module.h"

#include <sstream>

#include "../../common/global_macros.h"
#include "../../common/logger.h"
#include "base.h"

namespace Pathfinder {

bool replace(std::string &str, const std::string &from, const std::string &to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos) {
        return false;
    }
    str.replace(start_pos, from.length(), to);
    return true;
}

ShaderModuleGl::ShaderModuleGl(const std::vector<char> &source_code,
                               ShaderStage shader_stage,
                               const std::string &label) {
    label_ = label;

    switch (shader_stage) {
        case ShaderStage::Vertex: {
            id_ = glCreateShader(GL_VERTEX_SHADER);
        } break;
        case ShaderStage::Fragment: {
            id_ = glCreateShader(GL_FRAGMENT_SHADER);
        } break;
        case ShaderStage::Compute: {
            id_ = glCreateShader(GL_COMPUTE_SHADER);
        } break;
        default:
            abort();
    }

    /// Has to pass string.c_str(), as vector<char>.data() doesn't work.
    std::string code_string = {source_code.begin(), source_code.end()};

#ifdef PATHFINDER_MINIMUM_SHADER_VERSION_SUPPORT
    if (shader_stage == ShaderStage::Compute) {
        replace(code_string, "#version 430", "#version 310 es");
    } else {
        replace(code_string, "#version 310 es", "#version 300 es");
    }
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
