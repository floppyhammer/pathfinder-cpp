#ifndef PATHFINDER_GPU_SHADER_MODULE_GL_H
#define PATHFINDER_GPU_SHADER_MODULE_GL_H

#include <regex>

namespace Pathfinder {

class ShaderModuleGl : public ShaderModule {
    friend class DeviceGl;

public:
    ~ShaderModuleGl() override {
        glDeleteShader(id_);
    }

    unsigned int get_handle() const {
        return id_;
    }

private:
    ShaderModuleGl(const std::vector<char>& source_code, ShaderStage shader_stage, const std::string& label = "") {
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
            code_string = std::regex_replace(code_string, std::regex("#version 430"), "#version 310 es");
        } else {
            code_string = std::regex_replace(code_string, std::regex("#version 310 es"), "#version 300 es");
        }
#endif

        auto code_cstr = code_string.c_str();

        glShaderSource(id_, 1, &code_cstr, nullptr);
        glCompileShader(id_);

        check_compile_errors();
    }

    /// Utility function for checking shader compilation errors.
    void check_compile_errors() const {
        GLint success;
        glGetShaderiv(id_, GL_COMPILE_STATUS, &success);

        if (!success) {
            GLchar info_log[1024];
            glGetShaderInfoLog(id_, 1024, nullptr, info_log);

            std::ostringstream string_stream;
            string_stream << "SHADER_COMPILATION_ERROR of : " << label_ << "\n" << info_log;
            Logger::error(string_stream.str(), "OpenGL");
        }
    }

    unsigned int id_{};
};

} // namespace Pathfinder

#endif // PATHFINDER_GPU_SHADER_MODULE_GL_H
