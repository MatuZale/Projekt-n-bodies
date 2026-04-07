#include <SFML/Graphics.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <deque>

constexpr double G = 1.0;
const std::size_t MAX_HISTORY = 200;

struct Cialo {
    double m;
    double x, y;
    double vx, vy;
    double ax, ay;

    std::deque<std::pair<double, double>> historia;

    sf::Color color;

    void kick(double dt) {
        vx += ax * dt * 0.5;
        vy += ay * dt * 0.5;
    }

    void drift(double dt) {
        x += vx * dt;
        y += vy * dt;
    }
};

void obliczPrzyspieszenie(std::vector<Cialo>& ps) {
    double eps2 = 0.001;

    for (auto& p : ps) {
        p.ax = 0.0;
        p.ay = 0.0;
    }

    for (std::size_t i = 0; i < ps.size(); ++i) {
        for (std::size_t j = i + 1; j < ps.size(); ++j) {
            double dx = ps[j].x - ps[i].x;
            double dy = ps[j].y - ps[i].y;

            double r2 = dx*dx + dy*dy + eps2;
            double r = std::sqrt(r2);

            double factor = G / (r * r2);

            double fx = factor * dx;
            double fy = factor * dy;

            ps[i].ax += fx * ps[j].m;
            ps[i].ay += fy * ps[j].m;

            ps[j].ax -= fx * ps[i].m;
            ps[j].ay -= fy * ps[i].m;
        }
    }
}

void rysuj(const std::vector<Cialo>& ciala, sf::RenderWindow& window) {
    const float scale = 200.0f;
    const float centerX = 400.0f;
    const float centerY = 300.0f;

    for (const auto& c : ciala) {

        // Trajektoria
        if (c.historia.size() > 1) {
            sf::VertexArray trail(sf::PrimitiveType::LineStrip, c.historia.size());

            for (std::size_t i = 0; i < c.historia.size(); ++i) {
                float x = c.historia[i].first * scale + centerX;
                float y = centerY - c.historia[i].second * scale;

                trail[i].position = {x, y};
                trail[i].color = c.color;
            }

            window.draw(trail);
        }

        float radius = 2.0f + std::sqrt(c.m) * 3.0f;

        sf::CircleShape shape;
        shape.setRadius(radius);
        shape.setFillColor(c.color);
        shape.setOrigin({radius, radius});

        float screenX = static_cast<float>(c.x * scale + centerX);
        float screenY = static_cast<float>(centerY - c.y * scale);

        shape.setPosition({screenX, screenY});
        window.draw(shape);
    }
}

int main() {

    std::vector<Cialo> ciala = {
        {1.0,  0.9700, -0.2430,  0.9324/2,  0.8647/2, 0,0, {}, sf::Color::Red},
        {1.0, -0.9700,  0.2430,  0.9324/2,  0.8647/2, 0,0, {}, sf::Color::Green},
        {1.0,  0.0,     0.0,    -0.9324,   -0.8647,   0,0, {}, sf::Color::Blue}
    };
    obliczPrzyspieszenie(ciala);

    sf::RenderWindow window(sf::VideoMode({800, 600}), "N-body animation");

    float dt = 0.01f;
    float accumulator = 0.0f;
    sf::Clock clock;

    while (window.isOpen()) {

        // 1. Eventy
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        /*
        // informacjao połozeniu na ekranie
        for (const auto& c : ciala) {
            printf("x=%.2f y=%.2f -> screenX=%.2f screenY=%.2f\n",
            c.x, c.y,
            (float)(c.x * 200.0 + 400.0),
            (float)(300.0 - c.y * 200.0));
            }
        */

        // 2. Czas
        float frameTime = clock.restart().asSeconds();
        accumulator += frameTime;

        // 3. Symulacja
        while (accumulator >= dt) {

            // 1. pół kroku prędkości
            for (auto& c : ciala)
                c.kick(dt);

            // 2. pełny krok pozycji
            for (auto& c : ciala)
                c.drift(dt);

            for (auto& c : ciala) {
                c.historia.emplace_back(c.x, c.y);

            if (c.historia.size() > MAX_HISTORY)
                c.historia.pop_front();
            }

            // 3. nowe przyspieszenia
            obliczPrzyspieszenie(ciala);

            // 4. drugie pół kroku prędkości
            for (auto& c : ciala)
                c.kick(dt);

            accumulator -= dt;
        }

        // 4. Render
        window.clear();
        rysuj(ciala, window);
        window.display();
    }
}