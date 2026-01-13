#pragma once

#include <vector>
#include <string>
#include <cstdint>
#define _USE_MATH_DEFINES
#include <cmath>

namespace dynamit::builders
{

class PolarBuilder
{
public:
    PolarBuilder();

    PolarBuilder& formula(const std::wstring& formula);
    PolarBuilder& formula(const std::string& formula);
    PolarBuilder& domain(float start, float end);
    PolarBuilder& domain(float end);
    PolarBuilder& domain_shift(float new_end);
    PolarBuilder& sectors(int sectors);
    PolarBuilder& slices(int slices);
    PolarBuilder& slices_sectors(int slices, int sectors);
    PolarBuilder& sectors_slices(int sectors, int slices);
    PolarBuilder& turbo(bool enabled = true);
    PolarBuilder& smooth(bool enabled = true);
    PolarBuilder& edged(bool enabled = true);
    PolarBuilder& doubleCoated(bool enabled = true);
    PolarBuilder& singleCoated(bool enabled = true);
    PolarBuilder& reversed(bool enabled = true);
    PolarBuilder& nonreversed(bool enabled = true);

    // Cone methods
    PolarBuilder& buildCone(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords);
    PolarBuilder& buildConeIndexed(
        std::vector<float>& verts,
        std::vector<float>& norms,
        std::vector<float>& texCoords,
        std::vector<uint32_t>& indices);
    PolarBuilder& buildConeIndexedInternal(
        std::vector<float>& verts,
        std::vector<float>& norms,
        std::vector<float>& texCoords,
        std::vector<uint32_t>& indices,
        bool isSecondCoat
    );
    PolarBuilder& reBuildCone(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords);
    PolarBuilder& reBuildConeIndexed(
        std::vector<float>& verts,
        std::vector<float>& norms,
        std::vector<float>& texCoords,
        std::vector<uint32_t>& indices);

    // Cylinder methods
    PolarBuilder& buildCylinder(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords);
    PolarBuilder& buildCylinderIndexed(
        std::vector<float>& verts,
        std::vector<float>& norms,
        std::vector<float>& texCoords,
        std::vector<uint32_t>& indices);
    PolarBuilder& reBuildCylinder(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords);
    PolarBuilder& reBuildCylinderIndexed(
        std::vector<float>& verts,
        std::vector<float>& norms,
        std::vector<float>& texCoords,
        std::vector<uint32_t>& indices);

    // Legacy overloads (without texCoords)
    PolarBuilder& buildCone(std::vector<float>& verts, std::vector<float>& norms);
    PolarBuilder& buildConeIndexed(
        std::vector<float>& verts,
        std::vector<float>& norms,
        std::vector<uint32_t>& indices);
    PolarBuilder& reBuildCone(std::vector<float>& verts, std::vector<float>& norms);
    PolarBuilder& reBuildConeIndexed(
        std::vector<float>& verts,
        std::vector<float>& norms,
        std::vector<uint32_t>& indices);

    PolarBuilder& buildCylinder(std::vector<float>& verts, std::vector<float>& norms);
    PolarBuilder& buildCylinderIndexed(
        std::vector<float>& verts,
        std::vector<float>& norms,
        std::vector<uint32_t>& indices);
    PolarBuilder& reBuildCylinder(std::vector<float>& verts, std::vector<float>& norms);
    PolarBuilder& reBuildCylinderIndexed(
        std::vector<float>& verts,
        std::vector<float>& norms,
        std::vector<uint32_t>& indices);

private:
    // Private discrete builders
    PolarBuilder& buildCylinderDiscrete(
        std::vector<float>& verts,
        std::vector<float>& norms,
        std::vector<float>& texCoords);
    PolarBuilder& buildConeDiscrete(
        std::vector<float>& verts,
        std::vector<float>& norms,
        std::vector<float>& texCoords);
    PolarBuilder& buildConeDiscreteInternal(
        std::vector<float>& verts,
        std::vector<float>& norms,
        std::vector<float>& texCoords,
        bool isSecondCoat);

    std::wstring m_formula;
    float m_domainStart;
    float m_domainEnd;
    int m_sectors;
    int m_slices;
    bool m_turbo;
    bool m_smooth;
    bool m_doubleCoated;
    bool m_reversed;
};

class Builder
{
public:
    static PolarBuilder polar();
};

} // namespace dynamit::builders