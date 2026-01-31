#pragma once

#include <vector>
#include <array>
#include <memory>
#include <cmath>
#include <Dynamit.h>

using namespace dynamit;

// Renders XYZ axes, light direction, and reference grid
class VisualizationHelpers
{
public:
    VisualizationHelpers();

    void initialize();
    void render(const std::array<float, 16>& viewProjection);

    void setLightDirection(float x, float y, float z);

    // Individual axis visibility
    void setShowAxisX(bool show) { m_showAxisX = show; }
    void setShowAxisY(bool show) { m_showAxisY = show; }
    void setShowAxisZ(bool show) { m_showAxisZ = show; }
    bool getShowAxisX() const { return m_showAxisX; }
    bool getShowAxisY() const { return m_showAxisY; }
    bool getShowAxisZ() const { return m_showAxisZ; }

    // 3D Grid visibility
    void setShowGrid(bool show) { m_showGrid = show; }
    bool getShowGrid() const { return m_showGrid; }

    // 2D Grid visibility (off by default)
    void setShowGridXZ(bool show) { m_showGridXZ = show; }
    void setShowGridXY(bool show) { m_showGridXY = show; }
    void setShowGridYZ(bool show) { m_showGridYZ = show; }
    bool getShowGridXZ() const { return m_showGridXZ; }
    bool getShowGridXY() const { return m_showGridXY; }
    bool getShowGridYZ() const { return m_showGridYZ; }

    // Light direction visibility
    void setShowLight(bool show) { m_showLight = show; }
    bool getShowLight() const { return m_showLight; }

private:
    void buildAxes();
    void buildLightArrow();
    void buildGrid();
    void buildGrid2D();

    // Separate renderers for each axis (line + cone)
    std::unique_ptr<Dynamit> m_axisXRenderer;
    std::unique_ptr<Dynamit> m_axisYRenderer;
    std::unique_ptr<Dynamit> m_axisZRenderer;
    std::unique_ptr<Dynamit> m_coneXRenderer;
    std::unique_ptr<Dynamit> m_coneYRenderer;
    std::unique_ptr<Dynamit> m_coneZRenderer;

    std::unique_ptr<Dynamit> m_lightRenderer;
    std::unique_ptr<Dynamit> m_gridRenderer;

    std::vector<float> m_axisXVerts, m_axisXColors;
    std::vector<float> m_axisYVerts, m_axisYColors;
    std::vector<float> m_axisZVerts, m_axisZColors;
    std::vector<float> m_coneXVerts, m_coneXColors;
    std::vector<float> m_coneYVerts, m_coneYColors;
    std::vector<float> m_coneZVerts, m_coneZColors;

    std::vector<float> m_lightVerts;
    std::vector<float> m_lightColors;

    std::vector<float> m_gridVerts;
    std::vector<float> m_gridColors;

    // 2D grids
    std::unique_ptr<Dynamit> m_gridXZRenderer;
    std::unique_ptr<Dynamit> m_gridXYRenderer;
    std::unique_ptr<Dynamit> m_gridYZRenderer;
    std::vector<float> m_gridXZVerts, m_gridXZColors;
    std::vector<float> m_gridXYVerts, m_gridXYColors;
    std::vector<float> m_gridYZVerts, m_gridYZColors;

    std::array<float, 3> m_lightDir = { -0.577f, -0.577f, 0.577f };

    bool m_showAxisX = true;
    bool m_showAxisY = true;
    bool m_showAxisZ = true;
    bool m_showLight = true;
    bool m_showGrid = true;
    bool m_showGridXZ = false;  // 2D grids off by default
    bool m_showGridXY = false;
    bool m_showGridYZ = false;
    bool m_initialized = false;
};

// Simple line shader - no lighting, just vertex colors
static const char* lineVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec4 color;
out vec4 colorVary;
uniform mat4 transformMatrix;
void main()
{
    gl_Position = transformMatrix * vec4(vertex, 1.0);
    colorVary = color;
}
)";

