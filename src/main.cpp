#include <glad/glad.h>
#include <q3d/window/window.hpp>
#include <q3d/res/resources.hpp>
#include <q3d/gl/shader.hpp>
#include <q3d/gl/vao.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <q3d/res/stb_image.h>

int main(int argc, char const *argv[]) {
    q3d::Window window {"q3d editor", {800, 600}};
    auto res = q3d::Resources::getInstance(argv[0]);

    const GLfloat verticies[] = {
    //   X      Y      Z            R      G      B         U    V
       -0.5f, -0.5f,  0.0f,        1.f,   0.f,   0.f,      0.f, 0.f,
        0.5f, -0.5f,  0.0f,        0.f,   1.f,   0.f,      1.f, 0.f,
        0.5f,  0.5f,  0.0f,        0.f,   0.f,   1.f,      1.f, 1.f,
       -0.5f,  0.5f,  0.0f,        1.f,   0.f,   1.f,      0.f, 1.f,
    };

    const GLuint ind[] = {
        0, 1, 2,
        2, 3, 0,
    };

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);

    int width, height, channels;
    unsigned char* data = stbi_load(
        (res->getExePath() + "res/texture.png").c_str(),
        &width, &height, &channels,
        0
    );

    if (!data) return -1;

    GLenum fmt = GL_RGB;
    if (channels == 4) fmt = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, fmt, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, GL_TEXTURE0);

    // Scope for the variables, that must be destroyed before OpenGL Context closed // TODO: Encapsulate
    {
        float scale = 1.f;
        q3d::gl::Vao vao;

        q3d::gl::buffer::Layout l_xyz_rgb_uv = {
            q3d::gl::buffer::DataType::float3,
            q3d::gl::buffer::DataType::float3,
            q3d::gl::buffer::DataType::float2,
        };

        q3d::gl::Vbo vbo(verticies, sizeof(verticies), l_xyz_rgb_uv);
        q3d::gl::Ibo ibo(ind, sizeof(ind) / sizeof(GLuint));

        vao.addVbo(vbo);
        vao.setIbo(ibo);

        q3d::gl::Shader shader;
        shader.attach(res->readFile("res/main.vert"), q3d::gl::Shader::Type::Vertex);
        shader.attach(res->readFile("res/main.frag"), q3d::gl::Shader::Type::Fragment);
        shader.link();

        glClearColor(0.f, 0.f, 0.f, 1.f);
        while (window.isOpen()) {
            glClear(GL_COLOR_BUFFER_BIT);
            shader.use();

            shader.uniform("u_scale", scale);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            shader.uniform("u_texture", 0);

            vao.draw();
            
            shader.unuse();
            window.update();
        }
    } q3d::Window::terminate();
    return 0;
}
