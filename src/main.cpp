#include <q3d/window/window.hpp>

int main(int argc, char const *argv[]) {
    q3d::Window window {"q3d editor", {800, 600}};

    while (window.isOpen()) {
        window.update();
    }

    q3d::Window::terminate();
    return 0;
}
