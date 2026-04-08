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

int main() {

    // myszka
    float yaw   = 0.0f;
    float pitch = 0.0f;
    float radius = 5.0f;
    bool mousePressed = false;
    sf::Vector2i lastMousePos;

    // okno
    sf::Window window(
        sf::VideoMode({800, 600}),
        "N-body 3D",
        sf::Style::Default,
        sf::State::Windowed,
        sf::ContextSettings(24, 8, 0, 3, 3)
    );
    window.setFramerateLimit(60);

    gladLoadGL();

    glEnable(GL_DEPTH_TEST);

    GLuint prog = createProgram(vertSrc, fragSrc);

    auto verts   = generateSphere(1.0f, 20, 20);
    auto indices = generateSphereIndices(20, 20);

    // VAO i VBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    sf::Clock clock;

    std::vector<Cialo> ciala = {
    {1.0,  1.0,  0.0,  0.5,   0.0,  0.7, -0.3,  0,0,0},
    {1.0, -1.0,  0.0, -0.5,   0.0, -0.7,  0.3,  0,0,0},
    {1.0,  0.0,  0.0,  0.0,   0.0,  0.0,  0.0,  0,0,0}
    };

    obliczPrzyspieszenie(ciala);

    // trajektoria
    const int MAX_HISTORY = 1000;
    std::vector<std::deque<glm::vec3>> historie(ciala.size());

    // główna pętla
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {

            if (auto* e = event->getIf<sf::Event::MouseButtonPressed>()) {
               if (e->button == sf::Mouse::Button::Left) {
                   mousePressed = true;
                   lastMousePos = sf::Mouse::getPosition(window);
               }
           }
           if (auto* e = event->getIf<sf::Event::MouseButtonReleased>()) {
               if (e->button == sf::Mouse::Button::Left)
                   mousePressed = false;
           }
           if (auto* e = event->getIf<sf::Event::MouseMoved>()) {
               if (mousePressed) {
                   sf::Vector2i current = sf::Mouse::getPosition(window);
                   float dx = (current.x - lastMousePos.x) * 0.005f;
                   float dy = (current.y - lastMousePos.y) * 0.005f;
                   yaw   += dx;
                   pitch += dy;
                   pitch = glm::clamp(pitch, -1.5f, 1.5f);  // limit żeby nie przekręcić
                   lastMousePos = current;
               }
           }
            if (event->is<sf::Event::Closed>()) window.close();
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(prog);

        float time = clock.getElapsedTime().asSeconds();
    
        glm::vec3 camPos = glm::vec3(
            radius * cos(pitch) * sin(yaw),
            radius * sin(pitch),
            radius * cos(pitch) * cos(yaw)
        );
        glm::mat4 view = glm::lookAt(camPos, glm::vec3(0,0,0), glm::vec3(0,1,0));
        glm::mat4 proj  = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);

        GLint mvpLoc = glGetUniformLocation(prog, "MVP");
        glBindVertexArray(VAO);

        // krok fizyki
        double dt = 0.005;
        for (auto& c : ciala) c.kick(dt);
        for (auto& c : ciala) c.drift(dt);
        obliczPrzyspieszenie(ciala);
        for (auto& c : ciala) c.kick(dt);

        // historia trajektorii
        for (std::size_t i = 0; i < ciala.size(); i++) {
            historie[i].push_back(glm::vec3(ciala[i].x, ciala[i].y, ciala[i].z));
            if (historie[i].size() > MAX_HISTORY)
                historie[i].pop_front();
        }

        // rysowanie
        for (auto& c : ciala) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), 
                glm::vec3((float)c.x, (float)c.y, (float)c.z));
            model = glm::scale(model, glm::vec3(0.1f));  // mała sfera
            glm::mat4 MVP = proj * view * model;
        
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(MVP));
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }

        for (auto& historia : historie) {
            if (historia.size() < 2) continue;

            std::vector<float> punkty;
            for (auto& p : historia) {
                punkty.push_back(p.x);
                punkty.push_back(p.y);
                punkty.push_back(p.z);
            }
        
            GLuint trailVAO, trailVBO;
            glGenVertexArrays(1, &trailVAO);
            glGenBuffers(1, &trailVBO);
        
            glBindVertexArray(trailVAO);
            glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
            glBufferData(GL_ARRAY_BUFFER, punkty.size() * sizeof(float), punkty.data(), GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
        
            glm::mat4 MVP = proj * view * glm::mat4(1.0f);
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(MVP));
            glDrawArrays(GL_LINE_STRIP, 0, punkty.size() / 3);
        
            glDeleteVertexArrays(1, &trailVAO);
            glDeleteBuffers(1, &trailVBO);
            glBindVertexArray(0);
        }

        window.display();
    }

    return 0;
}