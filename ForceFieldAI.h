#pragma once
#include "ForceField.h"
#include <vector>

class ForceFieldAI
{
private:
    std::vector<VectorND> sources;
    ForceField field;

public:
    ForceFieldAI(double strength = 1.0) : field(strength) {}
    void addSource(const VectorND& source) { sources.push_back(source); }
    void clearSources() { sources.clear(); }
    void setStrength(double strength) { field.setStrength(strength); }

    VectorND computeNetForce(const VectorND& position) const
    {
        if (sources.empty()) return VectorND(position.dimension());
        VectorND netForce(position.dimension());
        for (const auto& src : sources) netForce = netForce + field.computeForce(position, src);
        return netForce;
    }
};