#pragma once

#include <vector>
#include <array>
#include <memory>
#include <cmath>
#include <Dynamit.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace dynamit;

// Simple line shader (same as VisualizationHelpers)
static const char* gizmoVertexShader = R"(
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

static const char* gizmoFragmentShader = R"(
#version 330 core
in vec4 colorVary;
out vec4 fragColor;
void main()
{
    fragColor = colorVary;
}
)";

class TransformGizmo
{
public:
    enum class Mode { Translate, Rotate, Scale };
    enum class Axis { None, X, Y, Z };

    void initialize();
    void render(const std::array<float, 16>& viewProjection, const glm::vec3& shapePos);

    // Hit test: returns which axis handle the ray hits (or None)
    Axis hitTest(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& shapePos);

    // Drag interaction — returns delta interpreted per mode:
    //   Translate: position delta (world units)
    //   Rotate: angle delta (degrees)
    //   Scale: scale delta (additive)
    void beginDrag(Axis axis, const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& shapePos);
    glm::vec3 updateDrag(const glm::vec3& rayOrigin, const glm::vec3& rayDir);
    void endDrag();

    bool isDragging() const { return m_dragging; }
    Axis getActiveAxis() const { return m_activeAxis; }
    Mode getMode() const { return m_mode; }
    void setMode(Mode mode) { m_mode = mode; }

private:
    void buildTranslateHandles();
    void buildRotateHandles();
    void buildScaleHandles();

    // Hit test per mode
    Axis hitTestTranslate(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& shapePos);
    Axis hitTestRotate(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& shapePos);
    Axis hitTestScale(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const glm::vec3& shapePos);

    // Closest point on a line to a ray (returns parameter t on the line)
    static float closestPointOnLine(const glm::vec3& lineOrigin, const glm::vec3& lineDir,
                                     const glm::vec3& rayOrigin, const glm::vec3& rayDir);

    // Intersect ray with plane, returns hit point. Returns false if parallel.
    static bool rayPlaneIntersect(const glm::vec3& planePoint, const glm::vec3& planeNormal,
                                   const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                   glm::vec3& hitPoint);

    // Compute angle in rotation plane for a given axis
    float computeRotationAngle(Axis axis, const glm::vec3& point, const glm::vec3& center);

    // Get axis direction vector
    static glm::vec3 axisDir(Axis axis);

    Mode m_mode = Mode::Translate;

    // ─── Translation handles ───
    std::unique_ptr<Dynamit> m_arrowXLine, m_arrowYLine, m_arrowZLine;
    std::unique_ptr<Dynamit> m_arrowXCone, m_arrowYCone, m_arrowZCone;
    std::vector<float> m_lineXVerts, m_lineXColors;
    std::vector<float> m_lineYVerts, m_lineYColors;
    std::vector<float> m_lineZVerts, m_lineZColors;
    std::vector<float> m_coneXVerts, m_coneXColors;
    std::vector<float> m_coneYVerts, m_coneYColors;
    std::vector<float> m_coneZVerts, m_coneZColors;

    // ─── Rotation handles ───
    std::unique_ptr<Dynamit> m_ringX, m_ringY, m_ringZ;
    std::vector<float> m_ringXVerts, m_ringXColors;
    std::vector<float> m_ringYVerts, m_ringYColors;
    std::vector<float> m_ringZVerts, m_ringZColors;

    // ─── Scale handles ───
    std::unique_ptr<Dynamit> m_scaleXLine, m_scaleYLine, m_scaleZLine;
    std::unique_ptr<Dynamit> m_scaleXCube, m_scaleYCube, m_scaleZCube;
    std::vector<float> m_scaleLineXVerts, m_scaleLineXColors;
    std::vector<float> m_scaleLineYVerts, m_scaleLineYColors;
    std::vector<float> m_scaleLineZVerts, m_scaleLineZColors;
    std::vector<float> m_cubeXVerts, m_cubeXColors;
    std::vector<float> m_cubeYVerts, m_cubeYColors;
    std::vector<float> m_cubeZVerts, m_cubeZColors;

    // ─── Drag state ───
    Axis m_activeAxis = Axis::None;
    bool m_dragging = false;
    float m_dragStartT = 0.0f;        // For translate/scale: parameter on axis
    float m_dragStartAngle = 0.0f;    // For rotate: angle in plane (radians)
    glm::vec3 m_dragShapePos;

    bool m_initialized = false;

