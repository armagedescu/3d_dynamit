#include "pch.h"
#define _USE_MATH_DEFINES
#include "builders.h"

#include "expression_compiler.h"
#include "geometry.h"
#include <cmath>
#include <iostream> // For debug output


namespace dynamit::builders
{

PolarBuilder::PolarBuilder()
    : m_formula(L"1")
    , m_domainStart(0.0f)
    , m_domainEnd(static_cast<float>(2 * M_PI))
    , m_sectors(5)
    , m_slices(1)
    , m_turbo(true)
    , m_smooth(true)
    , m_doubleCoated(false)
    , m_reversed(false)
{
}

PolarBuilder& PolarBuilder::formula(const std::wstring& formula)
{
    m_formula = formula;
    return *this;
}

PolarBuilder& PolarBuilder::formula(const std::string& formula)
{
    m_formula = std::wstring(formula.begin(), formula.end());
    return *this;
}

PolarBuilder& PolarBuilder::domain(float start, float end)
{
    m_domainStart = start;
    m_domainEnd = end;
    return *this;
}
PolarBuilder& PolarBuilder::domain(float end)
{
    m_domainStart = 0.0f;
    m_domainEnd = end;
    return *this;
}
PolarBuilder& PolarBuilder::domain_shift(float new_end)
{
    m_domainStart = m_domainEnd;
    m_domainEnd = new_end;
    return *this;
}

PolarBuilder& PolarBuilder::sectors(int sectors)
{
    m_sectors = sectors;
    return *this;
}

PolarBuilder& PolarBuilder::slices(int slices)
{
    m_slices = slices;
    return *this;
}
PolarBuilder& PolarBuilder::slices_sectors(int slices, int sectors)
{
    m_slices = slices;
    m_sectors = sectors;
    return *this;
}

PolarBuilder& PolarBuilder::sectors_slices(int sectors, int slices)
{
    m_sectors = sectors;
    m_slices = slices;
    return *this;
}

PolarBuilder& PolarBuilder::turbo(bool enabled)
{
    m_turbo = enabled;
    return *this;
}

PolarBuilder& PolarBuilder::smooth(bool enabled)
{
    m_smooth = enabled;
    return *this;
}
PolarBuilder& PolarBuilder::edged(bool enabled)
{
    return smooth(!enabled);
}

PolarBuilder& PolarBuilder::doubleCoated(bool enabled)
{
    m_doubleCoated = enabled;
    return *this;
}
PolarBuilder& PolarBuilder::singleCoated(bool enabled)
{
    return doubleCoated(!enabled);
}

PolarBuilder& PolarBuilder::reversed(bool enabled)
{
    m_reversed = enabled;
    return *this;
}
PolarBuilder& PolarBuilder::nonreversed(bool enabled)
{
    return  reversed(!enabled);
}

// Helper to compute cross product normal from 3 points using geometry.h utilities
static void crossProductNormalLefthanded(
    float x0, float y0, float z0,
    float x1, float y1, float z1,
    float x2, float y2, float z2,
    float& nx, float& ny, float& nz,
    bool reversed)
{
    // Use left-handed cross product from geometry.h
    std::array<float, 3> p0 = { x0, y0, z0 };
    std::array<float, 3> p1 = { x1, y1, z1 };
    std::array<float, 3> p2 = { x2, y2, z2 };
    
    std::array<float, 3> normal = cross3pl(p0, p1, p2);
    
    nx = normal[0];
    ny = normal[1];
    nz = normal[2];
    
    // Normalize
    float len = hypn<float>(nx, ny, nz);
    if (len > 0.0001f)
    {
        shrink(len, nx, ny, nz);
    }
    
    if (reversed)
    {
        nx = -nx;
        ny = -ny;
        nz = -nz;
    }
}

PolarBuilder& PolarBuilder::buildConeIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<float>& texCoords,
    std::vector<uint32_t>& indices)
{
    return buildConeIndexedInternal(
        verts,
        norms,
        texCoords,
        indices, false);
}

