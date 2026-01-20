#pragma once

#include <vector>
#include <string>
#include <cstdint>
#define _USE_MATH_DEFINES
#include <cmath>
#include <array>

#include "geometry.h"
namespace dynamit::builders
{
using namespace dynamit::geo;

//// 4x4 transformation matrix (column-major, like GLM)
//using Matrix4 = std::array<float, 16>;
//template<typename T = float> using mat4 = std::array<T, 16>;


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

    // Cone - base (no transform)
    PolarBuilder& buildCone(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords);
    PolarBuilder& buildCone(std::vector<float>& verts, std::vector<float>& norms);
    PolarBuilder& buildConeIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices);
    PolarBuilder& buildConeIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<uint32_t>& indices);
    PolarBuilder& reBuildCone(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords);
    PolarBuilder& reBuildCone(std::vector<float>& verts, std::vector<float>& norms);
    PolarBuilder& reBuildConeIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices);
    PolarBuilder& reBuildConeIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<uint32_t>& indices);

    // Cone - variadic transforms
    template<typename... Transforms>
    PolarBuilder& buildCone(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& buildCone(std::vector<float>& verts, std::vector<float>& norms, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& buildConeIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& buildConeIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<uint32_t>& indices, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& reBuildCone(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& reBuildCone(std::vector<float>& verts, std::vector<float>& norms, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& reBuildConeIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& reBuildConeIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<uint32_t>& indices, const Transforms&... transforms);

    // Cylinder - base (no transform)
    PolarBuilder& buildCylinder(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords);
    PolarBuilder& buildCylinder(std::vector<float>& verts, std::vector<float>& norms);
    PolarBuilder& buildCylinderIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices);
    PolarBuilder& buildCylinderIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<uint32_t>& indices);
    PolarBuilder& reBuildCylinder(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords);
    PolarBuilder& reBuildCylinder(std::vector<float>& verts, std::vector<float>& norms);
    PolarBuilder& reBuildCylinderIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices);
    PolarBuilder& reBuildCylinderIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<uint32_t>& indices);

    // Cylinder - variadic transforms
    template<typename... Transforms>
    PolarBuilder& buildCylinder(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& buildCylinder(std::vector<float>& verts, std::vector<float>& norms, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& buildCylinderIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& buildCylinderIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<uint32_t>& indices, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& reBuildCylinder(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& reBuildCylinder(std::vector<float>& verts, std::vector<float>& norms, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& reBuildCylinderIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices, const Transforms&... transforms);
    template<typename... Transforms>
    PolarBuilder& reBuildCylinderIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<uint32_t>& indices, const Transforms&... transforms);

private:
    PolarBuilder& buildConeIndexedInternal(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices, bool isSecondCoat);
    PolarBuilder& buildConeDiscrete(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords);
    PolarBuilder& buildConeDiscreteInternal(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, bool isSecondCoat);
    PolarBuilder& buildCylinderIndexedInternal(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices, bool isSecondCoat);
    PolarBuilder& buildCylinderDiscrete(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords);
    PolarBuilder& buildCylinderDiscreteInternal(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, bool isSecondCoat);
    PolarBuilder& buildCylinderDiscreteIndexedInternal(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices, bool isSecondCoat);

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

// ============================================================================
// VARIADIC TEMPLATE IMPLEMENTATIONS (must be in header)
// ============================================================================

// Cone with transforms
template<typename... Transforms>
PolarBuilder& PolarBuilder::buildCone(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, const Transforms&... transforms)
{
    size_t startVertex = verts.size() / 3;
    buildCone(verts, norms, texCoords);
    applyTransformsToRange(verts, norms, startVertex, transforms...);
    return *this;
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::buildCone(std::vector<float>& verts, std::vector<float>& norms, const Transforms&... transforms)
{
    std::vector<float> texCoords;
    return buildCone(verts, norms, texCoords, transforms...);
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::buildConeIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices, const Transforms&... transforms)
{
    size_t startVertex = verts.size() / 3;
    buildConeIndexed(verts, norms, texCoords, indices);
    applyTransformsToRange(verts, norms, startVertex, transforms...);
    return *this;
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::buildConeIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<uint32_t>& indices, const Transforms&... transforms)
{
    std::vector<float> texCoords;
    return buildConeIndexed(verts, norms, texCoords, indices, transforms...);
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::reBuildCone(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, const Transforms&... transforms)
{
    verts.clear(); norms.clear(); texCoords.clear();
    return buildCone(verts, norms, texCoords, transforms...);
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::reBuildCone(std::vector<float>& verts, std::vector<float>& norms, const Transforms&... transforms)
{
    std::vector<float> texCoords;
    return reBuildCone(verts, norms, texCoords, transforms...);
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::reBuildConeIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices, const Transforms&... transforms)
{
    verts.clear(); norms.clear(); texCoords.clear(); indices.clear();
    return buildConeIndexed(verts, norms, texCoords, indices, transforms...);
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::reBuildConeIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<uint32_t>& indices, const Transforms&... transforms)
{
    std::vector<float> texCoords;
    return reBuildConeIndexed(verts, norms, texCoords, indices, transforms...);
}

// Cylinder with transforms
template<typename... Transforms>
PolarBuilder& PolarBuilder::buildCylinder(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, const Transforms&... transforms)
{
    size_t startVertex = verts.size() / 3;
    buildCylinder(verts, norms, texCoords);
    applyTransformsToRange(verts, norms, startVertex, transforms...);
    return *this;
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::buildCylinder(std::vector<float>& verts, std::vector<float>& norms, const Transforms&... transforms)
{
    std::vector<float> texCoords;
    return buildCylinder(verts, norms, texCoords, transforms...);
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::buildCylinderIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices, const Transforms&... transforms)
{
    size_t startVertex = verts.size() / 3;
    buildCylinderIndexed(verts, norms, texCoords, indices);
    applyTransformsToRange(verts, norms, startVertex, transforms...);
    return *this;
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::buildCylinderIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<uint32_t>& indices, const Transforms&... transforms)
{
    std::vector<float> texCoords;
    return buildCylinderIndexed(verts, norms, texCoords, indices, transforms...);
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::reBuildCylinder(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, const Transforms&... transforms)
{
    verts.clear(); norms.clear(); texCoords.clear();
    return buildCylinder(verts, norms, texCoords, transforms...);
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::reBuildCylinder(std::vector<float>& verts, std::vector<float>& norms, const Transforms&... transforms)
{
    std::vector<float> texCoords;
    return reBuildCylinder(verts, norms, texCoords, transforms...);
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::reBuildCylinderIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords, std::vector<uint32_t>& indices, const Transforms&... transforms)
{
    verts.clear(); norms.clear(); texCoords.clear(); indices.clear();
    return buildCylinderIndexed(verts, norms, texCoords, indices, transforms...);
}

template<typename... Transforms>
PolarBuilder& PolarBuilder::reBuildCylinderIndexed(std::vector<float>& verts, std::vector<float>& norms, std::vector<uint32_t>& indices, const Transforms&... transforms)
{
    std::vector<float> texCoords;
    return reBuildCylinderIndexed(verts, norms, texCoords, indices, transforms...);
}

class Builder
{
public:
    static PolarBuilder polar();
};

} // namespace dynamit::builders