    // ─── Constants ───
    static constexpr float ARROW_LEN = 0.5f;
    static constexpr float CONE_LEN = 0.12f;
    static constexpr float CONE_RAD = 0.04f;
    static constexpr int CONE_SEGS = 8;
    static constexpr float HIT_THRESHOLD = 0.08f;

    static constexpr float RING_RADIUS = 0.45f;
    static constexpr int RING_SEGS = 48;
    static constexpr float RING_HIT_THRESHOLD = 0.06f;

    static constexpr float SCALE_LINE_LEN = 0.4f;
    static constexpr float CUBE_HALF = 0.035f;
};

// ═══════════════════════════════════════════════════════════════
//  Implementation
// ═══════════════════════════════════════════════════════════════

inline glm::vec3 TransformGizmo::axisDir(Axis axis)
{
    switch (axis) {
        case Axis::X: return glm::vec3(1, 0, 0);
        case Axis::Y: return glm::vec3(0, 1, 0);
        case Axis::Z: return glm::vec3(0, 0, 1);
        default: return glm::vec3(0);
    }
}

inline void TransformGizmo::initialize()
{
    if (m_initialized) return;
    buildTranslateHandles();
    buildRotateHandles();
    buildScaleHandles();
    m_initialized = true;
}

// ─── Translate handles (arrows with cones) ─────────────────────

inline void TransformGizmo::buildTranslateHandles()
{
    auto buildCone = [&](std::vector<float>& verts, std::vector<float>& colors,
                         float tipX, float tipY, float tipZ,
                         float dirX, float dirY, float dirZ,
                         float r, float g, float b) {
        float perpX1, perpY1, perpZ1;
        if (std::abs(dirX) < 0.9f) {
            perpX1 = 0; perpY1 = -dirZ; perpZ1 = dirY;
        } else {
            perpX1 = -dirZ; perpY1 = 0; perpZ1 = dirX;
        }
        float len1 = std::sqrt(perpX1*perpX1 + perpY1*perpY1 + perpZ1*perpZ1);
        if (len1 > 0.0001f) { perpX1 /= len1; perpY1 /= len1; perpZ1 /= len1; }

        float perpX2 = dirY * perpZ1 - dirZ * perpY1;
        float perpY2 = dirZ * perpX1 - dirX * perpZ1;
        float perpZ2 = dirX * perpY1 - dirY * perpX1;

        float baseX = tipX - dirX * CONE_LEN;
        float baseY = tipY - dirY * CONE_LEN;
        float baseZ = tipZ - dirZ * CONE_LEN;

        for (int i = 0; i < CONE_SEGS; i++) {
            float angle1 = 2.0f * 3.14159f * i / CONE_SEGS;
            float angle2 = 2.0f * 3.14159f * (i + 1) / CONE_SEGS;
            float c1 = std::cos(angle1), s1 = std::sin(angle1);
            float c2 = std::cos(angle2), s2 = std::sin(angle2);

            float bx1 = baseX + CONE_RAD * (c1 * perpX1 + s1 * perpX2);
            float by1 = baseY + CONE_RAD * (c1 * perpY1 + s1 * perpY2);
            float bz1 = baseZ + CONE_RAD * (c1 * perpZ1 + s1 * perpZ2);
            float bx2 = baseX + CONE_RAD * (c2 * perpX1 + s2 * perpX2);
            float by2 = baseY + CONE_RAD * (c2 * perpY1 + s2 * perpY2);
            float bz2 = baseZ + CONE_RAD * (c2 * perpZ1 + s2 * perpZ2);

            verts.insert(verts.end(), { tipX, tipY, tipZ, bx1, by1, bz1, bx2, by2, bz2 });
            for (int j = 0; j < 3; j++)
                colors.insert(colors.end(), { r, g, b, 1.0f });
        }
    };

    auto makeLine = [&](std::unique_ptr<Dynamit>& renderer, std::vector<float>& verts, std::vector<float>& colors) {
        renderer = std::make_unique<Dynamit>();
        renderer->withVertices3d(verts).withColors4d(colors)
                .withTransformMatrix4f("transformMatrix")
                .withShaderSources(gizmoVertexShader, gizmoFragmentShader);
    };
    auto makeTris = [&](std::unique_ptr<Dynamit>& renderer, std::vector<float>& verts, std::vector<float>& colors) {
        renderer = std::make_unique<Dynamit>();
        renderer->withVertices3d(verts).withColors4d(colors)
                .withTransformMatrix4f("transformMatrix")
                .withShaderSources(gizmoVertexShader, gizmoFragmentShader);
    };

    // X (red)
    m_lineXVerts = { 0,0,0, ARROW_LEN,0,0 };
    m_lineXColors = { 1,0,0,1, 1,0,0,1 };
    m_coneXVerts.clear(); m_coneXColors.clear();
    buildCone(m_coneXVerts, m_coneXColors, ARROW_LEN,0,0, 1,0,0, 1,0,0);
    makeLine(m_arrowXLine, m_lineXVerts, m_lineXColors);
    makeTris(m_arrowXCone, m_coneXVerts, m_coneXColors);

    // Y (green)
    m_lineYVerts = { 0,0,0, 0,ARROW_LEN,0 };
    m_lineYColors = { 0,1,0,1, 0,1,0,1 };
    m_coneYVerts.clear(); m_coneYColors.clear();
    buildCone(m_coneYVerts, m_coneYColors, 0,ARROW_LEN,0, 0,1,0, 0,1,0);
    makeLine(m_arrowYLine, m_lineYVerts, m_lineYColors);
    makeTris(m_arrowYCone, m_coneYVerts, m_coneYColors);

    // Z (blue)
    m_lineZVerts = { 0,0,0, 0,0,ARROW_LEN };
    m_lineZColors = { 0,0,1,1, 0,0,1,1 };
    m_coneZVerts.clear(); m_coneZColors.clear();
    buildCone(m_coneZVerts, m_coneZColors, 0,0,ARROW_LEN, 0,0,1, 0,0,1);
    makeLine(m_arrowZLine, m_lineZVerts, m_lineZColors);
    makeTris(m_arrowZCone, m_coneZVerts, m_coneZColors);
}

