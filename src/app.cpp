#include <glad/glad.h>
#include "app.hpp"
#include "config.txx"
#include <format>
#include <q3d/q3d.hpp>
#include <q3d/gl/fbo.hpp>
#include <q3d/log/log.hpp>

Application::Application(std::string_view argv0)
 : window("q3d editor", { 1280, 720 }), res(nullptr) {
    res = q3d::Resources::getInstance(argv0);
    cam = std::make_shared<q3d::core::Camera>(window.getAspectRatio(), 90.f);
    q3d::core::ActiveCamera::set(cam);

    window.setVSync(false);
    window.fpsMax(1000);

    cam->setFar(1000.f);
}

void Application::run() {
    q3d::core::Scene scene;
    q3d::ui::Canvas canvas(window.getSize());
    q3d::Screen screen;

    res->loadShader("object", "res/main.vert", "res/main.frag");
    res->loadShader("text", "res/text.vert", "res/text.frag");
    res->loadShader("post", "res/post.vert", "res/post.frag");
    res->loadTexture("box", "res/box.png");
    res->loadTexture("grass", "res/grass.png")->uv = glm::vec2(200.f, 200.f);
    res->loadFont("default", "res/font.ttf", 40);
    res->loadMaterial("default", "res/default.json");

    screen.setShader(res->getShader("post"));
    screen.setTexture(res->getTexture("box"));

    scene.create<q3d::object::Box>("box", res->getShader("object"), res->getTexture("box"), q3d::phys::Transform{});
    scene.create<q3d::object::Plane>("plane", res->getShader("object"), res->getTexture("grass"), q3d::phys::Transform(glm::vec3(0.f, -7.f, 0.f), glm::vec3(-90.f, 0.f, 0.f), glm::vec3(100.f, 100.f, 100.f)));

    auto debug = canvas.create<q3d::ui::Text>("debug", res->getShader("text"), res->getFont("default"), "", q3d::phys::Transform{}, q3d::core::Color::White);

    canvas["debug"]->transform.position.x = 10.f;
    canvas["debug"]->transform.position.y = 40.f;

    scene["box"]->material = res->getMaterial("default");
    scene["plane"]->material = res->getMaterial("default");

    cam->setPosition(glm::vec3(0.f, 1.5f, 5.f));

    q3d::gl::Fbo fbo(window.getFBSize());

    window.onResize([&](q3d::Window& win, glm::vec2 size){
        canvas.updateSize(size);
        cam->setAspect(size.x / size.y);
    });

    window.onFBResize([&](q3d::Window& win, glm::vec2 size){
        fbo.updateSize(size);
    });

    auto targetPos = cam->getPosition();
    auto targetRot = cam->getRotation();

    auto lastPos = targetPos;
    auto lastRot = targetRot;

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

        glm::vec2 rotationDelta(
            currentRot.y - lastRot.y,
            currentRot.x - lastRot.x
        );

        glm::vec3 translationDelta = currentPos - lastPos;

        glm::vec2 linearMovement(translationDelta.x, translationDelta.y);

        glm::vec2 blurVector = (rotationDelta * 0.05f) + (linearMovement * 0.3f);

        float blurForward = std::abs(translationDelta.z) * 0.3f;

        lastPos = currentPos;
        lastRot = currentRot;

        debug->setText(std::format(R"(
FPS: {:.2f}
DT: {:.4f}
Position: {:.2f}; {:.2f}; {:.2f}
Rotation: {:.2f}; {:.2f}; {:.2f}
            )",
            1 / dt, dt,
            cam->getPosition().x, cam->getPosition().y, cam->getPosition().z,
            cam->getRotation().x, cam->getRotation().y, cam->getRotation().z
        ));

        // GPU

        fbo.bind();

        q3d::gl::clearColor(q3d::core::Color::Gray);
        q3d::gl::clear();

        scene.render();

        fbo.unbind();

        auto post = res->getShader("post");
        post->use();
        post->uniform("u_blurVector", blurVector);
        post->uniform("u_blurForward", blurForward);
        post->uniform("u_vignettePower", 1.f);
        post->uniform("u_chromaticIntensity", 0.003f);
        screen.setTexture(fbo.getTexture());
        screen.draw();

        canvas.render();

        window.update();
    }
}

Application::~Application() {
    q3d::Window::terminate();
}