PolarBuilder& PolarBuilder::buildConeIndexedInternal(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<float>& texCoords,
    std::vector<uint32_t>& indices, bool isSecondCoat)
{
    using expresie_tokenizer::expression_token_compiler;
    using expresie_tokenizer::expression;
    expression_token_compiler compiler;

    long double theta = 0.0L;

    std::unique_ptr<expression> expr_r = compiler.compile(m_formula);
    expr_r->bind(L"theta", &theta);

    std::unique_ptr<expression> expr_dr = simplify(expr_r->derivative(L"theta"));
    expr_dr->bind(L"theta", &theta);

    // Geometry position depends on m_reversed (user's choice)
    const float z_tip = m_reversed ? 0.0f : -1.0f;
    const float z_base = m_reversed ? -1.0f : 0.0f;

    // Winding: second coat flips the winding
    const bool flipWinding = isSecondCoat;

    auto addVertex = [&](float x, float y, float z, float nx, float ny, float nz, float u, float v) -> uint32_t {
        uint32_t idx = static_cast<uint32_t>(verts.size() / 3);
        verts.insert(verts.end(), { x, y, z });
        norms.insert(norms.end(), { nx, ny, nz });
        texCoords.insert(texCoords.end(), { u, v });
        return idx;
    };

    uint32_t tipIndex = addVertex(0.0f, 0.0f, z_tip, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f);

    std::vector<float> baseX(m_sectors + 1);
    std::vector<float> baseY(m_sectors + 1);
    std::vector<float> baseNx(m_sectors + 1);
    std::vector<float> baseNy(m_sectors + 1);
    std::vector<float> baseNz(m_sectors + 1);
    std::vector<uint32_t> baseRing(m_sectors + 1);

    float domainRange = m_domainEnd - m_domainStart;

    for (int i = 0; i <= m_sectors; i++)
    {
        theta = m_domainStart + domainRange * i / m_sectors;
        float u = static_cast<float>(i) / m_sectors;

        float r = static_cast<float>(expr_r->eval());
        float dr = static_cast<float>(expr_dr->eval());

        float x = static_cast<float>(expr_r->cyl_x(theta)) / m_slices;
        float y = static_cast<float>(expr_r->cyl_y(theta)) / m_slices;

        float cos_t = std::cos(static_cast<float>(theta));
        float sin_t = std::sin(static_cast<float>(theta));
        float nx = (dr * sin_t + r * cos_t);
        float ny = -(dr * cos_t - r * sin_t);
        float nz = -1.0f;
        if (isSecondCoat)
            nz = 1.0f;
        if (m_reversed)
        {
            if (!isSecondCoat)
            {
                nx *= -1.0f;
                ny *= -1.0f;
            }
        }
        else
        {
            if (isSecondCoat)
            {
                nx *= -1.0f;
                ny *= -1.0f;
            }
        }
        float len = std::sqrt(nx * nx + ny * ny + nz * nz);
        if (len > 0.0001f)
        {
            baseNx[i] = nx / len;
            baseNy[i] = ny / len;
            baseNz[i] = nz / len;
        }
        else
        {
            baseNx[i] = nx;
            baseNy[i] = ny;
            baseNz[i] = nz;
        }

        baseX[i] = x * m_slices;
        baseY[i] = y * m_slices;

        float firstRingZ = z_tip + (z_base - z_tip) / m_slices;
        baseRing[i] = addVertex(x, y, firstRingZ, baseNx[i], baseNy[i], baseNz[i], u, 1.0f / m_slices);
    }

    // Tip triangles
    for (int i = 0; i < m_sectors; i++)
    {
        if (!isSecondCoat)
        {
            indices.push_back(tipIndex);
            indices.push_back(baseRing[i]);
            indices.push_back(baseRing[i + 1]);
        }
        else
        {
            indices.push_back(tipIndex);
            indices.push_back(baseRing[i + 1]);
            indices.push_back(baseRing[i]);
        }
    }

    if (m_slices > 1)
    {
        std::vector<uint32_t> prevRing = baseRing;

        for (int h = 1; h < m_slices; h++)
        {
            float h2n = static_cast<float>(h + 1) / m_slices;
            float z = z_tip + (z_base - z_tip) * h2n;

            std::vector<uint32_t> currRing(m_sectors + 1);

            for (int i = 0; i <= m_sectors; i++)
            {
                float x, y, nx, ny, nz;
                float u = static_cast<float>(i) / m_sectors;

                if (m_turbo)
                {
                    x = baseX[i] * h2n;
                    y = baseY[i] * h2n;
                    nx = baseNx[i];
                    ny = baseNy[i];
                    nz = baseNz[i];
                }
                else
                {
                    theta = m_domainStart + domainRange * i / m_sectors;

                    float r = static_cast<float>(expr_r->eval());
                    float dr = static_cast<float>(expr_dr->eval());

                    x = static_cast<float>(expr_r->cyl_x(theta)) * h2n;
                    y = static_cast<float>(expr_r->cyl_y(theta)) * h2n;

                    float cos_t = std::cos(static_cast<float>(theta));
                    float sin_t = std::sin(static_cast<float>(theta));
                    nx = (dr * sin_t + r * cos_t);
                    ny = -(dr * cos_t - r * sin_t);
                    nz = -1.0f;
                    if (isSecondCoat)
                        nz = 1.0f;
                    if (m_reversed)
                    {
                        if (!isSecondCoat)
                        {
                            nx *= -1.0f;
                            ny *= -1.0f;
                        }
                    }
                    else
                    {
                        if (isSecondCoat)
                        {
                            nx *= -1.0f;
                            ny *= -1.0f;
                        }
                    }

                    float len = std::sqrt(nx * nx + ny * ny + nz * nz);
                    if (len > 0.0001f)
                    {
                        nx /= len;
                        ny /= len;
                        nz /= len;
                    }
                }

                currRing[i] = addVertex(x, y, z, nx, ny, nz, u, h2n);
            }

            for (int i = 0; i < m_sectors; i++)
            {
                uint32_t v00 = prevRing[i];
                uint32_t v01 = prevRing[i + 1];
                uint32_t v10 = currRing[i];
                uint32_t v11 = currRing[i + 1];

                if (isSecondCoat)
                {
                    indices.push_back(v00);
                    indices.push_back(v01);
                    indices.push_back(v10);

                    indices.push_back(v01);
                    indices.push_back(v11);
                    indices.push_back(v10);
                }
                else
                {
                    indices.push_back(v00);
                    indices.push_back(v10);
                    indices.push_back(v01);

                    indices.push_back(v01);
                    indices.push_back(v10);
                    indices.push_back(v11);
                }
            }

            prevRing = currRing;
        }
    }

    if (!isSecondCoat && m_doubleCoated)
    {
        buildConeIndexedInternal(verts, norms, texCoords, indices, true);
    }

    return *this;
}

