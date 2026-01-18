#pragma once
#include <GL/glew.h>
#include "Shape.h"
#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <optional>
#include <array>

namespace dynamit
{

    // Forward declarations
    class GlArrayBuffer;
    class ShaderStrategy;

    //========================================
    // GlArrayBuffer - Wraps GL_ARRAY_BUFFER
    //========================================
    class GlArrayBuffer
    {
    private:
        GLuint bufferId = 0;
        GLuint attribLocation;
        std::string bufferName;
        std::vector<float> data;
        GLenum drawType = GL_STATIC_DRAW;

        // Vertex attribute properties
        GLint dimension = 0;
        GLenum dataType = GL_FLOAT;
        GLboolean normalized = GL_FALSE;
        GLsizei stride = 0;
        const void* offset = nullptr;

    public:
        GlArrayBuffer(GLuint location, const std::string& name);
        ~GlArrayBuffer();

        // Fluent API
        GlArrayBuffer& withDrawType(GLenum type);
        GlArrayBuffer& withData(const std::vector<float>& newData);
        GlArrayBuffer& withData(const float* newData, size_t count);

        // Build and initialize buffer
        void build();
        void bufferData();
        void bufferData(const std::vector<float>& newData);

        // Set vertex attribute pointer
        void attrib(GLint dim, GLenum type = GL_FLOAT, GLboolean norm = GL_FALSE,
            GLsizei str = 0, const void* off = nullptr);

        void bindBuffer() const;

        // Getters
        const std::string& name() const;
        std::string nameVary() const;
        GLint getDimension() const;
        GLuint getLocation() const;
        size_t count() const;

        // Shader declarations
        std::string decl() const;
        std::string declVary() const;
        std::string defaultVaryAssign() const;
        std::string vec4(float defaultW = 1.0f) const;
        std::string vec4vary(float defaultW = 1.0f) const;
    };

    //========================================
    // StrideAttribute - Describes one attribute in interleaved data
    //========================================
    struct StrideAttribute
    {
        std::string name;
        GLuint location;
        GLint size;           // Component count (2, 3, 4)
        GLenum type;          // GL_FLOAT, GL_INT, etc.
        GLboolean normalized;
        GLsizei offset;       // Byte offset within stride
    };

    //========================================
    // StrideLayout - Defines interleaved buffer layout
    //========================================
    class StrideLayout
    {
    private:
        std::vector<StrideAttribute> attributes;
        GLsizei totalStride = 0;
        GLsizei currentOffset = 0;

    public:
        void addAttribute(const std::string& name, GLuint location, GLint size,
            GLenum type = GL_FLOAT, GLboolean normalized = GL_FALSE);
        void setOffset(GLsizei offset);
        void setStride(GLsizei stride);
        void finalize();

        const std::vector<StrideAttribute>& getAttributes() const;
        GLsizei getStride() const;
        const StrideAttribute* getAttribute(const std::string& name) const;
        bool hasAttribute(const std::string& name) const;

        static GLsizei sizeOf(GLenum type);
    };

    //========================================
    // Light and Uniform structures
    //========================================
    struct LightDirection
    {
        std::string name;
        std::array<float, 3> data;
        bool normalize = true;
        bool isConst = true;
        GLint location = -1;

        std::string toGLSLConst() const;
        std::string toGLSLUniform() const;
    };

    struct ConstColor
    {
        std::string name;
        std::array<float, 4> data;

        std::string toGLSL() const;
    };

    struct Translation
    {
        std::string name;
        std::array<float, 4> data;
        GLint location = -1;

        std::string toGLSLConst() const;
        std::string toGLSLUniform() const;
    };

    struct ConstTranslation
    {
        std::string name;
        std::array<float, 4> data;

        std::string toGLSL() const;
    };

    struct TransformMatrix3
    {
        std::string name;
        std::array<float, 9> data = {
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f
        };
        GLint location = -1;

        std::string toGLSLUniform() const
        {
            return "uniform mat3 " + name + ";";
        }
    };

    struct TransformMatrix4
    {
        std::string name;
        std::array<float, 16> data = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        GLint location = -1;

