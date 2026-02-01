#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "ShapeManager.h"

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

    // JSON helpers
    static std::string escapeJson(const std::string& s)
    {
        std::string result;
        for (char c : s)
        {
            switch (c)
            {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
            }
        }
        return result;
    }

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

inline bool ProjectManager::saveToFile(const std::wstring& filePath)
{
    std::ofstream file(filePath);
    if (!file.is_open())
    {
        m_lastError = L"Failed to open file for writing";
        return false;
    }

    ShapeManager& sm = m_app->getShapeManager();
    VisualizationHelpers& viz = m_app->getVisualizationHelpers();

    std::ostringstream json;
    json << std::fixed << std::setprecision(6);

    json << "{\n";
    json << "  \"version\": 1,\n";
    json << "  \"projectType\": \"dynamit_designer\",\n";

    // View settings
    json << "  \"view\": {\n";
    json << "    \"rotationX\": " << m_app->getRotationX() << ",\n";
    json << "    \"rotationY\": " << m_app->getRotationY() << ",\n";
    json << "    \"zoom\": " << m_app->getZoom() << ",\n";
    json << "    \"showNormals\": " << (m_app->getShowNormals() ? "true" : "false") << ",\n";
    json << "    \"showAxisX\": " << (viz.getShowAxisX() ? "true" : "false") << ",\n";
    json << "    \"showAxisY\": " << (viz.getShowAxisY() ? "true" : "false") << ",\n";
    json << "    \"showAxisZ\": " << (viz.getShowAxisZ() ? "true" : "false") << ",\n";
    json << "    \"showGrid3D\": " << (viz.getShowGrid() ? "true" : "false") << ",\n";
    json << "    \"showGridXZ\": " << (viz.getShowGridXZ() ? "true" : "false") << ",\n";
    json << "    \"showGridXY\": " << (viz.getShowGridXY() ? "true" : "false") << ",\n";
    json << "    \"showGridYZ\": " << (viz.getShowGridYZ() ? "true" : "false") << ",\n";
    json << "    \"showLight\": " << (viz.getShowLight() ? "true" : "false") << "\n";
    json << "  },\n";

    // Selected shape
    json << "  \"selectedShape\": " << m_app->getSelectedShapeIndex() << ",\n";

    // Shapes array
    json << "  \"shapes\": [\n";

    for (int i = 0; i < sm.getShapeCount(); ++i)
    {
        const ShapeInstance* shape = sm.getShape(i);
        if (!shape) continue;

        const ShapeConfig& cfg = shape->config;

        if (i > 0) json << ",\n";

        json << "    {\n";
        json << "      \"name\": \"" << escapeJson(cfg.name) << "\",\n";
        json << "      \"type\": \"" << (cfg.type == ShapeConfig::Type::Cone ? "cone" : "cylinder") << "\",\n";
        json << "      \"formula\": \"" << escapeJson(wstringToUtf8(cfg.formula)) << "\",\n";
        json << "      \"domainStart\": " << cfg.domainStart << ",\n";
        json << "      \"domainEnd\": " << cfg.domainEnd << ",\n";
        json << "      \"sectors\": " << cfg.sectors << ",\n";
        json << "      \"slices\": " << cfg.slices << ",\n";
        json << "      \"smooth\": " << (cfg.smooth ? "true" : "false") << ",\n";
        json << "      \"turbo\": " << (cfg.turbo ? "true" : "false") << ",\n";
        json << "      \"doubleCoated\": " << (cfg.doubleCoated ? "true" : "false") << ",\n";
        json << "      \"reversed\": " << (cfg.reversed ? "true" : "false") << ",\n";
        json << "      \"visible\": " << (shape->visible ? "true" : "false") << ",\n";
        json << "      \"outerColor\": [" << cfg.outerColor[0] << ", " << cfg.outerColor[1] << ", "
             << cfg.outerColor[2] << ", " << cfg.outerColor[3] << "],\n";
        json << "      \"innerColor\": [" << cfg.innerColor[0] << ", " << cfg.innerColor[1] << ", "
             << cfg.innerColor[2] << ", " << cfg.innerColor[3] << "],\n";
        json << "      \"position\": [" << cfg.posX << ", " << cfg.posY << ", " << cfg.posZ << "],\n";
        json << "      \"rotation\": [" << cfg.rotX << ", " << cfg.rotY << ", " << cfg.rotZ << "],\n";
        json << "      \"scale\": [" << cfg.scaleX << ", " << cfg.scaleY << ", " << cfg.scaleZ << "]\n";
        json << "    }";
    }

    json << "\n  ]\n";
    json << "}\n";

    file << json.str();
    file.close();

    m_currentProjectPath = filePath;
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

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    // Simple JSON parser for our known format
    auto findValue = [&](const std::string& key, size_t startPos = 0) -> std::string {
        std::string searchKey = "\"" + key + "\":";
        size_t pos = content.find(searchKey, startPos);
        if (pos == std::string::npos) return "";

        pos += searchKey.length();
        while (pos < content.size() && (content[pos] == ' ' || content[pos] == '\t')) pos++;

        if (pos >= content.size()) return "";

        // Handle different value types
        if (content[pos] == '"')
        {
            // String value
            pos++;
            size_t end = pos;
            while (end < content.size() && content[end] != '"')
            {
                if (content[end] == '\\') end++; // Skip escaped char
                end++;
            }
            return content.substr(pos, end - pos);
        }
        else if (content[pos] == '[')
        {
            // Array value
            size_t end = content.find(']', pos);
            if (end == std::string::npos) return "";
            return content.substr(pos, end - pos + 1);
        }
        else
        {
            // Number or boolean
            size_t end = pos;
            while (end < content.size() && content[end] != ',' && content[end] != '\n' && content[end] != '}')
                end++;
            std::string val = content.substr(pos, end - pos);
            // Trim whitespace
            while (!val.empty() && (val.back() == ' ' || val.back() == '\t' || val.back() == '\r'))
                val.pop_back();
            return val;
        }
    };

    auto parseFloat = [](const std::string& s) -> float {
        try { return std::stof(s); }
        catch (...) { return 0.0f; }
    };

    auto parseInt = [](const std::string& s) -> int {
        try { return std::stoi(s); }
        catch (...) { return 0; }
    };

    auto parseBool = [](const std::string& s) -> bool {
        return s == "true";
    };

    auto parseFloatArray = [&](const std::string& arr, float* out, int count) {
        // Format: [1.0, 2.0, 3.0]
        size_t pos = 1; // Skip '['
        for (int i = 0; i < count && pos < arr.size(); ++i)
        {
            while (pos < arr.size() && (arr[pos] == ' ' || arr[pos] == ',')) pos++;
            size_t end = pos;
            while (end < arr.size() && arr[end] != ',' && arr[end] != ']') end++;
            if (end > pos)
            {
                out[i] = parseFloat(arr.substr(pos, end - pos));
            }
            pos = end + 1;
        }
    };

    auto unescapeJson = [](const std::string& s) -> std::string {
        std::string result;
        for (size_t i = 0; i < s.size(); ++i)
        {
            if (s[i] == '\\' && i + 1 < s.size())
            {
                switch (s[i + 1])
                {
                case '"': result += '"'; i++; break;
                case '\\': result += '\\'; i++; break;
                case 'n': result += '\n'; i++; break;
                case 'r': result += '\r'; i++; break;
                case 't': result += '\t'; i++; break;
                default: result += s[i]; break;
                }
            }
            else
            {
                result += s[i];
            }
        }
        return result;
    };

    // Check version
    int version = parseInt(findValue("version"));
    if (version != 1)
    {
        m_lastError = L"Unsupported project version";
        return false;
    }

    // Load view settings
    VisualizationHelpers& viz = m_app->getVisualizationHelpers();

    std::string rotX = findValue("rotationX");
    std::string rotY = findValue("rotationY");
    std::string zoom = findValue("zoom");

    if (!rotX.empty()) m_app->setRotationX(parseFloat(rotX));
    if (!rotY.empty()) m_app->setRotationY(parseFloat(rotY));
    if (!zoom.empty()) m_app->setZoom(parseFloat(zoom));

    m_app->setShowNormals(parseBool(findValue("showNormals")));
    viz.setShowAxisX(parseBool(findValue("showAxisX")));
    viz.setShowAxisY(parseBool(findValue("showAxisY")));
    viz.setShowAxisZ(parseBool(findValue("showAxisZ")));
    viz.setShowGrid(parseBool(findValue("showGrid3D")));
    viz.setShowGridXZ(parseBool(findValue("showGridXZ")));
    viz.setShowGridXY(parseBool(findValue("showGridXY")));
    viz.setShowGridYZ(parseBool(findValue("showGridYZ")));
    viz.setShowLight(parseBool(findValue("showLight")));

    int selectedShape = parseInt(findValue("selectedShape"));

    // Clear existing shapes
    m_app->getShapeManager().clearAll();

    // Find shapes array
    size_t shapesPos = content.find("\"shapes\":");
    if (shapesPos == std::string::npos)
    {
        m_lastError = L"No shapes found in project";
        return false;
    }

    // Parse each shape object
    size_t searchPos = shapesPos;
    while (true)
    {
        size_t shapeStart = content.find('{', searchPos);
        if (shapeStart == std::string::npos) break;

        size_t shapeEnd = content.find('}', shapeStart);
        if (shapeEnd == std::string::npos) break;

        // Check if this is inside shapes array
        size_t arrayEnd = content.find(']', shapesPos);
        if (shapeStart > arrayEnd) break;

        std::string shapeJson = content.substr(shapeStart, shapeEnd - shapeStart + 1);

        // Parse shape properties
        ShapeConfig cfg;

        std::string name = findValue("name", shapeStart);
        if (!name.empty()) cfg.name = unescapeJson(name);

        std::string type = findValue("type", shapeStart);
        cfg.type = (type == "cylinder") ? ShapeConfig::Type::Cylinder : ShapeConfig::Type::Cone;

        std::string formula = findValue("formula", shapeStart);
        if (!formula.empty()) cfg.formula = utf8ToWstring(unescapeJson(formula));

        std::string domainStart = findValue("domainStart", shapeStart);
        std::string domainEnd = findValue("domainEnd", shapeStart);
        if (!domainStart.empty()) cfg.domainStart = parseFloat(domainStart);
        if (!domainEnd.empty()) cfg.domainEnd = parseFloat(domainEnd);

        std::string sectors = findValue("sectors", shapeStart);
        std::string slices = findValue("slices", shapeStart);
        if (!sectors.empty()) cfg.sectors = parseInt(sectors);
        if (!slices.empty()) cfg.slices = parseInt(slices);

        cfg.smooth = parseBool(findValue("smooth", shapeStart));
        cfg.turbo = parseBool(findValue("turbo", shapeStart));
        cfg.doubleCoated = parseBool(findValue("doubleCoated", shapeStart));
        cfg.reversed = parseBool(findValue("reversed", shapeStart));

        std::string outerColor = findValue("outerColor", shapeStart);
        std::string innerColor = findValue("innerColor", shapeStart);
        if (!outerColor.empty()) parseFloatArray(outerColor, cfg.outerColor.data(), 4);
        if (!innerColor.empty()) parseFloatArray(innerColor, cfg.innerColor.data(), 4);

        std::string position = findValue("position", shapeStart);
        std::string rotation = findValue("rotation", shapeStart);
        std::string scale = findValue("scale", shapeStart);

        if (!position.empty())
        {
            float pos[3] = { 0, 0, 0 };
            parseFloatArray(position, pos, 3);
            cfg.posX = pos[0]; cfg.posY = pos[1]; cfg.posZ = pos[2];
        }
        if (!rotation.empty())
        {
            float rot[3] = { 0, 0, 0 };
            parseFloatArray(rotation, rot, 3);
            cfg.rotX = rot[0]; cfg.rotY = rot[1]; cfg.rotZ = rot[2];
        }
        if (!scale.empty())
        {
            float scl[3] = { 1, 1, 1 };
            parseFloatArray(scale, scl, 3);
            cfg.scaleX = scl[0]; cfg.scaleY = scl[1]; cfg.scaleZ = scl[2];
        }

        // Add shape
        int idx = m_app->getShapeManager().addShape(cfg);

        // Set visibility
        ShapeInstance* inst = m_app->getShapeManager().getShape(idx);
        if (inst)
        {
            std::string visible = findValue("visible", shapeStart);
            inst->visible = visible.empty() ? true : parseBool(visible);
        }

        searchPos = shapeEnd + 1;
    }

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

    return true;
}