PolarBuilder& PolarBuilder::buildCone(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords)
{
    if (!m_smooth)
    {
        return buildConeDiscrete(verts, norms, texCoords);
    }

    std::vector<float> indexedVerts, indexedNorms, indexedTexCoords;
    std::vector<uint32_t> indices;

    buildConeIndexed(indexedVerts, indexedNorms, indexedTexCoords, indices);

    size_t additionalSize = indices.size() * 3;
    verts.reserve(verts.size() + additionalSize);
    norms.reserve(norms.size() + additionalSize);
    texCoords.reserve(texCoords.size() + (indices.size() * 2));

    for (uint32_t idx : indices)
    {
        size_t offset3 = idx * 3;
        size_t offset2 = idx * 2;
        
        verts.push_back(indexedVerts[offset3]);
        verts.push_back(indexedVerts[offset3 + 1]);
        verts.push_back(indexedVerts[offset3 + 2]);

        norms.push_back(indexedNorms[offset3]);
        norms.push_back(indexedNorms[offset3 + 1]);
        norms.push_back(indexedNorms[offset3 + 2]);

        texCoords.push_back(indexedTexCoords[offset2]);
        texCoords.push_back(indexedTexCoords[offset2 + 1]);
    }

    return *this;
}

PolarBuilder& PolarBuilder::reBuildCone(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords)
{
    verts.clear();
    norms.clear();
    texCoords.clear();
    return buildCone(verts, norms, texCoords);
}

PolarBuilder& PolarBuilder::reBuildConeIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<float>& texCoords,
    std::vector<uint32_t>& indices)
{
    verts.clear();
    norms.clear();
    texCoords.clear();
    indices.clear();
    return buildConeIndexed(verts, norms, texCoords, indices);
}

