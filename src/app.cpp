#include <glad/glad.h>
#include "app.hpp"
#include <memory>
#include <q3d/gl/gl.hpp>
#include <q3d/core/active_camera.hpp>
#include <q3d/obj/2d/plane.hpp>
#include <q3d/obj/3d/box.hpp>
#include <q3d/obj/3d/model.hpp>
#include <q3d/core/scene.hpp>
#include <q3d/ui/canvas.hpp>
#include <glm/glm.hpp>
#include "config.txx"
#include "glm/ext/vector_float3.hpp"
#include "q3d/core/camera.hpp"
#include "q3d/core/color.hpp"
#include "q3d/window/keys.hpp"
#include "q3d/window/window.hpp"

Application::Application(std::string_view argv0)
 : window("q3d editor", { 854, 480 }), res(nullptr) {
    res = q3d::Resources::getInstance(argv0);
    cam = std::make_shared<q3d::core::Camera>(window.getAspectRatio(), 90.f);
    q3d::core::ActiveCamera::set(cam);
}

void Application::run() {
    auto texture = res->loadTexture("texture", "res/sticker.png");
    texture->setFilter(q3d::gl::Texture::Filter::NearestMMNearest, q3d::gl::Texture::Filter::Nearest);

    auto grass = res->loadTexture("grass", "res/grass.png");

    auto shader = res->loadShader("main", "res/main.vert", "res/main.frag");

    auto plane = std::make_shared<q3d::object::Plane>(shader, q3d::phys::Transform(), texture);
    auto box = std::make_shared<q3d::object::Box>(shader, q3d::phys::Transform(), grass);
    auto customModel = res->loadModel("example", "res/example.obj", shader, texture);

    box->transform.position.z = -5.f;
    box->transform.scale_fac.x = 2.f;
    customModel->transform.position.x = 6.f;
    plane->transform.scale_fac = glm::vec3(100.f);

    cam->setPosition(glm::vec3(0.f, 0.f, 3.f));

    q3d::core::Scene scene;
    q3d::ui::Canvas canvas(window.getSize());

    canvas.add("plane", plane);
    scene.add("box", box);
    scene.add("custom", customModel);

    window.onResize([&](q3d::Window& win, glm::vec2 size){
        canvas.updateSize(size);
        cam->setAspect(size.x / size.y);
    });

    q3d::gl::clearColor(q3d::core::Color::Cyan);
    q3d::gl::enable(q3d::gl::feature::depthTest);
    q3d::gl::enable(q3d::gl::feature::blend);
    while (window.isOpen()) {
        // CPU (math)

        const auto dt = window.getDeltaTime();
        const auto dm = window.getDeltaMouse();
        const float cameraMove = cfg::cameraSpeed * dt;
        auto cameraMoveDelta = glm::vec3(0.f);
        auto cameraRotateDelta = glm::vec3(0.f);

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
        if (window.isKeyPressed(q3d::key::F)) {
            plane->transform.scale_fac += glm::vec3(1.f);
        }
        if (window.isKeyPressed(q3d::key::G)) {
            plane->transform.scale_fac -= glm::vec3(1.f);
        }

        if (window.isMouseButtonPressed(q3d::button::RIGHT)) {
            window.hideCursor();
            cameraRotateDelta.x -= dm.y * cfg::cameraSensetivity * dt;
            cameraRotateDelta.y -= dm.x * cfg::cameraSensetivity * dt;
        } else window.showCursor();

        if (window.isMouseButtonPressed(q3d::button::LEFT)) {
            auto m = window.getMousePos();
            plane->transform.position.x = m.x - window.getSize().x / 2;
            plane->transform.position.y = -m.y + window.getSize().y / 2;
        }

        cam->moveRotate(cameraMoveDelta, cameraRotateDelta);

        // GPU

        q3d::gl::clear();

        scene.render();

        canvas.render();

        window.update();
    }
}

Application::~Application() {
    q3d::Window::terminate();
}
