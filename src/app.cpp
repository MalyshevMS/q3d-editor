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
    res->loadShader("depth", "res/depth.vert", "res/depth.frag");
    res->loadShader("light", "res/light.vert", "res/light.frag");
    res->loadShader("point_shadow", "res/point_shadow.vert", "res/point_shadow.frag", "res/point_shadow.geom");
    res->loadTexture("box", "res/box.png");
    res->loadTexture("grass", "res/grass.png")->uv = glm::vec2(200.f, 200.f);
    res->loadFont("default", "/usr/share/fonts/TTF/Impact.TTF", 40);
    res->loadMaterial("default", "res/default.json");
    res->loadModel("sphere", "res/sphere.obj", res->getShader("object"), res->getTexture("grass"));

    screen.setShader(res->getShader("post"));

    scene.initShadows(res->getShader("depth"));
    scene.initPointShadows(res->getShader("point_shadow"));

    scene.create<q3d::object::Box>("box", res->getShader("object"), res->getTexture("box"), q3d::phys::Transform{});
    scene.create<q3d::object::Plane>("plane", res->getShader("object"), res->getTexture("grass"), q3d::phys::Transform(glm::vec3(0.f, -3.f, 0.f), glm::vec3(-90.f, 0.f, 0.f), glm::vec3(100.f, 100.f, 100.f)));
    scene.create<q3d::object::Plane>("plane2", res->getShader("object"), res->getTexture("box"), q3d::phys::Transform(glm::vec3(5.f, 0.f, 0.f)));
    scene.add("custom", res->getModel("sphere"));

    scene.addDirLight("sun", q3d::object::DirLight(res->getShader("light"), q3d::phys::Transform(glm::vec3(5.f))));

    scene.addSpotLight("spot", q3d::object::SpotLightInternal{
        .position = glm::vec3(0.f, 5.f, 0.f),
        .direction = glm::vec3(0.f, -1.f, 0.f),
        .linear = 0.007f,
        .quadratic = 0.0002f,
    });

    auto debug = canvas.create<q3d::ui::Text>("debug", res->getShader("text"), res->getFont("default"), "", q3d::phys::Transform{}, q3d::core::Color::White);

    canvas["debug"]->transform.position.x = 10.f;
    canvas["debug"]->transform.position.y = 40.f;

    scene["box"]->material = res->getMaterial("default");
    scene["plane"]->material = res->getMaterial("default");
    scene["custom"]->material = res->getMaterial("default");

    scene["custom"]->transform.position = glm::vec3(5.f, 0.f, -5.f);

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

    float bias1 = 0.02f;
    float bias2 = 0.0001f;

    while (window.isOpen()) {
        // CPU (math)

        const float dt = window.getDeltaTime();
        const auto dm = window.getDeltaMouse();
        const float targetMoveStep = cfg::cameraSpeed * dt;
        glm::vec3 moveOffset(0.f);

        if (window.isKeyPressed(q3d::key::SPACE)) {
            auto dir = -glm::vec3(glm::inverse(cam->getView())[2]);
            scene.getSpotLight("spot")->direction = glm::normalize(dir);
            scene.getSpotLight("spot")->position = cam->getPosition();
        }

        if (window.isKeyPressed(q3d::key::W)) moveOffset.z += targetMoveStep;
        if (window.isKeyPressed(q3d::key::S)) moveOffset.z -= targetMoveStep;
        if (window.isKeyPressed(q3d::key::D)) moveOffset.x += targetMoveStep;
        if (window.isKeyPressed(q3d::key::A)) moveOffset.x -= targetMoveStep;
        if (window.isKeyPressed(q3d::key::E)) moveOffset.y += targetMoveStep;
        if (window.isKeyPressed(q3d::key::Q)) moveOffset.y -= targetMoveStep;

        if (window.isKeyPressed(q3d::key::O)) scene["plane2"]->transform.position.y += dt * 5;
        if (window.isKeyPressed(q3d::key::U)) scene["plane2"]->transform.position.y -= dt * 5;
        if (window.isKeyPressed(q3d::key::I)) scene["plane2"]->transform.position.z -= dt * 5;
        if (window.isKeyPressed(q3d::key::K)) scene["plane2"]->transform.position.z += dt * 5;
        if (window.isKeyPressed(q3d::key::J)) scene["plane2"]->transform.position.x -= dt * 5;
        if (window.isKeyPressed(q3d::key::L)) scene["plane2"]->transform.position.x += dt * 5;

        if (window.isKeyPressed(q3d::key::F)) scene["plane2"]->transform.rotation.y += dt * 60;
        if (window.isKeyPressed(q3d::key::H)) scene["plane2"]->transform.rotation.y -= dt * 60;
        if (window.isKeyPressed(q3d::key::R)) scene["plane2"]->transform.rotation.z += dt * 60;
        if (window.isKeyPressed(q3d::key::Y)) scene["plane2"]->transform.rotation.z -= dt * 60;
        if (window.isKeyPressed(q3d::key::T)) scene["plane2"]->transform.rotation.x += dt * 60;
        if (window.isKeyPressed(q3d::key::G)) scene["plane2"]->transform.rotation.x -= dt * 60;

        if (window.isKeyPressed(q3d::key::X)) bias1 += 0.001f * dt;
        if (window.isKeyPressed(q3d::key::Z)) bias1 -= 0.001f * dt;
        if (window.isKeyPressed(q3d::key::V)) bias2 += 0.001f * dt;
        if (window.isKeyPressed(q3d::key::C)) bias2 -= 0.001f * dt;

        if (window.isKeyPressed(q3d::key::UP))    scene.getDirLight("sun")->transform.position.x += 5 * dt;
        if (window.isKeyPressed(q3d::key::DOWN))  scene.getDirLight("sun")->transform.position.x -= 5 * dt;
        if (window.isKeyPressed(q3d::key::RIGHT)) scene.getDirLight("sun")->transform.position.z += 5 * dt;
        if (window.isKeyPressed(q3d::key::LEFT))  scene.getDirLight("sun")->transform.position.z -= 5 * dt;

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
Bias: {}; {}
            )",
            1 / dt, dt,
            cam->getPosition().x, cam->getPosition().y, cam->getPosition().z,
            cam->getRotation().x, cam->getRotation().y, cam->getRotation().z,
            bias1, bias2
        ));

        // GPU

        fbo.bind();

        q3d::gl::clearColor(q3d::core::Color::Gray);
        q3d::gl::clear();

        res->getShader("object")->use();
        res->getShader("object")->uniform("u_bias1", bias1);
        res->getShader("object")->uniform("u_bias2", bias2);
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
