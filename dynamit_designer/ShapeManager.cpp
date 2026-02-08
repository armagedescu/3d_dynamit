#include "ShapeManager.h"
#include <builders.h>
#include <geometry.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

using namespace dynamit::builders;
using namespace dynamit::geo;
using namespace dynamit;

ShapeManager::ShapeManager()
{
}

ShapeManager::~ShapeManager()
{
    clearAll();
}

int ShapeManager::addShape(const ShapeConfig& config)
{
    ShapeInstance shape;
    shape.config = config;
    shape.dirty = true;

    m_shapes.push_back(std::move(shape));
    int index = static_cast<int>(m_shapes.size() - 1);
    std::cout << "[ShapeManager] Added shape '" << config.name << "', total count: " << m_shapes.size() << std::endl;
    return index;
}

void ShapeManager::removeShape(int index)
{
    if (index >= 0 && index < static_cast<int>(m_shapes.size()))
    {
        m_shapes.erase(m_shapes.begin() + index);
    }
}

void ShapeManager::clearAll()
{
    m_shapes.clear();
}

ShapeInstance* ShapeManager::getShape(int index)
{
    if (index >= 0 && index < static_cast<int>(m_shapes.size()))
    {
        return &m_shapes[index];
    }
    return nullptr;
}

const ShapeInstance* ShapeManager::getShape(int index) const
{
    if (index >= 0 && index < static_cast<int>(m_shapes.size()))
    {
        return &m_shapes[index];
    }
    return nullptr;
}

void ShapeManager::rebuildShape(int index)
{
    ShapeInstance* shape = getShape(index);
    if (!shape)
        return;

    // Try to build - if formula is invalid, keep existing geometry
    std::vector<float> newVerts, newNorms, newColors;
    std::vector<uint32_t> newIndices;

    try
    {
        // Build based on type
        if (shape->config.type == ShapeConfig::Type::Cone)
        {
            buildConeData(shape->config, newVerts, newNorms, newColors, newIndices);
        }
        else
        {
            buildCylinderData(shape->config, newVerts, newNorms, newColors, newIndices);
        }

        // Success - update shape data
        shape->verts = std::move(newVerts);
        shape->norms = std::move(newNorms);
        shape->colors = std::move(newColors);
        shape->indices = std::move(newIndices);

        // Setup Dynamit renderer with auto-generated shaders
        setupDynamitRenderer(*shape);

        shape->lastError.clear();
    }
    catch (const std::exception& e)
    {
        // Formula parsing failed - keep existing geometry, log error
        shape->lastError = e.what();
        std::cerr << "Formula error: " << e.what() << std::endl;
    }
    catch (...)
    {
        // Unknown error - keep existing geometry
        shape->lastError = "Unknown error parsing formula";
        std::cerr << "Unknown formula error" << std::endl;
    }

    shape->dirty = false;
}

void ShapeManager::rebuildAllDirty()
{
    for (int i = 0; i < static_cast<int>(m_shapes.size()); ++i)
    {
        if (m_shapes[i].dirty)
        {
            rebuildShape(i);
        }
    }
}

void ShapeManager::swapShapes(int indexA, int indexB)
{
    if (indexA >= 0 && indexA < static_cast<int>(m_shapes.size()) &&
        indexB >= 0 && indexB < static_cast<int>(m_shapes.size()) &&
        indexA != indexB)
    {
        std::swap(m_shapes[indexA], m_shapes[indexB]);
    }
}

