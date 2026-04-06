#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>

constexpr double G = 1.0;

struct Cialo {
    double m;
    double x, y;
    double vx, vy;
    double ax, ay;

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

int main() {

    double dt = 0.01;
    const int steps = 5000;

    std::vector<Cialo> ps;
    // stabilna orbita dla 3 cial 2000
    ps.push_back({1.0,  0.97000436, -0.24308753,  0.93240737/2,  0.86473146/2, 0.0, 0.0});
    ps.push_back({1.0, -0.97000436,  0.24308753,  0.93240737/2,  0.86473146/2, 0.0, 0.0});
    ps.push_back({1.0,  0.0,         0.0,         -0.93240737,  -0.86473146,   0.0, 0.0});  

    // zapis do pliku
    std::ofstream file("nbody_2d.csv");
    file << "t";
    for (std::size_t i = 0; i < ps.size(); ++i) {
    file << ",x" << i << ",y" << i << ",px" << i << ",py" << i;
    }
    file << "\n";
    obliczPrzyspieszenie(ps);
    for (int step = 0; step < steps; ++step) {
        double t = step * dt;

        for (auto& p : ps) {
            p.kick(dt);
        }

        for (auto& p : ps) {
            p.drift(dt);
        }

        obliczPrzyspieszenie(ps);

        for (auto& p : ps) {
            p.kick(dt);
        }

        // zapis do pliku
        file << t;
        for (std::size_t i = 0; i < ps.size(); ++i) {
            file << "," << ps[i].x << "," << ps[i].y << "," << ps[i].m * ps[i].vx << "," << ps[i].m * ps[i].vy;
        }
    file << "\n";
}
    std::cout << "Saved: nbody_2d.csv\n";
    file.close();
    return 0;
}