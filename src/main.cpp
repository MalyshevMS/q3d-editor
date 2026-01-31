#include <glad/glad.h>
#include <q3d/window/window.hpp>
#include <q3d/system/camera.hpp>
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
    q3d::Camera cam(window.getAspectRatio(), 90.f);

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

        q3d::gl::clearColor(q3d::Color::Cyan);
        while (window.isOpen()) {
            // CPU (math)

            const float dt = window.getDeltaTime();
            const glm::vec2 dm = window.getDeltaMouse();
            const float speed = 360.f;
            const float cameraSpeed = 10.f;
            const float sensetivity = 5.f;
            float cameraMove = cameraSpeed * dt;
            glm::vec3 cameraMoveDelta = glm::vec3(0.f);
            glm::vec3 cameraRotateDelta = glm::vec3(0.f);

            model = glm::translate(model, glm::vec3(0.f, -1.f, 0.f));
            model = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 0.f, 1.f));
            angle += speed * dt;
            if (angle >= 360.f) angle = 0.f;

            if (window.isKeyPressed('W')) {
                cameraMoveDelta.z += cameraMove;
            }
            if (window.isKeyPressed('S')) {
                cameraMoveDelta.z -= cameraMove;
            }
            if (window.isKeyPressed('D')) {
                cameraMoveDelta.x += cameraMove;
            }
            if (window.isKeyPressed('A')) {
                cameraMoveDelta.x -= cameraMove;
            }
            if (window.isKeyPressed('Q')) {
                cameraMoveDelta.y -= cameraMove;
            }
            if (window.isKeyPressed('E')) {
                cameraMoveDelta.y += cameraMove;
            }

            if (window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
                window.hideCursor();
                cameraRotateDelta.x -= dm.y * sensetivity * dt;
                cameraRotateDelta.y -= dm.x * sensetivity * dt;
            } else window.showCursor();

            cam.moveRotate(cameraMoveDelta, cameraRotateDelta);

            // GPU

            q3d::gl::clear();
            shader.use();
            
            texture.use(shader);
            shader.uniform("u_mvp", cam.getMatrix() * model);
            vao.draw();
            
            shader.unuse();
            window.update();
        }
    } q3d::Window::terminate();
    return 0;
}
