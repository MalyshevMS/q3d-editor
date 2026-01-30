#include <glad/glad.h>
#include <q3d/window/window.hpp>
#include <q3d/res/resources.hpp>
#include <q3d/gl/shader.hpp>
#include <q3d/gl/vao.hpp>
#include <q3d/gl/texture.hpp>
#include <q3d/gl/gl.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

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

    auto texture = *res->loadTexture("texture", "res/texture.png");

    // Scope for the variables, that must be destroyed before OpenGL Context closed // TODO: Encapsulate
    {
        float angle = 0.f;
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

        glm::mat4 model = glm::mat4(1.f); // Order: T,R,S
        glm::mat4 view = glm::mat4(1.f);
        glm::mat4 proj = glm::mat4(1.f);

        q3d::gl::clearColor(q3d::Color::Cyan);
        while (window.isOpen()) {
            // CPU (math)

            const float dt = window.getDeltaTime();
            const float speed = 360.f;

            proj[0][0] = window.getInversedAspectRatio();
            model = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 0.f, 1.f));
            angle += speed * dt;
            if (angle >= 360.f) angle = 0.f;

            // GPU

            q3d::gl::clear();
            shader.use();
            
            shader.uniform("u_mvp", proj * view * model);

            texture.use(shader);
            vao.draw();
            
            shader.unuse();
            window.update();
        }
    } q3d::Window::terminate();
    return 0;
}