// ─── Rotate handles (circles) ──────────────────────────────────

inline void TransformGizmo::buildRotateHandles()
{
    auto buildRing = [&](std::vector<float>& verts, std::vector<float>& colors,
                         int planeAxis, float r, float g, float b) {
        verts.clear(); colors.clear();
        for (int i = 0; i <= RING_SEGS; i++) {
            float angle = 2.0f * 3.14159f * i / RING_SEGS;
            float ca = std::cos(angle), sa = std::sin(angle);
            float px = 0, py = 0, pz = 0;
            switch (planeAxis) {
                case 0: py = RING_RADIUS * ca; pz = RING_RADIUS * sa; break; // X ring: YZ plane
                case 1: pz = RING_RADIUS * ca; px = RING_RADIUS * sa; break; // Y ring: XZ plane
                case 2: px = RING_RADIUS * ca; py = RING_RADIUS * sa; break; // Z ring: XY plane
            }
            verts.insert(verts.end(), { px, py, pz });
            colors.insert(colors.end(), { r, g, b, 1.0f });
        }
    };

    auto makeRenderer = [&](std::unique_ptr<Dynamit>& renderer, std::vector<float>& verts, std::vector<float>& colors) {
        renderer = std::make_unique<Dynamit>();
        renderer->withVertices3d(verts).withColors4d(colors)
                .withTransformMatrix4f("transformMatrix")
                .withShaderSources(gizmoVertexShader, gizmoFragmentShader);
    };

    buildRing(m_ringXVerts, m_ringXColors, 0, 1, 0, 0);  // X: red
    buildRing(m_ringYVerts, m_ringYColors, 1, 0, 1, 0);  // Y: green
    buildRing(m_ringZVerts, m_ringZColors, 2, 0, 0, 1);  // Z: blue

    makeRenderer(m_ringX, m_ringXVerts, m_ringXColors);
    makeRenderer(m_ringY, m_ringYVerts, m_ringYColors);
    makeRenderer(m_ringZ, m_ringZVerts, m_ringZColors);
}

// ─── Scale handles (lines with cubes) ──────────────────────────

