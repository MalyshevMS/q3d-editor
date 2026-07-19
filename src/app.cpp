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
    res->loadFont("default", "res/font.ttf", 40);

    scene.create<q3d::object::Box>("box", res->getShader("object"), res->getTexture("box"), q3d::phys::Transform{});
    scene.add("example-model", res->getModel("example"));

    scene["example-model"]->transform.position.y = 2.f;
    scene["example-model"]->transform.rotation.x = -90.f;

    auto debug = canvas.create<q3d::ui::Text>("debug", res->getShader("text"), res->getFont("default"), "", q3d::phys::Transform{}, q3d::core::Color::White);

    canvas["debug"]->transform.position.x = 10.f;
    canvas["debug"]->transform.position.y = 40.f;

    cam->setPosition(glm::vec3(0.f, 0.f, 3.f));

    window.onResize([&](q3d::Window& win, glm::vec2 size){
        canvas.updateSize(size);
        cam->setAspect(size.x / size.y);
    });

    q3d::gl::clearColor(q3d::core::Color(0.5f, 0.5f, 0.5f));

    auto targetPos = cam->getPosition();
    auto targetRot = cam->getRotation();

    while (window.isOpen()) {
        // CPU (math)

        const float dt = window.getDeltaTime();
        const auto dm = window.getDeltaMouse();
        const float targetMoveStep = cfg::cameraSpeed * dt;
        glm::vec3 moveOffset(0.f);

        if (window.isKeyPressed(q3d::key::W)) moveOffset.z += targetMoveStep;
        if (window.isKeyPressed(q3d::key::S)) moveOffset.z -= targetMoveStep;
        if (window.isKeyPressed(q3d::key::D)) moveOffset.x += targetMoveStep;
        if (window.isKeyPressed(q3d::key::A)) moveOffset.x -= targetMoveStep;
        if (window.isKeyPressed(q3d::key::E)) moveOffset.y += targetMoveStep;
        if (window.isKeyPressed(q3d::key::Q)) moveOffset.y -= targetMoveStep;

        if (moveOffset != glm::vec3(0.f)) {
            glm::vec3 oldPos = cam->getPosition();
            cam->setPosition(targetPos);
            cam->move(moveOffset);
            targetPos = cam->getPosition();
            cam->setPosition(oldPos);
        }

        if (window.isMouseButtonPressed(q3d::button::RIGHT)) {
            window.hideCursor();

            targetRot.x -= dm.y * cfg::cameraSensetivity;
            targetRot.y -= dm.x * cfg::cameraSensetivity;
        } else window.showCursor();

        if (window.isMouseButtonPressed(q3d::button::LEFT)) {
            debug->transform.position.x += dm.x;
            debug->transform.position.y -= dm.y;
        }

        const float alpha = std::min(1.f, cfg::smoothness * dt);
        auto currentRot = glm::mix(cam->getRotation(), targetRot, alpha);
        auto currentPos = glm::mix(cam->getPosition(), targetPos, alpha);

        cam->set(currentPos, currentRot);

        debug->setText(std::format(R"(
FPS: {0:.2f}
DT: {1:.4f}
Position: {2:.2f}; {3:.2f}; {4:.2f} 
Rotation: {5:.2f}; {6:.2f}; {7:.2f} 
            )",
            1 / dt, dt,
            cam->getPosition().x, cam->getPosition().y, cam->getPosition().z,
            cam->getRotation().x, cam->getRotation().y, cam->getRotation().z
        ));

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
