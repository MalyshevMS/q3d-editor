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
#include <q3d/ui/font.hpp>
#include <q3d/ui/text.hpp>
#include <glm/glm.hpp>
#include "config.txx"
#include "q3d/phys/transform.hpp"
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <q3d/core/camera.hpp>
#include <q3d/core/color.hpp>
#include <q3d/log/log.hpp>
#include <q3d/window/keys.hpp>
#include <q3d/window/window.hpp>

Application::Application(std::string_view argv0)
 : window("q3d editor", { 1280, 720 }), res(nullptr) {
    res = q3d::Resources::getInstance(argv0);
    cam = std::make_shared<q3d::core::Camera>(window.getAspectRatio(), 90.f);
    q3d::core::ActiveCamera::set(cam);
}

void Application::run() {
    auto texture = res->loadTexture("texture", "res/texture.png");
    texture->setFilter(q3d::gl::Texture::Filter::NearestMMNearest, q3d::gl::Texture::Filter::Nearest);

    auto grass = res->loadTexture("grass", "res/grass.png");

    auto shader = res->loadShader("main", "res/main.vert", "res/main.frag");
    auto textShader = res->loadShader("text", "res/text.vert", "res/text.frag");

    auto impact = std::make_shared<q3d::ui::Font>("/usr/share/fonts/TTF/Impact.TTF", 36);

    auto plane = std::make_shared<q3d::object::Plane>(shader, q3d::phys::Transform(), texture);
    auto plane2 = std::make_shared<q3d::object::Plane>(shader, q3d::phys::Transform(),texture);
    auto box = std::make_shared<q3d::object::Box>(shader, q3d::phys::Transform(), grass);
    auto customModel = res->loadModel("example", "res/example.obj", shader, texture);
    auto text = std::make_shared<q3d::ui::Text>(textShader, impact, "Hello, World!", q3d::phys::Transform(), q3d::core::Color::Red);

    box->transform.position.z = -5.f;
    box->transform.scale_fac.x = 2.f;
    customModel->transform.position.x = 6.f;
    plane->transform.scale_fac = glm::vec3(50.f);
    plane->transform.position.x = 150.f;
    plane->transform.position.y = -150.f;

    text->transform.position.x = 250.f;
    text->transform.position.y = -250.f;
    text->transform.position.z = 1.f;

    cam->setPosition(glm::vec3(0.f, 0.f, 3.f));

    q3d::core::Scene scene;
    q3d::ui::Canvas canvas(window.getSize());

    canvas.add("plane", plane);
    canvas.add("text", text);
    scene.add("plane2", plane2);
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

        if (window.isKeyPressed(q3d::key::Z)) {
            auto m = window.getMousePos();
            plane->transform.position.x = m.x;
            plane->transform.position.y = -m.y;
        }

        if (window.isKeyPressed(q3d::key::X)) {
            auto m = window.getMousePos();
            text->transform.position.x = m.x;
            text->transform.position.y = -m.y;
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