static const char* lineFragmentShader = R"(
#version 330 core
in vec4 colorVary;
out vec4 fragColor;
void main()
{
    fragColor = colorVary;
}
)";

inline VisualizationHelpers::VisualizationHelpers()
{
}

inline void VisualizationHelpers::initialize()
{
    if (m_initialized) return;

    buildAxes();
    buildLightArrow();
    buildGrid();
    buildGrid2D();

    m_initialized = true;
}

inline void VisualizationHelpers::buildAxes()
{
    // XYZ axes symmetric from -len to +len
    // Left-handed: X right, Y up, Z into screen
    float len = 2.0f;
    float coneLen = 0.15f;    // Length of cone tip
    float coneRad = 0.05f;    // Radius of cone base
    int coneSegs = 8;         // Cone segments

    // Helper to build cone geometry
    auto buildCone = [&](std::vector<float>& verts, std::vector<float>& colors,
                         float tipX, float tipY, float tipZ,
                         float dirX, float dirY, float dirZ,
                         float r, float g, float b) {
        // Find perpendicular vectors
        float perpX1, perpY1, perpZ1, perpX2, perpY2, perpZ2;
        if (std::abs(dirX) < 0.9f) {
            perpX1 = 0; perpY1 = -dirZ; perpZ1 = dirY;
        } else {
            perpX1 = -dirZ; perpY1 = 0; perpZ1 = dirX;
        }
        float len1 = std::sqrt(perpX1*perpX1 + perpY1*perpY1 + perpZ1*perpZ1);
        if (len1 > 0.0001f) { perpX1 /= len1; perpY1 /= len1; perpZ1 /= len1; }
        // Cross product for second perpendicular
        perpX2 = dirY * perpZ1 - dirZ * perpY1;
        perpY2 = dirZ * perpX1 - dirX * perpZ1;
        perpZ2 = dirX * perpY1 - dirY * perpX1;

        // Base center
        float baseX = tipX - dirX * coneLen;
        float baseY = tipY - dirY * coneLen;
        float baseZ = tipZ - dirZ * coneLen;

        // Generate cone triangles
        for (int i = 0; i < coneSegs; i++) {
            float angle1 = 2.0f * 3.14159f * i / coneSegs;
            float angle2 = 2.0f * 3.14159f * (i + 1) / coneSegs;
            float c1 = std::cos(angle1), s1 = std::sin(angle1);
            float c2 = std::cos(angle2), s2 = std::sin(angle2);

            float bx1 = baseX + coneRad * (c1 * perpX1 + s1 * perpX2);
            float by1 = baseY + coneRad * (c1 * perpY1 + s1 * perpY2);
            float bz1 = baseZ + coneRad * (c1 * perpZ1 + s1 * perpZ2);
            float bx2 = baseX + coneRad * (c2 * perpX1 + s2 * perpX2);
            float by2 = baseY + coneRad * (c2 * perpY1 + s2 * perpY2);
            float bz2 = baseZ + coneRad * (c2 * perpZ1 + s2 * perpZ2);

            verts.insert(verts.end(), { tipX, tipY, tipZ, bx1, by1, bz1, bx2, by2, bz2 });
            for (int j = 0; j < 3; j++)
                colors.insert(colors.end(), { r, g, b, 1.0f });
        }
    };

    // X axis (red)
    m_axisXVerts = { -len, 0.0f, 0.0f,  len, 0.0f, 0.0f };
    m_axisXColors = { 1.0f, 0.0f, 0.0f, 1.0f,  1.0f, 0.0f, 0.0f, 1.0f };
    m_coneXVerts.clear(); m_coneXColors.clear();
    buildCone(m_coneXVerts, m_coneXColors, len, 0, 0, 1, 0, 0, 1.0f, 0.0f, 0.0f);

    m_axisXRenderer = std::make_unique<Dynamit>();
    m_axisXRenderer->withVertices3d(m_axisXVerts).withColors4d(m_axisXColors)
                   .withTransformMatrix4f("transformMatrix")
                   .withShaderSources(lineVertexShader, lineFragmentShader);
    m_coneXRenderer = std::make_unique<Dynamit>();
    m_coneXRenderer->withVertices3d(m_coneXVerts).withColors4d(m_coneXColors)
                   .withTransformMatrix4f("transformMatrix")
                   .withShaderSources(lineVertexShader, lineFragmentShader);

    // Y axis (green)
    m_axisYVerts = { 0.0f, -len, 0.0f,  0.0f, len, 0.0f };
    m_axisYColors = { 0.0f, 1.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 1.0f };
    m_coneYVerts.clear(); m_coneYColors.clear();
    buildCone(m_coneYVerts, m_coneYColors, 0, len, 0, 0, 1, 0, 0.0f, 1.0f, 0.0f);

    m_axisYRenderer = std::make_unique<Dynamit>();
    m_axisYRenderer->withVertices3d(m_axisYVerts).withColors4d(m_axisYColors)
                   .withTransformMatrix4f("transformMatrix")
                   .withShaderSources(lineVertexShader, lineFragmentShader);
    m_coneYRenderer = std::make_unique<Dynamit>();
    m_coneYRenderer->withVertices3d(m_coneYVerts).withColors4d(m_coneYColors)
                   .withTransformMatrix4f("transformMatrix")
                   .withShaderSources(lineVertexShader, lineFragmentShader);

    // Z axis (blue)
    m_axisZVerts = { 0.0f, 0.0f, -len,  0.0f, 0.0f, len };
    m_axisZColors = { 0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 0.0f, 1.0f, 1.0f };
    m_coneZVerts.clear(); m_coneZColors.clear();
    buildCone(m_coneZVerts, m_coneZColors, 0, 0, len, 0, 0, 1, 0.0f, 0.0f, 1.0f);

    m_axisZRenderer = std::make_unique<Dynamit>();
    m_axisZRenderer->withVertices3d(m_axisZVerts).withColors4d(m_axisZColors)
                   .withTransformMatrix4f("transformMatrix")
                   .withShaderSources(lineVertexShader, lineFragmentShader);
    m_coneZRenderer = std::make_unique<Dynamit>();
    m_coneZRenderer->withVertices3d(m_coneZVerts).withColors4d(m_coneZColors)
                   .withTransformMatrix4f("transformMatrix")
                   .withShaderSources(lineVertexShader, lineFragmentShader);
}

