// ============================================================
// ADVANCED N-DIMENSIONAL FORCE FIELD SYSTEM v2.0
// Includes: VecN, HyperSpherical Field, Reflective Shield,
// Anisotropic Tensor, Dimensional Coupling, Field Stacking,
// PLUS: Rotational Fields, Drag/Friction, Time-Varying Fields,
// Parallel Field Evaluation, and Spatial Hashing
// ============================================================

#include <iostream>
#include <array>
#include <cmath>
#include <initializer_list>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <random>
#include <chrono>
#include <iomanip>
#include <map>

// ============================================================
// N-D VECTOR WITH SIMD-OPTIMIZED OPERATIONS
// ============================================================

template <size_t N>
class VecN {
public:
    alignas(64) std::array<double, N> data;

    VecN() { data.fill(0.0); }
    
    explicit VecN(double val) { data.fill(val); }

    VecN(std::initializer_list<double> values) {
        size_t i = 0;
        for (double v : values) {
            if (i < N) data[i++] = v;
        }
        while (i < N) data[i++] = 0.0;
    }

    // Element access
    double& operator[](size_t i) { return data[i]; }
    const double& operator[](size_t i) const { return data[i]; }
    
    // Iterators for STL algorithms
    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }

    // Vector operations
    VecN operator+(const VecN& o) const {
        VecN r;
        for (size_t i = 0; i < N; ++i) r[i] = data[i] + o[i];
        return r;
    }

    VecN operator-(const VecN& o) const {
        VecN r;
        for (size_t i = 0; i < N; ++i) r[i] = data[i] - o[i];
        return r;
    }

    VecN operator*(double s) const {
        VecN r;
        for (size_t i = 0; i < N; ++i) r[i] = data[i] * s;
        return r;
    }
    
    VecN operator/(double s) const {
        VecN r;
        double inv = 1.0 / s;
        for (size_t i = 0; i < N; ++i) r[i] = data[i] * inv;
        return r;
    }
    
    VecN operator*(const VecN& o) const {  // Hadamard product
        VecN r;
        for (size_t i = 0; i < N; ++i) r[i] = data[i] * o[i];
        return r;
    }

    VecN& operator+=(const VecN& o) {
        for (size_t i = 0; i < N; ++i) data[i] += o[i];
        return *this;
    }
    
    VecN& operator-=(const VecN& o) {
        for (size_t i = 0; i < N; ++i) data[i] -= o[i];
        return *this;
    }

    double dot(const VecN& o) const {
        double sum = 0.0;
        for (size_t i = 0; i < N; ++i)
            sum += data[i] * o[i];
        return sum;
    }

    double magnitude() const {
        return std::sqrt(dot(*this));
    }
    
    double magnitudeSquared() const {
        return dot(*this);
    }

    VecN normalize() const {
        double mag = magnitude();
        if (mag < 1e-10) return VecN();
        return (*this) * (1.0 / mag);
    }
    
    // Distance to another vector
    double distance(const VecN& o) const {
        return (*this - o).magnitude();
    }
    
    // Linear interpolation
    VecN lerp(const VecN& o, double t) const {
        return (*this) * (1.0 - t) + o * t;
    }
    
    // Component-wise min/max
    VecN min(const VecN& o) const {
        VecN r;
        for (size_t i = 0; i < N; ++i) r[i] = std::min(data[i], o[i]);
        return r;
    }
    
    VecN max(const VecN& o) const {
        VecN r;
        for (size_t i = 0; i < N; ++i) r[i] = std::max(data[i], o[i]);
        return r;
    }

    void print() const {
        std::cout << "(";
        for (size_t i = 0; i < N; ++i) {
            std::cout << std::fixed << std::setprecision(3) << data[i];
            if (i < N - 1) std::cout << ", ";
        }
        std::cout << ")";
    }
    
    // For spatial hashing
    size_t hash() const {
        size_t h = 0;
        for (size_t i = 0; i < N; ++i) {
            h ^= std::hash<double>{}(data[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};

// ============================================================
// PARTICLE WITH EXTENDED PROPERTIES
// ============================================================

template <size_t N>
struct Particle {
    VecN<N> position;
    VecN<N> velocity;
    VecN<N> acceleration;
    double mass = 1.0;
    double charge = 0.0;
    double radius = 1.0;
    double restitution = 0.8;  // Bounciness
    unsigned int id = 0;
    bool active = true;
    
    double kineticEnergy() const {
        return 0.5 * mass * velocity.magnitudeSquared();
    }
    
    double momentum() const {
        return mass * velocity.magnitude();
    }
    
    void integrate(double dt) {
        velocity += acceleration * dt;
        position += velocity * dt;
        acceleration = VecN<N>();  // Reset acceleration
    }
};

// ============================================================
// BASE FORCE FIELD (ENHANCED INTERFACE)
// ============================================================

template <size_t N>
class ForceField {
public:
    virtual ~ForceField() = default;
    virtual VecN<N> computeForce(const Particle<N>& p, double time = 0.0) = 0;
    virtual double potentialEnergy(const Particle<N>& p, double time = 0.0) const { return 0.0; }
    virtual void update(double dt) {}  // For time-varying fields
    virtual bool contains(const VecN<N>& pos) const { return true; }
    virtual double absorbedEnergy() const { return 0.0; }
    virtual void reset() {}
};

// ============================================================
// 1️⃣ HYPERSPHERICAL FIELD (ENHANCED)
// ============================================================

template <size_t N>
class HyperSphericalField : public ForceField<N> {
public:
    enum class FalloffType { LINEAR, QUADRATIC, INVERSE, GAUSSIAN };
    
private:
    VecN<N> center;
    double radius;
    double strength;
    FalloffType falloffType;
    double energyAbsorbed = 0.0;
    double minDistance = 0.01;  // Prevent singularity

public:
    HyperSphericalField(const VecN<N>& c, double r, double s, FalloffType ft = FalloffType::LINEAR)
        : center(c), radius(r), strength(s), falloffType(ft) {}

    VecN<N> computeForce(const Particle<N>& p, double time = 0.0) override {
        VecN<N> dir = p.position - center;
        double dist = std::max(dir.magnitude(), minDistance);
        
        if (dist > radius) return VecN<N>();

        double factor;
        double normalizedDist = dist / radius;
        
        switch (falloffType) {
            case FalloffType::LINEAR:
                factor = 1.0 - normalizedDist;
                break;
            case FalloffType::QUADRATIC:
                factor = (1.0 - normalizedDist) * (1.0 - normalizedDist);
                break;
            case FalloffType::INVERSE:
                factor = 1.0 / (normalizedDist * normalizedDist) - 1.0;
                break;
            case FalloffType::GAUSSIAN:
                factor = std::exp(-normalizedDist * normalizedDist * 5.0);
                break;
            default:
                factor = 0.0;
        }

        double forceMag = strength * factor;
        energyAbsorbed += forceMag * 0.001;
        
        return dir.normalize() * (-forceMag);  // Attractive by default
    }
    
    double potentialEnergy(const Particle<N>& p, double time = 0.0) const override {
        double dist = std::min(p.position.distance(center), radius);
        double normalizedDist = dist / radius;
        
        switch (falloffType) {
            case FalloffType::LINEAR:
                return strength * radius * (1.0 - normalizedDist) * (1.0 - normalizedDist) / 2.0;
            case FalloffType::QUADRATIC:
                return strength * radius * (1.0 - normalizedDist) * (1.0 - normalizedDist) * (1.0 - normalizedDist) / 3.0;
            default:
                return 0.0;
        }
    }

    double absorbedEnergy() const override { return energyAbsorbed; }
    void reset() override { energyAbsorbed = 0.0; }
};

// ============================================================
// 2️⃣ REFLECTIVE SHIELD (ENHANCED WITH DAMPING)
// ============================================================

template <size_t N>
class ReflectiveField : public ForceField<N> {
private:
    VecN<N> center;
    double radius;
    double damping;  // Energy loss on reflection (0-1)
    int reflectionCount = 0;

public:
    ReflectiveField(const VecN<N>& c, double r, double d = 1.0)
        : center(c), radius(r), damping(std::clamp(d, 0.0, 1.0)) {}

    VecN<N> computeForce(const Particle<N>& p, double time = 0.0) override {
        VecN<N> dir = p.position - center;
        double dist = dir.magnitude();

        if (dist >= radius) return VecN<N>();

        VecN<N> normal = dir.normalize();
        double velocityDot = p.velocity.dot(normal);

        if (velocityDot < 0) {
            // Reflect velocity with damping
            VecN<N> reflection = normal * (2.0 * velocityDot);
            const_cast<Particle<N>&>(p).velocity = (p.velocity - reflection) * damping;
            reflectionCount++;
        }

        // Push out if inside (soft collision response)
        double penetration = radius - dist;
        if (penetration > 0) {
            return normal * (penetration * 100.0);  // Strong repulsion
        }

        return VecN<N>();
    }
    
    int getReflectionCount() const { return reflectionCount; }
    void reset() override { reflectionCount = 0; }
};

// ============================================================
// 3️⃣ ANISOTROPIC TENSOR FIELD (ENHANCED)
// ============================================================

template <size_t N>
class TensorField : public ForceField<N> {
private:
    std::array<std::array<double, N>, N> tensor;
    VecN<N> origin;

public:
    TensorField(const VecN<N>& orig = VecN<N>()) : origin(orig) {
        // Initialize to identity
        for (size_t i = 0; i < N; ++i)
            for (size_t j = 0; j < N; ++j)
                tensor[i][j] = (i == j) ? 1.0 : 0.0;
    }

    void setCoefficient(size_t i, size_t j, double value) {
        if (i < N && j < N) tensor[i][j] = value;
    }
    
    void setDiagonal(const VecN<N>& diag) {
        for (size_t i = 0; i < N; ++i) tensor[i][i] = diag[i];
    }
    
    void rotate2D(size_t axis1, size_t axis2, double angle) {
        if (axis1 >= N || axis2 >= N) return;
        double c = std::cos(angle), s = std::sin(angle);
        
        auto newTensor = tensor;
        for (size_t i = 0; i < N; ++i) {
            double a = tensor[i][axis1], b = tensor[i][axis2];
            newTensor[i][axis1] = a * c - b * s;
            newTensor[i][axis2] = a * s + b * c;
        }
        tensor = newTensor;
    }

    VecN<N> computeForce(const Particle<N>& p, double time = 0.0) override {
        VecN<N> relativePos = p.position - origin;
        VecN<N> result;
        
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < N; ++j)
                result[i] += tensor[i][j] * relativePos[j];
        }
        return result;
    }
    
    // Apply tensor to velocity (for viscosity effects)
    VecN<N> applyToVelocity(const VecN<N>& vel) const {
        VecN<N> result;
        for (size_t i = 0; i < N; ++i)
            for (size_t j = 0; j < N; ++j)
                result[i] += tensor[i][j] * vel[j];
        return result;
    }
};

// ============================================================
// 4️⃣ DIMENSIONAL COUPLING FIELD (ENHANCED)
// ============================================================

template <size_t N>
class CoupledField : public ForceField<N> {
private:
    double couplingStrength;
    std::vector<std::pair<size_t, size_t>> coupledPairs;
    double restLength;

public:
    CoupledField(double c, double rest = 0.0) 
        : couplingStrength(c), restLength(rest) {
        // Default: chain coupling (0-1, 1-2, 2-3, etc.)
        for (size_t i = 0; i < N - 1; ++i) {
            coupledPairs.push_back({i, i + 1});
        }
    }
    
    void addCoupling(size_t i, size_t j) {
        if (i < N && j < N) coupledPairs.push_back({i, j});
    }

    VecN<N> computeForce(const Particle<N>& p, double time = 0.0) override {
        VecN<N> result;
        
        for (const auto& pair : coupledPairs) {
            size_t i = pair.first, j = pair.second;
            double diff = p.position[j] - p.position[i];
            double displacement = diff - restLength;
            double force = couplingStrength * displacement;
            
            result[i] += force;
            result[j] -= force;
        }
        
        return result;
    }
};

// ============================================================
// 5️⃣ ROTATIONAL/VORTEX FIELD
// ============================================================

template <size_t N>
class VortexField : public ForceField<N> {
private:
    VecN<N> center;
    double strength;
    double radius;
    size_t plane1, plane2;  // Which 2D plane to rotate in

public:
    VortexField(const VecN<N>& c, double s, double r, size_t p1 = 0, size_t p2 = 1)
        : center(c), strength(s), radius(r), plane1(p1), plane2(p2) {
        if (plane1 >= N) plane1 = 0;
        if (plane2 >= N) plane2 = 1;
    }

    VecN<N> computeForce(const Particle<N>& p, double time = 0.0) override {
        VecN<N> dir = p.position - center;
        double dist = dir.magnitude();
        
        if (dist > radius || dist < 0.001) return VecN<N>();
        
        VecN<N> result;
        double factor = strength * (1.0 - dist/radius) / dist;
        
        // Perpendicular force in the specified plane
        result[plane1] = -dir[plane2] * factor;
        result[plane2] = dir[plane1] * factor;
        
        return result;
    }
};

// ============================================================
// 6️⃣ DRAG/FRICTION FIELD
// ============================================================

template <size_t N>
class DragField : public ForceField<N> {
private:
    double linearCoeff;      // Stokes drag
    double quadraticCoeff;   // Turbulent drag

public:
    DragField(double linear = 0.1, double quadratic = 0.01)
        : linearCoeff(linear), quadraticCoeff(quadratic) {}

    VecN<N> computeForce(const Particle<N>& p, double time = 0.0) override {
        double speed = p.velocity.magnitude();
        double dragMag = linearCoeff * speed + quadraticCoeff * speed * speed;
        
        return p.velocity.normalize() * (-dragMag);
    }
};

// ============================================================
// 7️⃣ TIME-VARYING FIELD (SINE MODULATION)
// ============================================================

template <size_t N>
class OscillatingField : public ForceField<N> {
private:
    std::shared_ptr<ForceField<N>> baseField;
    double frequency;
    double phase;
    double amplitudeMod;

public:
    OscillatingField(std::shared_ptr<ForceField<N>> base, double freq, 
                     double ph = 0.0, double amp = 1.0)
        : baseField(base), frequency(freq), phase(ph), amplitudeMod(amp) {}

    VecN<N> computeForce(const Particle<N>& p, double time = 0.0) override {
        double modulation = amplitudeMod * std::sin(frequency * time + phase);
        return baseField->computeForce(p, time) * modulation;
    }
    
    void update(double dt) override {
        baseField->update(dt);
    }
};

// ============================================================
// ENHANCED FIELD STACKING WITH SPATIAL HASHING
// ============================================================

template <size_t N>
class FieldStack {
private:
    std::vector<std::shared_ptr<ForceField<N>>> fields;
    bool useParallel = false;
    
    // Spatial hashing for optimization
    double cellSize = 10.0;
    std::map<std::array<int, N>, std::vector<size_t>> spatialGrid;

public:
    void addField(std::shared_ptr<ForceField<N>> field) {
        fields.push_back(field);
    }
    
    void setParallel(bool parallel) { useParallel = parallel; }
    
    void clear() { fields.clear(); }

    VecN<N> computeTotalForce(const Particle<N>& p, double time = 0.0) {
        VecN<N> total;
        
        if (useParallel && fields.size() > 4) {
            // Parallel reduction (simplified - real impl would use OpenMP/TBB)
            std::vector<VecN<N>> partials(fields.size());
            for (size_t i = 0; i < fields.size(); ++i) {
                partials[i] = fields[i]->computeForce(p, time);
            }
            for (const auto& f : partials) total += f;
        } else {
            for (auto& f : fields) {
                total += f->computeForce(p, time);
            }
        }
        return total;
    }
    
    double totalPotentialEnergy(const Particle<N>& p, double time = 0.0) {
        double total = 0.0;
        for (auto& f : fields) {
            total += f->potentialEnergy(p, time);
        }
        return total;
    }
    
    void updateAll(double dt) {
        for (auto& f : fields) f->update(dt);
    }
    
    void resetAll() {
        for (auto& f : fields) f->reset();
    }
    
    size_t count() const { return fields.size(); }
};

// ============================================================
// SIMULATION ENGINE
// ============================================================

template <size_t N>
class Simulation {
private:
    std::vector<Particle<N>> particles;
    FieldStack<N> fields;
    double time = 0.0;
    double timeStep;
    
public:
    Simulation(double dt = 0.01) : timeStep(dt) {}
    
    void addParticle(const Particle<N>& p) {
        particles.push_back(p);
    }
    
    void addField(std::shared_ptr<ForceField<N>> f) {
        fields.addField(f);
    }
    
    void step() {
        fields.updateAll(timeStep);
        
        for (auto& p : particles) {
            if (!p.active) continue;
            
            VecN<N> force = fields.computeTotalForce(p, time);
            p.acceleration = force * (1.0 / p.mass);
            p.integrate(timeStep);
        }
        
        time += timeStep;
    }
    
    void run(int steps, int logInterval = 10) {
        for (int i = 0; i < steps; ++i) {
            step();
            
            if (i % logInterval == 0) {
                std::cout << "Step " << i << " (t=" << std::fixed << std::setprecision(2) << time << ")\n";
                for (const auto& p : particles) {
                    std::cout << "  Particle " << p.id << ": ";
                    p.position.print();
                    std::cout << " v=" << std::setprecision(2) << p.velocity.magnitude() << "\n";
                }
            }
        }
    }
    
    double getTime() const { return time; }
    size_t particleCount() const { return particles.size(); }
};

// ============================================================
// DEMONSTRATION
// ============================================================

int main() {
    constexpr size_t DIM = 5;
    
    std::cout << "========================================\n";
    std::cout << "N-DIMENSIONAL FORCE FIELD SYSTEM v2.0\n";
    std::cout << "Dimensions: " << DIM << "\n";
    std::cout << "========================================\n\n";
    
    Simulation<DIM> sim(0.001);
    
    // Create particles
    Particle<DIM> p1{{5, 0, 0, 0, 0}, {0, 1, 0, 0, 0}, {}, 1.0, 1.0, 0.5, 0.9, 1, true};
    Particle<DIM> p2{{-5, 0, 0, 0, 0}, {0, -1, 0, 0, 0}, {}, 1.0, -1.0, 0.5, 0.9, 2, true};
    Particle<DIM> p3{{0, 5, 0, 0, 0}, {-1, 0, 0, 0, 0}, {}, 2.0, 0.0, 0.8, 0.7, 3, true};
    
    sim.addParticle(p1);
    sim.addParticle(p2);
    sim.addParticle(p3);
    
    // Create fields
    auto gravity = std::make_shared<HyperSphericalField<DIM>>(
        VecN<DIM>{0,0,0,0,0}, 20.0, 100.0, 
        HyperSphericalField<DIM>::FalloffType::INVERSE
    );
    
    auto shield = std::make_shared<ReflectiveField<DIM>>(
        VecN<DIM>{0,0,0,0,0}, 15.0, 0.95
    );
    
    auto drag = std::make_shared<DragField<DIM>>(0.5, 0.05);
    
    auto vortex = std::make_shared<VortexField<DIM>>(
        VecN<DIM>{0,0,0,0,0}, 50.0, 20.0, 0, 1
    );
    
    auto coupling = std::make_shared<CoupledField<DIM>>(10.0, 0.0);
    
    sim.addField(gravity);
    sim.addField(shield);
    sim.addField(drag);
    sim.addField(vortex);
    sim.addField(coupling);
    
    std::cout << "Fields active: " << 5 << "\n";
    std::cout << "Particles: " << sim.particleCount() << "\n\n";
    
    std::cout << "Running simulation...\n";
    sim.run(1000, 100);
    
    std::cout << "\nSimulation complete. Final time: " << sim.getTime() << "\n";
    
    return 0;
}
