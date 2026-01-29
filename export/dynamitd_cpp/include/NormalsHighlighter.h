#pragma once

#include <GL/glew.h>
#include "Shape.h"
#include <vector>
#include <array>
#include <cstdint>

namespace dynamit
{

// Default shaders for normal visualization
inline const char* normalsHighlighterVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in uint aEndpoint;
out vec3 vColor;
uniform mat4 uTransform;
uniform vec3 uColorStart;
uniform vec3 uColorEnd;
void main() {
    gl_Position = uTransform * vec4(aPos, 1.0);
    vColor = (aEndpoint == 0u) ? uColorStart : uColorEnd;
}
)";

inline const char* normalsHighlighterFragmentShader = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

class NormalsHighlighter : public Shape
{
    GLint transformLoc = 0;
    GLint colorStartLoc = 0;
    GLint colorEndLoc = 0;
    GLuint vao = 0, vbo = 0, endpointVbo = 0;

    std::vector<float> normalLines;
    std::vector<uint8_t> endpointMap;
    float normalLength = 0.1f;

    std::array<float, 3> colorStart = { 1.0f, 0.0f, 1.0f };
    std::array<float, 3> colorEnd = { 1.0f, 1.0f, 0.0f };

    void generateLinesFromVertsNorms(
        const std::vector<float>& verts,
        const std::vector<float>& norms,
        float length);

public:
    NormalsHighlighter(float length = 0.1f);
    NormalsHighlighter(const char* vertexShader, const char* fragmentShader, float length = 0.1f);
    ~NormalsHighlighter();

    // Fluent API for configuration
    NormalsHighlighter& withLength(float length);
    NormalsHighlighter& withColorStart(float r, float g, float b);
    NormalsHighlighter& withColorStart(const std::array<float, 3>& color);
    NormalsHighlighter& withColorEnd(float r, float g, float b);
    NormalsHighlighter& withColorEnd(const std::array<float, 3>& color);

    // Build from vertex/normal data
    void build(const std::vector<float>& verts, const std::vector<float>& norms);

    // Draw with transform matrix
    void draw(const float* transform);
    void draw(const std::array<float, 16>& transform);

    // Getters
    float getLength() const { return normalLength; }
    const std::array<float, 3>& getColorStart() const { return colorStart; }
    const std::array<float, 3>& getColorEnd() const { return colorEnd; }
};

} // namespace dynamit