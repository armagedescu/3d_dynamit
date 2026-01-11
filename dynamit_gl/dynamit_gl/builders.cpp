#include "pch.h"
#define _USE_MATH_DEFINES
#include "builders.h"

#include "expression_compiler.h"
#include <cmath>


namespace dynamit::builders
{

PolarBuilder::PolarBuilder()
    : m_formula(L"1")
    , m_domainStart(0.0f)
    , m_domainEnd(static_cast<float>(2 * M_PI))
    , m_sectors(5)
    , m_slices(1)
    , m_turbo(true)
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

PolarBuilder& PolarBuilder::buildConeIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<uint32_t>& indices)
{
    using expresie_tokenizer::expression_token_compiler;
    using expresie_tokenizer::expression;
    expression_token_compiler compiler;

    // Calculate index offset for appending
    uint32_t indexOffset = static_cast<uint32_t>(verts.size() / 3);

    long double theta = 0.0L;

    // Compile r(theta) expression
    std::unique_ptr<expression> expr_r = compiler.compile(m_formula);
    expr_r->bind(L"theta", &theta);

    // Compute dr/dtheta symbolically
    std::unique_ptr<expression> expr_dr = simplify(expr_r->derivative(L"theta"));
    expr_dr->bind(L"theta", &theta);

    const float z_tip = -1.f;

    auto addVertex = [&](float x, float y, float z, float nx, float ny, float nz) -> uint32_t {
        uint32_t idx = static_cast<uint32_t>(verts.size() / 3);
        verts.insert(verts.end(), { x, y, z });
        norms.insert(norms.end(), { nx, ny, nz });
        return idx;
    };

    // Add tip vertex
    uint32_t tipIndex = addVertex(0.0f, 0.0f, z_tip, 0.0f, 0.0f, 0.0f);

    // Build first ring (base) with m_sectors+1 vertices to include endpoint
    std::vector<uint32_t> baseRing(m_sectors + 1);
    
    // Store base ring data for turbo mode
    std::vector<float> baseX(m_sectors + 1);
    std::vector<float> baseY(m_sectors + 1);
    std::vector<float> baseNx(m_sectors + 1);
    std::vector<float> baseNy(m_sectors + 1);
    std::vector<float> baseNz(m_sectors + 1);

    for (int i = 0; i <= m_sectors; i++)
    {
        theta = m_domainStart + (m_domainEnd - m_domainStart) * i / m_sectors;

        float r = static_cast<float>(expr_r->eval());
        float dr = static_cast<float>(expr_dr->eval());

        float x = static_cast<float>(expr_r->cyl_x(theta)) / m_slices;
        float y = static_cast<float>(expr_r->cyl_y(theta)) / m_slices;

        float cos_t = std::cos(static_cast<float>(theta));
        float sin_t = std::sin(static_cast<float>(theta));
        float nx = dr * sin_t + r * cos_t;
        float ny = -(dr * cos_t - r * sin_t);
        float nz = z_tip;

        // Normalize for turbo mode storage
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

        // Store base positions (at full scale, i.e., multiplied by m_slices)
        baseX[i] = x * m_slices;
        baseY[i] = y * m_slices;

        baseRing[i] = addVertex(x, y, 0.0f, nx, ny, nz);
    }

    // Generate tip triangles (fan from tip to base ring)
    for (int i = 0; i < m_sectors; i++)
    {
        indices.push_back(tipIndex);
        indices.push_back(baseRing[i]);
        indices.push_back(baseRing[i + 1]);
    }

    // Build remaining rings and quads for slices
    if (m_slices > 1)
    {
        std::vector<uint32_t> prevRing = baseRing;

        for (int h = 1; h < m_slices; h++)
        {
            float h2n = static_cast<float>(h + 1) / m_slices;

            std::vector<uint32_t> currRing(m_sectors + 1);

            for (int i = 0; i <= m_sectors; i++)
            {
                float x, y, nx, ny, nz;

                if (m_turbo)
                {
                    // Turbo mode: scale base positions, reuse normals
                    x = baseX[i] * h2n;
                    y = baseY[i] * h2n;
                    nx = baseNx[i];
                    ny = baseNy[i];
                    nz = baseNz[i];
                }
                else
                {
                    // Non-turbo: full recalculation
                    theta = m_domainStart + (m_domainEnd - m_domainStart) * i / m_sectors;

                    float r = static_cast<float>(expr_r->eval());
                    float dr = static_cast<float>(expr_dr->eval());

                    x = static_cast<float>(expr_r->cyl_x(theta)) * h2n;
                    y = static_cast<float>(expr_r->cyl_y(theta)) * h2n;

                    float cos_t = std::cos(static_cast<float>(theta));
                    float sin_t = std::sin(static_cast<float>(theta));
                    nx = dr * sin_t + r * cos_t;
                    ny = -(dr * cos_t - r * sin_t);
                    nz = -1.0f;

                    // Normalize
                    float len = std::sqrt(nx * nx + ny * ny + nz * nz);
                    if (len > 0.0001f)
                    {
                        nx /= len;
                        ny /= len;
                        nz /= len;
                    }
                }

                currRing[i] = addVertex(x, y, h2n, nx, ny, nz);
            }

            // Generate quads between prevRing and currRing
            for (int i = 0; i < m_sectors; i++)
            {
                uint32_t v00 = prevRing[i];
                uint32_t v01 = prevRing[i + 1];
                uint32_t v10 = currRing[i];
                uint32_t v11 = currRing[i + 1];

                // Triangle 1
                indices.push_back(v00);
                indices.push_back(v10);
                indices.push_back(v01);

                // Triangle 2
                indices.push_back(v01);
                indices.push_back(v10);
                indices.push_back(v11);
            }

            prevRing = currRing;
        }
    }

    return *this;
}