PolarBuilder& PolarBuilder::buildCylinderIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<float>& texCoords,
    std::vector<uint32_t>& indices)
{
    using expresie_tokenizer::expression_token_compiler;
    using expresie_tokenizer::expression;
    expression_token_compiler compiler;

    long double theta = 0.0L;

    std::unique_ptr<expression> expr_r = compiler.compile(m_formula);
    expr_r->bind(L"theta", &theta);

    std::unique_ptr<expression> expr_dr = simplify(expr_r->derivative(L"theta"));
    expr_dr->bind(L"theta", &theta);

    const float normalSign = m_reversed ? -1.0f : 1.0f;

    auto addVertex = [&](float x, float y, float z, float nx, float ny, float nz, float u, float v) -> uint32_t {
        uint32_t idx = static_cast<uint32_t>(verts.size() / 3);
        verts.insert(verts.end(), { x, y, z });
        norms.insert(norms.end(), { nx * normalSign, ny * normalSign, nz * normalSign });
        texCoords.insert(texCoords.end(), { u, v });
        return idx;
    };

    // Store first ring data for turbo mode
    std::vector<float> baseX(m_sectors + 1);
    std::vector<float> baseY(m_sectors + 1);
    std::vector<float> baseNx(m_sectors + 1);
    std::vector<float> baseNy(m_sectors + 1);

    float domainRange = m_domainEnd - m_domainStart;

    // Build first ring at z = 0
    std::vector<uint32_t> prevRing(m_sectors + 1);
    for (int i = 0; i <= m_sectors; i++)
    {
        theta = m_domainStart + domainRange * i / m_sectors;
        float u = static_cast<float>(i) / m_sectors;

        float r = static_cast<float>(expr_r->eval());
        float dr = static_cast<float>(expr_dr->eval());

        float x = static_cast<float>(expr_r->cyl_x(theta));
        float y = static_cast<float>(expr_r->cyl_y(theta));

        float cos_t = std::cos(static_cast<float>(theta));
        float sin_t = std::sin(static_cast<float>(theta));
        float nx = dr * sin_t + r * cos_t;
        float ny = -(dr * cos_t - r * sin_t);

        float len = std::sqrt(nx * nx + ny * ny);
        if (len > 0.0001f)
        {
            nx /= len;
            ny /= len;
        }

        baseX[i] = x;
        baseY[i] = y;
        baseNx[i] = nx;
        baseNy[i] = ny;

        prevRing[i] = addVertex(x, y, 0.0f, nx, ny, 0.0f, u, 0.0f);
    }

    // Build remaining rings and quads
    for (int h = 1; h <= m_slices; h++)
    {
        float z = static_cast<float>(h) / m_slices;
        float v = z;

        std::vector<uint32_t> currRing(m_sectors + 1);

        for (int i = 0; i <= m_sectors; i++)
        {
            float x, y, nx, ny;
            float u = static_cast<float>(i) / m_sectors;

            if (m_turbo)
            {
                x = baseX[i];
                y = baseY[i];
                nx = baseNx[i];
                ny = baseNy[i];
            }
            else
            {
                theta = m_domainStart + domainRange * i / m_sectors;

                float r = static_cast<float>(expr_r->eval());
                float dr = static_cast<float>(expr_dr->eval());

                x = static_cast<float>(expr_r->cyl_x(theta));
                y = static_cast<float>(expr_r->cyl_y(theta));

                float cos_t = std::cos(static_cast<float>(theta));
                float sin_t = std::sin(static_cast<float>(theta));
                nx = dr * sin_t + r * cos_t;
                ny = -(dr * cos_t - r * sin_t);

                float len = std::sqrt(nx * nx + ny * ny);
                if (len > 0.0001f)
                {
                    nx /= len;
                    ny /= len;
                }
            }

            currRing[i] = addVertex(x, y, z, nx, ny, 0.0f, u, v);
        }

        for (int i = 0; i < m_sectors; i++)
        {
            uint32_t v00 = prevRing[i];
            uint32_t v01 = prevRing[i + 1];
            uint32_t v10 = currRing[i];
            uint32_t v11 = currRing[i + 1];

            if (m_reversed)
            {
                indices.push_back(v00);
                indices.push_back(v10);
                indices.push_back(v01);

                indices.push_back(v01);
                indices.push_back(v10);
                indices.push_back(v11);
            }
            else
            {
                indices.push_back(v00);
                indices.push_back(v01);
                indices.push_back(v10);

                indices.push_back(v01);
                indices.push_back(v11);
                indices.push_back(v10);
            }
        }

        prevRing = currRing;
    }

    // Double coating
    if (m_doubleCoated)
    {
        m_doubleCoated = false;  // Prevent infinite recursion
        bool wasReversed = m_reversed;
        m_reversed = !m_reversed;
        buildCylinderIndexed(verts, norms, texCoords, indices);  // Make sure this says buildCylinderIndexed, NOT buildConeIndexed
        m_reversed = wasReversed;
        m_doubleCoated = true;   // Restore state
    }

    return *this;
}

