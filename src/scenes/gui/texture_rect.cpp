//
// Created by floppyhammer on 6/7/2021.
//

#include "texture_rect.h"

#include "../../common/math/basic.h"
#include "../../common/math/mat4x4.h"
#include "../../common/global_macros.h"

namespace Pathfinder {
    TextureRect::TextureRect(float viewport_width, float viewport_height) {
        rect_size.x = viewport_width;
        rect_size.y = viewport_height;

        const std::string vert_source =
#include "../src/shaders/blit.vert"
        ;
        const std::string frag_source =
#include "../src/shaders/blit.frag"
        ;

        shader = std::make_shared<RasterProgram>(vert_source, frag_source);

        // Set VAO&VBO.
        // ---------------------------
        // Set up vertex data (and buffer(s)) and configure vertex attributes.
        float vertices[] = {
                // Positions, colors, UVs.
                -1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
                1.0, -1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0,
                1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0,

                -1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
                1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0,
                -1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        // Bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // ---------------------------
    }

    TextureRect::~TextureRect() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    void TextureRect::set_texture(std::shared_ptr<Texture> p_texture) {
        texture = std::move(p_texture);
    }

    std::shared_ptr<Texture> TextureRect::get_texture() const {
        return texture;
    }

    void TextureRect::attach_shader(std::shared_ptr<RasterProgram> p_shader) {
        shader = std::move(p_shader);
    }

    void TextureRect::draw() {
        // Get MVP matrix.
        // -------------------------------------------------
        // Model position.
        auto model_mat = Mat4x4<float>(1.f);
        model_mat = model_mat.translate(Vec3<float>(rect_position.x / rect_size.x * 2.0f,
                                                    rect_position.y / rect_size.y * 2.0f,
                                                    0.0f));
        model_mat = model_mat.scale(Vec3<float>(rect_scale.x, rect_scale.y, 1.0f));
        //model_mat = model_mat.rotate(deg2rad(rect_rotation), Vec3<float>(0.0f, 0.0f, 1.0f));

        auto mvp_mat = model_mat;
        // -------------------------------------------------

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        // Use program.
        shader->use();

        // Set uniforms.
        shader->set_mat4("uMvpMat", mvp_mat);

        shader->set_vec3("uIsoFactor",
                         (float) texture->get_width() / rect_size.x,
                         (float) texture->get_height() / rect_size.y,
                         1.0);

        shader->bind_texture(0, "", texture->get_texture_id());

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // Position attribute.
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) nullptr);
        glEnableVertexAttribArray(0);
        // Color attribute.
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // UV attribute.
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}
