#include "enabler.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <Dynamit.h>
#include <geometry.h>
#include <config.h>
#include <callbacks.h>
#include <builders.h>
// Add at the top:
#include "OptionsDialog.h"
#define     GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

using namespace dynamit;
using namespace dynamit::builders;


// Build a simple cylinder along Z axis (from z=0 to z=-1)
void buildnCylinder(std::vector<float>& verts, std::vector<float>& norms,
    std::vector<uint32_t>& indices, int sectors, int slices)
{
    float radius = 1.0f;
    uint32_t baseIdx = static_cast<uint32_t>(verts.size() / 3);

	float dtheta = 2.0f * static_cast<float>(M_PI) / sectors;
    for (int h = 0; h <= slices; h++)
    {
        float z = -static_cast<float>(h) / slices;  // Reverted: negative Z direction

        for (int i = 0; i <= sectors; i++)
        {
            float theta = dtheta * i;
            float x = radius * std::cos(theta);
            float y = radius * std::sin(theta);

            // Normal points radially outward
            float nx = std::cos(theta);
            float ny = std::sin(theta);
            float nz = 0.0f;

            verts.push_back(x);
            verts.push_back(y);
            verts.push_back(z);

            norms.push_back(nx);
            norms.push_back(ny);
            norms.push_back(nz);
        }
    }

    // Generate indices - CCW winding for left-handed system
    for (int h = 0; h < slices; h++)
    {
        uint32_t row0 = baseIdx + h * (sectors + 1);
        uint32_t row1 = baseIdx + (h + 1) * (sectors + 1);

        for (int i = 0; i < sectors; i++)
        {
            uint32_t v00 = row0 + i;
            uint32_t v01 = row0 + i + 1;
            uint32_t v10 = row1 + i;
            uint32_t v11 = row1 + i + 1;

            indices.push_back(v00);
            indices.push_back(v01);
            indices.push_back(v10);

            indices.push_back(v01);
            indices.push_back(v11);
            indices.push_back(v10);
        }
    }
}

// Transform vertices and normals
void transformnGeometry(std::vector<float>& verts, std::vector<float>& norms, const mat4<float>& m, size_t startIdx = 0)
{
    for (size_t i = startIdx; i < verts.size(); i += 3)
    {
        float x = verts[i], y = verts[i + 1], z = verts[i + 2];
        verts[i] = m[0] * x + m[4] * y + m[8] * z + m[12];
        verts[i + 1] = m[1] * x + m[5] * y + m[9] * z + m[13];
        verts[i + 2] = m[2] * x + m[6] * y + m[10] * z + m[14];

        float nx = norms[i], ny = norms[i + 1], nz = norms[i + 2];
        float tnx = m[0] * nx + m[4] * ny + m[8] * nz;
        float tny = m[1] * nx + m[5] * ny + m[9] * nz;
        float tnz = m[2] * nx + m[6] * ny + m[10] * nz;
        float len = std::sqrt(tnx * tnx + tny * tny + tnz * tnz);
        if (len > 0.0001f) { tnx /= len; tny /= len; tnz /= len; }
        norms[i] = tnx; norms[i + 1] = tny; norms[i + 2] = tnz;
    }
}

const char* normalsShowerVertexShader = R"(
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
const char* normalsShowerFragmentShader = R"(
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
    //GLint normalLengthLoc = 0;
    GLint colorStartLoc = 0;
    GLint colorEndLoc = 0;
    GLuint vao = 0, vbo = 0, endpointVbo = 0;