inline void VisualizationHelpers::buildLightArrow()
{
    // Light direction arrow from origin
    // Normalize and scale for visibility
    float scale = 1.5f;
    float lx = -m_lightDir[0] * scale;  // Negate to show direction light comes FROM
    float ly = -m_lightDir[1] * scale;
    float lz = -m_lightDir[2] * scale;

    m_lightVerts = {
        0.0f, 0.0f, 0.0f,  lx, ly, lz,
    };

    // Yellow for light
    m_lightColors = {
        1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 0.0f, 1.0f,
    };

    m_lightRenderer = std::make_unique<Dynamit>();
    m_lightRenderer->withVertices3d(m_lightVerts)
                   .withColors4d(m_lightColors)
                   .withTransformMatrix4f("transformMatrix")
                   .withShaderSources(lineVertexShader, lineFragmentShader);
}

inline void VisualizationHelpers::buildGrid()
{
    // 3D cube grid from -1 to 1 on all axes
    float minVal = -1.0f;
    float maxVal = 1.0f;
    int divisions = 4;
    float step = (maxVal - minVal) / divisions;

    m_gridVerts.clear();
    m_gridColors.clear();

    auto addLine = [&](float x1, float y1, float z1, float x2, float y2, float z2) {
        m_gridVerts.insert(m_gridVerts.end(), { x1, y1, z1, x2, y2, z2 });
        // Gray color
        for (int j = 0; j < 2; j++)
            m_gridColors.insert(m_gridColors.end(), { 0.4f, 0.4f, 0.4f, 1.0f });
    };

    // Lines parallel to X axis (on YZ grid)
    for (int iy = 0; iy <= divisions; iy++)
    {
        for (int iz = 0; iz <= divisions; iz++)
        {
            float y = minVal + iy * step;
            float z = minVal + iz * step;
            addLine(minVal, y, z, maxVal, y, z);
        }
    }

    // Lines parallel to Y axis (on XZ grid)
    for (int ix = 0; ix <= divisions; ix++)
    {
        for (int iz = 0; iz <= divisions; iz++)
        {
            float x = minVal + ix * step;
            float z = minVal + iz * step;
            addLine(x, minVal, z, x, maxVal, z);
        }
    }

    // Lines parallel to Z axis (on XY grid)
    for (int ix = 0; ix <= divisions; ix++)
    {
        for (int iy = 0; iy <= divisions; iy++)
        {
            float x = minVal + ix * step;
            float y = minVal + iy * step;
            addLine(x, y, minVal, x, y, maxVal);
        }
    }

    m_gridRenderer = std::make_unique<Dynamit>();
    m_gridRenderer->withVertices3d(m_gridVerts)
                  .withColors4d(m_gridColors)
                  .withTransformMatrix4f("transformMatrix")
                  .withShaderSources(lineVertexShader, lineFragmentShader);
}