PolarBuilder& PolarBuilder::buildCone(std::vector<float>& verts, std::vector<float>& norms)
{
    std::vector<float> indexedVerts, indexedNorms;
    std::vector<uint32_t> indices;

    buildConeIndexed(indexedVerts, indexedNorms, indices);

    size_t additionalSize = indices.size() * 3;
    verts.reserve(verts.size() + additionalSize);
    norms.reserve(norms.size() + additionalSize);

    for (uint32_t idx : indices)
    {
        size_t offset = idx * 3;
        verts.push_back(indexedVerts[offset]);
        verts.push_back(indexedVerts[offset + 1]);
        verts.push_back(indexedVerts[offset + 2]);

        norms.push_back(indexedNorms[offset]);
        norms.push_back(indexedNorms[offset + 1]);
        norms.push_back(indexedNorms[offset + 2]);
    }

    return *this;
}

PolarBuilder& PolarBuilder::reBuildCone(std::vector<float>& verts, std::vector<float>& norms)
{
    verts.clear();
    norms.clear();
    return buildCone(verts, norms);
}

PolarBuilder& PolarBuilder::reBuildConeIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<uint32_t>& indices)
{
    verts.clear();
    norms.clear();
    indices.clear();
    return buildConeIndexed(verts, norms, indices);
}