        std::string toGLSLUniform() const
        {
            return "uniform mat4 " + name + ";";
        }
    };

    //========================================
    // GlSet - Holds all rendering state
    //========================================
    class GlSet
    {
    private:
        std::string precision = "mediump float";
        std::unique_ptr<GlArrayBuffer> vertexBuffer;
        std::unique_ptr<GlArrayBuffer> normalsBuffer;
        std::unique_ptr<GlArrayBuffer> colorsBuffer;
        std::optional<ConstColor> constColor;
        std::optional<LightDirection> lightDirection;
        std::optional<Translation> translation;
        std::optional<ConstTranslation> constTranslation;
        std::optional<TransformMatrix3> transformMatrix3;
        std::optional<TransformMatrix4> transformMatrix4;

    public:
        // Getters
        const std::string& getPrecision() const;
        GlArrayBuffer* getVertexBuffer() const;
        GlArrayBuffer* getNormalsBuffer() const;
        GlArrayBuffer* getColorsBuffer() const;
        const std::optional<ConstColor>& getConstColor() const;
        const std::optional<LightDirection>& getLightDirection() const;
        const std::optional<Translation>& getTranslation() const;
        const std::optional<ConstTranslation>& getConstTranslation() const;
        const std::optional<TransformMatrix3>& getTransformMatrix3() const;
        const std::optional<TransformMatrix4>& getTransformMatrix4() const;

        // Setters
        void setPrecision(const std::string& p);
        void setVertexBuffer(std::unique_ptr<GlArrayBuffer> buffer);
        void setNormalsBuffer(std::unique_ptr<GlArrayBuffer> buffer);
        void setColorsBuffer(std::unique_ptr<GlArrayBuffer> buffer);
        void setConstColor(const ConstColor& color);
        void setLightDirection(const LightDirection& light);
        void setTranslation(const Translation& trans);
        void setConstTranslation(const ConstTranslation& trans);
        void setTransformMatrix3(const TransformMatrix3& matrix)
        {
            transformMatrix3 = matrix;
        }
        void setTransformMatrix4(const TransformMatrix4& matrix)
        {
            transformMatrix4 = matrix;
        }

        void requireColor(const std::array<float, 4>& defaultValue = { 0.7f, 0.7f, 0.7f, 1.0f },
            const std::string& name = "constColor");
        void requireLightDirection(const std::array<float, 3>& defaultValue = { 0.0f, 0.5f, 1.0f },
            bool normalize = true,
            const std::string& name = "lightDirection");
    };

    //========================================
    // Shader Builder
    //========================================
    class ShaderBuilder
    {
    protected:
        std::vector<std::string> headLines;
        std::vector<std::string> mainLines;
        std::string customSource;

    public:
        void withSource(const std::string& source);
        void addHead(const std::string& line);
        void addMain(const std::string& line);
        void reset();
        std::string build() const;
        virtual std::string buildCandidate() const = 0;
        bool hasCustomSource() const;
    };

    class VertexShaderBuilder : public ShaderBuilder
    {
    public:
        std::string buildCandidate() const override;
    };

    class FragmentShaderBuilder : public ShaderBuilder
    {
    public:
        std::string buildCandidate() const override;
    };

    //========================================
    // Shader Strategy - Compositional Approach
    //========================================
    class ShaderStrategy
    {
    public:
        struct ShaderSources
        {
            std::string vertexShader;
            std::string fragmentShader;
        };

        ShaderStrategy(GlSet& set);
        ShaderStrategy(GlSet& set, const StrideLayout* strideLayout);
        ShaderStrategy(const std::string& vsSource, const std::string& fsSource, GlSet& set);

        ShaderSources build();

    private:
        VertexShaderBuilder vsBuilder;
        FragmentShaderBuilder fsBuilder;
        GlSet& glSet;
        const StrideLayout* strideLayout = nullptr;

        void setRequirements();
        ShaderSources buildCompositional();
        void addDeclarations();
        void addStrideDeclarations();
        std::string buildPositionExpression();
        std::string buildColorExpression();
        std::string buildLightingFactor();
        void composeVertexMain();
        void composeFragmentMain();
    };