inline void VisualizationHelpers::buildGrid2D()
{
    // 2D grids from -1 to 1
    float minVal = -1.0f;
    float maxVal = 1.0f;
    int divisions = 4;
    float step = (maxVal - minVal) / divisions;

    auto build2DGrid = [&](std::vector<float>& verts, std::vector<float>& colors,
                           int axis1, int axis2, int fixedAxis) {
        verts.clear();
        colors.clear();

        auto addLine = [&](float x1, float y1, float z1, float x2, float y2, float z2) {
            verts.insert(verts.end(), { x1, y1, z1, x2, y2, z2 });
            for (int j = 0; j < 2; j++)
                colors.insert(colors.end(), { 0.5f, 0.5f, 0.5f, 0.6f });
        };

        // Lines parallel to axis1
        for (int i = 0; i <= divisions; i++)
        {
            float v = minVal + i * step;
            float p1[3] = {0, 0, 0};
            float p2[3] = {0, 0, 0};
            p1[axis2] = v;
            p2[axis2] = v;
            p1[axis1] = minVal;
            p2[axis1] = maxVal;
            addLine(p1[0], p1[1], p1[2], p2[0], p2[1], p2[2]);
        }

        // Lines parallel to axis2
        for (int i = 0; i <= divisions; i++)
        {
            float v = minVal + i * step;
            float p1[3] = {0, 0, 0};
            float p2[3] = {0, 0, 0};
            p1[axis1] = v;
            p2[axis1] = v;
            p1[axis2] = minVal;
            p2[axis2] = maxVal;
            addLine(p1[0], p1[1], p1[2], p2[0], p2[1], p2[2]);
        }
    };

    // XZ grid (Y=0 plane)
    build2DGrid(m_gridXZVerts, m_gridXZColors, 0, 2, 1);  // axis1=X, axis2=Z, fixed=Y
    m_gridXZRenderer = std::make_unique<Dynamit>();
    m_gridXZRenderer->withVertices3d(m_gridXZVerts).withColors4d(m_gridXZColors)
                    .withTransformMatrix4f("transformMatrix")
                    .withShaderSources(lineVertexShader, lineFragmentShader);

    // XY grid (Z=0 plane)
    build2DGrid(m_gridXYVerts, m_gridXYColors, 0, 1, 2);  // axis1=X, axis2=Y, fixed=Z
    m_gridXYRenderer = std::make_unique<Dynamit>();
    m_gridXYRenderer->withVertices3d(m_gridXYVerts).withColors4d(m_gridXYColors)
                    .withTransformMatrix4f("transformMatrix")
                    .withShaderSources(lineVertexShader, lineFragmentShader);

    // YZ grid (X=0 plane)
    build2DGrid(m_gridYZVerts, m_gridYZColors, 1, 2, 0);  // axis1=Y, axis2=Z, fixed=X
    m_gridYZRenderer = std::make_unique<Dynamit>();
    m_gridYZRenderer->withVertices3d(m_gridYZVerts).withColors4d(m_gridYZColors)
                    .withTransformMatrix4f("transformMatrix")
                    .withShaderSources(lineVertexShader, lineFragmentShader);
}

