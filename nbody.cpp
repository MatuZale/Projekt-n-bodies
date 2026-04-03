#include <iostream>
#include <concepts>
#include <vector>
#include <cmath>
#include <fstream>

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

void obliczPrzyspieszenie(std::vector<Cialo>& ps) {
    // softening
    double eps2 = 0.01;

    for (std::size_t i = 0; i < ps.size(); ++i) {
        for (std::size_t j = i + 1; j < ps.size(); ++j) {
            double dx = ps[j].x - ps[i].x;
            double dy = ps[j].y - ps[i].y;

            double r2 = dx*dx + dy*dy + eps2;
            double r = std::sqrt(r2);

            double factor_i = G * ps[j].m / (r * r2);
            double factor_j = G * ps[i].m / (r * r2);

            ps[i].ax += factor_i * dx;
            ps[i].ay += factor_i * dy;

            ps[j].ax -= factor_j * dx;
            ps[j].ay -= factor_j * dy;
        }
    }
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
    const int steps = 500;


    std::vector<Cialo> ps;
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
    for (int step = 0; step < steps; ++step) {
        double t = step * dt;

        obliczPrzyspieszenie(ps);

        for (auto& p : ps) {
            aktualizuj(p, dt);
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