    //========================================
    // Dynamit - Main class with multi-VAO support
    //========================================
    class Dynamit : public Shape
    {
    public:
        // Per-VAO data container
        struct VAOData
        {
            GLuint vao = 0;
            GlSet glSet;
            size_t vertexCount = 0;
            GLuint strideBuffer = 0;  // Buffer ID for interleaved data
            GLuint indexBuffer = 0;   // Buffer ID for element indices
            size_t indexCount = 0;    // Number of indices
            GLenum indexType = GL_UNSIGNED_INT;  // GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, GL_UNSIGNED_INT
            GLenum primitiveType = GL_TRIANGLES; // GL_TRIANGLES, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP, etc.
        };

    private:
        std::vector<VAOData> vaoList;
        size_t currentVaoIndex = 0;

        GLuint locationCount = 0;
        std::unique_ptr<ShaderStrategy> strategy;
        bool programBuilt = false;

        std::string customVertexShader;
        std::string customFragmentShader;

        // Stride mode state
        bool strideMode = false;
        StrideLayout strideLayout;

        ShaderStrategy* ensureStrategy();
        VAOData& currentVao();
        GLuint getLocationFor(const std::string& attribName);
        void applyStrideLayout(VAOData& vd);

    public:
        Dynamit();
        ~Dynamit();

        // VAO indexing
        Dynamit& operator[](size_t index);
        GLuint vao() const { return vaoList.empty() ? 0 : vaoList[0].vao; }
        size_t vaoCount() const { return vaoList.size(); }

        // Fluent API - Index buffer
        Dynamit& withPrimitive(GLenum primitive);
        Dynamit& withIndices(const std::vector<uint32_t>& indices);
        Dynamit& withIndices(const std::vector<uint16_t>& indices);
        Dynamit& withIndices(const std::vector<uint8_t>& indices);
        Dynamit& withIndices(const void* data, size_t count, GLenum type);

        // Fluent API - Interleaved/Stride mode
        Dynamit& withStride(const std::vector<float>& data, GLsizei strideBytes);
        Dynamit& withStride(const void* data, size_t sizeBytes, GLsizei strideBytes);
        Dynamit& withStrideOffset(GLsizei bytes);
        Dynamit& withStrideVertices(GLint size, GLboolean normalized = GL_FALSE, GLenum type = GL_FLOAT);
        Dynamit& withStrideNormals(GLint size = 3, GLboolean normalized = GL_FALSE, GLenum type = GL_FLOAT);
        Dynamit& withStrideTexCoords(GLint size = 2, GLboolean normalized = GL_FALSE, GLenum type = GL_FLOAT);
        Dynamit& withStrideColors(GLint size = 4, GLboolean normalized = GL_FALSE, GLenum type = GL_FLOAT);

        // Fluent API - Vertices (separate buffers)
        Dynamit& withVertices2d(const std::vector<float>& data);
        Dynamit& withVertices2d(const float* data, size_t count);
        Dynamit& withVertices3d(const std::vector<float>& data);
        Dynamit& withVertices3d(const float* data, size_t count);
        Dynamit& withVertices3d();

        // Fluent API - Normals (separate buffers)
        Dynamit& withNormals3d(const std::vector<float>& data);
        Dynamit& withNormals3d(const float* data, size_t count);

        // Fluent API - Colors (separate buffers)
        Dynamit& withColors3d(const std::vector<float>& data);
        Dynamit& withColors4d(const std::vector<float>& data);

        // Fluent API - Const values
        Dynamit& withConstColor(const std::array<float, 4>& color, const std::string& name = "constColor");
        Dynamit& withConstColor(float r, float g, float b, float a = 1.0f, const std::string& name = "constColor");
        Dynamit& withConstLightDirection(const std::array<float, 3>& dir, bool normalize = true,
            const std::string& name = "lightDirection");
        Dynamit& withConstLightDirection(float x, float y, float z, bool normalize = true,
            const std::string& name = "lightDirection");
        Dynamit& withConstTranslation(const std::array<float, 4>& trans, const std::string& name = "Translation");
        Dynamit& withConstTranslation(float x, float y, float z, float w, const std::string& name = "Translation");

