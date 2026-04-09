#include <SFML/Window.hpp>

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cmath>
#include <iostream>
#include <deque>

#include "nbody.h"

const float PI = 3.14159265359f;

// Kompilacja pojedynczego shadera

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    // sprawdź błędy
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader error: " << log << "\n";
    }
    return shader;
}

// Linkowanie programu (vertex + fragment)

GLuint createProgram(const char* vertSrc, const char* fragSrc) {
    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);
    
    glDeleteShader(vert);
    glDeleteShader(frag);
    return prog;
}

// vertex
const char* vertSrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 MVP;
out vec3 fragPos;

void main() {
    gl_Position = MVP * vec4(aPos, 1.0);
    fragPos = aPos;
}
)";

// fragment
const char* fragSrc = R"(
#version 330 core
in vec3 fragPos;
out vec4 FragColor;

void main() {
    FragColor = vec4(fragPos * 0.5 + 0.5, 1.0);
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

// ====================== STRUKTURA TRAIL ======================
struct Trail {
    std::vector<glm::vec3> points;
    size_t start = 0;
    size_t count = 0;

    Trail(size_t maxSize) {
        points.resize(maxSize);
    }

    void add(glm::vec3 p) {
        size_t idx = (start + count) % points.size();
        points[idx] = p;

        if (count < points.size())
            count++;
        else
            start = (start + 1) % points.size();   // nadpisujemy najstarszy
    }

    size_t size() const { return count; }
};

int main() {

    // ====================== USTAWIENIA ======================
    float yaw   = 0.0f;
    float pitch = 0.0f;
    float radius = 5.0f;
    bool mousePressed = false;
    sf::Vector2i lastMousePos;

    sf::Window window(sf::VideoMode({1920, 1080}), "N-body 3D", 
                      sf::Style::Default, sf::State::Windowed,
                      sf::ContextSettings(24, 8, 0, 3, 3));
    window.setFramerateLimit(60);

    gladLoadGL();
    glEnable(GL_DEPTH_TEST);

    GLuint prog = createProgram(vertSrc, fragSrc);

    // ====================== SFERA ======================
    auto verts   = generateSphere(1.0f, 20, 20);
    auto indices = generateSphereIndices(20, 20);

    GLuint sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // ====================== TRAJEKTORIE ======================
    const int MAX_HISTORY = 2000;

    std::vector<Cialo> ciala = {
    {1.0, -1.0,  0.0, 0.0,  0.392955,  0.097579, 0.0},
    {1.0,  1.0,  0.0, 0.0,  0.392955,  0.097579, 0.0},
    {1.0,  0.0,  0.0, 0.0, -0.78591,  -0.195158, 0.0}
    };

    std::vector<GLuint> trailVAOs(ciala.size());
    std::vector<GLuint> trailVBOs(ciala.size());

    for (size_t i = 0; i < ciala.size(); ++i) {
        glGenVertexArrays(1, &trailVAOs[i]);
        glGenBuffers(1, &trailVBOs[i]);

        glBindVertexArray(trailVAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, trailVBOs[i]);

        glBufferData(GL_ARRAY_BUFFER, MAX_HISTORY * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }

    std::vector<Trail> historie;
    for (size_t i = 0; i < ciala.size(); ++i) {
        historie.emplace_back(MAX_HISTORY);
    }

    obliczPrzyspieszenie(ciala);

    sf::Clock clock;

    // ====================== GŁÓWNA PĘTLA ======================
    while (window.isOpen()) {
        // --- Eventy ---
        while (auto event = window.pollEvent()) {
            // ... Twój kod eventów (mouse) bez zmian ...
            if (event->is<sf::Event::Closed>()) window.close();
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(prog);

        // Kamera
        glm::vec3 camPos = glm::vec3(
            radius * cos(pitch) * sin(yaw),
            radius * sin(pitch),
            radius * cos(pitch) * cos(yaw)
        );
        glm::mat4 view = glm::lookAt(camPos, glm::vec3(0,0,0), glm::vec3(0,1,0));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

        GLint mvpLoc = glGetUniformLocation(prog, "MVP");

        // --- Fizyka ---
        double dt = 0.005;
        for (auto& c : ciala) c.kick(dt);
        for (auto& c : ciala) c.drift(dt);
        obliczPrzyspieszenie(ciala);
        for (auto& c : ciala) c.kick(dt);

        // --- Aktualizacja historii ---
        for (size_t i = 0; i < ciala.size(); i++) {
            historie[i].add(glm::vec3(ciala[i].x, ciala[i].y, ciala[i].z));
        }

        // --- Rysowanie ciał ---
        glBindVertexArray(sphereVAO);
        for (auto& c : ciala) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(c.x, c.y, c.z));
            model = glm::scale(model, glm::vec3(0.1f));

            glm::mat4 MVP = proj * view * model;
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(MVP));
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }

        // ==================== Rysowanie trajektorii ====================
        for (size_t i = 0; i < historie.size(); i++) {
            const auto& trail = historie[i];
            if (trail.size() < 2) continue;
        
            std::vector<float> punkty;
            punkty.reserve(trail.size() * 3);
        
            for (size_t j = 0; j < trail.size(); ++j) {
                glm::vec3 p = trail.points[(trail.start + j) % trail.points.size()];
                punkty.push_back(p.x);
                punkty.push_back(p.y);
                punkty.push_back(p.z);
            }
        
            glBindVertexArray(trailVAOs[i]);
            glBindBuffer(GL_ARRAY_BUFFER, trailVBOs[i]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, punkty.size() * sizeof(float), punkty.data());
        
            glm::mat4 MVP = proj * view * glm::mat4(1.0f);
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(MVP));
        
            glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)trail.size());
        }

        glBindVertexArray(0);
        window.display();
    }

    // ====================== SPRZĄTANIE ======================
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);

    for (size_t i = 0; i < trailVAOs.size(); i++) {
        glDeleteVertexArrays(1, &trailVAOs[i]);
        glDeleteBuffers(1, &trailVBOs[i]);
    }

    return 0;
}