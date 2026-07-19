#include "app.hpp"
#include "config.txx"
#include <format>
#include <q3d/q3d.hpp>

Application::Application(std::string_view argv0)
 : window("q3d editor", { 1280, 720 }), res(nullptr) {
    res = q3d::Resources::getInstance(argv0);
    cam = std::make_shared<q3d::core::Camera>(window.getAspectRatio(), 90.f);
    q3d::core::ActiveCamera::set(cam);

    window.setVSync(false);
    window.fpsMax(500);
}

void Application::run() {
    q3d::core::Scene scene;
    q3d::ui::Canvas canvas(window.getSize());

    res->loadShader("object", "res/main.vert", "res/main.frag");
    res->loadShader("text", "res/text.vert", "res/text.frag");
    res->loadTexture("box", "res/box.png");
    res->loadModel("example", "res/example.obj", res->getShader("object"), res->getTexture("box"));
    res->loadFont("impact", "/usr/share/fonts/TTF/Impact.TTF", 30);

    scene.create<q3d::object::Box>("box", res->getShader("object"), res->getTexture("box"), q3d::phys::Transform{});
    scene.add("example-model", res->getModel("example"));

    scene["example-model"]->transform.position.y = 2.f;
    scene["example-model"]->transform.rotation.x = -90.f;

    auto fpst = canvas.create<q3d::ui::Text>("FPS", res->getShader("text"), res->getFont("impact"), "FPS: 0.0", q3d::phys::Transform{}, q3d::core::Color::Red);

    canvas["FPS"]->transform.position.x = 10.f;
    canvas["FPS"]->transform.position.y = -40.f;

    cam->setPosition(glm::vec3(0.f, 0.f, 3.f));

    window.onResize([&](q3d::Window& win, glm::vec2 size){
        canvas.updateSize(size);
        cam->setAspect(size.x / size.y);
    });

    q3d::gl::clearColor(q3d::core::Color(0.5f, 0.5f, 0.5f));

    float targetPitch = 0.f;
    float targetYaw   = 0.f;

    while (window.isOpen()) {
        // CPU (math)

        const float dt = window.getDeltaTime();
        const float cameraMove = cfg::cameraSpeed * dt;

        if (window.isKeyPressed(q3d::key::W)) cam->move({0, 0,  cameraMove});
        if (window.isKeyPressed(q3d::key::S)) cam->move({0, 0, -cameraMove});
        if (window.isKeyPressed(q3d::key::D)) cam->move({ cameraMove, 0, 0});
        if (window.isKeyPressed(q3d::key::A)) cam->move({-cameraMove, 0, 0});
        if (window.isKeyPressed(q3d::key::E)) cam->move({0,  cameraMove, 0});
        if (window.isKeyPressed(q3d::key::Q)) cam->move({0, -cameraMove, 0});

        if (window.isMouseButtonPressed(q3d::button::RIGHT)) {
            window.hideCursor();
            const auto dm = window.getDeltaMouse();

            targetPitch -= dm.y * cfg::cameraSensetivity;
            targetYaw   -= dm.x * cfg::cameraSensetivity;
        } else window.showCursor();

        float currentPitch = glm::mix(cam->getRotation().x, targetPitch, std::min(1.f, cfg::smoothness * dt));
        float currentYaw   = glm::mix(cam->getRotation().y, targetYaw  , std::min(1.f, cfg::smoothness * dt));

        cam->setRotation({currentPitch, currentYaw, 0.f});

        fpst->setText(std::format("FPS: {:.2f}", 1 / dt));

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
