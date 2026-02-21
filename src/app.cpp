#include "app.hpp"
#include <q3d/gl/gl.hpp>
#include <q3d/gl/vao.hpp>
#include <q3d/core/active_camera.hpp>
#include <q3d/2d/plane.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Application::Application(std::string_view argv0)
 : window("q3d editor", { 1280, 720 }), cam(window.getAspectRatio(), 90.f), res(nullptr) {
    res = q3d::Resources::getInstance(argv0);
    q3d::core::ActiveCamera::getInstance(std::shared_ptr<q3d::core::Camera>(&cam));
}

void Application::run() {

    // Scope for the variables, that must be destroyed before OpenGL Context closed
    {
        auto texture = res->loadTexture("tex1", "res/texture.png");

        texture->setFilter(q3d::gl::Texture::Filter::NearestMMNearest, q3d::gl::Texture::Filter::Nearest);
        texture->uv = glm::vec2(2, 2);

        auto shader = std::make_shared<q3d::gl::Shader>();
        shader->attach(res->readFile("res/main.vert"), q3d::gl::Shader::Type::Vertex);
        shader->attach(res->readFile("res/main.frag"), q3d::gl::Shader::Type::Fragment);
        shader->link();

        q3d::q2d::Plane plane(shader, {}, texture);

        cam.setPosition(glm::vec3(0.f, 0.f, 3.f));

        q3d::gl::clearColor(q3d::core::Color::Cyan);
        while (window.isOpen()) {
            // CPU (math)

            const float dt = window.getDeltaTime();
            const glm::vec2 dm = window.getDeltaMouse();
            const float cameraSpeed = 10.f;
            const float sensetivity = 5.f;
            const float cameraMove = cameraSpeed * dt;
            glm::vec3 cameraMoveDelta = glm::vec3(0.f);
            glm::vec3 cameraRotateDelta = glm::vec3(0.f);

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

            if (window.isMouseButtonPressed(q3d::button::RIGHT)) {
                window.hideCursor();
                cameraRotateDelta.x -= dm.y * sensetivity * dt;
                cameraRotateDelta.y -= dm.x * sensetivity * dt;
            } else window.showCursor();

            cam.moveRotate(cameraMoveDelta, cameraRotateDelta);

            // GPU
// 
            q3d::gl::clear();
            
            plane.draw();

            window.update();
        }
    } q3d::Window::terminate();
}
