#pragma once

#include <vector>
#include <string>
#include <cstdint>
#define _USE_MATH_DEFINES
#include <cmath>
#include <array>

namespace dynamit::builders
{

// 4x4 transformation matrix (column-major, like GLM)
using Matrix4 = std::array<float, 16>;

// Identity matrix
inline Matrix4 identityMatrix()
{
    return {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

// Transform a position (applies full 4x4 transform including translation)
inline void transformPosition(const Matrix4& m, float& x, float& y, float& z)
{
    float w = 1.0f;
    float nx = m[0] * x + m[4] * y + m[8] * z + m[12] * w;
    float ny = m[1] * x + m[5] * y + m[9] * z + m[13] * w;
    float nz = m[2] * x + m[6] * y + m[10] * z + m[14] * w;
    x = nx;
    y = ny;
    z = nz;
}

// Transform a normal (applies only rotation/scale, no translation)
inline void transformNormal(const Matrix4& m, float& nx, float& ny, float& nz)
{
    float tnx = m[0] * nx + m[4] * ny + m[8] * nz;
    float tny = m[1] * nx + m[5] * ny + m[9] * nz;
    float tnz = m[2] * nx + m[6] * ny + m[10] * nz;
    
    float len = std::sqrt(tnx * tnx + tny * tny + tnz * tnz);
    if (len > 0.0001f)
    {
        tnx /= len;
        tny /= len;
        tnz /= len;
    }
    
    nx = tnx;
    ny = tny;
    nz = tnz;
}

// Apply single transformation to a range of vertices and normals
inline void applyTransformToRange(
    const Matrix4& m,
    std::vector<float>& verts,
    std::vector<float>& norms,
    size_t startVertex)
{
    size_t startIdx = startVertex * 3;
    
    for (size_t i = startIdx; i < verts.size(); i += 3)
    {
        transformPosition(m, verts[i], verts[i + 1], verts[i + 2]);
    }
    
    for (size_t i = startIdx; i < norms.size(); i += 3)
    {
        transformNormal(m, norms[i], norms[i + 1], norms[i + 2]);
    }
}

// Variadic: apply multiple transformations in sequence
template<typename... Transforms>
inline void applyTransformsToRange(
    std::vector<float>& verts,
    std::vector<float>& norms,
    size_t startVertex,
    const Transforms&... transforms)
{
    (applyTransformToRange(transforms, verts, norms, startVertex), ...);
}

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