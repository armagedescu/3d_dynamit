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
    return static_cast<int>(m_shapes.size() - 1);
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

    // Clear existing data
    shape->verts.clear();
    shape->norms.clear();
    shape->colors.clear();
    shape->indices.clear();

    // Build based on type
    if (shape->config.type == ShapeConfig::Type::Cone)
    {
        buildCone(*shape);
    }
    else
    {
        buildCylinder(*shape);
    }

    // Setup Dynamit renderer with auto-generated shaders
    setupDynamitRenderer(*shape);

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

void ShapeManager::buildCone(ShapeInstance& shape)
{
    const ShapeConfig& cfg = shape.config;

    PolarBuilder builder = Builder::polar();

    builder.formula(cfg.formula)
        .domain(cfg.domainStart, cfg.domainEnd)
        .sectors(cfg.sectors)
        .slices(cfg.slices)
        .smooth(cfg.smooth)
        .turbo(cfg.turbo)
        .doubleCoated(cfg.doubleCoated)
        .reversed(cfg.reversed)
        .color(cfg.outerColor, cfg.innerColor);

    builder.buildConeIndexedWithColor(shape.verts, shape.norms, shape.colors, shape.indices);

    std::cout << "Built cone: " << shape.verts.size() / 3 << " vertices, "
              << shape.indices.size() / 3 << " triangles" << std::endl;
}

void ShapeManager::buildCylinder(ShapeInstance& shape)
{
    const ShapeConfig& cfg = shape.config;

    PolarBuilder builder = Builder::polar();

    builder.formula(cfg.formula)
        .domain(cfg.domainStart, cfg.domainEnd)
        .sectors(cfg.sectors)
        .slices(cfg.slices)
        .smooth(cfg.smooth)
        .turbo(cfg.turbo)
        .doubleCoated(cfg.doubleCoated)
        .reversed(cfg.reversed)
        .color(cfg.outerColor, cfg.innerColor);

    builder.buildCylinderIndexedWithColor(shape.verts, shape.norms, shape.colors, shape.indices);

    std::cout << "Built cylinder: " << shape.verts.size() / 3 << " vertices, "
              << shape.indices.size() / 3 << " triangles" << std::endl;
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
