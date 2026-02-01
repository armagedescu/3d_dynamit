#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <vector>
#include <string>
#include <array>
#include <cstdint>
#include <memory>

#include <Dynamit.h>  // Use dynamit for rendering! (includes NormalsHighlighter.h)

// Shape configuration
struct ShapeConfig
{
    enum class Type { Cone, Cylinder };

    // Builder settings
    std::wstring formula = L"1";
    float domainStart = 0.0f;
    float domainEnd = 6.2832f;
    int sectors = 16;
    int slices = 8;
    bool smooth = true;
    bool turbo = true;
    bool doubleCoated = true;
    bool reversed = true;

    // Colors (RGBA)
    std::array<float, 4> outerColor = { 1.0f, 0.0f, 0.501961f, 1.0f };
    std::array<float, 4> innerColor = { 0.0f, 1.0f, 0.0f, 1.0f };

    // Transform
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    float rotX = 0.0f, rotY = 0.0f, rotZ = 0.0f;  // degrees
    float scaleX = 1.0f, scaleY = 1.0f, scaleZ = 1.0f;

    // Shape type
    Type type = Type::Cone;

    // Name
    std::string name = "Shape";
};

// Shape instance using Dynamit for rendering
struct ShapeInstance
{
    ShapeConfig config;

    // CPU buffers (for rebuild)
    std::vector<float> verts;
    std::vector<float> norms;
    std::vector<float> colors;
    std::vector<uint32_t> indices;

    // Dynamit instance for rendering (auto-generates shaders!)
    std::unique_ptr<dynamit::Dynamit> renderer;

    // Normals visualization
    std::unique_ptr<dynamit::NormalsHighlighter> normalsHighlighter;

    bool dirty = true;
    bool visible = true;

    // Last error from formula parsing (empty if no error)
    std::string lastError;
};

class ShapeManager
{
public:
    ShapeManager();
    ~ShapeManager();

    // Shape management
    int addShape(const ShapeConfig& config);
    void removeShape(int index);
    void clearAll();

    // Access
    int getShapeCount() const { return static_cast<int>(m_shapes.size()); }
    ShapeInstance* getShape(int index);
    const ShapeInstance* getShape(int index) const;

    // Building
    void rebuildShape(int index);
    void rebuildAllDirty();

    // Rendering - no shader program needed, dynamit handles it!
    void render(const std::array<float, 16>& viewProjection, bool showNormals = false);

    // Get transform matrix for a shape
    std::array<float, 16> getTransformMatrix(int index) const;

private:
    void buildConeData(const ShapeConfig& cfg,
        std::vector<float>& verts, std::vector<float>& norms,
        std::vector<float>& colors, std::vector<uint32_t>& indices);
    void buildCylinderData(const ShapeConfig& cfg,
        std::vector<float>& verts, std::vector<float>& norms,
        std::vector<float>& colors, std::vector<uint32_t>& indices);
    void setupDynamitRenderer(ShapeInstance& shape);

    std::vector<ShapeInstance> m_shapes;
};
