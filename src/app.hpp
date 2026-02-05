#pragma once

#include <q3d/window/window.hpp>
#include <q3d/system/camera.hpp>
#include <q3d/res/resources.hpp>
#include <q3d/gl/shader.hpp>

class Application {
private:
    q3d::Window window;
    q3d::Camera cam;
    q3d::Resources* res;
public:
    Application(std::string_view argv0);

    void run();
};