public:

    NormalsHighlighter(float normalLength = 0.1f) : NormalsHighlighter(normalsShowerVertexShader, normalsShowerFragmentShader, normalLength)
    {
    }
    NormalsHighlighter(const char* vertexPath, const char* fragmentPath, float _normalLength = 0.1f) : Shape(vertexPath, fragmentPath), normalLength(_normalLength)
    {
    }
    std::vector<float> normalLines;
    std::vector<uint8_t> endpointMap;  // Alternating 0 and 1 for start/end of each spike
    float normalLength = 0.1f;

    void visualNormalFromVertsNorms(
        const std::vector<float>& verts,
        const std::vector<float>& norms,
        std::vector<float>& lineVerts,
        std::vector<uint8_t>& endpoints, float _normalLength = 0.1f)
    {
        lineVerts.reserve(lineVerts.size() + verts.size() * 2);
        endpoints.reserve(endpoints.size() + (verts.size() / 3) * 2);

        for (size_t vi = 0; vi < verts.size(); vi += 3)
        {
            float vx = verts[vi];
            float vy = verts[vi + 1];
            float vz = verts[vi + 2];

            float nx = norms[vi];
            float ny = norms[vi + 1];
            float nz = norms[vi + 2];

            // Start point (vertex position)
            lineVerts.push_back(vx);
            lineVerts.push_back(vy);
            lineVerts.push_back(vz);
            endpoints.push_back(0);  // Start of spike

            // End point (vertex + normal * length)
            lineVerts.push_back(vx + nx *  _normalLength);
            lineVerts.push_back(vy + ny *  _normalLength);
            lineVerts.push_back(vz + nz *  _normalLength);
            endpoints.push_back(1);  // End of spike
        }
    };

    void build(std::vector<float>& verts, std::vector<float>& norms) {
        visualNormalFromVertsNorms(verts, norms, normalLines, endpointMap, normalLength);

        transformLoc = glGetUniformLocation(*this, "uTransform");
        //normalLengthLoc = glGetUniformLocation(*this, "uNormalLength");
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

        // Endpoint map buffer (0 = start, 1 = end)
        glBindBuffer(GL_ARRAY_BUFFER, endpointVbo);
        glBufferData(GL_ARRAY_BUFFER, endpointMap.size() * sizeof(uint8_t), endpointMap.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 1, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(uint8_t), nullptr);
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void draw(mat4<float>& transform, 
              //float normalLength = 0.1f,
              const float* colorStart = nullptr, 
              const float* colorEnd = nullptr)
    {
        static const float defaultColorStart[3] = { 1.0f, 0.0f, 1.0f };  // Yellow
        static const float defaultColorEnd[3] = { 1.0f, 1.0f, 0.0f };    // Red

        glUseProgram(*this);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform.data());
        //glUniform1f(normalLengthLoc, normalLength);
        glUniform3fv(colorStartLoc, 1, colorStart ? colorStart : defaultColorStart);
        glUniform3fv(colorEndLoc, 1, colorEnd ? colorEnd : defaultColorEnd);

        glBindVertexArray(vao);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(normalLines.size() / 3));
    }
};

const char* cyliderVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
out vec3 vNormal;
uniform mat4 uTransform;
void main() {
    gl_Position = uTransform * vec4(aPos, 1.0);
    vNormal = vec3 (uTransform * vec4(aNormal, 0.0));
}
)";
const char* cylinderFragmentShader = R"(
#version 330 core
in  vec3 vNormal;
out vec4 fragColor;
const vec3 lightDir  = vec3(-0.577, -0.577, 0.577);
const vec3 baseColor = vec3(0.0, 1.0, 0.5);
void main() {
    float diff = dot(normalize(-lightDir),  normalize(vNormal));
    fragColor  = vec4(baseColor * diff, 1.0) + vec4(0.2, 0.2, 0.2, 0.0);
}
)";
class Cylinder : public Shape
{
    GLuint vao, vbo, nbo, ebo;
    GLint transformLoc = 0;

public:
    std::vector<float> verts, norms;
    std::vector<uint32_t> indices;


    Cylinder() : Cylinder(cyliderVertexShader, cylinderFragmentShader)
    {
    }
    Cylinder(const char* vertexPath, const char* fragmentPath) : Shape(vertexPath, fragmentPath)
    {
        build();
    }