inline bool ProjectManager::createEmptyProject()
{
    if (!m_hasProject || m_currentProjectPath.empty())
    {
        m_lastError = L"No project initialized";
        return false;
    }

    // Create an empty project file with default settings
    std::ofstream file(m_currentProjectPath);
    if (!file.is_open())
    {
        m_lastError = L"Failed to create project file";
        return false;
    }

    // Write minimal valid project JSON
    file << "{\n";
    file << "  \"version\": 1,\n";
    file << "  \"projectType\": \"dynamit_designer\",\n";
    file << "  \"projectName\": \"" << wstringToUtf8(m_projectName) << "\",\n";
    file << "  \"view\": {\n";
    file << "    \"rotationX\": 0.3,\n";
    file << "    \"rotationY\": 0.5,\n";
    file << "    \"zoom\": 3.0,\n";
    file << "    \"showNormals\": false,\n";
    file << "    \"showAxisX\": true,\n";
    file << "    \"showAxisY\": true,\n";
    file << "    \"showAxisZ\": true,\n";
    file << "    \"showGrid3D\": false,\n";
    file << "    \"showGridXZ\": true,\n";
    file << "    \"showGridXY\": false,\n";
    file << "    \"showGridYZ\": false,\n";
    file << "    \"showLight\": false\n";
    file << "  },\n";
    file << "  \"selectedShape\": -1,\n";
    file << "  \"shapes\": []\n";
    file << "}\n";

    file.close();
    m_isNewProject = false;  // No longer "new" after creating the file
    return true;
}
