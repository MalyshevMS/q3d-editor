#include "app.hpp"
#include <q3d/gl/gl.hpp>
#include <q3d/gl/vao.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Application::Application(std::string_view argv0)
 : window("q3d editor", { 1280, 720 }), cam(window.getAspectRatio(), 90.f), res(nullptr) {
    res = q3d::Resources::getInstance(argv0);
}

void Application::run() {
    const GLfloat verticies[] = {
    //   X      Y      Z            R      G      B         U    V
       -0.5f, -0.5f,  0.0f,        1.f,   0.f,   0.f,      0.f, 0.f,
        0.5f, -0.5f,  0.0f,        0.f,   1.f,   0.f,      2.f, 0.f,
        0.5f,  0.5f,  0.0f,        0.f,   0.f,   1.f,      2.f, 2.f,
       -0.5f,  0.5f,  0.0f,        1.f,   0.f,   1.f,      0.f, 2.f,
    };

    const GLuint ind[] = {
        0, 1, 2,
        2, 3, 0,
    };

    // Scope for the variables, that must be destroyed before OpenGL Context closed
    {
        auto texture = *res->loadTexture("tex1", "res/texture.png");

        texture.setFilter(q3d::gl::Texture::Filter::NearestMMNearest, q3d::gl::Texture::Filter::Nearest);

        q3d::gl::buffer::Layout l_xyz_rgb_uv = {
            q3d::gl::buffer::DataType::float3,
            q3d::gl::buffer::DataType::float3,
            q3d::gl::buffer::DataType::float2,
        };

        q3d::gl::Vao vao;
        q3d::gl::Vbo vbo(verticies, sizeof(verticies), l_xyz_rgb_uv);
        q3d::gl::Ibo ibo(ind, 6);

        vao.addVbo(vbo);
        vao.setIbo(ibo);

        q3d::gl::Shader shader;
        shader.attach(res->readFile("res/main.vert"), q3d::gl::Shader::Type::Vertex);
        shader.attach(res->readFile("res/main.frag"), q3d::gl::Shader::Type::Fragment);
        shader.link();

        glm::mat4 model = glm::mat4(1.f); // Order: T,R,S

        model = glm::translate(model, glm::vec3(0.f, 0.f, -1.f));
        model = glm::rotate(model, glm::radians(0.f), glm::vec3(0.f, 0.f, 1.f));
        model = glm::scale(model, glm::vec3(1.f));

        q3d::gl::clearColor(q3d::Color::Cyan);
        while (window.isOpen()) {
            // CPU (math)

            const float dt = window.getDeltaTime();
            const glm::vec2 dm = window.getDeltaMouse();
            const float speed = 360.f;
            const float cameraSpeed = 10.f;
            const float sensetivity = 5.f;
            const float cameraMove = cameraSpeed * dt;
            glm::vec3 cameraMoveDelta = glm::vec3(0.f);
            glm::vec3 cameraRotateDelta = glm::vec3(0.f);

            model = glm::rotate(model, glm::radians(speed * dt), glm::vec3(0.f, 0.f, 1.f));

            if (window.isKeyPressed(q3d::key::W)) {
                cameraMoveDelta.z += cameraMove;
            }
            if (window.isKeyPressed(q3d::key::S)) {
                cameraMoveDelta.z -= cameraMove;
            }
            if (window.isKeyPressed(q3d::key::D)) {
                cameraMoveDelta.x += cameraMove;
            }
            if (window.isKeyPressed(q3d::key::A)) {
                cameraMoveDelta.x -= cameraMove;
            }
            if (window.isKeyPressed(q3d::key::Q)) {
                cameraMoveDelta.y -= cameraMove;
            }
            if (window.isKeyPressed(q3d::key::E)) {
                cameraMoveDelta.y += cameraMove;
            }
            if (window.isKeyPressed(q3d::key::X)) {
                model = glm::scale(model, glm::vec3(1.1f));
            }
            if (window.isKeyPressed(q3d::key::Z)) {
                model = glm::scale(model, glm::vec3(0.9f));
            }

            if (window.isMouseButtonPressed(q3d::button::RIGHT)) {
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
}