PolarBuilder& PolarBuilder::buildCylinder(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords)
{
    if (!m_smooth)
    {
        return buildCylinderDiscrete(verts, norms, texCoords);
    }

    std::vector<float> indexedVerts, indexedNorms, indexedTexCoords;
    std::vector<uint32_t> indices;

    buildCylinderIndexed(indexedVerts, indexedNorms, indexedTexCoords, indices);

    size_t additionalSize = indices.size() * 3;
    verts.reserve(verts.size() + additionalSize);
    norms.reserve(norms.size() + additionalSize);
    texCoords.reserve(texCoords.size() + (indices.size() * 2));

    for (uint32_t idx : indices)
    {
        size_t offset3 = idx * 3;
        size_t offset2 = idx * 2;
        
        verts.push_back(indexedVerts[offset3]);
        verts.push_back(indexedVerts[offset3 + 1]);
        verts.push_back(indexedVerts[offset3 + 2]);

        norms.push_back(indexedNorms[offset3]);
        norms.push_back(indexedNorms[offset3 + 1]);
        norms.push_back(indexedNorms[offset3 + 2]);

        texCoords.push_back(indexedTexCoords[offset2]);
        texCoords.push_back(indexedTexCoords[offset2 + 1]);
    }

    return *this;
}

PolarBuilder& PolarBuilder::reBuildCylinder(std::vector<float>& verts, std::vector<float>& norms, std::vector<float>& texCoords)
{
    verts.clear();
    norms.clear();
    texCoords.clear();
    return buildCylinder(verts, norms, texCoords);
}

PolarBuilder& PolarBuilder::reBuildCylinderIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<float>& texCoords,
    std::vector<uint32_t>& indices)
{
    verts.clear();
    norms.clear();
    texCoords.clear();
    indices.clear();
    return buildCylinderIndexed(verts, norms, texCoords, indices);
}

// Legacy overloads (without texCoords) - use dummy vector
PolarBuilder& PolarBuilder::buildCone(std::vector<float>& verts, std::vector<float>& norms)
{
    std::vector<float> texCoords;
    return buildCone(verts, norms, texCoords);
}

PolarBuilder& PolarBuilder::buildConeIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<uint32_t>& indices)
{
    std::vector<float> texCoords;
    return buildConeIndexed(verts, norms, texCoords, indices);
}

PolarBuilder& PolarBuilder::reBuildCone(std::vector<float>& verts, std::vector<float>& norms)
{
    std::vector<float> texCoords;
    return reBuildCone(verts, norms, texCoords);
}

PolarBuilder& PolarBuilder::reBuildConeIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<uint32_t>& indices)
{
    std::vector<float> texCoords;
    return reBuildConeIndexed(verts, norms, texCoords, indices);
}

PolarBuilder& PolarBuilder::buildCylinder(std::vector<float>& verts, std::vector<float>& norms)
{
    std::vector<float> texCoords;
    return buildCylinder(verts, norms, texCoords);
}

PolarBuilder& PolarBuilder::buildCylinderIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<uint32_t>& indices)
{
    std::vector<float> texCoords;
    return buildCylinderIndexed(verts, norms, texCoords, indices);
}

PolarBuilder& PolarBuilder::reBuildCylinder(std::vector<float>& verts, std::vector<float>& norms)
{
    std::vector<float> texCoords;
    return reBuildCylinder(verts, norms, texCoords);
}

PolarBuilder& PolarBuilder::reBuildCylinderIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<uint32_t>& indices)
{
    std::vector<float> texCoords;
    return reBuildCylinderIndexed(verts, norms, texCoords, indices);
}

PolarBuilder Builder::polar()
{
    return PolarBuilder();
}
// ============================================================================
// CYLINDER - DISCRETE (FLAT SHADING)
// ============================================================================

