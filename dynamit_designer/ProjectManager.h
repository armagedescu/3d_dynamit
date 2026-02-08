#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#include <string>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>
#include "ShapeManager.h"

using json = nlohmann::json;

// Project file format: JSON
// Extension: .dproj (Dynamit Project)

class DesignerApp;  // Forward declaration

class ProjectManager
{
public:
    ProjectManager(DesignerApp* app) : m_app(app), m_hasProject(false) {}

    // Initialize with project info (from startup dialog)
    void initProject(const std::wstring& projectPath,
                     const std::wstring& projectDir,
                     const std::wstring& projectName,
                     bool isNew)
    {
        m_currentProjectPath = projectPath;
        m_projectDirectory = projectDir;
        m_projectName = projectName;
        m_hasProject = true;
        m_isNewProject = isNew;
    }

    bool hasProject() const { return m_hasProject; }
    bool isNewProject() const { return m_isNewProject; }

    // Save to current project file (no dialog)
    bool saveProject()
    {
        if (!m_hasProject || m_currentProjectPath.empty())
            return false;
        return saveToFile(m_currentProjectPath);
    }

    // Save with dialog (Save As)
    bool saveProjectAs(HWND parentHwnd)
    {
        std::wstring filePath = showSaveDialog(parentHwnd);
        if (filePath.empty())
            return false;

        // Update project info based on new location
        size_t lastSlash = filePath.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos)
        {
            m_projectDirectory = filePath.substr(0, lastSlash);
            std::wstring fileName = filePath.substr(lastSlash + 1);
            size_t dotPos = fileName.find_last_of(L'.');
            if (dotPos != std::wstring::npos)
                m_projectName = fileName.substr(0, dotPos);
            else
                m_projectName = fileName;
        }
        m_currentProjectPath = filePath;

        return saveToFile(filePath);
    }

    bool loadProject(HWND parentHwnd)
    {
        std::wstring filePath = showOpenDialog(parentHwnd);
        if (filePath.empty())
            return false;

        return loadFromFile(filePath);
    }

    bool saveToFile(const std::wstring& filePath);
    bool loadFromFile(const std::wstring& filePath);

    // Create empty project file for new projects
    bool createEmptyProject();

    const std::wstring& getLastError() const { return m_lastError; }
    const std::wstring& getCurrentProjectPath() const { return m_currentProjectPath; }
    const std::wstring& getProjectDirectory() const { return m_projectDirectory; }
    const std::wstring& getProjectName() const { return m_projectName; }

private:
    std::wstring showSaveDialog(HWND parentHwnd)
    {
        wchar_t fileName[MAX_PATH] = L"untitled.dproj";

        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = parentHwnd;
        ofn.lpstrFilter = L"Dynamit Project (*.dproj)\0*.dproj\0All Files (*.*)\0*.*\0";
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrDefExt = L"dproj";
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
        ofn.lpstrTitle = L"Save Dynamit Project";

        if (GetSaveFileNameW(&ofn))
        {
            return fileName;
        }
        return L"";
    }

    std::wstring showOpenDialog(HWND parentHwnd)
    {
        wchar_t fileName[MAX_PATH] = L"";

        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = parentHwnd;
        ofn.lpstrFilter = L"Dynamit Project (*.dproj)\0*.dproj\0All Files (*.*)\0*.*\0";
        ofn.lpstrFile = fileName;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        ofn.lpstrTitle = L"Open Dynamit Project";

        if (GetOpenFileNameW(&ofn))
        {
            return fileName;
        }
        return L"";
    }

    // String conversion helpers
    static std::string wstringToUtf8(const std::wstring& ws)
    {
        if (ws.empty()) return "";
        int size = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string result(size - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &result[0], size, nullptr, nullptr);
        return result;
    }

    static std::wstring utf8ToWstring(const std::string& s)
    {
        if (s.empty()) return L"";
        int size = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
        std::wstring result(size - 1, 0);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &result[0], size);
        return result;
    }

    DesignerApp* m_app;
    std::wstring m_lastError;
    std::wstring m_currentProjectPath;
    std::wstring m_projectDirectory;
    std::wstring m_projectName;
    bool m_hasProject;
    bool m_isNewProject;
};

// Include implementation after DesignerApp is fully defined
#include "DesignerApp.h"
#include "VisualizationHelpers.h"
#include "dialogs/StartupDialog.h"

