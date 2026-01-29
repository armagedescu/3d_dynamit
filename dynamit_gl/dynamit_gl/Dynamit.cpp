#include "pch.h"
#include "Dynamit.h"
#include <iostream>
#include <cassert>

namespace dynamit
{

    //========================================
    // GlArrayBuffer Implementation
    //========================================

    GlArrayBuffer::GlArrayBuffer(GLuint location, const std::string& name)
        : attribLocation(location), bufferName(name) {
    }

    GlArrayBuffer::~GlArrayBuffer()
    {
        if (bufferId != 0)
            glDeleteBuffers(1, &bufferId);
    }

    GlArrayBuffer& GlArrayBuffer::withDrawType(GLenum type) { drawType = type; return *this; }
    GlArrayBuffer& GlArrayBuffer::withData(const std::vector<float>& newData) { data = newData; return *this; }
    GlArrayBuffer& GlArrayBuffer::withData(const float* newData, size_t count)
    {
        data.assign(newData, newData + count);
        return *this;
    }

    void GlArrayBuffer::build()
    {
        glGenBuffers(1, &bufferId);
        bindBuffer();
    }

    void GlArrayBuffer::bufferData()
    {
        if (data.empty()) return;
        bindBuffer();
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), drawType);
    }

    void GlArrayBuffer::bufferData(const std::vector<float>& newData)
    {
        data = newData;
        bufferData();
    }

    void GlArrayBuffer::attrib(GLint dim, GLenum type, GLboolean norm, GLsizei str, const void* off)
    {
        dimension = dim;
        dataType = type;
        normalized = norm;
        stride = str;
        offset = off;

        bindBuffer();
        glVertexAttribPointer(attribLocation, dimension, dataType, normalized, stride, offset);
        glEnableVertexAttribArray(attribLocation);
    }

    void GlArrayBuffer::bindBuffer() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    }

    const std::string& GlArrayBuffer::name() const { return bufferName; }
    std::string GlArrayBuffer::nameVary() const { return bufferName + "Vary"; }
    GLint GlArrayBuffer::getDimension() const { return dimension; }
    GLuint GlArrayBuffer::getLocation() const { return attribLocation; }
    size_t GlArrayBuffer::count() const { return data.size() / dimension; }

    std::string GlArrayBuffer::decl() const
    {
        return "vec" + std::to_string(dimension) + " " + bufferName;
    }

    std::string GlArrayBuffer::declVary() const
    {
        return "vec" + std::to_string(dimension) + " " + nameVary();
    }

    std::string GlArrayBuffer::defaultVaryAssign() const
    {
        return nameVary() + " = " + bufferName;
    }

    std::string GlArrayBuffer::vec4(float defaultW) const
    {
        if (dimension == 4) return bufferName;

        std::ostringstream oss;
        oss << "vec4(" << bufferName;
        for (int i = dimension; i < 3; i++)
            oss << ", 0.0";
        if (dimension < 4)
            oss << ", " << defaultW;
        oss << ")";
        return oss.str();
    }

    std::string GlArrayBuffer::vec4vary(float defaultW) const
    {
        if (dimension == 4) return nameVary();

        std::ostringstream oss;
        oss << "vec4(" << nameVary();
        for (int i = dimension; i < 3; i++)
            oss << ", 0.0";
        if (dimension < 4)
            oss << ", " << defaultW;
        oss << ")";
        return oss.str();
    }

    //========================================
    // StrideLayout Implementation
    //========================================

    GLsizei StrideLayout::sizeOf(GLenum type)
    {
        switch (type)
        {
        case GL_FLOAT:         return sizeof(GLfloat);
        case GL_INT:           return sizeof(GLint);
        case GL_UNSIGNED_INT:  return sizeof(GLuint);
        case GL_SHORT:         return sizeof(GLshort);
        case GL_UNSIGNED_SHORT:return sizeof(GLushort);
        case GL_BYTE:          return sizeof(GLbyte);
        case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
        case GL_DOUBLE:        return sizeof(GLdouble);
        default:               return sizeof(GLfloat);
        }
    }

    void StrideLayout::addAttribute(const std::string& name, GLuint location, GLint size,
        GLenum type, GLboolean normalized)
    {
        StrideAttribute attr;
        attr.name = name;
        attr.location = location;
        attr.size = size;
        attr.type = type;
        attr.normalized = normalized;
        attr.offset = currentOffset;

        attributes.push_back(attr);
        currentOffset += size * sizeOf(type);
    }

    void StrideLayout::setOffset(GLsizei offset)
    {
        currentOffset = offset;
    }

    void StrideLayout::setStride(GLsizei stride)
    {
        totalStride = stride;
    }

    void StrideLayout::finalize()
    {
        totalStride = currentOffset;
    }

    const std::vector<StrideAttribute>& StrideLayout::getAttributes() const
    {
        return attributes;
    }

    GLsizei StrideLayout::getStride() const
    {
        return totalStride;
    }

    const StrideAttribute* StrideLayout::getAttribute(const std::string& name) const
    {
        for (const auto& attr : attributes)
            if (attr.name == name)
                return &attr;
        return nullptr;
    }

    bool StrideLayout::hasAttribute(const std::string& name) const
    {
        return getAttribute(name) != nullptr;
    }

    //========================================
    // LightDirection Implementation
    //========================================

    std::string LightDirection::toGLSLConst() const
    {
        std::ostringstream oss;
        oss << "const vec3 " << name << " = vec3("
            << data[0] << ", " << data[1] << ", " << data[2] << ");";
        return oss.str();
    }

    std::string LightDirection::toGLSLUniform() const
    {
        return "uniform vec3 " + name + ";";
    }

    //========================================
    // ConstColor Implementation
    //========================================

    std::string ConstColor::toGLSL() const
    {
        std::ostringstream oss;
        oss << "const vec4 " << name << " = vec4("
            << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << ");";
        return oss.str();
    }

    //========================================
    // Translation Implementation
    //========================================

    std::string Translation::toGLSLConst() const
    {
        std::ostringstream oss;
        oss << "const vec4 " << name << " = vec4("
            << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << ");";
        return oss.str();
    }

    std::string Translation::toGLSLUniform() const
    {
        return "uniform vec4 " + name + ";";
    }

    //========================================
    // ConstTranslation Implementation
    //========================================

    std::string ConstTranslation::toGLSL() const
    {
        std::ostringstream oss;
        oss << "const vec4 const" << name << " = vec4("
            << data[0] << ", " << data[1] << ", " << data[2] << ", " << data[3] << ");";
        return oss.str();
    }

    //========================================
    // GlSet Implementation
    //========================================

    const std::string& GlSet::getPrecision() const { return precision; }
    GlArrayBuffer* GlSet::getVertexBuffer() const { return vertexBuffer.get(); }
    GlArrayBuffer* GlSet::getNormalsBuffer() const { return normalsBuffer.get(); }
    GlArrayBuffer* GlSet::getColorsBuffer() const { return colorsBuffer.get(); }
    const std::optional<ConstColor>& GlSet::getConstColor() const { return constColor; }
    const std::optional<LightDirection>& GlSet::getLightDirection() const { return lightDirection; }
    const std::optional<Translation>& GlSet::getTranslation() const { return translation; }
    const std::optional<ConstTranslation>& GlSet::getConstTranslation() const { return constTranslation; }
    const std::optional<TransformMatrix3>& GlSet::getTransformMatrix3() const { return transformMatrix3; }
    const std::optional<TransformMatrix4>& GlSet::getTransformMatrix4() const { return transformMatrix4; }

    void GlSet::setPrecision(const std::string& p) { precision = p; }

    void GlSet::setVertexBuffer(std::unique_ptr<GlArrayBuffer> buffer)
    {
        if (vertexBuffer)
            throw std::runtime_error("Vertex buffer can only be set once");
        vertexBuffer = std::move(buffer);
    }

    void GlSet::setNormalsBuffer(std::unique_ptr<GlArrayBuffer> buffer)
    {
        normalsBuffer = std::move(buffer);
    }

    void GlSet::setColorsBuffer(std::unique_ptr<GlArrayBuffer> buffer)
    {
        constColor.reset();
        colorsBuffer = std::move(buffer);
    }

    void GlSet::setConstColor(const ConstColor& color)
    {
        if (colorsBuffer)
            throw std::runtime_error("Cannot set const color when color buffer is used");
        constColor = color;
    }

    void GlSet::setLightDirection(const LightDirection& light)
    {
        lightDirection = light;
    }

    void GlSet::setTranslation(const Translation& trans)
    {
        translation = trans;
    }

    void GlSet::setConstTranslation(const ConstTranslation& trans)
    {
        constTranslation = trans;
    }

    //void GlSet::setTransformMatrix3(std::unique_ptr<GlArrayBuffer> buffer)
    //{
    //    transformMatrix3 = std::move(buffer);
    //}

    void GlSet::requireColor(const std::array<float, 4>& defaultValue, const std::string& name)
    {
        if (colorsBuffer || constColor) return;
        setConstColor({ name, defaultValue });
    }

    void GlSet::requireLightDirection(const std::array<float, 3>& defaultValue, bool normalize, const std::string& name)
    {
        if (lightDirection) return;
        setLightDirection({ name, defaultValue, normalize, true, -1 });
    }

    //========================================
    // ShaderBuilder Implementation
    //========================================

    void ShaderBuilder::withSource(const std::string& source) { customSource = source; }
    void ShaderBuilder::addHead(const std::string& line) { headLines.push_back(line); }
    void ShaderBuilder::addMain(const std::string& line) { mainLines.push_back(line); }

    void ShaderBuilder::reset()
    {
        headLines.clear();
        mainLines.clear();
    }

    std::string ShaderBuilder::build() const
    {
        if (!customSource.empty()) return customSource;
        return buildCandidate();
    }

    bool ShaderBuilder::hasCustomSource() const { return !customSource.empty(); }

    //========================================
    // VertexShaderBuilder Implementation
    //========================================

    std::string VertexShaderBuilder::buildCandidate() const
    {
        std::ostringstream oss;
        oss << "#version 330 core\n";
        for (const std::string& line : headLines)
            oss << line << "\n";
        oss << "\nvoid main()\n{\n";
        for (const std::string& line : mainLines)
            oss << "    " << line << "\n";
        oss << "}\n";
        return oss.str();
    }

    //========================================
    // FragmentShaderBuilder Implementation
    //========================================

    std::string FragmentShaderBuilder::buildCandidate() const
    {
        std::ostringstream oss;
        oss << "#version 330 core\n";
        for (const std::string& line : headLines)
            oss << line << "\n";
        oss << "\nvoid main()\n{\n";
        for (const std::string& line : mainLines)
            oss << "    " << line << "\n";
        oss << "}\n";
        return oss.str();
    }

    //========================================
    // ShaderStrategy Implementation
    //========================================

    ShaderStrategy::ShaderStrategy(GlSet& set) : glSet(set) {}

    ShaderStrategy::ShaderStrategy(GlSet& set, const StrideLayout* layout)
        : glSet(set), strideLayout(layout) {
    }

    ShaderStrategy::ShaderStrategy(const std::string& vsSource, const std::string& fsSource, GlSet& set)
        : glSet(set)
    {
        vsBuilder.withSource(vsSource);
        fsBuilder.withSource(fsSource);
    }

    ShaderStrategy::ShaderSources ShaderStrategy::build()
    {
        if (vsBuilder.hasCustomSource() && fsBuilder.hasCustomSource())
            return { vsBuilder.build(), fsBuilder.build() };

        // For stride mode, we don't require traditional vertex buffer
        if (!strideLayout && !glSet.getVertexBuffer())
            throw std::runtime_error("Vertex buffer is required");

        setRequirements();
        return buildCompositional();
    }

    void ShaderStrategy::setRequirements()
    {
        glSet.requireColor();

        bool hasNormals = glSet.getNormalsBuffer() != nullptr;
        if (strideLayout)
            hasNormals = hasNormals || strideLayout->hasAttribute("normal");

        if (hasNormals)
            glSet.requireLightDirection();
    }

    ShaderStrategy::ShaderSources ShaderStrategy::buildCompositional()
    {
        vsBuilder.reset();
        fsBuilder.reset();

        if (strideLayout && !strideLayout->getAttributes().empty())
            addStrideDeclarations();
        else
            addDeclarations();

        composeVertexMain();
        composeFragmentMain();

        return { vsBuilder.buildCandidate(), fsBuilder.buildCandidate() };
    }

    void ShaderStrategy::addStrideDeclarations()
    {
        fsBuilder.addHead("precision " + glSet.getPrecision() + ";");
        fsBuilder.addHead("out vec4 fragColor;");

        for (const StrideAttribute& attr : strideLayout->getAttributes())
        {
            std::string vecType = "vec" + std::to_string(attr.size);
            vsBuilder.addHead("layout (location = " + std::to_string(attr.location) +
                ") in " + vecType + " " + attr.name + ";");

            // Add varyings for normals and colors
            if (attr.name == "normal" || attr.name == "color")
            {
                vsBuilder.addHead("out " + vecType + " " + attr.name + "Vary;");
                fsBuilder.addHead("in " + vecType + " " + attr.name + "Vary;");
            }
        }

        if (glSet.getConstColor())
            fsBuilder.addHead(glSet.getConstColor()->toGLSL());

        if (glSet.getLightDirection())
        {
            const LightDirection& light = *glSet.getLightDirection();
            if (light.isConst)
                fsBuilder.addHead(light.toGLSLConst());
            else
                fsBuilder.addHead(light.toGLSLUniform());
        }

        if (glSet.getTranslation())
            vsBuilder.addHead(glSet.getTranslation()->toGLSLUniform());

        if (glSet.getConstTranslation())
            vsBuilder.addHead(glSet.getConstTranslation()->toGLSL());

        if (glSet.getTransformMatrix3())
            vsBuilder.addHead(glSet.getTransformMatrix3()->toGLSLUniform());

        if (glSet.getTransformMatrix4())
            vsBuilder.addHead(glSet.getTransformMatrix4()->toGLSLUniform());
    }

    void ShaderStrategy::addDeclarations()
    {
        fsBuilder.addHead("precision " + glSet.getPrecision() + ";");
        fsBuilder.addHead("out vec4 fragColor;");

        GlArrayBuffer* vb = glSet.getVertexBuffer();
        vsBuilder.addHead("layout (location = " + std::to_string(vb->getLocation()) +
            ") in " + vb->decl() + ";");

        if (GlArrayBuffer* nb = glSet.getNormalsBuffer())
        {
            vsBuilder.addHead("layout (location = " + std::to_string(nb->getLocation()) +
                ") in " + nb->decl() + ";");
            vsBuilder.addHead("out " + nb->declVary() + ";");
            fsBuilder.addHead("in " + nb->declVary() + ";");
        }

        if (GlArrayBuffer* cb = glSet.getColorsBuffer())
        {
            vsBuilder.addHead("layout (location = " + std::to_string(cb->getLocation()) +
                ") in " + cb->decl() + ";");
            vsBuilder.addHead("out " + cb->declVary() + ";");
            fsBuilder.addHead("in " + cb->declVary() + ";");
        }

        if (glSet.getConstColor())
            fsBuilder.addHead(glSet.getConstColor()->toGLSL());

        if (glSet.getLightDirection())
        {
            const LightDirection& light = *glSet.getLightDirection();
            if (light.isConst)
                fsBuilder.addHead(light.toGLSLConst());
            else
                fsBuilder.addHead(light.toGLSLUniform());
        }

        if (glSet.getTranslation())
            vsBuilder.addHead(glSet.getTranslation()->toGLSLUniform());

        if (glSet.getConstTranslation())
            vsBuilder.addHead(glSet.getConstTranslation()->toGLSL());

        if (glSet.getTransformMatrix3())
            vsBuilder.addHead(glSet.getTransformMatrix3()->toGLSLUniform());

        if (glSet.getTransformMatrix4())
            vsBuilder.addHead(glSet.getTransformMatrix4()->toGLSLUniform());
    }

    std::string ShaderStrategy::buildPositionExpression()
    {
        std::string pos;

        // Get the raw vertex expression
        std::string vertexExpr;
        int vertexDim = 3;
        
        if (strideLayout && strideLayout->hasAttribute("vertex"))
        {
            const StrideAttribute* attr = strideLayout->getAttribute("vertex");
            vertexExpr = "vertex";
            vertexDim = attr->size;
        }
        else if (glSet.getVertexBuffer())
        {
            GlArrayBuffer* vb = glSet.getVertexBuffer();
            vertexExpr = vb->name();
            vertexDim = vb->getDimension();
        }
        else
        {
            vertexExpr = "vec3(0.0, 0.0, 0.0)";
            vertexDim = 3;
        }

        // Apply transform matrix if present
        if (glSet.getTransformMatrix4())
        {
            // mat4 * vec4 -> vec4
            if (vertexDim == 4)
                pos = glSet.getTransformMatrix4()->name + " * " + vertexExpr;
            else if (vertexDim == 3)
                pos = glSet.getTransformMatrix4()->name + " * vec4(" + vertexExpr + ", 1)";
            else
                pos = glSet.getTransformMatrix4()->name + " * vec4(" + vertexExpr + ", 0.0, 1)";
        }
        else if (glSet.getTransformMatrix3())
        {
            // mat3 * vec3 -> vec3, then wrap in vec4
            std::string vec3Expr;
            if (vertexDim == 3)
                vec3Expr = vertexExpr;
            else if (vertexDim == 2)
                vec3Expr = "vec3(" + vertexExpr + ", 0.0)";
            else
                vec3Expr = vertexExpr + ".xyz";
            
            pos = "vec4(" + glSet.getTransformMatrix3()->name + " * " + vec3Expr + ", 1)";
        }
        else
        {
            // No transform, just convert to vec4
            if (vertexDim == 4)
                pos = vertexExpr;
            else if (vertexDim == 3)
                pos = "vec4(" + vertexExpr + ", 1)";
            else
                pos = "vec4(" + vertexExpr + ", 0.0, 1)";
        }

        if (glSet.getConstTranslation())
            pos += " + const" + glSet.getConstTranslation()->name;

        if (glSet.getTranslation())
            pos += " + " + glSet.getTranslation()->name;

        return pos;
    }

    std::string ShaderStrategy::buildColorExpression()
    {
        if (strideLayout && strideLayout->hasAttribute("color"))
        {
            const StrideAttribute* attr = strideLayout->getAttribute("color");
            if (attr->size == 4)
                return "colorVary";
            else
                return "vec4(colorVary, 1.0)";
        }

        if (GlArrayBuffer* cb = glSet.getColorsBuffer())
            return cb->vec4vary();

        if (glSet.getConstColor())
            return glSet.getConstColor()->name;

        return "vec4(1.0, 1.0, 1.0, 1.0)";
    }

    std::string ShaderStrategy::buildLightingFactor()
    {
        bool hasNormals = false;

        if (strideLayout && strideLayout->hasAttribute("normal"))
            hasNormals = true;
        else if (glSet.getNormalsBuffer())
            hasNormals = true;

        if (!hasNormals || !glSet.getLightDirection())
            return "";

        const LightDirection& light = *glSet.getLightDirection();
        std::string normalVar = "normalVary";

        if (light.normalize)
            return "-dot(normalize(" + light.name + "), normalize(" + normalVar + "))";
        else
            return "-dot(" + light.name + ", " + normalVar + ")";
    }

    void ShaderStrategy::composeVertexMain()
    {
        vsBuilder.addMain("gl_Position = " + buildPositionExpression() + ";");

        if (strideLayout)
        {
            if (strideLayout->hasAttribute("normal"))
            {
                if (glSet.getTransformMatrix4())
                    vsBuilder.addMain("normalVary = mat3(" + glSet.getTransformMatrix4()->name + ") * normal;");
                else if (glSet.getTransformMatrix3())
                    vsBuilder.addMain("normalVary = " + glSet.getTransformMatrix3()->name + " * normal;");
                else
                    vsBuilder.addMain("normalVary = normal;");
            }
            if (strideLayout->hasAttribute("color"))
                vsBuilder.addMain("colorVary = color;");
        }
        else
        {
            if (GlArrayBuffer* nb = glSet.getNormalsBuffer())
            {
                if (glSet.getTransformMatrix4())
                    vsBuilder.addMain(nb->nameVary() + " = mat3(" + glSet.getTransformMatrix4()->name + ") * " + nb->name() + ";");
                else if (glSet.getTransformMatrix3())
                    vsBuilder.addMain(nb->nameVary() + " = " + glSet.getTransformMatrix3()->name + " * " + nb->name() + ";");
                else
                    vsBuilder.addMain(nb->defaultVaryAssign() + ";");
            }

            if (GlArrayBuffer* cb = glSet.getColorsBuffer())
                vsBuilder.addMain(cb->defaultVaryAssign() + ";");
        }
    }

    void ShaderStrategy::composeFragmentMain()
    {
        std::string colorExpr = buildColorExpression();
        std::string lightingFactor = buildLightingFactor();

        if (!lightingFactor.empty())
        {
            fsBuilder.addMain("float prod = " + lightingFactor + ";");
            fsBuilder.addMain("fragColor = vec4(" + colorExpr + ".rgb * prod, 1.0);");
        }
        else
        {
            fsBuilder.addMain("fragColor = " + colorExpr + ";");
        }
    }

    //========================================
    // Dynamit Implementation
    //========================================

    Dynamit::Dynamit()
    {
        vaoList.emplace_back();
        glGenVertexArrays(1, &vaoList[0].vao);
    }

    Dynamit::~Dynamit()
    {
        for (auto& vd : vaoList)
        {
            if (vd.vao != 0)
                glDeleteVertexArrays(1, &vd.vao);
            if (vd.strideBuffer != 0)
                glDeleteBuffers(1, &vd.strideBuffer);
            if (vd.indexBuffer != 0)
                glDeleteBuffers(1, &vd.indexBuffer);
        }
    }

    Dynamit& Dynamit::operator[](size_t index)
    {
        while (index >= vaoList.size())
        {
            vaoList.emplace_back();
            glGenVertexArrays(1, &vaoList.back().vao);
        }
        currentVaoIndex = index;
        return *this;
    }

    Dynamit::VAOData& Dynamit::currentVao()
    {
        return vaoList[currentVaoIndex];
    }

    GLuint Dynamit::getLocationFor(const std::string& attribName)
    {
        // Check stride layout first
        if (strideMode)
        {
            const StrideAttribute* existing = strideLayout.getAttribute(attribName);
            if (existing)
                return existing->location;
        }

        // Check existing buffers in root VAO
        if (!vaoList.empty())
        {
            GlArrayBuffer* existing = nullptr;
            if (attribName == "vertex")
                existing = vaoList[0].glSet.getVertexBuffer();
            else if (attribName == "normal")
                existing = vaoList[0].glSet.getNormalsBuffer();
            else if (attribName == "color")
                existing = vaoList[0].glSet.getColorsBuffer();

            if (existing)
                return existing->getLocation();
        }
        return locationCount++;
    }

    void Dynamit::applyStrideLayout(VAOData& vd)
    {
        if (vd.strideBuffer == 0)
            return;

        glBindVertexArray(vd.vao);
        glBindBuffer(GL_ARRAY_BUFFER, vd.strideBuffer);

        GLsizei stride = strideLayout.getStride();

        for (const StrideAttribute& attr : strideLayout.getAttributes())
        {
            glVertexAttribPointer(attr.location, attr.size, attr.type, attr.normalized,
                stride, reinterpret_cast<const void*>(static_cast<intptr_t>(attr.offset)));
            glEnableVertexAttribArray(attr.location);
        }
    }

    ShaderStrategy* Dynamit::ensureStrategy()
    {
        if (!strategy)
        {
            if (strideMode)
                strategy = std::make_unique<ShaderStrategy>(vaoList[0].glSet, &strideLayout);
            else
                strategy = std::make_unique<ShaderStrategy>(vaoList[0].glSet);
        }
        return strategy.get();
    }

    // Stride mode methods

    Dynamit& Dynamit::withStride(const std::vector<float>& data, GLsizei strideBytes)
    {
        return withStride(data.data(), data.size() * sizeof(float), strideBytes);
    }

    Dynamit& Dynamit::withStride(const void* data, size_t sizeBytes, GLsizei strideBytes)
    {
        VAOData& vd = currentVao();
        glBindVertexArray(vd.vao);

        // Store stride for layout
        strideLayout.setStride(strideBytes);

        // Create or reuse stride buffer
        if (vd.strideBuffer == 0)
            glGenBuffers(1, &vd.strideBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, vd.strideBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeBytes, data, GL_STATIC_DRAW);

        // Calculate vertex count
        vd.vertexCount = sizeBytes / strideBytes;

        // If this is a child VAO (index > 0) and layout is defined, apply it
        if (currentVaoIndex > 0 && strideMode && strideLayout.getStride() > 0)
        {
            applyStrideLayout(vd);
        }

        strideMode = true;
        return *this;
    }

    Dynamit& Dynamit::withStrideOffset(GLsizei bytes)
    {
        strideLayout.setOffset(bytes);
        return *this;
    }

    Dynamit& Dynamit::withStrideVertices(GLint size, GLboolean normalized, GLenum type)
    {
        GLuint location = getLocationFor("vertex");
        strideLayout.addAttribute("vertex", location, size, type, normalized);
        return *this;
    }

    Dynamit& Dynamit::withStrideNormals(GLint size, GLboolean normalized, GLenum type)
    {
        GLuint location = getLocationFor("normal");
        strideLayout.addAttribute("normal", location, size, type, normalized);
        return *this;
    }

    Dynamit& Dynamit::withStrideTexCoords(GLint size, GLboolean normalized, GLenum type)
    {
        GLuint location = getLocationFor("texCoord");
        strideLayout.addAttribute("texCoord", location, size, type, normalized);
        return *this;
    }

    Dynamit& Dynamit::withStrideColors(GLint size, GLboolean normalized, GLenum type)
    {
        GLuint location = getLocationFor("color");
        strideLayout.addAttribute("color", location, size, type, normalized);
        return *this;
    }

    // Original separate buffer methods

    Dynamit& Dynamit::withVertices2d(const std::vector<float>& data)
    {
        VAOData& vd = currentVao();
        glBindVertexArray(vd.vao);

        GLuint location = getLocationFor("vertex");
        auto buffer = std::make_unique<GlArrayBuffer>(location, "vertex");
        buffer->build();
        buffer->withData(data);
        buffer->bufferData();
        buffer->attrib(2, GL_FLOAT);
        vd.glSet.setVertexBuffer(std::move(buffer));
        vd.vertexCount = data.size() / 2;
        return *this;
    }

    Dynamit& Dynamit::withVertices2d(const float* data, size_t count)
    {
        return withVertices2d(std::vector<float>(data, data + count));
    }

    Dynamit& Dynamit::withVertices3d(const std::vector<float>& data)
    {
        VAOData& vd = currentVao();
        glBindVertexArray(vd.vao);

        GLuint location = getLocationFor("vertex");
        auto buffer = std::make_unique<GlArrayBuffer>(location, "vertex");
        buffer->build();
        buffer->withData(data);
        buffer->bufferData();
        buffer->attrib(3, GL_FLOAT);
        vd.glSet.setVertexBuffer(std::move(buffer));
        vd.vertexCount = data.size() / 3;
        return *this;
    }

    Dynamit& Dynamit::withVertices3d(const float* data, size_t count)
    {
        return withVertices3d(std::vector<float>(data, data + count));
    }

    Dynamit& Dynamit::withVertices3d()
    {
        return withVertices3d(std::vector<float>{0.0f, 0.0f, 0.0f});
    }

    Dynamit& Dynamit::withNormals3d(const std::vector<float>& data)
    {
        VAOData& vd = currentVao();
        glBindVertexArray(vd.vao);

        GLuint location = getLocationFor("normal");
        auto buffer = std::make_unique<GlArrayBuffer>(location, "normal");
        buffer->build();
        buffer->withData(data);
        buffer->bufferData();
        buffer->attrib(3, GL_FLOAT);
        vd.glSet.setNormalsBuffer(std::move(buffer));
        return *this;
    }

    Dynamit& Dynamit::withNormals3d(const float* data, size_t count)
    {
        return withNormals3d(std::vector<float>(data, data + count));
    }

    Dynamit& Dynamit::withColors3d(const std::vector<float>& data)
    {
        VAOData& vd = currentVao();
        glBindVertexArray(vd.vao);

        GLuint location = getLocationFor("color");
        auto buffer = std::make_unique<GlArrayBuffer>(location, "color");
        buffer->build();
        buffer->withData(data);
        buffer->bufferData();
        buffer->attrib(3, GL_FLOAT);
        vd.glSet.setColorsBuffer(std::move(buffer));
        return *this;
    }

    Dynamit& Dynamit::withColors4d(const std::vector<float>& data)
    {
        VAOData& vd = currentVao();
        glBindVertexArray(vd.vao);

        GLuint location = getLocationFor("color");
        auto buffer = std::make_unique<GlArrayBuffer>(location, "color");
        buffer->build();
        buffer->withData(data);
        buffer->bufferData();
        buffer->attrib(4, GL_FLOAT);
        vd.glSet.setColorsBuffer(std::move(buffer));
        return *this;
    }

    Dynamit& Dynamit::withConstColor(const std::array<float, 4>& color, const std::string& name)
    {
        currentVao().glSet.setConstColor({ name, color });
        return *this;
    }

    Dynamit& Dynamit::withConstColor(float r, float g, float b, float a, const std::string& name)
    {
        currentVao().glSet.setConstColor({ name, {r, g, b, a} });
        return *this;
    }

    Dynamit& Dynamit::withConstLightDirection(const std::array<float, 3>& dir, bool normalize, const std::string& name)
    {
        currentVao().glSet.setLightDirection({ name, dir, normalize, true, -1 });
        return *this;
    }

    Dynamit& Dynamit::withConstLightDirection(float x, float y, float z, bool normalize, const std::string& name)
    {
        currentVao().glSet.setLightDirection({ name, {x, y, z}, normalize, true, -1 });
        return *this;
    }

    Dynamit& Dynamit::withConstTranslation(const std::array<float, 4>& trans, const std::string& name)
    {
        currentVao().glSet.setConstTranslation({ name, trans });
        return *this;
    }

    Dynamit& Dynamit::withConstTranslation(float x, float y, float z, float w, const std::string& name)
    {
        return withConstTranslation({ x, y, z, w }, name);
    }

    Dynamit& Dynamit::withTranslation4f(const std::string& name)
    {
        currentVao().glSet.setTranslation({ name, {0.0f, 0.0f, 0.0f, 0.0f}, -1 });
        return *this;
    }

    Dynamit& Dynamit::withLightDirection3f(float x, float y, float z, bool normalize, const std::string& name)
    {
        currentVao().glSet.setLightDirection({ name, {x, y, z}, normalize, false, -1 });
        return *this;
    }

    Dynamit& Dynamit::withLightDirection3f(const std::array<float, 3>& dir, bool normalize, const std::string& name)
    {
        return withLightDirection3f(dir[0], dir[1], dir[2], normalize, name);
    }

    Dynamit& Dynamit::withPrecision(const std::string& precision)
    {
        currentVao().glSet.setPrecision(precision);
        return *this;
    }

    Dynamit& Dynamit::withShaderSources(const std::string& vs, const std::string& fs)
    {
        customVertexShader = vs;
        customFragmentShader = fs;
        return *this;
    }

    Dynamit& Dynamit::withVertexShader(const std::string& vs)
    {
        customVertexShader = vs;
        return *this;
    }

    Dynamit& Dynamit::withFragmentShader(const std::string& fs)
    {
        customFragmentShader = fs;
        return *this;
    }

    void Dynamit::buildProgram()
    {
        if (programBuilt) return;

        // Finalize stride layout if in stride mode
        if (strideMode)
        {
            strideLayout.finalize();

            // Apply layout to root VAO if it has a stride buffer
            if (vaoList[0].strideBuffer != 0)
            {
                applyStrideLayout(vaoList[0]);
                // Calculate vertex count from buffer size
                GLint bufferSize = 0;
                glBindBuffer(GL_ARRAY_BUFFER, vaoList[0].strideBuffer);
                glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
                vaoList[0].vertexCount = bufferSize / strideLayout.getStride();
            }
        }

        bool isOverridden = !customVertexShader.empty() || !customFragmentShader.empty();

        if (isOverridden)
        {
            assert(!customVertexShader.empty() && !customFragmentShader.empty() &&
                "Both vertex and fragment shaders must be provided when overriding");

            program.buildVertexFragmentShaders(customVertexShader.c_str(), customFragmentShader.c_str());
            programBuilt = true;
            return;
        }

        if (!strategy)
        {
            if (strideMode)
                strategy = std::make_unique<ShaderStrategy>(vaoList[0].glSet, &strideLayout);
            else
                strategy = std::make_unique<ShaderStrategy>(vaoList[0].glSet);
        }

        ShaderStrategy::ShaderSources sources = strategy->build();
        program.buildVertexFragmentShaders(sources.vertexShader.c_str(), sources.fragmentShader.c_str());
        programBuilt = true;
    }

    ShaderStrategy::ShaderSources Dynamit::getShaders()
    {
        if (!customVertexShader.empty() && !customFragmentShader.empty())
            return { customVertexShader, customFragmentShader };

        if (!strategy)
        {
            if (strideMode)
                strategy = std::make_unique<ShaderStrategy>(vaoList[0].glSet, &strideLayout);
            else
                strategy = std::make_unique<ShaderStrategy>(vaoList[0].glSet);
        }

        return strategy->build();
    }

    void Dynamit::logShaders(const std::string& message)
    {
        ShaderStrategy::ShaderSources sources = getShaders();

        bool vsOverridden = !customVertexShader.empty();
        bool fsOverridden = !customFragmentShader.empty();

        if (!message.empty())
            std::cout << message << "\n";

        std::cout << "=== VERTEX SHADER ("
            << (vsOverridden ? "OVERRIDDEN" : "GENERATED")
            << ") ===\n" << sources.vertexShader << "\n";

        std::cout << "=== FRAGMENT SHADER ("
            << (fsOverridden ? "OVERRIDDEN" : "GENERATED")
            << ") ===\n" << sources.fragmentShader << "\n";
    }

    void Dynamit::logGeneratedShaders(const std::string& message)
    {
        if (!strategy)
        {
            if (strideMode)
                strategy = std::make_unique<ShaderStrategy>(vaoList[0].glSet, &strideLayout);
            else
                strategy = std::make_unique<ShaderStrategy>(vaoList[0].glSet);
        }

        ShaderStrategy::ShaderSources sources = strategy->build();

        if (!message.empty())
            std::cout << message << "\n";
        std::cout << "=== VERTEX SHADER (GENERATED) ===\n" << sources.vertexShader << "\n";
        std::cout << "=== FRAGMENT SHADER (GENERATED) ===\n" << sources.fragmentShader << "\n";
    }

    ShaderStrategy::ShaderSources Dynamit::getGeneratedShaders()
    {
        if (!strategy)
        {
            if (strideMode)
                strategy = std::make_unique<ShaderStrategy>(vaoList[0].glSet, &strideLayout);
            else
                strategy = std::make_unique<ShaderStrategy>(vaoList[0].glSet);
        }
        return strategy->build();
    }

    void Dynamit::bindVertexArray() const
    {
        if (!vaoList.empty())
            glBindVertexArray(vaoList[0].vao);
    }

    void Dynamit::bindVertexArray(size_t index) const
    {
        if (index < vaoList.size())
            glBindVertexArray(vaoList[index].vao);
    }

    void Dynamit::useProgram()
    {
        buildProgram();
        glUseProgram(program.id);
    }

    void Dynamit::bind()
    {
        useProgram();
        bindVertexArray();
    }

    Dynamit& Dynamit::withPrimitive(GLenum primitive)
    {
        currentVao().primitiveType = primitive;
        return *this;
    }

    void Dynamit::drawTriangles(GLint start)
    {
        useProgram();

        for (const auto& vd : vaoList)
        {
            if (vd.vao != 0 && vd.vertexCount > 0)
            {
                glBindVertexArray(vd.vao);
                glDrawArrays(vd.primitiveType, start, static_cast<GLsizei>(vd.vertexCount));
            }
        }
    }

    void Dynamit::drawTriangleFan(GLint start)
    {
        useProgram();

        for (const auto& vd : vaoList)
        {
            if (vd.vao != 0 && vd.vertexCount > 0)
            {
                glBindVertexArray(vd.vao);
                glDrawArrays(GL_TRIANGLE_FAN, start, static_cast<GLsizei>(vd.vertexCount));
            }
        }
    }

    void Dynamit::drawArrays(GLenum mode, GLint start, GLsizei count)
    {
        bind();
        glDrawArrays(mode, start, count);
    }

    void Dynamit::translate4f(float x, float y, float z, float w)
    {
        if (!vaoList[0].glSet.getTranslation())
            throw std::runtime_error("Translation uniform not initialized. Call withTranslation4f() first.");

        useProgram();

        Translation& trans = const_cast<Translation&>(*vaoList[0].glSet.getTranslation());

        if (trans.location == -1)
        {
            trans.location = glGetUniformLocation(program.id, trans.name.c_str());
            if (trans.location == -1)
                throw std::runtime_error("Translation uniform not found in shader");
        }

        glUniform4f(trans.location, x, y, z, w);
        trans.data = { x, y, z, w };
    }

    void Dynamit::lightDirection3f(float x, float y, float z)
    {
        if (!vaoList[0].glSet.getLightDirection())
            throw std::runtime_error("Light direction uniform not initialized. Call withLightDirection3f() first.");

        useProgram();

        LightDirection& light = const_cast<LightDirection&>(*vaoList[0].glSet.getLightDirection());

        if (light.location == -1)
        {
            light.location = glGetUniformLocation(program.id, light.name.c_str());
            if (light.location == -1)
                throw std::runtime_error("Light direction uniform not found in shader");
        }

        glUniform3f(light.location, x, y, z);
        light.data = { x, y, z };
    }

    void Dynamit::updateVertices(const std::vector<float>& newData)
    {
        VAOData& vd = currentVao();
        if (!vd.glSet.getVertexBuffer())
            throw std::runtime_error("Vertex buffer not initialized");

        glBindVertexArray(vd.vao);
        vd.glSet.getVertexBuffer()->bufferData(newData);
        vd.vertexCount = newData.size() / vd.glSet.getVertexBuffer()->getDimension();
    }

    void Dynamit::updateVertices(const float* data, size_t count)
    {
        updateVertices(std::vector<float>(data, data + count));
    }

    // Index buffer methods

    Dynamit& Dynamit::withIndices(const std::vector<uint32_t>& indices)
    {
        return withIndices(indices.data(), indices.size(), GL_UNSIGNED_INT);
    }

    Dynamit& Dynamit::withIndices(const std::vector<uint16_t>& indices)
    {
        return withIndices(indices.data(), indices.size(), GL_UNSIGNED_SHORT);
    }

    Dynamit& Dynamit::withIndices(const std::vector<uint8_t>& indices)
    {
        return withIndices(indices.data(), indices.size(), GL_UNSIGNED_BYTE);
    }

    Dynamit& Dynamit::withIndices(const void* data, size_t count, GLenum type)
    {
        VAOData& vd = currentVao();
        glBindVertexArray(vd.vao);

        // Create or reuse index buffer
        if (vd.indexBuffer == 0)
            glGenBuffers(1, &vd.indexBuffer);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vd.indexBuffer);

        // Calculate byte size based on type
        size_t elementSize = 0;
        switch (type)
        {
        case GL_UNSIGNED_BYTE:  elementSize = sizeof(uint8_t);  break;
        case GL_UNSIGNED_SHORT: elementSize = sizeof(uint16_t); break;
        case GL_UNSIGNED_INT:   elementSize = sizeof(uint32_t); break;
        default:                elementSize = sizeof(uint32_t); break;
        }

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * elementSize, data, GL_STATIC_DRAW);

        vd.indexCount = count;
        vd.indexType = type;

        return *this;
    }

    // Drawing methods
    void Dynamit::drawTrianglesIndexed()
    {
        useProgram();

        for (const auto& vd : vaoList)
        {
            if (vd.vao != 0 && vd.indexCount > 0)
            {
                glBindVertexArray(vd.vao);
                glDrawElements(vd.primitiveType, static_cast<GLsizei>(vd.indexCount),
                    vd.indexType, nullptr);
            }
        }
    }

    void Dynamit::drawElements(GLenum mode, GLsizei count, GLenum type, const void* offset)
    {
        bind();
        glDrawElements(mode, count, type, offset);
    }

    std::unique_ptr<NormalsHighlighter> Dynamit::createNormalsHighlighter(float length)
    {
        VAOData& vd = currentVao();
        
        GlArrayBuffer* vertBuf = vd.glSet.getVertexBuffer();
        GlArrayBuffer* normBuf = vd.glSet.getNormalsBuffer();
        
        if (!vertBuf || !normBuf)
            throw std::runtime_error("Cannot create NormalsHighlighter: Dynamit must have vertices and normals");
        
        std::unique_ptr<NormalsHighlighter> highlighter = std::make_unique<NormalsHighlighter>(length);
        highlighter->build(vertBuf->getData(), normBuf->getData());
        
        return highlighter;
    }
} // namespace dynamit