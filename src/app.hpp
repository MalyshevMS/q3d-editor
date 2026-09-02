#pragma once

#include <q3d/window/window.hpp>
#include <q3d/core/camera.hpp>
#include <q3d/res/resources.hpp>
#include <q3d/gl/shader.hpp>

class Application {
private:
    q3d::Window window;
    q3d::ptr<q3d::core::Camera> cam;
    q3d::ResourceManager res;
public:
    Application();

    void run();
    ~Application();
};
