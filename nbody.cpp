#include <iostream>
#include <concepts>
#include <vector>
#include <cmath>
constexpr double G = 1.0;

struct Cialo {
    double m;
    double x;
    double y;
    double vx;
    double vy;
    double ax;
    double ay;
};

void obliczPrzyspieszenie(Cialo& a, const Cialo& b) {
    double dx = b.x - a.x;
    double dy = b.y - a.y;

    double r2 = dx*dx + dy*dy;
    double r = std::sqrt(r2);

    if (r == 0.0) return;

    double factor = G * b.m / (r * r2);

    a.ax += factor * dx;
    a.ay += factor * dy;
}

void aktualizuj(Cialo& c, double dt) {
    c.vx += c.ax * dt;
    c.vy += c.ay * dt;

    c.x += c.vx * dt;
    c.y += c.vy * dt;

    c.ax = 0;
    c.ay = 0;
}

int main() {

    double dt = 0.1;

    Cialo a{5.0, 0.0, 0.0};
    Cialo b{10.0, 1.0, 0.0};


    for (int step = 0; step < 5; ++step) {
        obliczPrzyspieszenie(a, b);
        obliczPrzyspieszenie(b, a);

        aktualizuj(a, dt);
        aktualizuj(b, dt);

        std::cout << "Krok " << step << "\n";
        std::cout << "a: (" << a.x << ", " << a.y << ")\n";
        std::cout << "b: (" << b.x << ", " << b.y << ")\n";
        std::cout << "------------------\n";
    }

    return 0;
}