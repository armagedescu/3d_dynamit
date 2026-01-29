#include "pch.h"
#include "NormalsHighlighter.h"

namespace dynamit
{

NormalsHighlighter::NormalsHighlighter(float length)
    : NormalsHighlighter(normalsHighlighterVertexShader, normalsHighlighterFragmentShader, length)
{
}

NormalsHighlighter::NormalsHighlighter(const char* vertexShader, const char* fragmentShader, float length)
    : Shape(vertexShader, fragmentShader), normalLength(length)
{
}

NormalsHighlighter::~NormalsHighlighter()
{
    if (vao) glDeleteVertexArrays(1, &vao);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (endpointVbo) glDeleteBuffers(1, &endpointVbo);
}

NormalsHighlighter& NormalsHighlighter::withLength(float length)
{
    normalLength = length;
    return *this;
}

NormalsHighlighter& NormalsHighlighter::withColorStart(float r, float g, float b)
{
    colorStart = { r, g, b };
    return *this;
}

NormalsHighlighter& NormalsHighlighter::withColorStart(const std::array<float, 3>& color)
{
    colorStart = color;
    return *this;
}

NormalsHighlighter& NormalsHighlighter::withColorEnd(float r, float g, float b)
{
    colorEnd = { r, g, b };
    return *this;
}

NormalsHighlighter& NormalsHighlighter::withColorEnd(const std::array<float, 3>& color)
{
    colorEnd = color;
    return *this;
}

void NormalsHighlighter::generateLinesFromVertsNorms(
    const std::vector<float>& verts,
    const std::vector<float>& norms,
    float length)
{
    normalLines.clear();
    endpointMap.clear();
    normalLines.reserve(verts.size() * 2);
    endpointMap.reserve((verts.size() / 3) * 2);

    for (size_t vi = 0; vi < verts.size(); vi += 3)
    {
        float vx = verts[vi];
        float vy = verts[vi + 1];
        float vz = verts[vi + 2];

        float nx = norms[vi];
        float ny = norms[vi + 1];
        float nz = norms[vi + 2];

        // Start point (vertex position)
        normalLines.push_back(vx);
        normalLines.push_back(vy);
        normalLines.push_back(vz);
        endpointMap.push_back(0);

        // End point (vertex + normal * length)
        normalLines.push_back(vx + nx * length);
        normalLines.push_back(vy + ny * length);
        normalLines.push_back(vz + nz * length);
        endpointMap.push_back(1);
    }
}

void NormalsHighlighter::build(const std::vector<float>& verts, const std::vector<float>& norms)
{
    generateLinesFromVertsNorms(verts, norms, normalLength);

    transformLoc = glGetUniformLocation(*this, "uTransform");
    colorStartLoc = glGetUniformLocation(*this, "uColorStart");
    colorEndLoc = glGetUniformLocation(*this, "uColorEnd");

    // Create VAO/VBOs
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &endpointVbo);

    glBindVertexArray(vao);

    // Position buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, normalLines.size() * sizeof(float), normalLines.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // Endpoint map buffer
    glBindBuffer(GL_ARRAY_BUFFER, endpointVbo);
    glBufferData(GL_ARRAY_BUFFER, endpointMap.size() * sizeof(uint8_t), endpointMap.data(), GL_STATIC_DRAW);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, sizeof(uint8_t), nullptr);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void NormalsHighlighter::draw(const float* transform)
{
    glUseProgram(*this);
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform);
    glUniform3fv(colorStartLoc, 1, colorStart.data());
    glUniform3fv(colorEndLoc, 1, colorEnd.data());

    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(normalLines.size() / 3));
}

void NormalsHighlighter::draw(const std::array<float, 16>& transform)
{
    draw(transform.data());
}

} // namespace dynamit