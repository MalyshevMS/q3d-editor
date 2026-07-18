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
#include <q3d/phys/transform.hpp>
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
    q3d::core::Scene scene;
    q3d::ui::Canvas canvas(window.getSize());

    auto texture = res->loadTexture("texture", "res/texture.png");
    texture->setFilter(q3d::gl::Texture::Filter::NearestMMNearest, q3d::gl::Texture::Filter::Nearest);

    auto grass = res->loadTexture("grass", "res/grass.png");

    auto shader = res->loadShader("main", "res/main.vert", "res/main.frag");
    auto textShader = res->loadShader("text", "res/text.vert", "res/text.frag");

    auto impact = res->loadFont("impact", "/usr/share/fonts/TTF/Impact.TTF", 40);

    auto customModel = res->loadModel("example", "res/example.obj", shader, texture);
    auto box = scene.create<q3d::object::Box>("box", shader, q3d::phys::Transform(), grass);
    auto plane = scene.create<q3d::object::Plane>("plane", shader, q3d::phys::Transform(), texture);
    auto plane2 = canvas.create<q3d::object::Plane>("ui-plane", shader, q3d::phys::Transform({}, {}, glm::vec3(150.f)), texture);
    auto text = canvas.create<q3d::ui::Text>("text", textShader, impact, "Hello, World!\nЯ также поддерживаю русский язык (если есть в шрифте)!\n\tА также управляющие символы (\\n, \\t и другие)!", q3d::phys::Transform(), q3d::core::Color::Black);
    // text->setText("Можно также изменить текст после создания!");

    scene.add("custom", customModel);

    box->transform.position.z = -5.f;
    box->transform.scale_fac.x = 2.f;
    customModel->transform.position.x = 6.f;

    text->transform.position.x = 8.f;
    text->transform.position.y = -40.f;
    text->transform.position.z = 0.f;

    cam->setPosition(glm::vec3(0.f, 0.f, 3.f));


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
            plane2->transform.scale_fac += glm::vec3(1.f);
        }
        if (window.isKeyPressed(q3d::key::G)) {
            plane2->transform.scale_fac -= glm::vec3(1.f);
        }

        if (window.isMouseButtonPressed(q3d::button::RIGHT)) {
            window.hideCursor();
            cameraRotateDelta.x -= dm.y * cfg::cameraSensetivity * dt;
            cameraRotateDelta.y -= dm.x * cfg::cameraSensetivity * dt;
        } else window.showCursor();

        if (window.isKeyPressed(q3d::key::Z)) {
            auto m = window.getMousePos();
            plane2->transform.position.x = m.x;
            plane2->transform.position.y = -m.y;
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