PolarBuilder& PolarBuilder::buildCylinderDiscrete(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<float>& texCoords)
{
    using expresie_tokenizer::expression_token_compiler;
    using expresie_tokenizer::expression;
    expression_token_compiler compiler;

    long double theta = 0.0L;

    std::unique_ptr<expression> expr_r = compiler.compile(m_formula);
    expr_r->bind(L"theta", &theta);

    float domainRange = m_domainEnd - m_domainStart;

    // Precompute ring positions
    std::vector<std::vector<float>> ringX(m_slices + 1, std::vector<float>(m_sectors + 1));
    std::vector<std::vector<float>> ringY(m_slices + 1, std::vector<float>(m_sectors + 1));
    std::vector<std::vector<float>> ringZ(m_slices + 1, std::vector<float>(m_sectors + 1));

    for (int h = 0; h <= m_slices; h++)
    {
        float z = static_cast<float>(h) / m_slices;

        for (int i = 0; i <= m_sectors; i++)
        {
            theta = m_domainStart + domainRange * i / m_sectors;

            float x = static_cast<float>(expr_r->cyl_x(theta));
            float y = static_cast<float>(expr_r->cyl_y(theta));

            ringX[h][i] = x;
            ringY[h][i] = y;
            ringZ[h][i] = z;
        }
    }

    // Generate triangles with flat normals
    for (int h = 0; h < m_slices; h++)
    {
        float v0 = static_cast<float>(h) / m_slices;
        float v1 = static_cast<float>(h + 1) / m_slices;

        for (int i = 0; i < m_sectors; i++)
        {
            float u0 = static_cast<float>(i) / m_sectors;
            float u1 = static_cast<float>(i + 1) / m_sectors;

            // Quad corners
            float x00 = ringX[h][i],         y00 = ringY[h][i],         z00 = ringZ[h][i];
            float x01 = ringX[h][i + 1],     y01 = ringY[h][i + 1],     z01 = ringZ[h][i + 1];
            float x10 = ringX[h + 1][i],     y10 = ringY[h + 1][i],     z10 = ringZ[h + 1][i];
            float x11 = ringX[h + 1][i + 1], y11 = ringY[h + 1][i + 1], z11 = ringZ[h + 1][i + 1];

            // Triangle 1: v00, v10, v01
            float nx1, ny1, nz1;
            if (m_reversed)
                crossProductNormalLefthanded(x00, y00, z00, x01, y01, z01, x10, y10, z10, nx1, ny1, nz1, false);
            else
                crossProductNormalLefthanded(x00, y00, z00, x10, y10, z10, x01, y01, z01, nx1, ny1, nz1, false);

            if (m_reversed)
            {
                verts.insert(verts.end(), { x00, y00, z00 });
                verts.insert(verts.end(), { x01, y01, z01 });
                verts.insert(verts.end(), { x10, y10, z10 });
            }
            else
            {
                verts.insert(verts.end(), { x00, y00, z00 });
                verts.insert(verts.end(), { x10, y10, z10 });
                verts.insert(verts.end(), { x01, y01, z01 });
            }

            norms.insert(norms.end(), { nx1, ny1, nz1 });
            norms.insert(norms.end(), { nx1, ny1, nz1 });
            norms.insert(norms.end(), { nx1, ny1, nz1 });

            texCoords.insert(texCoords.end(), { u0, v0 });
            texCoords.insert(texCoords.end(), { m_reversed ? u1 : u0, m_reversed ? v0 : v1 });
            texCoords.insert(texCoords.end(), { m_reversed ? u0 : u1, m_reversed ? v1 : v0 });

            // Triangle 2: v01, v10, v11
            float nx2, ny2, nz2;
            if (m_reversed)
                crossProductNormalLefthanded(x01, y01, z01, x11, y11, z11, x10, y10, z10, nx2, ny2, nz2, false);
            else
                crossProductNormalLefthanded(x01, y01, z01, x10, y10, z10, x11, y11, z11, nx2, ny2, nz2, false);

            if (m_reversed)
            {
                verts.insert(verts.end(), { x01, y01, z01 });
                verts.insert(verts.end(), { x11, y11, z11 });
                verts.insert(verts.end(), { x10, y10, z10 });
            }
            else
            {
                verts.insert(verts.end(), { x01, y01, z01 });
                verts.insert(verts.end(), { x10, y10, z10 });
                verts.insert(verts.end(), { x11, y11, z11 });
            }

            norms.insert(norms.end(), { nx2, ny2, nz2 });
            norms.insert(norms.end(), { nx2, ny2, nz2 });
            norms.insert(norms.end(), { nx2, ny2, nz2 });

            texCoords.insert(texCoords.end(), { u1, v0 });
            texCoords.insert(texCoords.end(), { m_reversed ? u1 : u0, m_reversed ? v1 : v1 });
            texCoords.insert(texCoords.end(), { m_reversed ? u0 : u1, m_reversed ? v0 : v1 });
        }
    }

    // Double coating
    if (m_doubleCoated)
    {
        m_doubleCoated = false;
        bool wasReversed = m_reversed;
        m_reversed = !m_reversed;
        buildCylinderDiscrete(verts, norms, texCoords);
        m_reversed = wasReversed;
        m_doubleCoated = true;
    }

    return *this;
}