inline bool ProjectManager::saveToFile(const std::wstring& filePath)
{
    ShapeManager& sm = m_app->getShapeManager();
    VisualizationHelpers& viz = m_app->getVisualizationHelpers();

    std::cout << "[Save] Saving " << sm.getShapeCount() << " shapes to project" << std::endl;

    // Build JSON structure
    json j;
    j["version"] = 1;
    j["projectType"] = "dynamit_designer";

    // View settings
    j["view"] = {
        {"rotationX", m_app->getRotationX()},
        {"rotationY", m_app->getRotationY()},
        {"zoom", m_app->getZoom()},
        {"showNormals", m_app->getShowNormals()},
        {"showAxisX", viz.getShowAxisX()},
        {"showAxisY", viz.getShowAxisY()},
        {"showAxisZ", viz.getShowAxisZ()},
        {"showGrid3D", viz.getShowGrid()},
        {"showGridXZ", viz.getShowGridXZ()},
        {"showGridXY", viz.getShowGridXY()},
        {"showGridYZ", viz.getShowGridYZ()},
        {"showLight", viz.getShowLight()}
    };

    j["selectedShape"] = m_app->getSelectedShapeIndex();

    // Shapes array
    j["shapes"] = json::array();

    for (int i = 0; i < sm.getShapeCount(); ++i)
    {
        const ShapeInstance* shape = sm.getShape(i);
        if (!shape) continue;

        const ShapeConfig& cfg = shape->config;

        json shapeJson = {
            {"name", cfg.name},
            {"type", cfg.type == ShapeConfig::Type::Cone ? "cone" : "cylinder"},
            {"formula", wstringToUtf8(cfg.formula)},
            {"domainStart", cfg.domainStart},
            {"domainEnd", cfg.domainEnd},
            {"sectors", cfg.sectors},
            {"slices", cfg.slices},
            {"smooth", cfg.smooth},
            {"turbo", cfg.turbo},
            {"doubleCoated", cfg.doubleCoated},
            {"reversed", cfg.reversed},
            {"visible", shape->visible},
            {"outerColor", {cfg.outerColor[0], cfg.outerColor[1], cfg.outerColor[2], cfg.outerColor[3]}},
            {"innerColor", {cfg.innerColor[0], cfg.innerColor[1], cfg.innerColor[2], cfg.innerColor[3]}},
            {"position", {cfg.posX, cfg.posY, cfg.posZ}},
            {"rotation", {cfg.rotX, cfg.rotY, cfg.rotZ}},
            {"scale", {cfg.scaleX, cfg.scaleY, cfg.scaleZ}}
        };

        // Save formula segments (multi-segment support)
        if (!cfg.segments.empty())
        {
            json segmentsJson = json::array();
            for (const auto& seg : cfg.segments)
            {
                segmentsJson.push_back({
                    {"formula", wstringToUtf8(seg.formula)},
                    {"domainStart", seg.domainStart},
                    {"domainEnd", seg.domainEnd}
                });
            }
            shapeJson["segments"] = segmentsJson;
        }

        j["shapes"].push_back(shapeJson);
    }

    // Write to file
    std::ofstream file(filePath);
    if (!file.is_open())
    {
        m_lastError = L"Failed to open file for writing";
        return false;
    }

    file << j.dump(2);  // Pretty print with 2-space indent
    file.close();

    m_currentProjectPath = filePath;

    // Add to recent projects
    StartupDialog::AddToRecentProjects(filePath, m_projectName);

    return true;
}