bool ShapeManager::weldAllShapes()
{
    if (m_shapes.size() < 2)
        return false;

    // Create a new welded shape
    ShapeInstance welded;
    welded.config.name = "Welded Shape";
    welded.config.type = m_shapes[0].config.type;
    welded.config.segments.clear();

    // Combine all geometry from all shapes
    for (size_t i = 0; i < m_shapes.size(); ++i)
    {
        ShapeInstance& shape = m_shapes[i];

        // Rebuild if needed
        if (shape.dirty)
        {
            rebuildShape(static_cast<int>(i));
        }

        // Get transform matrix for this shape
        std::array<float, 16> transform = getTransformMatrix(static_cast<int>(i));

        // Apply transform to vertices and append
        uint32_t indexOffset = static_cast<uint32_t>(welded.verts.size() / 3);

        for (size_t v = 0; v < shape.verts.size(); v += 3)
        {
            float x = shape.verts[v];
            float y = shape.verts[v + 1];
            float z = shape.verts[v + 2];

            // Apply 4x4 transform matrix
            float tx = transform[0] * x + transform[4] * y + transform[8] * z + transform[12];
            float ty = transform[1] * x + transform[5] * y + transform[9] * z + transform[13];
            float tz = transform[2] * x + transform[6] * y + transform[10] * z + transform[14];

            welded.verts.push_back(tx);
            welded.verts.push_back(ty);
            welded.verts.push_back(tz);
        }

        // Transform normals (use 3x3 rotation part of matrix, no translation)
        for (size_t n = 0; n < shape.norms.size(); n += 3)
        {
            float nx = shape.norms[n];
            float ny = shape.norms[n + 1];
            float nz = shape.norms[n + 2];

            // Apply rotation only (3x3 upper-left of transform)
            float tnx = transform[0] * nx + transform[4] * ny + transform[8] * nz;
            float tny = transform[1] * nx + transform[5] * ny + transform[9] * nz;
            float tnz = transform[2] * nx + transform[6] * ny + transform[10] * nz;

            // Re-normalize
            float len = std::sqrt(tnx * tnx + tny * tny + tnz * tnz);
            if (len > 0.0001f)
            {
                tnx /= len;
                tny /= len;
                tnz /= len;
            }

            welded.norms.push_back(tnx);
            welded.norms.push_back(tny);
            welded.norms.push_back(tnz);
        }

        // Copy colors directly
        welded.colors.insert(welded.colors.end(), shape.colors.begin(), shape.colors.end());

        // Adjust indices and append
        for (uint32_t idx : shape.indices)
        {
            welded.indices.push_back(idx + indexOffset);
        }

        // Collect segments for reference (though the welded shape won't use formulas)
        auto segs = shape.config.getEffectiveSegments();
        welded.config.segments.insert(welded.config.segments.end(), segs.begin(), segs.end());
    }

    // Clear all shapes and add the welded one
    m_shapes.clear();

    // Set up the welded shape's renderer
    welded.dirty = false;
    welded.visible = true;
    setupDynamitRenderer(welded);

    m_shapes.push_back(std::move(welded));

    std::cout << "Welded all shapes: " << welded.verts.size() / 3 << " vertices, "
              << welded.indices.size() / 3 << " triangles" << std::endl;

    return true;
}

void ShapeManager::buildConeData(const ShapeConfig& cfg,
    std::vector<float>& verts, std::vector<float>& norms,
    std::vector<float>& colors, std::vector<uint32_t>& indices)
{
    verts.clear();
    norms.clear();
    colors.clear();
    indices.clear();

    auto segments = cfg.getEffectiveSegments();

    for (const auto& seg : segments)
    {
        PolarBuilder builder = Builder::polar();

        builder.formula(seg.formula)
            .domain(seg.domainStart, seg.domainEnd)
            .sectors(cfg.sectors)
            .slices(cfg.slices)
            .smooth(cfg.smooth)
            .turbo(cfg.turbo)
            .doubleCoated(cfg.doubleCoated)
            .reversed(cfg.reversed)
            .color(cfg.outerColor, cfg.innerColor);

        std::vector<float> segVerts, segNorms, segColors;
        std::vector<uint32_t> segIndices;
        builder.buildConeIndexedWithColor(segVerts, segNorms, segColors, segIndices);

        // Adjust indices for concatenation
        uint32_t indexOffset = static_cast<uint32_t>(verts.size() / 3);
        for (auto& idx : segIndices)
        {
            idx += indexOffset;
        }

        // Append segment data
        verts.insert(verts.end(), segVerts.begin(), segVerts.end());
        norms.insert(norms.end(), segNorms.begin(), segNorms.end());
        colors.insert(colors.end(), segColors.begin(), segColors.end());
        indices.insert(indices.end(), segIndices.begin(), segIndices.end());
    }

    std::cout << "Built cone (" << segments.size() << " segments): "
              << verts.size() / 3 << " vertices, "
              << indices.size() / 3 << " triangles" << std::endl;
}