// ============================================================================
// CONE - DISCRETE (FLAT SHADING)
// ============================================================================

PolarBuilder& PolarBuilder::buildConeDiscrete(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<float>& texCoords)
{
    return buildConeDiscreteInternal(verts, norms, texCoords, false);
}

PolarBuilder& PolarBuilder::buildConeDiscreteInternal(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<float>& texCoords,
    bool isSecondCoat)
{
    using expresie_tokenizer::expression_token_compiler;
    using expresie_tokenizer::expression;
    expression_token_compiler compiler;

    long double theta = 0.0L;

    std::unique_ptr<expression> expr_r = compiler.compile(m_formula);
    expr_r->bind(L"theta", &theta);

    // Geometry position depends on m_reversed (user's choice)
    const float z_tip = m_reversed ? 0.0f : -1.0f;
    const float z_base = m_reversed ? -1.0f : 0.0f;

    float domainRange = m_domainEnd - m_domainStart;

    // Precompute ring positions (ring 0 is closest to tip)
    std::vector<std::vector<float>> ringX(m_slices + 1, std::vector<float>(m_sectors + 1));
    std::vector<std::vector<float>> ringY(m_slices + 1, std::vector<float>(m_sectors + 1));
    std::vector<std::vector<float>> ringZ(m_slices + 1, std::vector<float>(m_sectors + 1));

    for (int h = 0; h <= m_slices; h++)
    {
        float scale = static_cast<float>(h) / m_slices;
        float z = z_tip + (z_base - z_tip) * scale;

        for (int i = 0; i <= m_sectors; i++)
        {
            theta = m_domainStart + domainRange * i / m_sectors;

            float x = static_cast<float>(expr_r->cyl_x(theta)) * scale;
            float y = static_cast<float>(expr_r->cyl_y(theta)) * scale;

            ringX[h][i] = x;
            ringY[h][i] = y;
            ringZ[h][i] = z;
        }
    }

    // Tip triangles (h=0 ring is at the tip, all vertices collapse to origin)
    float tipX = 0.0f, tipY = 0.0f, tipZ = z_tip;

    for (int i = 0; i < m_sectors; i++)
    {
        float u0 = static_cast<float>(i) / m_sectors;
        float u1 = static_cast<float>(i + 1) / m_sectors;

        // First ring vertices
        float x0 = ringX[1][i], y0 = ringY[1][i], z0 = ringZ[1][i];
        float x1 = ringX[1][i + 1], y1 = ringY[1][i + 1], z1 = ringZ[1][i + 1];

        // Compute flat normal for tip triangle
        float nx, ny, nz;
        if (!isSecondCoat)
        {
            crossProductNormalLefthanded(tipX, tipY, tipZ, x0, y0, z0, x1, y1, z1, nx, ny, nz, false);
            verts.insert(verts.end(), { tipX, tipY, tipZ });
            verts.insert(verts.end(), { x0, y0, z0 });
            verts.insert(verts.end(), { x1, y1, z1 });
        }
        else
        {
            crossProductNormalLefthanded(tipX, tipY, tipZ, x1, y1, z1, x0, y0, z0, nx, ny, nz, false);
            verts.insert(verts.end(), { tipX, tipY, tipZ });
            verts.insert(verts.end(), { x1, y1, z1 });
            verts.insert(verts.end(), { x0, y0, z0 });
        }

        norms.insert(norms.end(), { nx, ny, nz });
        norms.insert(norms.end(), { nx, ny, nz });
        norms.insert(norms.end(), { nx, ny, nz });

        texCoords.insert(texCoords.end(), { 0.5f, 0.0f });
        texCoords.insert(texCoords.end(), { u0, 1.0f / m_slices });
        texCoords.insert(texCoords.end(), { u1, 1.0f / m_slices });
    }

    // Remaining quads
    for (int h = 1; h < m_slices; h++)
    {
        float v0 = static_cast<float>(h) / m_slices;
        float v1 = static_cast<float>(h + 1) / m_slices;

        for (int i = 0; i < m_sectors; i++)
        {
            float u0 = static_cast<float>(i) / m_sectors;
            float u1 = static_cast<float>(i + 1) / m_sectors;

            // Quad corners
            float x00 = ringX[h][i],         y00 = ringY[h][i],         z00 = ringZ[h][i];
            float x01 = ringX[h][i + 1],     y01 = ringY[h][i + 1],     z01 = ringZ[h][i + 1];
            float x10 = ringX[h + 1][i],     y10 = ringY[h + 1][i],     z10 = ringZ[h + 1][i];
            float x11 = ringX[h + 1][i + 1], y11 = ringY[h + 1][i + 1], z11 = ringZ[h + 1][i + 1];

            // Triangle 1
            float nx1, ny1, nz1;
            if (!isSecondCoat)
            {
                crossProductNormalLefthanded(x00, y00, z00, x10, y10, z10, x01, y01, z01, nx1, ny1, nz1, false);
                verts.insert(verts.end(), { x00, y00, z00 });
                verts.insert(verts.end(), { x10, y10, z10 });
                verts.insert(verts.end(), { x01, y01, z01 });
            }
            else
            {
                crossProductNormalLefthanded(x00, y00, z00, x01, y01, z01, x10, y10, z10, nx1, ny1, nz1, false);
                verts.insert(verts.end(), { x00, y00, z00 });
                verts.insert(verts.end(), { x01, y01, z01 });
                verts.insert(verts.end(), { x10, y10, z10 });
            }

            norms.insert(norms.end(), { nx1, ny1, nz1 });
            norms.insert(norms.end(), { nx1, ny1, nz1 });
            norms.insert(norms.end(), { nx1, ny1, nz1 });

            texCoords.insert(texCoords.end(), { u0, v0 });
            texCoords.insert(texCoords.end(), { isSecondCoat ? u1 : u0, isSecondCoat ? v0 : v1 });
            texCoords.insert(texCoords.end(), { isSecondCoat ? u0 : u1, isSecondCoat ? v1 : v0 });

            // Triangle 2
            float nx2, ny2, nz2;
            if (!isSecondCoat)
            {
                crossProductNormalLefthanded(x01, y01, z01, x10, y10, z10, x11, y11, z11, nx2, ny2, nz2, false);
                verts.insert(verts.end(), { x01, y01, z01 });
                verts.insert(verts.end(), { x10, y10, z10 });
                verts.insert(verts.end(), { x11, y11, z11 });
            }
            else
            {
                crossProductNormalLefthanded(x01, y01, z01, x11, y11, z11, x10, y10, z10, nx2, ny2, nz2, false);
                verts.insert(verts.end(), { x01, y01, z01 });
                verts.insert(verts.end(), { x11, y11, z11 });
                verts.insert(verts.end(), { x10, y10, z10 });
            }

            norms.insert(norms.end(), { nx2, ny2, nz2 });
            norms.insert(norms.end(), { nx2, ny2, nz2 });
            norms.insert(norms.end(), { nx2, ny2, nz2 });

            texCoords.insert(texCoords.end(), { u1, v0 });
            texCoords.insert(texCoords.end(), { isSecondCoat ? u1 : u0, isSecondCoat ? v1 : v1 });
            texCoords.insert(texCoords.end(), { isSecondCoat ? u0 : u1, isSecondCoat ? v0 : v1 });
        }
    }

    if (!isSecondCoat && m_doubleCoated)
    {
        buildConeDiscreteInternal(verts, norms, texCoords, true);
    }

    return *this;
}

} // namespace dynamit::builders

