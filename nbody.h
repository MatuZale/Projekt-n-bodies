#pragma once
#include <vector>

constexpr double G = 1.0;

struct Cialo {
    double m;
    double x, y, z;
    double vx, vy, vz;
    double ax, ay, az;

    void kick(double dt);
    void drift(double dt);
};

void obliczPrzyspieszenie(std::vector<Cialo>& ps);