inline void TransformGizmo::buildScaleHandles()
{
    auto buildCube = [&](std::vector<float>& verts, std::vector<float>& colors,
                         float cx, float cy, float cz,
                         float r, float g, float b) {
        float h = CUBE_HALF;
        // 8 corners
        float c0[8][3] = {
            {cx-h, cy-h, cz-h}, {cx+h, cy-h, cz-h},
            {cx+h, cy+h, cz-h}, {cx-h, cy+h, cz-h},
            {cx-h, cy-h, cz+h}, {cx+h, cy-h, cz+h},
            {cx+h, cy+h, cz+h}, {cx-h, cy+h, cz+h}
        };
        // 6 faces × 2 triangles
        int tris[12][3] = {
            {0,2,1},{0,3,2},  // front
            {5,7,4},{5,6,7},  // back
            {4,3,0},{4,7,3},  // left
            {1,6,5},{1,2,6},  // right
            {3,6,2},{3,7,6},  // top
            {4,1,5},{4,0,1},  // bottom
        };
        verts.clear(); colors.clear();
        for (auto& t : tris) {
            for (int i = 0; i < 3; i++) {
                verts.insert(verts.end(), { c0[t[i]][0], c0[t[i]][1], c0[t[i]][2] });
                colors.insert(colors.end(), { r, g, b, 1.0f });
            }
        }
    };

    auto makeLine = [&](std::unique_ptr<Dynamit>& renderer, std::vector<float>& verts, std::vector<float>& colors) {
        renderer = std::make_unique<Dynamit>();
        renderer->withVertices3d(verts).withColors4d(colors)
                .withTransformMatrix4f("transformMatrix")
                .withShaderSources(gizmoVertexShader, gizmoFragmentShader);
    };
    auto makeTris = [&](std::unique_ptr<Dynamit>& renderer, std::vector<float>& verts, std::vector<float>& colors) {
        renderer = std::make_unique<Dynamit>();
        renderer->withVertices3d(verts).withColors4d(colors)
                .withTransformMatrix4f("transformMatrix")
                .withShaderSources(gizmoVertexShader, gizmoFragmentShader);
    };

    // X (red)
    m_scaleLineXVerts = { 0,0,0, SCALE_LINE_LEN,0,0 };
    m_scaleLineXColors = { 1,0,0,1, 1,0,0,1 };
    buildCube(m_cubeXVerts, m_cubeXColors, SCALE_LINE_LEN,0,0, 1,0,0);
    makeLine(m_scaleXLine, m_scaleLineXVerts, m_scaleLineXColors);
    makeTris(m_scaleXCube, m_cubeXVerts, m_cubeXColors);

    // Y (green)
    m_scaleLineYVerts = { 0,0,0, 0,SCALE_LINE_LEN,0 };
    m_scaleLineYColors = { 0,1,0,1, 0,1,0,1 };
    buildCube(m_cubeYVerts, m_cubeYColors, 0,SCALE_LINE_LEN,0, 0,1,0);
    makeLine(m_scaleYLine, m_scaleLineYVerts, m_scaleLineYColors);
    makeTris(m_scaleYCube, m_cubeYVerts, m_cubeYColors);

    // Z (blue)
    m_scaleLineZVerts = { 0,0,0, 0,0,SCALE_LINE_LEN };
    m_scaleLineZColors = { 0,0,1,1, 0,0,1,1 };
    buildCube(m_cubeZVerts, m_cubeZColors, 0,0,SCALE_LINE_LEN, 0,0,1);
    makeLine(m_scaleZLine, m_scaleLineZVerts, m_scaleLineZColors);
    makeTris(m_scaleZCube, m_cubeZVerts, m_cubeZColors);
}

// ═══════════════════════════════════════════════════════════════
//  Rendering
// ═══════════════════════════════════════════════════════════════