    void build() {
        //GLuint VAO, VBO, NBO, EBO;
        buildnCylinder(verts, norms, indices, 8, 8);

        // Scale Z to make it shorter, then rotate -90° around X to point up
        mat4<float> scale = scaleMatrix(0.05f, 0.05f, 1.0f);
        mat4<float> rotX = rotation_x_mat4(static_cast<float>(-M_PI / 2));

        transformnGeometry(verts, norms, scale);

        glGenVertexArrays(1, &vao);
        glUseProgram(*this);
        glBindVertexArray(vao);
        
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &nbo);
        glGenBuffers(1, &ebo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, nbo);
        glBufferData(GL_ARRAY_BUFFER, norms.size() * sizeof(float), norms.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
        transformLoc = glGetUniformLocation(program, "uTransform");


    }
    void draw(mat4<float>& transform)
    {
        glUseProgram(*this);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform.data());
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);

    }
};

int main_polarArrowWithColorParametricRawResearch()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window) return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;


    TimeController tc(glfwGetTime());
    // In main_polarArrowWithColorParametric2(), before the render loop:
    RenderOptions options;
    std::unique_ptr<OptionsDialog> optsDlg = std::make_unique<OptionsDialog>(options); //new OptionsDialog(options);
    assert(optsDlg && "Failed to create OptionsDialog");
    HWND hWndGlfw = glfwGetWin32Window(window);
    optsDlg->CreateModeless(hWndGlfw);

    Cylinder cylinder;
	NormalsHighlighter normalsHighlighter;
	normalsHighlighter.build(cylinder.verts, cylinder.norms);

    mat4<float> transform = identity_mat4();
    float anglex = 0.f, angley = 0.f;
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
    while (!glfwWindowShouldClose(window))
    {
        tc.update(glfwGetTime());

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) anglex += static_cast<float>(tc.deltaTime) * 0.5f;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) anglex -= static_cast<float>(tc.deltaTime) * 0.5f;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) angley += static_cast<float>(tc.deltaTime) * 0.5f;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) angley -= static_cast<float>(tc.deltaTime) * 0.5f;
        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);
        //if(glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS) glFrontFace(GL_CW); else glFrontFace(GL_CCW);


        // Apply options:
        if (options.cullFace) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        //glFrontFace(options.cwWinding ? GL_CW : GL_CCW);
        glPolygonMode(GL_FRONT_AND_BACK, options.wireframe ? GL_LINE : GL_FILL);

        processInputs(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rotation_x_mat4(anglex, transform);
        rotate_y_mat4(angley, transform);

        //std::wcout << L"draw shapes" << std::endl;
        cylinder.draw(transform);
        if (options.showNormals) normalsHighlighter.draw(transform);


        // In the render loop, add key handler to open dialog:
        if (glfwGetKey(window, GLFW_KEY_F9) == GLFW_PRESS)
            optsDlg->ShowWindow(SW_SHOW);

        // Process Windows messages for the dialog
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (!optsDlg || !IsDialogMessage(optsDlg->m_hWnd, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }


        // Draw normals conditionally:
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    if (optsDlg && optsDlg->IsWindow())
        optsDlg->DestroyWindow();
    glfwTerminate();
    return 0;
}

int main_polarArrowWithColorParametric()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window)
        return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

    std::vector<float> verts, norms, colors, verts1, norms1, colors1, verts2, norms2, colors2;
    std::vector<uint32_t> indices, indices1, indices2 ;

    // Create transformation matrices for positioning multiple shapes

	float arrowHeadHeight = 0.5f, arrowHeadWidth = 0.1f, arrowShaftWidth = 0.05f;
	//float arrowHeadHeight = 0.2f, arrowHeadWidth = 0.1f, arrowShaftWidth = 0.025f;
    mat4<float> arrowShaftTranslate = translation_mat4(0.0f, 0.0f, 1.0f);
    mat4<float> arrowShaftScale = scaleMatrix(arrowShaftWidth, arrowShaftWidth, 2.0f - arrowHeadHeight);
    mat4<float> arrowTipScale = scaleMatrix(arrowHeadWidth, arrowHeadWidth, arrowHeadHeight);
    mat4<float> arrowTipTranslate = translation_mat4(0.0f, 0.0f, -1.0f + arrowHeadHeight);
    bool buildCircle = true, buildHeart = false, build5PetalRose = false;

    PolarBuilder builder = Builder::polar()
            .sectors_slices(4, 4).doubleCoated()
            .color(std::array<float, 3>{ 1.0f, 0.0f, 1.0f }, std::array<float, 3>{ 0.0f, 1.0f, 0.0f })  // Red
            .reversed()
            .buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(-M_PI / 2))
            .color(std::array<float, 3>{0.0f, 1.0f, 0.0f}, std::array<float, 3>{ 1.0f, 0.0f, 1.0f })    // Red
            .buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(-M_PI / 2))
            .color(std::array<float, 3>{ 1.0f, 0.0f, 1.0f }, std::array<float, 3>{ 0.0f, 1.0f, 0.0f })  // Red
            .buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_y_mat4(-M_PI / 2))
            .color(std::array<float, 3>{0.0f, 1.0f, 0.0f}, std::array<float, 3>{ 1.0f, 0.0f, 1.0f })  // Red
            .buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_y_mat4(-M_PI / 2))
            .color(std::array<float, 3>{ 1.0f, 0.0f, 1.0f }, std::array<float, 3>{ 0.0f, 1.0f, 0.0f })  // Red
            .reversed(false)
			.buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(M_PI))
            .color(std::array<float, 3>{0.0f, 1.0f, 0.0f}, std::array<float, 3>{ 1.0f, 0.0f, 1.0f })    // Red
            .buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(M_PI))
            ;

    // Setup:
    Dynamit shape;
    shape
        .withVertices3d(verts)
        .withNormals3d(norms)
        .withColors4d(colors)
        .withIndices(indices)
        //.withConstColor({ 0.0, 1.0, 0.5, 1.0 })
		.withConstLightDirection({ -0.577f, -0.577f, 0.577f })
        .withTransformMatrix4f()
        //.withTransformMatrix3f()
        ;
	shape.logGeneratedShaders();


    //// Generate normal debug lines
    NormalsHighlighter normalsHighlighter (0.05f);
    normalsHighlighter.build(verts, norms);


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 1.f, 0.9f);

    mat4<float> mat4Transform = {};
    // Render loop
    float anglex = 0.f, angley = 0.f, anglez = 0.f;
	TimeController tc(glfwGetTime());
    while (!glfwWindowShouldClose(window))
    {
		tc.update(glfwGetTime());
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) anglex += static_cast<float>(tc.deltaTime) * 0.5f; // slow rotation
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) anglex += static_cast<float>(tc.deltaTime) * -0.5f; // slow rotation
        if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        {
            //std::cout << "left alt pressed\n" << std::endl;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) anglez += static_cast<float>(tc.deltaTime) * 0.5f; // slow rotation
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) anglez += static_cast<float>(tc.deltaTime) * -0.5f; // slow rotation
		}
        else
        {
            //std::cout << "left alt not pressed\n" << std::endl;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) angley += static_cast<float>(tc.deltaTime) * 0.5f; // slow rotation
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) angley += static_cast<float>(tc.deltaTime) * -0.5f; // slow rotation
        }
        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);

        processInputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shape.useProgram();

        rotation_x_mat4(anglex, mat4Transform);
		rotate_y_mat4(angley, mat4Transform);
		//rotate_z_mat4(anglez, mat4Transform);

        shape.transformMatrix4f(mat4Transform);
        shape.drawTrianglesIndexed();
        normalsHighlighter.draw(mat4Transform);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

#include "enabler.h"
#ifdef __POLAR_ARROW_WITH_COLOR_PARAMETRIC_CPP__
//int main() { return main_polarArrowWithColorParametric(); }
int main() { return main_polarArrowWithColorParametricRawResearch(); }
#endif