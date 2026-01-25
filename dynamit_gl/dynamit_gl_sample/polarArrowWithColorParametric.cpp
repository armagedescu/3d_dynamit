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

using namespace dynamit;
using namespace dynamit::builders;

auto generateNormalDebugLinesFromIndices = [](
    const std::vector<float>& verts,
    const std::vector<float>& norms,
    const std::vector<uint32_t>& indices,
    std::vector<float>& lineVerts,
    float normalLength = 0.1f)
    {
        lineVerts.clear();
        lineVerts.reserve(indices.size() * 6);

        for (uint32_t idx : indices)
        {
            size_t vi = idx * 3;  // vertex index into verts array

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

            // End point (vertex + normal * length)
            lineVerts.push_back(vx + nx * normalLength);
            lineVerts.push_back(vy + ny * normalLength);
            lineVerts.push_back(vz + nz * normalLength);
        }
    };

// Build a simple cylinder along Z axis (from z=0 to z=-1)
void buildnCylinder(std::vector<float>& verts, std::vector<float>& norms,
    std::vector<uint32_t>& indices, int sectors, int slices)
{
	float radius = 1.0f;
    uint32_t baseIdx = static_cast<uint32_t>(verts.size() / 3);

    for (int h = 0; h <= slices; h++)
    {
        float z = -static_cast<float>(h) / slices;
        float v = static_cast<float>(h) / slices;

        for (int i = 0; i <= sectors; i++)
        {
            float theta = 2.0f * static_cast<float>(M_PI) * i / sectors;
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

    // Generate indices
    for (int h = 0; h < slices; h++)
    {
        for (int i = 0; i < sectors; i++)
        {
            uint32_t row0 = baseIdx + h * (sectors + 1);
            uint32_t row1 = baseIdx + (h + 1) * (sectors + 1);

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
void transformnGeometry(std::vector<float>& verts, std::vector<float>& norms,
    const mat4<float>& m, size_t startIdx = 0)
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

int main_polarArrowWithColorParametric2()
{
    GLFWwindow* window = openglWindowInit(720, 720);
    if (!window) return -1;

    std::cout << glGetString(GL_VERSION) << std::endl;

    std::vector<float> verts, norms;
    std::vector<uint32_t> indices;

    // Build cylinder along Z, then rotate to point along Y
    buildnCylinder(verts, norms, indices, 8, 2);

    // Scale Z to make it longer, then rotate -90° around X to point up
    mat4<float> scale = scaleMatrix(0.05f, 0.05f, 0.9f);
    mat4<float> rotX = rotation_x_mat4(static_cast<float>(-M_PI / 2));

    transformnGeometry(verts, norms, scale);
    transformnGeometry(verts, norms, rotX);

    //// Print normals to verify
    //std::cout << "First 10 normals after transform:\n";
    //for (size_t i = 0; i < std::min(size_t(30), norms.size()); i += 3) {
    //    std::cout << "  n[" << i / 3 << "] = (" << norms[i] << ", " << norms[i + 1] << ", " << norms[i + 2] << ")\n";
    //}

    // Shaders
    const char* vertSrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
out vec3 vNormal;
uniform mat4 uTransform;
void main() {
    gl_Position = uTransform * vec4(aPos, 1.0);
    vNormal = mat3(uTransform) * aNormal;
}
)";
    const char* fragSrc = R"(
#version 330 core
in vec3 vNormal;
out vec4 fragColor;
const vec3 lightDir = vec3(-0.577, -0.577, 0.577);
const vec3 baseColor = vec3(0.0, 1.0, 0.5);
void main() {
    vec3 n = normalize(vNormal);
    float diff = -dot(lightDir, n);
    fragColor = vec4(baseColor * diff, 1.0) + vec4(0.2, 0.2, 0.2, 0.0);
}
)";

    Program program(vertSrc, fragSrc);
    GLint transformLoc = glGetUniformLocation(program, "uTransform");

    // Setup VAO/VBO/EBO
    GLuint VAO, VBO, NBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, norms.size() * sizeof(float), norms.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    // Simple shader for lines
    // Generate normal debug lines
    std::vector<float> normalLines;
    generateNormalDebugLinesFromIndices(verts, norms, indices, normalLines);

    const char* lineVertSrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uTransform;
void main() {
    gl_Position = uTransform * vec4(aPos, 1.0);
}
)";
    const char* lineFragSrc = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}
)";
    Program lineProgram(lineVertSrc, lineFragSrc);

    GLint lineTransformLoc = glGetUniformLocation(lineProgram, "uTransform");

    // Create VAO/VBO for lines
    GLuint lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, normalLines.size() * sizeof(float), normalLines.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

    mat4<float> transform = identity_mat4();
    float anglex = 0.f, angley = 0.f;
    TimeController tc(glfwGetTime());

    while (!glfwWindowShouldClose(window))
    {
        tc.update(glfwGetTime());

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) anglex += static_cast<float>(tc.deltaTime) * 0.5f;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) anglex -= static_cast<float>(tc.deltaTime) * 0.5f;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) angley += static_cast<float>(tc.deltaTime) * 0.5f;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) angley -= static_cast<float>(tc.deltaTime) * 0.5f;
        glPolygonMode(GL_FRONT_AND_BACK, glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS ? GL_LINE : GL_FILL);
        if(glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS) glFrontFace(GL_CW); else glFrontFace(GL_CCW);

        processInputs(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rotation_x_mat4(anglex, transform);
        rotate_y_mat4(angley, transform);


        glUseProgram(lineProgram);
        glUniformMatrix4fv(lineTransformLoc, 1, GL_FALSE, transform.data());
        glBindVertexArray(lineVAO);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(normalLines.size() / 3));
        glUseProgram(program);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform.data());
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

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

    PolarBuilder builder = Builder::polar();
    if(false)
    {

        builder.doubleCoated().turbo(false)
            .sectors_slices(100, 100)
            .buildConeIndexed(verts, norms, indices, rotation_x_mat4(-M_PI / 2))
            //.buildCylinderIndexed(verts, norms, indices, rotation_x_mat4(-M_PI / 2))
            ;
    } else   
        if (buildCircle)
    {
        builder//.doubleCoated().turbo(true)//.smooth(false)
            .sectors_slices(4, 4)
            .color(std::array<float, 3>{ 1.0f, 0.0f, 1.0f }, std::array<float, 3>{ 0.0f, 1.0f, 0.0f })  // Red
            .buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(-M_PI / 2))
            .color(std::array<float, 3>{0.0f, 1.0f, 0.0f}, std::array<float, 3>{ 1.0f, 0.0f, 1.0f })    // Red
            .buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(-M_PI / 2))
            .buildCylinderIndexedWithColor(verts1, norms1, colors1, indices1, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(-M_PI / 2))
            //////.buildCylinderIndexedWithColor(verts, norms, colors, indices, rotation_x_mat4(-M_PI / 2))// , arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(-M_PI / 2))
            //.color(std::array<float, 3>{ 1.0f, 0.0f, 1.0f }, std::array<float, 3>{ 0.0f, 1.0f, 0.0f })  // Red
            //.buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_y_mat4(-M_PI / 2))
            //.color(std::array<float, 3>{0.0f, 1.0f, 0.0f}, std::array<float, 3>{ 1.0f, 0.0f, 1.0f })  // Red
            //.buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_y_mat4(-M_PI / 2))
            //////.color(std::array<float, 3>{ 1.0f, 0.0f, 1.0f }, std::array<float, 3>{ 0.0f, 1.0f, 0.0f })  // Red
			//.buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(M_PI))
            //////.color(std::array<float, 3>{0.0f, 1.0f, 0.0f}, std::array<float, 3>{ 1.0f, 0.0f, 1.0f })  // Red
            //.buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(M_PI))
            ;
    }else  if (buildHeart)
    {
        builder//.//doubleCoated()
            .formula(L"theta / PI")
            .domain(M_PI)                       // first half
            .sectors_slices(100, 100)
            //.buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate)
            //.buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate)
            //.color(1.0f, 0.0f, 0.0f)  // Red
            .reversed(true).buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(-M_PI / 2))
            //.color(0.0f, 1.0f, 0.0f)  // Green
            .reversed(false).buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(-M_PI / 2))
            //.color(0.0f, 0.0f, 1.0f)  // Red
            //.reversed(true).buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_y_mat4(-M_PI / 2))
            //.reversed(false).buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_y_mat4(-M_PI / 2))
            //.reversed(true).buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(M_PI))
            //.reversed(false).buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(M_PI))
            .formula(L"(2*PI - theta) / PI")    // second half
            .domain_shift(2 * M_PI)
            //.buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate)
            //.buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate)
            //.color(1.0f, 0.0f, 0.0f)  // Red
            .reversed(true).buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(-M_PI / 2))
            //.color(0.0f, 1.0f, 0.0f)  // Green
            .reversed(false).buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(-M_PI / 2))
            //.color(0.0f, 0.0f, 1.0f)  // Red
            //.reversed(true).buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_y_mat4(-M_PI / 2))
            //.reversed(false).buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_y_mat4(-M_PI / 2))
            //.reversed(true).buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(M_PI))
            //.reversed(false).buildCylinderIndexedWithColor  (verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(M_PI))

            ;
    }else if (build5PetalRose)
    {
        builder.doubleCoated()
            .formula(L"cos(5 * theta)")
            .sectors_slices(100, 100)
            //.buildConeIndexed(verts, norms, indices, arrowTipScale, arrowTipTranslate)
            //.buildCylinderIndexed(verts, norms, indices, arrowShaftScale, arrowShaftTranslate)
            .buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(-M_PI / 2))
            .buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(-M_PI / 2))
            .buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_y_mat4(-M_PI / 2))
            .buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_y_mat4(-M_PI / 2))
            .buildConeIndexedWithColor(verts, norms, colors, indices, arrowTipScale, arrowTipTranslate, rotation_x_mat4(M_PI))
            .buildCylinderIndexedWithColor(verts, norms, colors, indices, arrowShaftScale, arrowShaftTranslate, rotation_x_mat4(M_PI))

            ;
    }
    std::cout << "First 10 normals:\n";
    for (size_t i = 0; i < std::min(size_t(30), norms1.size()); i += 3) {
        std::cout << "  n[" << i / 3 << "] = (" << norms1[i] << ", " << norms1[i + 1] << ", " << norms1[i + 2] << ")\n";
    }
    // Setup:
    Dynamit shape;
    shape
        .withVertices3d(verts)
        .withNormals3d(norms)
        //.withColors4d(colors)
        .withIndices(indices)
        .withConstColor({ 0.0, 1.0, 0.5, 1.0 })
		.withConstLightDirection({ -0.577f, -0.577f, 0.577f })
        .withTransformMatrix4f()
        //.withTransformMatrix3f()
        ;
	shape.logGeneratedShaders();


    // Generate normal debug lines
    std::vector<float> normalLines;
    generateNormalDebugLinesFromIndices(verts, norms, indices, normalLines, 0.05f);
    //generateNormalDebugLines(verts, norms, normalLines, 0.15f);

    // Simple shader for lines
    const char* lineVertSrc = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 uTransform;
void main() {
    gl_Position = uTransform * vec4(aPos, 1.0);
}
)";
    const char* lineFragSrc = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}
)";
	Program lineProgram(lineVertSrc, lineFragSrc);

    GLint lineTransformLoc = glGetUniformLocation(lineProgram, "uTransform");

    // Create VAO/VBO for lines
    GLuint lineVAO, lineVBO;
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, normalLines.size() * sizeof(float), normalLines.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

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

        // Draw normal debug lines
        glUseProgram(lineProgram);
        glUniformMatrix4fv(lineTransformLoc, 1, GL_FALSE, mat4Transform.data());
        glBindVertexArray(lineVAO);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(normalLines.size() / 3));

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}

#include "enabler.h"
#ifdef __POLAR_ARROW_WITH_COLOR_PARAMETRIC_CPP__
int main() { return main_polarArrowWithColorParametric2(); }
#endif