inline void TransformGizmo::render(const std::array<float, 16>& viewProjection, const glm::vec3& shapePos)
{
    if (!m_initialized) return;

    glm::mat4 vp = glm::make_mat4(viewProjection.data());
    glm::mat4 model = glm::translate(glm::mat4(1.0f), shapePos);
    glm::mat4 mvp = vp * model;

    std::array<float, 16> mvpArray;
    memcpy(mvpArray.data(), glm::value_ptr(mvp), 16 * sizeof(float));

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    if (m_mode == Mode::Translate)
    {
        glLineWidth(3.0f);
        m_arrowXLine->transformMatrix4f(mvpArray); m_arrowXLine->drawArrays(GL_LINES, 0, 2);
        m_arrowXCone->transformMatrix4f(mvpArray); m_arrowXCone->drawTriangles(0);
        m_arrowYLine->transformMatrix4f(mvpArray); m_arrowYLine->drawArrays(GL_LINES, 0, 2);
        m_arrowYCone->transformMatrix4f(mvpArray); m_arrowYCone->drawTriangles(0);
        m_arrowZLine->transformMatrix4f(mvpArray); m_arrowZLine->drawArrays(GL_LINES, 0, 2);
        m_arrowZCone->transformMatrix4f(mvpArray); m_arrowZCone->drawTriangles(0);
        glLineWidth(1.0f);
    }
    else if (m_mode == Mode::Rotate)
    {
        glLineWidth(2.5f);
        int ringVertCount = RING_SEGS + 1;
        m_ringX->transformMatrix4f(mvpArray); m_ringX->drawArrays(GL_LINE_STRIP, 0, ringVertCount);
        m_ringY->transformMatrix4f(mvpArray); m_ringY->drawArrays(GL_LINE_STRIP, 0, ringVertCount);
        m_ringZ->transformMatrix4f(mvpArray); m_ringZ->drawArrays(GL_LINE_STRIP, 0, ringVertCount);
        glLineWidth(1.0f);
    }
    else if (m_mode == Mode::Scale)
    {
        glLineWidth(3.0f);
        m_scaleXLine->transformMatrix4f(mvpArray); m_scaleXLine->drawArrays(GL_LINES, 0, 2);
        m_scaleXCube->transformMatrix4f(mvpArray); m_scaleXCube->drawTriangles(0);
        m_scaleYLine->transformMatrix4f(mvpArray); m_scaleYLine->drawArrays(GL_LINES, 0, 2);
        m_scaleYCube->transformMatrix4f(mvpArray); m_scaleYCube->drawTriangles(0);
        m_scaleZLine->transformMatrix4f(mvpArray); m_scaleZLine->drawArrays(GL_LINES, 0, 2);
        m_scaleZCube->transformMatrix4f(mvpArray); m_scaleZCube->drawTriangles(0);
        glLineWidth(1.0f);
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

// ═══════════════════════════════════════════════════════════════
//  Math helpers
// ═══════════════════════════════════════════════════════════════

inline float TransformGizmo::closestPointOnLine(const glm::vec3& lineOrigin, const glm::vec3& lineDir,
                                                  const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
    glm::vec3 w = lineOrigin - rayOrigin;
    float a = glm::dot(lineDir, lineDir);
    float b = glm::dot(lineDir, rayDir);
    float c = glm::dot(rayDir, rayDir);
    float d = glm::dot(lineDir, w);
    float e = glm::dot(rayDir, w);

    float denom = a * c - b * b;
    if (std::abs(denom) < 1e-6f)
        return 0.0f;

    return (b * e - c * d) / denom;
}

inline bool TransformGizmo::rayPlaneIntersect(const glm::vec3& planePoint, const glm::vec3& planeNormal,
                                                const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                                glm::vec3& hitPoint)
{
    float denom = glm::dot(rayDir, planeNormal);
    if (std::abs(denom) < 1e-6f)
        return false;

    float t = glm::dot(planePoint - rayOrigin, planeNormal) / denom;
    if (t < 0.0f)
        return false;

    hitPoint = rayOrigin + rayDir * t;
    return true;
}

inline float TransformGizmo::computeRotationAngle(Axis axis, const glm::vec3& point, const glm::vec3& center)
{
    glm::vec3 v = point - center;
    switch (axis) {
        case Axis::X: return std::atan2(v.z, v.y);
        case Axis::Y: return std::atan2(v.x, v.z);
        case Axis::Z: return std::atan2(v.y, v.x);
        default: return 0.0f;
    }
}

// ═══════════════════════════════════════════════════════════════
//  Hit testing
// ═══════════════════════════════════════════════════════════════

inline TransformGizmo::Axis TransformGizmo::hitTest(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                                      const glm::vec3& shapePos)
{
    switch (m_mode) {
        case Mode::Translate: return hitTestTranslate(rayOrigin, rayDir, shapePos);
        case Mode::Rotate:    return hitTestRotate(rayOrigin, rayDir, shapePos);
        case Mode::Scale:     return hitTestScale(rayOrigin, rayDir, shapePos);
        default: return Axis::None;
    }
}

inline TransformGizmo::Axis TransformGizmo::hitTestTranslate(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                                               const glm::vec3& shapePos)
{
    struct AxisHit { Axis axis; float dist; };
    AxisHit best = { Axis::None, HIT_THRESHOLD };

    Axis axisIds[3] = { Axis::X, Axis::Y, Axis::Z };
    for (int i = 0; i < 3; i++)
    {
        glm::vec3 dir = axisDir(axisIds[i]);
        float t = closestPointOnLine(shapePos, dir, rayOrigin, rayDir);
        if (t < -0.05f || t > ARROW_LEN + CONE_LEN) continue;

        glm::vec3 pointOnAxis = shapePos + dir * t;
        float s = glm::dot(pointOnAxis - rayOrigin, rayDir);
        glm::vec3 pointOnRay = rayOrigin + rayDir * s;
        float dist = glm::length(pointOnAxis - pointOnRay);

        if (dist < best.dist)
            best = { axisIds[i], dist };
    }
    return best.axis;
}

inline TransformGizmo::Axis TransformGizmo::hitTestRotate(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                                            const glm::vec3& shapePos)
{
    struct AxisHit { Axis axis; float dist; };
    AxisHit best = { Axis::None, RING_HIT_THRESHOLD };

    Axis axisIds[3] = { Axis::X, Axis::Y, Axis::Z };
    for (int i = 0; i < 3; i++)
    {
        glm::vec3 normal = axisDir(axisIds[i]);
        glm::vec3 hitPoint;
        if (!rayPlaneIntersect(shapePos, normal, rayOrigin, rayDir, hitPoint))
            continue;

        float distFromCenter = glm::length(hitPoint - shapePos);
        float distFromRing = std::abs(distFromCenter - RING_RADIUS);

        if (distFromRing < best.dist)
            best = { axisIds[i], distFromRing };
    }
    return best.axis;
}

inline TransformGizmo::Axis TransformGizmo::hitTestScale(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                                           const glm::vec3& shapePos)
{
    // Same as translate but with SCALE_LINE_LEN
    struct AxisHit { Axis axis; float dist; };
    AxisHit best = { Axis::None, HIT_THRESHOLD };

    Axis axisIds[3] = { Axis::X, Axis::Y, Axis::Z };
    for (int i = 0; i < 3; i++)
    {
        glm::vec3 dir = axisDir(axisIds[i]);
        float t = closestPointOnLine(shapePos, dir, rayOrigin, rayDir);
        if (t < -0.05f || t > SCALE_LINE_LEN + CUBE_HALF) continue;

        glm::vec3 pointOnAxis = shapePos + dir * t;
        float s = glm::dot(pointOnAxis - rayOrigin, rayDir);
        glm::vec3 pointOnRay = rayOrigin + rayDir * s;
        float dist = glm::length(pointOnAxis - pointOnRay);

        if (dist < best.dist)
            best = { axisIds[i], dist };
    }
    return best.axis;
}

// ═══════════════════════════════════════════════════════════════
//  Drag interaction
// ═══════════════════════════════════════════════════════════════

inline void TransformGizmo::beginDrag(Axis axis, const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                                        const glm::vec3& shapePos)
{
    m_activeAxis = axis;
    m_dragging = true;
    m_dragShapePos = shapePos;

    if (m_mode == Mode::Rotate)
    {
        // Compute initial angle by intersecting ray with rotation plane
        glm::vec3 normal = axisDir(axis);
        glm::vec3 hitPoint;
        if (rayPlaneIntersect(shapePos, normal, rayOrigin, rayDir, hitPoint))
            m_dragStartAngle = computeRotationAngle(axis, hitPoint, shapePos);
        else
            m_dragStartAngle = 0.0f;
    }
    else
    {
        // Translate or Scale: project onto axis
        m_dragStartT = closestPointOnLine(shapePos, axisDir(axis), rayOrigin, rayDir);
    }
}

inline glm::vec3 TransformGizmo::updateDrag(const glm::vec3& rayOrigin, const glm::vec3& rayDir)
{
    if (!m_dragging || m_activeAxis == Axis::None)
        return glm::vec3(0);

    glm::vec3 dir = axisDir(m_activeAxis);

    if (m_mode == Mode::Rotate)
    {
        glm::vec3 hitPoint;
        if (!rayPlaneIntersect(m_dragShapePos, dir, rayOrigin, rayDir, hitPoint))
            return glm::vec3(0);

        float currentAngle = computeRotationAngle(m_activeAxis, hitPoint, m_dragShapePos);
        float deltaRad = currentAngle - m_dragStartAngle;

        // Handle wraparound
        if (deltaRad > 3.14159f) deltaRad -= 2.0f * 3.14159f;
        if (deltaRad < -3.14159f) deltaRad += 2.0f * 3.14159f;

        float deltaDeg = deltaRad * (180.0f / 3.14159f);
        return dir * deltaDeg;
    }
    else
    {
        // Translate or Scale: project onto axis line
        float currentT = closestPointOnLine(m_dragShapePos, dir, rayOrigin, rayDir);
        float deltaT = currentT - m_dragStartT;
        return dir * deltaT;
    }
}

inline void TransformGizmo::endDrag()
{
    m_dragging = false;
    m_activeAxis = Axis::None;
}
