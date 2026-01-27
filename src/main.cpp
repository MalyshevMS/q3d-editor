#include <glad/glad.h>
#include <q3d/window/window.hpp>
#include <q3d/res/resources.hpp>
#include <q3d/gl/shader.hpp>

int main(int argc, char const *argv[]) {
    q3d::Window window {"q3d editor", {800, 600}};
    auto res = q3d::Resources::getInstance(argv[0]);

    const GLfloat verticies[] = {
//       X      Y      Z            R      G      B
        0.5f, -0.5f,  0.0f,        1.f,   0.f,   0.f,
       -0.5f, -0.5f,  0.0f,        0.f,   1.f,   0.f,
        0.0f,  0.5f,  0.0f,        0.f,   0.f,   1.f,
    };

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0x0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    auto shader = new q3d::gl::Shader;
    shader->attach(res->readFile("res/main.vert"), GL_VERTEX_SHADER);
    shader->attach(res->readFile("res/main.frag"), GL_FRAGMENT_SHADER);
    shader->link();

    glClearColor(0.f, 0.f, 0.f, 1.f);
    while (window.isOpen()) {
        glClear(GL_COLOR_BUFFER_BIT);
        shader->use();

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        shader->unuse();
        window.update();
    }
    
    delete shader;
    q3d::Window::terminate();
    return 0;
}