inline void VisualizationHelpers::setLightDirection(float x, float y, float z)
{
    m_lightDir = { x, y, z };
    if (m_initialized)
    {
        buildLightArrow();
    }
}

inline void VisualizationHelpers::render(const std::array<float, 16>& viewProjection)
{
    if (!m_initialized) return;

    glDisable(GL_CULL_FACE);  // Lines and small cones don't need culling

    // 3D cube grid
    if (m_showGrid && m_gridRenderer)
    {
        m_gridRenderer->transformMatrix4f(viewProjection);
        m_gridRenderer->drawArrays(GL_LINES, 0, static_cast<GLsizei>(m_gridVerts.size() / 3));
    }

    // 2D grids
    if (m_showGridXZ && m_gridXZRenderer)
    {
        m_gridXZRenderer->transformMatrix4f(viewProjection);
        m_gridXZRenderer->drawArrays(GL_LINES, 0, static_cast<GLsizei>(m_gridXZVerts.size() / 3));
    }
    if (m_showGridXY && m_gridXYRenderer)
    {
        m_gridXYRenderer->transformMatrix4f(viewProjection);
        m_gridXYRenderer->drawArrays(GL_LINES, 0, static_cast<GLsizei>(m_gridXYVerts.size() / 3));
    }
    if (m_showGridYZ && m_gridYZRenderer)
    {
        m_gridYZRenderer->transformMatrix4f(viewProjection);
        m_gridYZRenderer->drawArrays(GL_LINES, 0, static_cast<GLsizei>(m_gridYZVerts.size() / 3));
    }

    glLineWidth(2.0f);

    // X axis (red)
    if (m_showAxisX)
    {
        if (m_axisXRenderer)
        {
            m_axisXRenderer->transformMatrix4f(viewProjection);
            m_axisXRenderer->drawArrays(GL_LINES, 0, 2);
        }
        if (m_coneXRenderer)
        {
            m_coneXRenderer->transformMatrix4f(viewProjection);
            m_coneXRenderer->drawTriangles(0);
        }
    }

    // Y axis (green)
    if (m_showAxisY)
    {
        if (m_axisYRenderer)
        {
            m_axisYRenderer->transformMatrix4f(viewProjection);
            m_axisYRenderer->drawArrays(GL_LINES, 0, 2);
        }
        if (m_coneYRenderer)
        {
            m_coneYRenderer->transformMatrix4f(viewProjection);
            m_coneYRenderer->drawTriangles(0);
        }
    }

    // Z axis (blue)
    if (m_showAxisZ)
    {
        if (m_axisZRenderer)
        {
            m_axisZRenderer->transformMatrix4f(viewProjection);
            m_axisZRenderer->drawArrays(GL_LINES, 0, 2);
        }
        if (m_coneZRenderer)
        {
            m_coneZRenderer->transformMatrix4f(viewProjection);
            m_coneZRenderer->drawTriangles(0);
        }
    }

    glLineWidth(1.0f);

    // Light direction (yellow)
    if (m_showLight && m_lightRenderer)
    {
        glLineWidth(3.0f);
        m_lightRenderer->transformMatrix4f(viewProjection);
        m_lightRenderer->drawArrays(GL_LINES, 0, static_cast<GLsizei>(m_lightVerts.size() / 3));
        glLineWidth(1.0f);
    }

    glEnable(GL_CULL_FACE);
}
