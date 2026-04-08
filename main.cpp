#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glad/glad.h"

const char* vertSrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(aPos, 1.0);
}
)";

const char* fragSrc = R"(
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";

std::vector<float> generateSphere(float radius, int stacks, int slices) {
    std::vector<float> verts;
    for (int i = 0; i <= stacks; i++) {
        float phi = M_PI * i / stacks;  // 0 -> PI
        for (int j = 0; j <= slices; j++) {
            float theta = 2.0f * M_PI * j / slices;  // 0 -> 2PI
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);
            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(z);
        }
    }
    return verts;
}

std::vector<unsigned int> generateSphereIndices(int stacks, int slices) {
    std::vector<unsigned int> indices;
    for (int i = 0; i < stacks; i++) {
        for (int j = 0; j < slices; j++) {
            unsigned int a = i * (slices + 1) + j;
            unsigned int b = a + 1;
            unsigned int c = a + (slices + 1);
            unsigned int d = c + 1;
            // dwa trójkąty na jeden "kwadrat" siatki
            indices.push_back(a);
            indices.push_back(c);
            indices.push_back(b);
            indices.push_back(b);
            indices.push_back(c);
            indices.push_back(d);
        }
    }
    return indices;
}

int main() {
    sf::Window window(
        sf::VideoMode({800, 600}),
        "N-body 3D",
        sf::Style::Default,
        sf::State::Windowed,
        sf::ContextSettings(24, 8, 0, 3, 3)  // depth, stencil, AA, GL 3.3
    );
    window.setFramerateLimit(60);

    gladLoadGL();  // <-- tego jeszcze nie mamy, zaraz wyjaśnię

    // ...
}