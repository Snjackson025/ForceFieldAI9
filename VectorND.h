#pragma once
#include <vector>
#include <cmath>
#include <stdexcept>

class VectorND
{
private:
    std::vector<double> m_data;

public:
    VectorND() {}
    explicit VectorND(size_t dimension) : m_data(dimension, 0.0) {}
    VectorND(const std::initializer_list<double>& values) : m_data(values) {}

    size_t dimension() const { return m_data.size(); }
    double& operator[](size_t index) { return m_data.at(index); }
    double operator[](size_t index) const { return m_data.at(index); }

    VectorND operator+(const VectorND& other) const
    {
        if (dimension() != other.dimension()) throw std::runtime_error("Dimension mismatch");
        VectorND result(dimension());
        for (size_t i = 0; i < dimension(); ++i) result[i] = m_data[i] + other[i];
        return result;
    }

    VectorND operator-(const VectorND& other) const
    {
        if (dimension() != other.dimension()) throw std::runtime_error("Dimension mismatch");
        VectorND result(dimension());
        for (size_t i = 0; i < dimension(); ++i) result[i] = m_data[i] - other[i];
        return result;
    }

    VectorND operator*(double scalar) const
    {
        VectorND result(dimension());
        for (size_t i = 0; i < dimension(); ++i) result[i] = m_data[i] * scalar;
        return result;
    }

    double magnitude() const
    {
        double sum = 0.0;
        for (double v : m_data) sum += v * v;
        return std::sqrt(sum);
    }

    VectorND normalized() const
    {
        double mag = magnitude();
        if (mag == 0.0) throw std::runtime_error("Cannot normalize zero vector");
        return (*this) * (1.0 / mag);
    }
};