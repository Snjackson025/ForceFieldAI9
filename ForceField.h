#pragma once
#include "VectorND.h"

class ForceField
{
private:
    double strength;

public:
    explicit ForceField(double s = 1.0) : strength(s) {}
    void setStrength(double s) { strength = s; }
    double getStrength() const { return strength; }

    VectorND computeForce(const VectorND& position, const VectorND& source) const
    {
        VectorND direction = source - position;
        double distance = direction.magnitude();
        if (distance == 0.0) return VectorND(position.dimension());
        double forceMagnitude = strength / (distance * distance);
        return direction.normalized() * forceMagnitude;
    }
};