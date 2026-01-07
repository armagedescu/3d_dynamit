#include "pch.h"
#define _USE_MATH_DEFINES
#include "builders.h"

#include "expression_compiler.h"
#include <cmath>


namespace dynamit::builders
{

void buildConePolarIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<uint32_t>& indices,
    const std::wstring& formula,
    float domain_start,
    float domain_end,
    int nsectors,
    int nslices)
{
    using expresie_tokenizer::expression_token_compiler;
    using expresie_tokenizer::expression;
    expression_token_compiler compiler;

    verts.clear();
    norms.clear();
    indices.clear();

    long double theta = 0.0L;

    // Compile r(theta) expression
    std::unique_ptr<expression> expr_r = compiler.compile(formula);
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

    // Add tip vertex (index 0)
    verts.insert(verts.end(), { 0.0f, 0.0f, z_tip });
    norms.insert(norms.end(), { 0.0f, 0.0f, 0.0f });
    uint32_t tipIndex = 0;

    // Build first ring (base) with nsectors+1 vertices to include endpoint
    std::vector<uint32_t> baseRing(nsectors + 1);
    for (int i = 0; i <= nsectors; i++)
    {
        theta = domain_start + (domain_end - domain_start) * i / nsectors;

        float r = static_cast<float>(expr_r->eval());
        float dr = static_cast<float>(expr_dr->eval());

        float x = static_cast<float>(expr_r->cyl_x(theta)) / nslices;
        float y = static_cast<float>(expr_r->cyl_y(theta)) / nslices;

        float cos_t = std::cos(static_cast<float>(theta));
        float sin_t = std::sin(static_cast<float>(theta));
        float nx = dr * sin_t + r * cos_t;
        float ny = -(dr * cos_t - r * sin_t);
        float nz = z_tip;

        baseRing[i] = addVertex(x, y, 0.0f, nx, ny, nz);
    }

    // Generate tip triangles (fan from tip to base ring)
    for (int i = 0; i < nsectors; i++)
    {
        indices.push_back(tipIndex);
        indices.push_back(baseRing[i]);
        indices.push_back(baseRing[i + 1]);
    }

    // Build remaining rings and quads for slices
    if (nslices > 1)
    {
        std::vector<uint32_t> prevRing = baseRing;

        for (int h = 1; h < nslices; h++)
        {
            float h2n = static_cast<float>(h + 1) / nslices;

            std::vector<uint32_t> currRing(nsectors + 1);

            for (int i = 0; i <= nsectors; i++)
            {
                theta = domain_start + (domain_end - domain_start) * i / nsectors;

                float r = static_cast<float>(expr_r->eval());
                float dr = static_cast<float>(expr_dr->eval());

                float x = static_cast<float>(expr_r->cyl_x(theta)) * h2n;
                float y = static_cast<float>(expr_r->cyl_y(theta)) * h2n;

                float cos_t = std::cos(static_cast<float>(theta));
                float sin_t = std::sin(static_cast<float>(theta));
                float nx = dr * sin_t + r * cos_t;
                float ny = -(dr * cos_t - r * sin_t);
                float nz = -1.0f;

                // Normalize
                float len = std::sqrt(nx * nx + ny * ny + nz * nz);
                if (len > 0.0001f)
                {
                    nx /= len;
                    ny /= len;
                    nz /= len;
                }

                currRing[i] = addVertex(x, y, h2n, nx, ny, nz);
            }

            // Generate quads between prevRing and currRing
            for (int i = 0; i < nsectors; i++)
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
}

void buildConePolarIndexed(
    std::vector<float>& verts,
    std::vector<float>& norms,
    std::vector<uint32_t>& indices,
    const std::wstring& formula,
    int nsectors,
    int nslices)
{
    buildConePolarIndexed(verts, norms, indices, formula, 0.f, static_cast<float>(2 * M_PI), nsectors, nslices);
}

void buildConePolar(
    std::vector<float>& verts,
    std::vector<float>& norms,
    const std::wstring& formula,
    float domain_start,
    float domain_end,
    int nsectors,
    int nslices)
{
    std::vector<float> indexedVerts, indexedNorms;
    std::vector<uint32_t> indices;

    buildConePolarIndexed(indexedVerts, indexedNorms, indices, formula, domain_start, domain_end, nsectors, nslices);

    verts.clear();
    norms.clear();
    verts.reserve(indices.size() * 3);
    norms.reserve(indices.size() * 3);

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
}

void buildConePolar(
    std::vector<float>& verts,
    std::vector<float>& norms,
    const std::wstring& formula,
    int nsectors,
    int nslices)
{
    buildConePolar(verts, norms, formula, 0.f, static_cast<float>(2 * M_PI), nsectors, nslices);
}

} // namespace dynamit::builders