void ShapeManager::buildCylinderData(const ShapeConfig& cfg,
    std::vector<float>& verts, std::vector<float>& norms,
    std::vector<float>& colors, std::vector<uint32_t>& indices)
{
    verts.clear();
    norms.clear();
    colors.clear();
    indices.clear();

    auto segments = cfg.getEffectiveSegments();

    for (const auto& seg : segments)
    {
        PolarBuilder builder = Builder::polar();

        builder.formula(seg.formula)
            .domain(seg.domainStart, seg.domainEnd)
            .sectors(cfg.sectors)
            .slices(cfg.slices)
            .smooth(cfg.smooth)
            .turbo(cfg.turbo)
            .doubleCoated(cfg.doubleCoated)
            .reversed(cfg.reversed)
            .color(cfg.outerColor, cfg.innerColor);

        std::vector<float> segVerts, segNorms, segColors;
        std::vector<uint32_t> segIndices;
        builder.buildCylinderIndexedWithColor(segVerts, segNorms, segColors, segIndices);

        // Adjust indices for concatenation
        uint32_t indexOffset = static_cast<uint32_t>(verts.size() / 3);
        for (auto& idx : segIndices)
        {
            idx += indexOffset;
        }

        // Append segment data
        verts.insert(verts.end(), segVerts.begin(), segVerts.end());
        norms.insert(norms.end(), segNorms.begin(), segNorms.end());
        colors.insert(colors.end(), segColors.begin(), segColors.end());
        indices.insert(indices.end(), segIndices.begin(), segIndices.end());
    }

    std::cout << "Built cylinder (" << segments.size() << " segments): "
              << verts.size() / 3 << " vertices, "
              << indices.size() / 3 << " triangles" << std::endl;
}

void ShapeManager::setupDynamitRenderer(ShapeInstance& shape)
{
    // Create new Dynamit instance - it auto-generates shaders!
    shape.renderer = std::make_unique<Dynamit>();

    // Configure with our data - shaders are auto-generated based on what we provide
    shape.renderer->withVertices3d(shape.verts)
                  .withNormals3d(shape.norms)
                  .withColors4d(shape.colors)
                  .withIndices(shape.indices)
                  .withConstLightDirection({ -0.577f, -0.577f, 0.577f })
                  .withTransformMatrix4f("transformMatrix");

    // Create NormalsHighlighter for visualizing normals
    shape.normalsHighlighter = shape.renderer->createNormalsHighlighter(0.05f);
    shape.normalsHighlighter->build(shape.verts, shape.norms);

    // Log auto-generated shaders (for debugging)
    std::cout << "Auto-generated shaders for " << shape.config.name << ":" << std::endl;
    shape.renderer->logShaders();
}

std::array<float, 16> ShapeManager::getTransformMatrix(int index) const
{
    const ShapeInstance* shape = getShape(index);
    if (!shape)
    {
        return identity_mat4<float>();
    }

    const ShapeConfig& cfg = shape->config;

    // Build transform: scale -> rotate -> translate
    glm::mat4 transform = glm::mat4(1.0f);

    // Translation
    transform = glm::translate(transform, glm::vec3(cfg.posX, cfg.posY, cfg.posZ));

    // Rotation (in degrees)
    transform = glm::rotate(transform, glm::radians(cfg.rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    transform = glm::rotate(transform, glm::radians(cfg.rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    transform = glm::rotate(transform, glm::radians(cfg.rotZ), glm::vec3(0.0f, 0.0f, 1.0f));

    // Scale
    transform = glm::scale(transform, glm::vec3(cfg.scaleX, cfg.scaleY, cfg.scaleZ));

    std::array<float, 16> result;
    memcpy(result.data(), glm::value_ptr(transform), 16 * sizeof(float));
    return result;
}

void ShapeManager::render(const std::array<float, 16>& viewProjection, bool showNormals)
{
    for (int i = 0; i < static_cast<int>(m_shapes.size()); ++i)
    {
        ShapeInstance& shape = m_shapes[i];

        if (!shape.visible || !shape.renderer || shape.indices.empty())
            continue;

        // Compute MVP = viewProjection * model
        std::array<float, 16> modelMatrix = getTransformMatrix(i);

        // Multiply viewProjection * modelMatrix
        glm::mat4 vp = glm::make_mat4(viewProjection.data());
        glm::mat4 model = glm::make_mat4(modelMatrix.data());
        glm::mat4 mvp = vp * model;

        std::array<float, 16> mvpArray;
        memcpy(mvpArray.data(), glm::value_ptr(mvp), 16 * sizeof(float));

        // Set transform and draw - Dynamit handles everything!
        shape.renderer->transformMatrix4f(mvpArray);
        shape.renderer->drawTrianglesIndexed();

        // Draw normals if enabled
        if (showNormals && shape.normalsHighlighter)
        {
            shape.normalsHighlighter->draw(mvpArray);
        }
    }
}
