#pragma once

#include <vector>
#include <string>
#include <cstdint>
#define _USE_MATH_DEFINES
#include <cmath>
#include <array>
#include <iostream>

#include "geometry.h"
namespace dynamit::builders
{
using namespace dynamit::geo;

struct GeometryBuffers
{
    std::vector<float>& verts;
    std::vector<float>& norms;
    std::vector<float>& texCoords;
    std::vector<float>& colors;
    std::vector<uint32_t>& indices;
    
    GeometryBuffers(
        std::vector<float>& v,
        std::vector<float>& n,
        std::vector<float>& t,
        std::vector<float>& c,
        std::vector<uint32_t>& i)
        : verts(v), norms(n), texCoords(t), colors(c), indices(i) {}
};

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
    //PolarBuilder& color(float r, float g, float b, float a = 1.0f) { m_color_outer = { r, g, b, a }; return *this; }
	PolarBuilder& color(const std::array<float, 4>& rgba) { m_color_outer = rgba; m_color_inner = rgba; return *this; }
    PolarBuilder& color(const std::array<float, 3>& rgb) { m_color_outer = { rgb[0], rgb[1], rgb[2], 1.0f }; m_color_inner = { rgb[0], rgb[1], rgb[2], 1.0f }; return *this; }
    PolarBuilder& color(const std::array<float, 4>& rgbao, const std::array<float, 4>& rgbai) { m_color_outer = rgbao; m_color_inner = rgbai; return *this; }
    PolarBuilder& color(const std::array<float, 3>& rgbo, const std::array<float, 3>& rgbi) { m_color_outer = { rgbo[0], rgbo[1], rgbo[2], 1.0f }; m_color_inner = { rgbi[0], rgbi[1], rgbi[2], 1.0f }; return *this; }

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
    template<typename ...Transforms>
    PolarBuilder& buildConeIndexedWithColor(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& colors, std::vector<uint32_t>& indices, const Transforms & ...transforms);
    template<typename ...Transforms>
    PolarBuilder& buildCylinderIndexedWithColor(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& colors, std::vector<uint32_t>& indices, const Transforms & ...transforms);
    PolarBuilder& buildConeIndexedWithColor(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& colors, std::vector<uint32_t>& indices);
    PolarBuilder& buildCylinderIndexedWithColor(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& colors, std::vector<uint32_t>& indices);
private:
    PolarBuilder& buildConeIndexedInternal(GeometryBuffers& buffers, bool isSecondCoat);
    PolarBuilder& buildConeDiscrete(GeometryBuffers& buffers);
    PolarBuilder& buildConeDiscreteInternal(GeometryBuffers& buffers, bool isSecondCoat);
    PolarBuilder& buildCylinderIndexedInternal(GeometryBuffers& buffers, bool isSecondCoat);
    PolarBuilder& buildCylinderDiscrete(GeometryBuffers& buffers);
    PolarBuilder& buildCylinderDiscreteInternal(GeometryBuffers& buffers, bool isSecondCoat);
    PolarBuilder& buildCylinderDiscreteIndexedInternal(GeometryBuffers& buffers, bool isSecondCoat);

    std::wstring m_formula;
    float m_domainStart;
    float m_domainEnd;
    int m_sectors;
    int m_slices;
    bool m_turbo;
    bool m_smooth;
    bool m_doubleCoated;
    bool m_reversed;
    std::array<float, 4> m_color_outer = { 1.0f, 1.0f, 1.0f, 1.0f };
    std::array<float, 4> m_color_inner = { 1.0f, 1.0f, 1.0f, 1.0f };
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

// Cone with colors and transforms
template<typename... Transforms>
PolarBuilder& PolarBuilder::buildConeIndexedWithColor(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& colors, std::vector<uint32_t>& indices, const Transforms&... transforms)
{
    size_t startVertex = verts.size() / 3;
    buildConeIndexedWithColor(verts, norms, colors, indices);
    applyTransformsToRange(verts, norms, startVertex, transforms...);
    return *this;
}
// Cylinder with colors and transforms
template<typename... Transforms>
PolarBuilder& PolarBuilder::buildCylinderIndexedWithColor(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& colors, std::vector<uint32_t>& indices, const Transforms&... transforms)
{
    size_t startVertex = verts.size() / 3;
    buildCylinderIndexedWithColor(verts, norms, colors, indices);  // Now calls the colors overload
    applyTransformsToRange(verts, norms, startVertex, transforms...);
    return *this;
}

class Builder
{
public:
    static PolarBuilder polar();
};

} // namespace dynamit::builders