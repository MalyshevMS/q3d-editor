#include <glad/glad.h>
#include "app.hpp"
#include <q3d/gl/gl.hpp>
#include <q3d/core/active_camera.hpp>
#include <q3d/obj/2d/plane.hpp>
#include <q3d/obj/3d/box.hpp>
#include <glm/glm.hpp>
#include "config.txx"

Application::Application(std::string_view argv0)
 : window("q3d editor", { 1280, 720 }), cam(window.getAspectRatio(), 90.f), res(nullptr) {
    res = q3d::Resources::getInstance(argv0);
    q3d::core::ActiveCamera::getInstance(q3d::ptr<q3d::core::Camera>(&cam));
}

void Application::run() {
    auto texture = res->loadTexture("tex1", "res/texture.png");
    texture->setFilter(q3d::gl::Texture::Filter::NearestMMNearest, q3d::gl::Texture::Filter::Nearest);
    texture->uv = glm::vec2(1, 1);

    auto shader = res->loadShader("main", "res/main.vert", "res/main.frag");

    q3d::object::Plane plane(shader, {}, texture);
    q3d::object::Box box(shader, {}, texture);

    box.transform.position.z = -5.f;

    cam.setPosition(glm::vec3(0.f, 0.f, 3.f));

    q3d::gl::clearColor(q3d::core::Color::Cyan);
    q3d::gl::enable(q3d::gl::feature::depthTest);
    while (window.isOpen()) {
        // CPU (math)

        const auto dt = window.getDeltaTime();
        const auto dm = window.getDeltaMouse();
        const float cameraMove = cfg::cameraSpeed * dt;
        auto cameraMoveDelta = glm::vec3(0.f);
        auto cameraRotateDelta = glm::vec3(0.f);

        plane.transform.rotation.y += cfg::rotSpeed * dt;

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
            cameraRotateDelta.x -= dm.y * cfg::cameraSensetivity * dt;
            cameraRotateDelta.y -= dm.x * cfg::cameraSensetivity * dt;
        } else window.showCursor();

        cam.moveRotate(cameraMoveDelta, cameraRotateDelta);

        // GPU

        q3d::gl::clear();
        
        plane.draw();
        box.draw();

        window.update();
    }
}

Application::~Application() {
    q3d::Window::terminate();
}