inline bool ProjectManager::loadFromFile(const std::wstring& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        m_lastError = L"Failed to open file for reading";
        return false;
    }

    json j;
    try
    {
        file >> j;
    }
    catch (const json::parse_error& e)
    {
        m_lastError = L"JSON parse error: " + utf8ToWstring(e.what());
        return false;
    }
    file.close();

    // Check version
    int version = j.value("version", 0);
    if (version != 1)
    {
        m_lastError = L"Unsupported project version";
        return false;
    }

    // Load view settings
    VisualizationHelpers& viz = m_app->getVisualizationHelpers();

    if (j.contains("view"))
    {
        const auto& view = j["view"];
        m_app->setRotationX(view.value("rotationX", 0.0f));
        m_app->setRotationY(view.value("rotationY", 0.0f));
        m_app->setZoom(view.value("zoom", 3.0f));
        m_app->setShowNormals(view.value("showNormals", false));
        viz.setShowAxisX(view.value("showAxisX", true));
        viz.setShowAxisY(view.value("showAxisY", true));
        viz.setShowAxisZ(view.value("showAxisZ", true));
        viz.setShowGrid(view.value("showGrid3D", false));
        viz.setShowGridXZ(view.value("showGridXZ", true));
        viz.setShowGridXY(view.value("showGridXY", false));
        viz.setShowGridYZ(view.value("showGridYZ", false));
        viz.setShowLight(view.value("showLight", false));
    }

    int selectedShape = j.value("selectedShape", 0);

    // Clear existing shapes
    m_app->getShapeManager().clearAll();

    // Load shapes
    if (j.contains("shapes") && j["shapes"].is_array())
    {
        for (const auto& shapeJson : j["shapes"])
        {
            ShapeConfig cfg;

            cfg.name = shapeJson.value("name", "Shape");
            std::string type = shapeJson.value("type", "cone");
            cfg.type = (type == "cylinder") ? ShapeConfig::Type::Cylinder : ShapeConfig::Type::Cone;

            cfg.formula = utf8ToWstring(shapeJson.value("formula", "1"));
            cfg.domainStart = shapeJson.value("domainStart", 0.0f);
            cfg.domainEnd = shapeJson.value("domainEnd", 6.2832f);

            // Load formula segments (multi-segment support)
            if (shapeJson.contains("segments") && shapeJson["segments"].is_array())
            {
                cfg.segments.clear();
                for (const auto& segJson : shapeJson["segments"])
                {
                    FormulaSegment seg;
                    seg.formula = utf8ToWstring(segJson.value("formula", "1"));
                    seg.domainStart = segJson.value("domainStart", 0.0f);
                    seg.domainEnd = segJson.value("domainEnd", 6.2832f);
                    cfg.segments.push_back(seg);
                }
            }

            cfg.sectors = shapeJson.value("sectors", 16);
            cfg.slices = shapeJson.value("slices", 8);
            cfg.smooth = shapeJson.value("smooth", true);
            cfg.turbo = shapeJson.value("turbo", true);
            cfg.doubleCoated = shapeJson.value("doubleCoated", true);
            cfg.reversed = shapeJson.value("reversed", true);

            if (shapeJson.contains("outerColor") && shapeJson["outerColor"].is_array())
            {
                const auto& c = shapeJson["outerColor"];
                if (c.size() >= 4)
                {
                    cfg.outerColor = {c[0].get<float>(), c[1].get<float>(), c[2].get<float>(), c[3].get<float>()};
                }
            }

            if (shapeJson.contains("innerColor") && shapeJson["innerColor"].is_array())
            {
                const auto& c = shapeJson["innerColor"];
                if (c.size() >= 4)
                {
                    cfg.innerColor = {c[0].get<float>(), c[1].get<float>(), c[2].get<float>(), c[3].get<float>()};
                }
            }

            if (shapeJson.contains("position") && shapeJson["position"].is_array())
            {
                const auto& p = shapeJson["position"];
                if (p.size() >= 3)
                {
                    cfg.posX = p[0].get<float>();
                    cfg.posY = p[1].get<float>();
                    cfg.posZ = p[2].get<float>();
                }
            }

            if (shapeJson.contains("rotation") && shapeJson["rotation"].is_array())
            {
                const auto& r = shapeJson["rotation"];
                if (r.size() >= 3)
                {
                    cfg.rotX = r[0].get<float>();
                    cfg.rotY = r[1].get<float>();
                    cfg.rotZ = r[2].get<float>();
                }
            }

            if (shapeJson.contains("scale") && shapeJson["scale"].is_array())
            {
                const auto& s = shapeJson["scale"];
                if (s.size() >= 3)
                {
                    cfg.scaleX = s[0].get<float>();
                    cfg.scaleY = s[1].get<float>();
                    cfg.scaleZ = s[2].get<float>();
                }
            }

            // Add shape
            int idx = m_app->getShapeManager().addShape(cfg);

            // Set visibility
            ShapeInstance* inst = m_app->getShapeManager().getShape(idx);
            if (inst)
            {
                inst->visible = shapeJson.value("visible", true);
            }
        }
    }

    std::cout << "[Load] Loaded " << m_app->getShapeManager().getShapeCount() << " shapes" << std::endl;

    // Rebuild all shapes
    m_app->getShapeManager().rebuildAllDirty();

    // Select shape
    if (selectedShape >= 0 && selectedShape < m_app->getShapeManager().getShapeCount())
    {
        m_app->selectShape(selectedShape);
    }
    else if (m_app->getShapeManager().getShapeCount() > 0)
    {
        m_app->selectShape(0);
    }

    // Refresh shapes list panel
    m_app->refreshShapesList();

    // Update project info from loaded path
    m_currentProjectPath = filePath;
    size_t lastSlash = filePath.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos)
    {
        m_projectDirectory = filePath.substr(0, lastSlash);
        std::wstring fileName = filePath.substr(lastSlash + 1);
        size_t dotPos = fileName.find_last_of(L'.');
        if (dotPos != std::wstring::npos)
            m_projectName = fileName.substr(0, dotPos);
        else
            m_projectName = fileName;
    }
    m_hasProject = true;
    m_isNewProject = false;

    // Add to recent projects
    StartupDialog::AddToRecentProjects(filePath, m_projectName);

    return true;
}

inline bool ProjectManager::createEmptyProject()
{
    if (!m_hasProject || m_currentProjectPath.empty())
    {
        m_lastError = L"No project initialized";
        return false;
    }

    // Build JSON structure
    json j;
    j["version"] = 1;
    j["projectType"] = "dynamit_designer";
    j["projectName"] = wstringToUtf8(m_projectName);
    j["view"] = {
        {"rotationX", 0.3},
        {"rotationY", 0.5},
        {"zoom", 3.0},
        {"showNormals", false},
        {"showAxisX", true},
        {"showAxisY", true},
        {"showAxisZ", true},
        {"showGrid3D", false},
        {"showGridXZ", true},
        {"showGridXY", false},
        {"showGridYZ", false},
        {"showLight", false}
    };
    j["selectedShape"] = -1;
    j["shapes"] = json::array();

    // Write to file
    std::ofstream file(m_currentProjectPath);
    if (!file.is_open())
    {
        m_lastError = L"Failed to create project file";
        return false;
    }

    file << j.dump(2);
    file.close();

    m_isNewProject = false;  // No longer "new" after creating the file
    return true;
}
