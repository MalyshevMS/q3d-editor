#include <glad/glad.h>
#include "app.hpp"
#include "config.txx"
#include <format>
#include <q3d/q3d.hpp>
#include <q3d/log/log.hpp>

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
    res->loadShader("post", "res/post.vert", "res/post.frag");
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


    auto targetPos = cam->getPosition();
    auto targetRot = cam->getRotation();

    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    unsigned int fboTex;
    glGenTextures(1, &fboTex);
    glBindTexture(GL_TEXTURE_2D, fboTex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window.getFBSize().x, window.getFBSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);

    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window.getFBSize().x, window.getFBSize().y);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        q3d::log::error("Something went wrong...");
    }

    const float vert[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert), &vert, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

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

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        q3d::gl::clearColor(q3d::core::Color::Gray);
        q3d::gl::clear();

        scene.render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        q3d::gl::disable(q3d::gl::feature::depthTest);

        glClear(GL_COLOR_BUFFER_BIT);

        res->getShader("post")->use();

        glBindVertexArray(vao);
        glBindTexture(GL_TEXTURE_2D, fboTex);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        canvas.render();

        window.update();
    }
}

Application::~Application() {
    q3d::Window::terminate();
}
