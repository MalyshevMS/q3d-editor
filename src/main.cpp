#include <glad/glad.h>
#include <q3d/window/window.hpp>
#include <q3d/res/resources.hpp>
#include <q3d/gl/shader.hpp>
#include <q3d/gl/vao.hpp>

int main(int argc, char const *argv[]) {
    q3d::Window window {"q3d editor", {800, 600}};
    auto res = q3d::Resources::getInstance(argv[0]);

    const GLfloat verticies[] = {
    //   X      Y      Z            R      G      B         U    V
       -0.5f, -0.5f,  0.0f,        1.f,   0.f,   0.f,      0.f, 0.f,
        0.5f, -0.5f,  0.0f,        0.f,   1.f,   0.f,      0.f, 1.f,
        0.5f,  0.5f,  0.0f,        0.f,   0.f,   1.f,      1.f, 1.f,
       -0.5f,  0.5f,  0.0f,        1.f,   0.f,   1.f,      1.f, 0.f,
    };

    const GLuint ind[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Scope for the variables, that must be destroyed before OpenGL Context closed // TODO: Encapsulate
    {
        bool rising = true;
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
        shader.attach(res->readFile("res/main.vert"), GL_VERTEX_SHADER);
        shader.attach(res->readFile("res/main.frag"), GL_FRAGMENT_SHADER);
        shader.link();

        glClearColor(0.f, 0.f, 0.f, 1.f);
        while (window.isOpen()) {
            glClear(GL_COLOR_BUFFER_BIT);
            shader.use();

            shader.uniform("u_scale", scale);
            vao.draw();

            if (rising) scale += 0.01f;
            else scale -= 0.01f;

            if (scale >= 2.f) rising = false;
            if (scale <= 0.5f) rising = true;
            
            shader.unuse();
            window.update();
        }
    } q3d::Window::terminate();
    return 0;
}
