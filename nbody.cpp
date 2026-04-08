#include <vector>
#include <cmath>
#include "nbody.h"

void Cialo::kick(double dt) {
    vx += ax * dt * 0.5;
    vy += ay * dt * 0.5;
    vz += az * dt * 0.5;
}

void Cialo::drift(double dt) {
        x += vx * dt;
        y += vy * dt;
        z += vz * dt;
}


void obliczPrzyspieszenie(std::vector<Cialo>& ps) {
    double eps2 = 0.001;

    for (auto& p : ps) {
        p.ax = 0.0;
        p.ay = 0.0;
        p.az = 0.0;
    }

    for (std::size_t i = 0; i < ps.size(); ++i) {
        for (std::size_t j = i + 1; j < ps.size(); ++j) {
            double dx = ps[j].x - ps[i].x;
            double dy = ps[j].y - ps[i].y;
            double dz = ps[j].z - ps[i].z;

            double r2 = dx*dx + dy*dy + dz*dz + eps2;
            double r = std::sqrt(r2);

            double factor = G / (r * r2);

            double fx = factor * dx;
            double fy = factor * dy;
            double fz = factor * dz;

            ps[i].ax += fx * ps[j].m;
            ps[i].ay += fy * ps[j].m;
            ps[i].az += fz * ps[j].m;

            ps[j].ax -= fx * ps[i].m;
            ps[j].ay -= fy * ps[i].m;
            ps[j].az -= fz * ps[i].m;
        }
    }
}