PolarBuilder& PolarBuilder::buildCylinderIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<uint32_t>& indices)
{
    using expresie_tokenizer::expression_token_compiler;
    using expresie_tokenizer::expression;
    expression_token_compiler compiler;

    long double theta = 0.0L;

    // Compile r(theta) expression
    std::unique_ptr<expression> expr_r = compiler.compile(m_formula);
    expr_r->bind(L"theta", &theta);

    // Compute dr/dtheta symbolically
    std::unique_ptr<expression> expr_dr = simplify(expr_r->derivative(L"theta"));
    expr_dr->bind(L"theta", &theta);

    auto addVertex = [&](float x, float y, float z, float nx, float ny, float nz) -> uint32_t {
        uint32_t idx = static_cast<uint32_t>(verts.size() / 3);
        verts.insert(verts.end(), { x, y, z });
        norms.insert(norms.end(), { nx, ny, nz });
        return idx;
    };

    // Store first ring data for turbo mode
    std::vector<float> baseX(m_sectors + 1);
    std::vector<float> baseY(m_sectors + 1);
    std::vector<float> baseNx(m_sectors + 1);
    std::vector<float> baseNy(m_sectors + 1);

    // Build first ring at z = 0
    std::vector<uint32_t> prevRing(m_sectors + 1);
    for (int i = 0; i <= m_sectors; i++)
    {
        theta = m_domainStart + (m_domainEnd - m_domainStart) * i / m_sectors;

        float r = static_cast<float>(expr_r->eval());
        float dr = static_cast<float>(expr_dr->eval());

        float x = static_cast<float>(expr_r->cyl_x(theta));
        float y = static_cast<float>(expr_r->cyl_y(theta));

        float cos_t = std::cos(static_cast<float>(theta));
        float sin_t = std::sin(static_cast<float>(theta));
        float nx = dr * sin_t + r * cos_t;
        float ny = -(dr * cos_t - r * sin_t);

        // Normalize (nz = 0 for cylinder)
        float len = std::sqrt(nx * nx + ny * ny);
        if (len > 0.0001f)
        {
            nx /= len;
            ny /= len;
        }

        // Store for turbo mode
        baseX[i] = x;
        baseY[i] = y;
        baseNx[i] = nx;
        baseNy[i] = ny;

        prevRing[i] = addVertex(x, y, 0.0f, nx, ny, 0.0f);
    }

    // Build remaining rings and quads
    for (int h = 1; h <= m_slices; h++)
    {
        float z = static_cast<float>(h) / m_slices;

        std::vector<uint32_t> currRing(m_sectors + 1);

        for (int i = 0; i <= m_sectors; i++)
        {
            float x, y, nx, ny;

            if (m_turbo)
            {
                // Turbo mode: reuse x, y and normals, only z changes
                x = baseX[i];
                y = baseY[i];
                nx = baseNx[i];
                ny = baseNy[i];
            }
            else
            {
                // Non-turbo: full recalculation
                theta = m_domainStart + (m_domainEnd - m_domainStart) * i / m_sectors;

                float r = static_cast<float>(expr_r->eval());
                float dr = static_cast<float>(expr_dr->eval());

                x = static_cast<float>(expr_r->cyl_x(theta));
                y = static_cast<float>(expr_r->cyl_y(theta));

                float cos_t = std::cos(static_cast<float>(theta));
                float sin_t = std::sin(static_cast<float>(theta));
                nx = dr * sin_t + r * cos_t;
                ny = -(dr * cos_t - r * sin_t);

                // Normalize
                float len = std::sqrt(nx * nx + ny * ny);
                if (len > 0.0001f)
                {
                    nx /= len;
                    ny /= len;
                }
            }

            currRing[i] = addVertex(x, y, z, nx, ny, 0.0f);
        }

        // Generate quads between prevRing and currRing
        for (int i = 0; i < m_sectors; i++)
        {
            uint32_t v00 = prevRing[i];
            uint32_t v01 = prevRing[i + 1];
            uint32_t v10 = currRing[i];
            uint32_t v11 = currRing[i + 1];

            // Triangle 1
            indices.push_back(v00);
            indices.push_back(v10);
            indices.push_back(v01);

            // Triangle 2
            indices.push_back(v01);
            indices.push_back(v10);
            indices.push_back(v11);
        }

        prevRing = currRing;
    }

    return *this;
}

PolarBuilder& PolarBuilder::buildCylinder(std::vector<float>& verts, std::vector<float>& norms)
{
    std::vector<float> indexedVerts, indexedNorms;
    std::vector<uint32_t> indices;

    buildCylinderIndexed(indexedVerts, indexedNorms, indices);

    size_t additionalSize = indices.size() * 3;
    verts.reserve(verts.size() + additionalSize);
    norms.reserve(norms.size() + additionalSize);

    for (uint32_t idx : indices)
    {
        size_t offset = idx * 3;
        verts.push_back(indexedVerts[offset]);
        verts.push_back(indexedVerts[offset + 1]);
        verts.push_back(indexedVerts[offset + 2]);

        norms.push_back(indexedNorms[offset]);
        norms.push_back(indexedNorms[offset + 1]);
        norms.push_back(indexedNorms[offset + 2]);
    }

    return *this;
}

PolarBuilder& PolarBuilder::reBuildCylinder(std::vector<float>& verts, std::vector<float>& norms)
{
    verts.clear();
    norms.clear();
    return buildCylinder(verts, norms);
}

PolarBuilder& PolarBuilder::reBuildCylinderIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<uint32_t>& indices)
{
    verts.clear();
    norms.clear();
    indices.clear();
    return buildCylinderIndexed(verts, norms, indices);
}

PolarBuilder Builder::polar()
{
    return PolarBuilder();
}

} // namespace dynamit::builders