        // Fluent API - Uniforms (for animation)
        Dynamit& withTranslation4f(const std::string& name = "translation");
        Dynamit& withLightDirection3f(float x, float y, float z, bool normalize = true,
            const std::string& name = "lightDirection");
        Dynamit& withLightDirection3f(const std::array<float, 3>& dir, bool normalize = true,
            const std::string& name = "lightDirection");

        Dynamit& withPrecision(const std::string& precision);

        // Shader overrides - like JavaScript version
        Dynamit& withShaderSources(const std::string& vs, const std::string& fs);
        Dynamit& withVertexShader(const std::string& vs);
        Dynamit& withFragmentShader(const std::string& fs);

        // Program building
        GLuint getProgramAuto();
        void buildProgram();
        void logGeneratedShaders(const std::string& message = "");
        void logShaders(const std::string& message = "");  // Shows actual shaders used (overridden or generated)
        ShaderStrategy::ShaderSources getGeneratedShaders();
        ShaderStrategy::ShaderSources getShaders();  // Gets actual shaders used

        // Binding
        void bindVertexArray() const;
        void bindVertexArray(size_t index) const;
        void useProgram();
        void bind();

        // Drawing
        void drawTriangles(GLint start = 0);
        void drawTrianglesIndexed();
        void drawTriangleFan(GLint start = 0);
        void drawArrays(GLenum mode, GLint start, GLsizei count);
        void drawElements(GLenum mode, GLsizei count, GLenum type, const void* offset = nullptr);
        // Runtime updates
        void translate4f(float x, float y, float z, float w);
        void translate4f(const std::array<float, 4>& trans);
        void lightDirection3f(float x, float y, float z);
        void lightDirection3f(const std::array<float, 3>& dir);
        void updateVertices(const std::vector<float>& newData);
        void updateVertices(const float* data, size_t count);

        GLint getUniformLocation(const char* name)
        {
            useProgram();
            return glGetUniformLocation(program.id, name);
        }

        Dynamit& withTransformMatrix3f(const std::string& name = "transformMatrix")
        {
            currentVao().glSet.setTransformMatrix3({ name });
            return *this;
        }

        void transformMatrix3f(const float* data)
        {
            if (!vaoList[0].glSet.getTransformMatrix3())
                throw std::runtime_error("Transform matrix uniform not initialized. Call withTransformMatrix3f() first.");

            useProgram();

            TransformMatrix3& matrix = const_cast<TransformMatrix3&>(*vaoList[0].glSet.getTransformMatrix3());

            if (matrix.location == -1)
            {
                matrix.location = glGetUniformLocation(program.id, matrix.name.c_str());
                if (matrix.location == -1)
                    throw std::runtime_error("Transform matrix uniform not found in shader");
            }

            glUniformMatrix3fv(matrix.location, 1, GL_FALSE, data);
            std::copy(data, data + 9, matrix.data.begin());
        }

        void transformMatrix3f(const std::array<float, 9>& data)
        {
            transformMatrix3f(data.data());
        }

        Dynamit& withTransformMatrix4f(const std::string& name = "transformMatrix")
        {
            currentVao().glSet.setTransformMatrix4({ name });
            return *this;
        }

        void transformMatrix4f(const float* data)
        {
            if (!vaoList[0].glSet.getTransformMatrix4())
                throw std::runtime_error("Transform matrix 4x4 uniform not initialized. Call withTransformMatrix4f() first.");

            useProgram();

            TransformMatrix4& matrix = const_cast<TransformMatrix4&>(*vaoList[0].glSet.getTransformMatrix4());

            if (matrix.location == -1)
            {
                matrix.location = glGetUniformLocation(program.id, matrix.name.c_str());
                if (matrix.location == -1)
                    throw std::runtime_error("Transform matrix 4x4 uniform not found in shader");
            }

            glUniformMatrix4fv(matrix.location, 1, GL_FALSE, data);
            std::copy(data, data + 16, matrix.data.begin());
        }

        void transformMatrix4f(const std::array<float, 16>& data)
        {
            transformMatrix4f(data.data());
        }
    };

} // namespace dynamit