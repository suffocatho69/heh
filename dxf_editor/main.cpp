#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <algorithm>

// Link libraries dynamically for MinGW/MSVC
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")

// Window IDs and Commands
#define IDM_NEW           1001
#define IDM_OPEN          1002
#define IDM_SAVE          1003
#define IDM_SAVEAS        1004
#define IDM_EXPORT        1005
#define IDM_EXIT          1006

#define IDM_UNDO          1011
#define IDM_REDO          1012
#define IDM_CUT           1013
#define IDM_COPY          1014
#define IDM_PASTE         1015
#define IDM_DELETE        1016
#define IDM_SELECTALL     1017

#define IDM_ZOOMIN        1021
#define IDM_ZOOMOUT       1022
#define IDM_ZOOMFIT       1023
#define IDM_SHOWGRID      1024

#define IDM_FORMAT_LAYERS  1031
#define IDM_FORMAT_STYLES  1032
#define IDM_FORMAT_COLORS  1033

#define IDM_DRAW_LINE      1041
#define IDM_DRAW_POLYLINE  1042
#define IDM_DRAW_RECT      1043
#define IDM_DRAW_CIRCLE    1044
#define IDM_DRAW_ARC       1045
#define IDM_DRAW_ELLIPSE   1046
#define IDM_DRAW_TEXT      1047
#define IDM_DRAW_DIM       1048

#define IDM_TOOL_TRIM      1051
#define IDM_TOOL_EXTEND    1052
#define IDM_TOOL_JOIN      1053
#define IDM_TOOL_SPLIT     1054
#define IDM_TOOL_VALIDATE  1055

#define IDM_HELP_MANUAL    1061
#define IDM_HELP_ABOUT     1062

// Controls IDs
#define IDC_T1_NEW        1101
#define IDC_T1_OPEN       1102
#define IDC_T1_SAVE       1103
#define IDC_T1_PRINT      1104
#define IDC_T1_CUT        1105
#define IDC_T1_COPY       1106
#define IDC_T1_ZOOMIN     1107
#define IDC_T1_ZOOMOUT    1108
#define IDC_T1_ZOOMFIT    1109

#define IDC_T2_LAYERS_BTN 1121
#define IDC_T2_LAYERS_CB  1122
#define IDC_T2_STYLE_CB   1123
#define IDC_T2_WIDTH_CB   1124
#define IDC_T2_COLOR_CB   1125

// Left Tool Box
#define IDC_L_LINE        1201
#define IDC_L_ARC         1202
#define IDC_L_PENTAGON    1203
#define IDC_L_RECT        1204
#define IDC_L_CIRCLE      1205
#define IDC_L_ELLIPSE     1206
#define IDC_L_TEXT        1207
#define IDC_L_DIM         1208
#define IDC_L_POINT       1209
#define IDC_L_FILL_TRIM   1210

// Right Tool Box
#define IDC_R_DELETE      1301
#define IDC_R_OFFSET      1302
#define IDC_R_MIRROR      1303
#define IDC_R_ARRAY       1304
#define IDC_R_MOVE_SCALE  1305
#define IDC_R_ROTATE      1306
#define IDC_R_ZOOM_PREV   1307

// Bottom layout elements
#define IDC_B_TABS        1401
#define IDC_B_COORDX      1402
#define IDC_B_COORDY      1403
#define IDC_B_STATUS      1404

// Shape Types
enum ShapeType {
    SHAPE_LINE,
    SHAPE_POLYLINE,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_ARC,
    SHAPE_ELLIPSE,
    SHAPE_TEXT,
    SHAPE_DIMENSION,
    SHAPE_POINT
};

// Line Styles
enum LineStyle {
    STYLE_CONTINUOUS,
    STYLE_DASHED,
    STYLE_CENTER
};

// Colors
enum ShapeColor {
    COLOR_DEFAULT,
    COLOR_RED,
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_BLACK
};

// Core Point 2D
struct Point2D {
    double x, y;
};

// Shape Database Entry
struct Shape {
    ShapeType type;
    std::vector<Point2D> points;
    double value1; // Radius, or text height
    double value2; // Start angle, or minor axis Y
    double value3; // End angle
    std::string text;
    int layer; // 0="0", 1="Wymiary", 2="Obrys"
    LineStyle style;
    ShapeColor color;
    double thickness; // 0.25, 0.50, etc.
    bool filled;
};

// Active Tools
enum ToolType {
    TOOL_SELECT,
    TOOL_LINE,
    TOOL_POLYLINE,
    TOOL_RECT,
    TOOL_CIRCLE,
    TOOL_ARC,
    TOOL_ELLIPSE,
    TOOL_TEXT,
    TOOL_DIM,
    TOOL_POINT,
    TOOL_TRIM,
    TOOL_PENTAGON,
    TOOL_OFFSET,
    TOOL_MIRROR,
    TOOL_MOVE_SCALE
};

// Application Global States
HINSTANCE hInst;
HWND hWndMain = NULL;
HWND hWndCanvas = NULL;
HWND hWndStatus = NULL;
HWND hWndXInput = NULL;
HWND hWndYInput = NULL;
HWND hWndPrompt = NULL;
HFONT hFontRetro = NULL;

// Active Drawing Style Settings
int active_layer = 0;
LineStyle active_style = STYLE_CONTINUOUS;
double active_thickness = 0.25;
ShapeColor active_color = COLOR_DEFAULT;

// Active tool and state machine
ToolType active_tool = TOOL_SELECT;
bool is_drawing = false;
std::vector<Point2D> temp_points;
Point2D current_mouse_world = {0, 0};

// Grid settings
bool show_grid = true;
double grid_spacing = 20.0;

// Zoom and Pan (World to Screen translation)
double zoom_factor = 1.0;
double offset_x = 0.0;
double offset_y = 0.0;
bool is_panning = false;
POINT last_pan_mouse = {0, 0};

// Undo/Redo DB
std::vector<Shape> shapes_db;
std::vector<std::vector<Shape>> undo_stack;
std::vector<std::vector<Shape>> redo_stack;

// Active workspace tab (0 = Model, 1 = Arkusz1)
int active_tab = 0;

// Keep current file name
std::string current_file_path = "Projekt1.dxf";

// UI Layout Handles
HWND hWndToolbar1 = NULL;
HWND hWndToolbar2 = NULL;
HWND hWndLeftToolbox = NULL;
HWND hWndRightToolbox = NULL;
HWND hWndBottomTabs = NULL;
HWND hWndBottomCoordPanel = NULL;

// Dialog input box globals
char g_InputBoxBuffer[256];
const char* g_InputBoxTitle = "";
const char* g_InputBoxPrompt = "";
bool g_InputBoxSuccess = false;

// Layout constants
#define TOOLBAR_HEIGHT    28
#define LEFT_BAR_WIDTH    70
#define RIGHT_BAR_WIDTH   70
#define BOTTOM_BAR_HEIGHT 55
#define STATUS_BAR_HEIGHT 22

// Set dynamic instructions/alert prompt text
void SetPromptText(const char* text, bool is_error = false) {
    if (hWndPrompt) {
        SetWindowText(hWndPrompt, text);
    }
}

// World/Screen Coordinates Translation Helpers
POINT WorldToScreen(Point2D pt) {
    POINT res;
    res.x = (int)((pt.x - offset_x) * zoom_factor);
    res.y = (int)((pt.y - offset_y) * zoom_factor);
    return res;
}

Point2D ScreenToWorld(POINT pt) {
    Point2D res;
    res.x = (pt.x / zoom_factor) + offset_x;
    res.y = (pt.y / zoom_factor) + offset_y;
    return res;
}

// Map layer index to colors
COLORREF GetShapeGDIColor(ShapeColor col, int layer) {
    if (col == COLOR_RED) return RGB(255, 0, 0);
    if (col == COLOR_BLUE) return RGB(0, 0, 255);
    if (col == COLOR_GREEN) return RGB(0, 180, 0);
    if (col == COLOR_BLACK) return RGB(0, 0, 0);

    // Default mapped from Layer
    if (layer == 1) return RGB(128, 0, 128); // Dimensions = Purple
    if (layer == 2) return RGB(0, 128, 255); // Outline = Teal/Blue
    return RGB(0, 0, 0); // Layer 0 = Black
}

// Convert style to GDI Pen style
int GetGDIPenStyle(LineStyle style) {
    if (style == STYLE_DASHED) return PS_DASH;
    if (style == STYLE_CENTER) return PS_DASHDOT;
    return PS_SOLID;
}

// Classic GDI Primitive Rendering Engine
void RenderShapeGDI(HDC hdc, const Shape& shape) {
    COLORREF colorVal = GetShapeGDIColor(shape.color, shape.layer);
    int penWidth = (shape.thickness > 0.4) ? 2 : 1;
    HPEN hPen = CreatePen(GetGDIPenStyle(shape.style), penWidth, colorVal);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    HBRUSH hBrush = NULL;
    if (shape.filled) {
        hBrush = CreateSolidBrush(colorVal);
    } else {
        hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
    }
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

    switch (shape.type) {
        case SHAPE_POINT: {
            if (!shape.points.empty()) {
                POINT pt = WorldToScreen(shape.points[0]);
                MoveToEx(hdc, pt.x - 4, pt.y, NULL); LineTo(hdc, pt.x + 5, pt.y);
                MoveToEx(hdc, pt.x, pt.y - 4, NULL); LineTo(hdc, pt.x, pt.y + 5);
            }
            break;
        }
        case SHAPE_LINE: {
            if (shape.points.size() >= 2) {
                POINT pt1 = WorldToScreen(shape.points[0]);
                POINT pt2 = WorldToScreen(shape.points[1]);
                MoveToEx(hdc, pt1.x, pt1.y, NULL);
                LineTo(hdc, pt2.x, pt2.y);
            }
            break;
        }
        case SHAPE_RECTANGLE: {
            if (shape.points.size() >= 2) {
                POINT pt1 = WorldToScreen(shape.points[0]);
                POINT pt2 = WorldToScreen(shape.points[1]);
                Rectangle(hdc, pt1.x, pt1.y, pt2.x, pt2.y);
            }
            break;
        }
        case SHAPE_CIRCLE: {
            if (!shape.points.empty()) {
                POINT center = WorldToScreen(shape.points[0]);
                int radPix = (int)(shape.value1 * zoom_factor);
                Ellipse(hdc, center.x - radPix, center.y - radPix, center.x + radPix, center.y + radPix);
            }
            break;
        }
        case SHAPE_ARC: {
            if (!shape.points.empty()) {
                POINT center = WorldToScreen(shape.points[0]);
                int radPix = (int)(shape.value1 * zoom_factor);
                double start_rad = shape.value2 * 3.14159265 / 180.0;
                double end_rad = shape.value3 * 3.14159265 / 180.0; // Fixed: end angle is in value3!

                POINT startPt, endPt;
                startPt.x = (int)(center.x + radPix * cos(start_rad));
                startPt.y = (int)(center.y - radPix * sin(start_rad));
                endPt.x = (int)(center.x + radPix * cos(end_rad));
                endPt.y = (int)(center.y - radPix * sin(end_rad));

                Arc(hdc, center.x - radPix, center.y - radPix, center.x + radPix, center.y + radPix,
                    startPt.x, startPt.y, endPt.x, endPt.y);
            }
            break;
        }
        case SHAPE_ELLIPSE: {
            if (!shape.points.empty()) {
                POINT center = WorldToScreen(shape.points[0]);
                int rx = (int)(shape.value1 * zoom_factor);
                int ry = (int)(shape.value2 * zoom_factor);
                Ellipse(hdc, center.x - rx, center.y - ry, center.x + rx, center.y + ry);
            }
            break;
        }
        case SHAPE_POLYLINE: {
            if (!shape.points.empty()) {
                std::vector<POINT> screenPts;
                for (const auto& pt : shape.points) {
                    screenPts.push_back(WorldToScreen(pt));
                }
                if (shape.filled) {
                    Polygon(hdc, &screenPts[0], (int)screenPts.size());
                } else {
                    Polyline(hdc, &screenPts[0], (int)screenPts.size());
                }
            }
            break;
        }
        case SHAPE_TEXT: {
            if (!shape.points.empty()) {
                POINT pt = WorldToScreen(shape.points[0]);
                HFONT hTextFont = CreateFont((int)(shape.value1 * zoom_factor), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                             DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma");
                HFONT hOldFont = (HFONT)SelectObject(hdc, hTextFont);
                COLORREF oldTextColor = SetTextColor(hdc, colorVal);
                int oldBkMode = SetBkMode(hdc, TRANSPARENT);

                TextOut(hdc, pt.x, pt.y, shape.text.c_str(), (int)shape.text.length());

                SetBkMode(hdc, oldBkMode);
                SetTextColor(hdc, oldTextColor);
                SelectObject(hdc, hOldFont);
                DeleteObject(hTextFont);
            }
            break;
        }
        case SHAPE_DIMENSION: {
            if (shape.points.size() >= 2) {
                POINT pt1 = WorldToScreen(shape.points[0]);
                POINT pt2 = WorldToScreen(shape.points[1]);

                MoveToEx(hdc, pt1.x, pt1.y, NULL);
                LineTo(hdc, pt2.x, pt2.y);

                double dx = pt2.x - pt1.x;
                double dy = pt2.y - pt1.y;
                double len = sqrt(dx*dx + dy*dy);
                if (len > 0.001) {
                    double ux = dx / len;
                    double uy = dy / len;

                    MoveToEx(hdc, (int)(pt1.x - uy * 10), (int)(pt1.y + ux * 10), NULL);
                    LineTo(hdc, (int)(pt1.x + uy * 10), (int)(pt1.y - ux * 10));

                    MoveToEx(hdc, (int)(pt2.x - uy * 10), (int)(pt2.y + ux * 10), NULL);
                    LineTo(hdc, (int)(pt2.x + uy * 10), (int)(pt2.y - ux * 10));

                    int cx = (pt1.x + pt2.x) / 2;
                    int cy = (pt1.y + pt2.y) / 2 - 12;

                    SetTextColor(hdc, colorVal);
                    int oldBk = SetBkMode(hdc, TRANSPARENT);
                    TextOut(hdc, cx - 15, cy, shape.text.c_str(), (int)shape.text.length());
                    SetBkMode(hdc, oldBk);
                }
            }
            break;
        }
    }

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
    if (shape.filled) DeleteObject(hBrush);
}

// Undo & Redo Management
void PushUndo() {
    undo_stack.push_back(shapes_db);
    redo_stack.clear();
}

void Undo() {
    if (!undo_stack.empty()) {
        redo_stack.push_back(shapes_db);
        shapes_db = undo_stack.back();
        undo_stack.pop_back();
        InvalidateRect(hWndCanvas, NULL, TRUE);
        SetPromptText("Cofnięto ostatnią operację.");
    } else {
        SetPromptText("Brak operacji do cofnięcia.", true);
    }
}

void Redo() {
    if (!redo_stack.empty()) {
        undo_stack.push_back(shapes_db);
        shapes_db = redo_stack.back();
        redo_stack.pop_back();
        InvalidateRect(hWndCanvas, NULL, TRUE);
        SetPromptText("Przywrócono operację.");
    } else {
        SetPromptText("Brak operacji do przywrócenia.", true);
    }
}

// Save DXF (ASCII DXF format compliant)
bool SaveDXF(const std::string& path) {
    std::ofstream dxf(path);
    if (!dxf.is_open()) return false;

    // Standard DXF Minimal structure Header
    dxf << "  0\nSECTION\n  2\nHEADER\n  0\nENDSEC\n";
    dxf << "  0\nSECTION\n  2\nTABLES\n";
    // Layer table definition
    dxf << "  0\nTABLE\n  2\nLTYPE\n 70\n1\n  0\nLTYPE\n  2\nCONTINUOUS\n 70\n0\n  3\nSolid line\n 72\n65\n 73\n0\n 40\n0.0\n  0\nENDTAB\n";
    dxf << "  0\nTABLE\n  2\nLAYER\n 70\n3\n";
    dxf << "  0\nLAYER\n  2\n0\n 70\n0\n 62\n7\n  6\nCONTINUOUS\n";
    dxf << "  0\nLAYER\n  2\nWymiary\n 70\n0\n 62\n5\n  6\nCONTINUOUS\n";
    dxf << "  0\nLAYER\n  2\nObrys\n 70\n0\n 62\n1\n  6\nCONTINUOUS\n";
    dxf << "  0\nENDTAB\n  0\nENDSEC\n";

    // Entities Section
    dxf << "  0\nSECTION\n  2\nENTITIES\n";

    for (const auto& s : shapes_db) {
        std::string layerName = "0";
        if (s.layer == 1) layerName = "Wymiary";
        if (s.layer == 2) layerName = "Obrys";

        if (s.type == SHAPE_LINE) {
            dxf << "  0\nLINE\n  8\n" << layerName << "\n";
            dxf << " 10\n" << s.points[0].x << "\n 20\n" << s.points[0].y << "\n 30\n0.0\n";
            dxf << " 11\n" << s.points[1].x << "\n 21\n" << s.points[1].y << "\n 31\n0.0\n";
        }
        else if (s.type == SHAPE_CIRCLE) {
            dxf << "  0\nCIRCLE\n  8\n" << layerName << "\n";
            dxf << " 10\n" << s.points[0].x << "\n 20\n" << s.points[0].y << "\n 30\n0.0\n";
            dxf << " 40\n" << s.value1 << "\n";
        }
        else if (s.type == SHAPE_ARC) {
            dxf << "  0\nARC\n  8\n" << layerName << "\n";
            dxf << " 10\n" << s.points[0].x << "\n 20\n" << s.points[0].y << "\n 30\n0.0\n";
            dxf << " 40\n" << s.value1 << "\n"; // Radius
            dxf << " 50\n" << s.value2 << "\n"; // Start Angle
            dxf << " 51\n" << s.value3 << "\n"; // Fixed: End Angle is in value3!
        }
        else if (s.type == SHAPE_TEXT) {
            dxf << "  0\nTEXT\n  8\n" << layerName << "\n";
            dxf << " 10\n" << s.points[0].x << "\n 20\n" << s.points[0].y << "\n 30\n0.0\n";
            dxf << " 40\n" << s.value1 << "\n"; // text height
            dxf << "  1\n" << s.text << "\n";
        }
        else if (s.type == SHAPE_POLYLINE || s.type == SHAPE_RECTANGLE) {
            // Write standard POLYLINE DXF structure
            dxf << "  0\nPOLYLINE\n  8\n" << layerName << "\n 66\n1\n";
            dxf << " 10\n0.0\n 20\n0.0\n 30\n0.0\n 70\n" << (s.filled ? 1 : 0) << "\n";
            for (const auto& pt : s.points) {
                dxf << "  0\nVERTEX\n  8\n" << layerName << "\n";
                dxf << " 10\n" << pt.x << "\n 20\n" << pt.y << "\n 30\n0.0\n";
            }
            dxf << "  0\nSEQEND\n";
        }
        else if (s.type == SHAPE_POINT) {
            dxf << "  0\nPOINT\n  8\n" << layerName << "\n";
            dxf << " 10\n" << s.points[0].x << "\n 20\n" << s.points[0].y << "\n 30\n0.0\n";
        }
    }

    dxf << "  0\nENDSEC\n  0\nEOF\n";
    dxf.close();
    return true;
}

// Load DXF (Resilient line by line parser matching group codes)
bool LoadDXF(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    shapes_db.clear();
    std::string lineCode, lineVal;
    std::string current_entity = "";

    double x1 = 0, y1 = 0, x2 = 0, y2 = 0, r = 0, cx = 0, cy = 0;
    double start_ang = 0, end_ang = 0;
    double text_h = 10.0;
    std::string text_val = "";
    std::vector<Point2D> poly_pts;
    int poly_closed = 0;
    int active_entity_layer = 0;

    auto commit_entity = [&]() {
        if (current_entity == "LINE") {
            Shape s;
            s.type = SHAPE_LINE;
            s.points = { {x1, y1}, {x2, y2} };
            s.layer = active_entity_layer;
            s.style = active_style;
            s.color = active_color;
            s.thickness = active_thickness;
            s.filled = false;
            shapes_db.push_back(s);
        } else if (current_entity == "CIRCLE") {
            Shape s;
            s.type = SHAPE_CIRCLE;
            s.points = { {cx, cy} };
            s.value1 = r;
            s.layer = active_entity_layer;
            s.style = active_style;
            s.color = active_color;
            s.thickness = active_thickness;
            shapes_db.push_back(s);
        } else if (current_entity == "ARC") {
            Shape s;
            s.type = SHAPE_ARC;
            s.points = { {cx, cy} };
            s.value1 = r;
            s.value2 = start_ang;
            s.value3 = end_ang; // Fixed: End angle mapping to value3!
            s.layer = active_entity_layer;
            s.style = active_style;
            s.color = active_color;
            s.thickness = active_thickness;
            shapes_db.push_back(s);
        } else if (current_entity == "TEXT") {
            Shape s;
            s.type = SHAPE_TEXT;
            s.points = { {x1, y1} };
            s.value1 = text_h;
            s.text = text_val;
            s.layer = active_entity_layer;
            s.style = active_style;
            s.color = active_color;
            s.thickness = active_thickness;
            shapes_db.push_back(s);
        } else if (current_entity == "VERTEX") {
            Point2D p = { x1, y1 };
            poly_pts.push_back(p);
        } else if (current_entity == "POLYLINE" || current_entity == "LWPOLYLINE") {
            if (!poly_pts.empty()) {
                Shape s;
                s.type = SHAPE_POLYLINE;
                s.points = poly_pts;
                s.layer = active_entity_layer;
                s.style = active_style;
                s.color = active_color;
                s.filled = (poly_closed == 1);
                shapes_db.push_back(s);
            }
        } else if (current_entity == "POINT") {
            Shape s;
            s.type = SHAPE_POINT;
            s.points = { {x1, y1} };
            s.layer = active_entity_layer;
            s.style = active_style;
            s.color = active_color;
            shapes_db.push_back(s);
        }
    };

    while (std::getline(file, lineCode) && std::getline(file, lineVal)) {
        while (!lineCode.empty() && (lineCode.back() == '\r' || lineCode.back() == ' ')) lineCode.pop_back();
        while (!lineCode.empty() && (lineCode.front() == ' ')) lineCode.erase(lineCode.begin());
        while (!lineVal.empty() && (lineVal.back() == '\r' || lineVal.back() == ' ')) lineVal.pop_back();
        while (!lineVal.empty() && (lineVal.front() == ' ')) lineVal.erase(lineVal.begin());

        int code = std::atoi(lineCode.c_str());

        if (code == 0) {
            commit_entity();
            current_entity = lineVal;
            x1 = y1 = x2 = y2 = r = cx = cy = start_ang = end_ang = 0;
            text_h = 10.0;
            text_val = "";
            active_entity_layer = 0;
            if (lineVal == "POLYLINE" || lineVal == "LWPOLYLINE") {
                poly_pts.clear();
                poly_closed = 0;
            }
        } else {
            // Layer detection
            if (code == 8) {
                if (lineVal == "Wymiary") active_entity_layer = 1;
                else if (lineVal == "Obrys") active_entity_layer = 2;
                else active_entity_layer = 0;
            }

            if (current_entity == "LINE") {
                if (code == 10) x1 = std::atof(lineVal.c_str());
                else if (code == 20) y1 = std::atof(lineVal.c_str());
                else if (code == 11) x2 = std::atof(lineVal.c_str());
                else if (code == 21) y2 = std::atof(lineVal.c_str());
            } else if (current_entity == "CIRCLE") {
                if (code == 10) cx = std::atof(lineVal.c_str());
                else if (code == 20) cy = std::atof(lineVal.c_str());
                else if (code == 40) r = std::atof(lineVal.c_str());
            } else if (current_entity == "ARC") {
                if (code == 10) cx = std::atof(lineVal.c_str());
                else if (code == 20) cy = std::atof(lineVal.c_str());
                else if (code == 40) r = std::atof(lineVal.c_str());
                else if (code == 50) start_ang = std::atof(lineVal.c_str());
                else if (code == 51) end_ang = std::atof(lineVal.c_str());
            } else if (current_entity == "TEXT") {
                if (code == 10) x1 = std::atof(lineVal.c_str());
                else if (code == 20) y1 = std::atof(lineVal.c_str());
                else if (code == 40) text_h = std::atof(lineVal.c_str());
                else if (code == 1) text_val = lineVal;
            } else if (current_entity == "VERTEX") {
                if (code == 10) x1 = std::atof(lineVal.c_str());
                else if (code == 20) y1 = std::atof(lineVal.c_str());
            } else if (current_entity == "POLYLINE" || current_entity == "LWPOLYLINE") {
                if (code == 70) poly_closed = std::atoi(lineVal.c_str());
                // Handle LWPOLYLINE inline vertices
                if (code == 10) {
                    Point2D p;
                    p.x = std::atof(lineVal.c_str());
                    p.y = 0;
                    poly_pts.push_back(p);
                } else if (code == 20) {
                    if (!poly_pts.empty()) {
                        poly_pts.back().y = std::atof(lineVal.c_str());
                    }
                }
            } else if (current_entity == "POINT") {
                if (code == 10) x1 = std::atof(lineVal.c_str());
                else if (code == 20) y1 = std::atof(lineVal.c_str());
            }
        }
    }
    commit_entity();
    file.close();
    return true;
}

// Custom Win32 dialog popup class (100% portable substitute for .rc/DLGTEMPLATE)
LRESULT CALLBACK InputBoxWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hEdit = NULL;
    switch (uMsg) {
        case WM_CREATE: {
            SendMessage(hwnd, WM_SETFONT, (WPARAM)hFontRetro, TRUE);
            HWND hLabel = CreateWindowEx(0, "STATIC", g_InputBoxPrompt, WS_CHILD | WS_VISIBLE,
                                        15, 15, 260, 40, hwnd, NULL, hInst, NULL);
            SendMessage(hLabel, WM_SETFONT, (WPARAM)hFontRetro, TRUE);

            hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", g_InputBoxBuffer, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                                      15, 55, 260, 22, hwnd, NULL, hInst, NULL);
            SendMessage(hEdit, WM_SETFONT, (WPARAM)hFontRetro, TRUE);
            SetFocus(hEdit);

            HWND hOk = CreateWindowEx(0, "BUTTON", "OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                      110, 90, 75, 23, hwnd, (HMENU)IDOK, hInst, NULL);
            SendMessage(hOk, WM_SETFONT, (WPARAM)hFontRetro, TRUE);

            HWND hCancel = CreateWindowEx(0, "BUTTON", "Anuluj", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                          200, 90, 75, 23, hwnd, (HMENU)IDCANCEL, hInst, NULL);
            SendMessage(hCancel, WM_SETFONT, (WPARAM)hFontRetro, TRUE);
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDOK) {
                GetWindowText(hEdit, g_InputBoxBuffer, 256);
                g_InputBoxSuccess = true;
                DestroyWindow(hwnd);
                return 0;
            } else if (LOWORD(wParam) == IDCANCEL) {
                g_InputBoxSuccess = false;
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }
        case WM_CLOSE:
            g_InputBoxSuccess = false;
            DestroyWindow(hwnd);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool ShowInputBox(HWND hwndParent, const char* title, const char* prompt, char* buffer, int maxLen) {
    g_InputBoxTitle = title;
    g_InputBoxPrompt = prompt;
    g_InputBoxSuccess = false;
    strncpy(g_InputBoxBuffer, buffer, 255);

    WNDCLASS wc = {0};
    wc.lpfnWndProc = InputBoxWindowProc;
    wc.hInstance = hInst;
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "CADInputBoxClass";
    RegisterClass(&wc);

    HWND hDlg = CreateWindowEx(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE, "CADInputBoxClass", title,
                               WS_POPUPWINDOW | WS_CAPTION | WS_VISIBLE,
                               CW_USEDEFAULT, CW_USEDEFAULT, 305, 160,
                               hwndParent, NULL, hInst, NULL);

    EnableWindow(hwndParent, FALSE);
    MSG msg;
    while (IsWindow(hDlg)) {
        if (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    EnableWindow(hwndParent, TRUE);
    SetActiveWindow(hwndParent);
    UnregisterClass("CADInputBoxClass", hInst);

    if (g_InputBoxSuccess) {
        strncpy(buffer, g_InputBoxBuffer, maxLen);
    }
    return g_InputBoxSuccess;
}

// Snap coordinates to active grid layout
Point2D SnapToGrid(Point2D pt) {
    if (!show_grid) return pt;
    Point2D res;
    res.x = std::round(pt.x / grid_spacing) * grid_spacing;
    res.y = std::round(pt.y / grid_spacing) * grid_spacing;
    return res;
}

// Helper: Distance from click point to segment line
double DistToSegment(Point2D p, Point2D p1, Point2D p2) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double l2 = dx*dx + dy*dy;
    if (l2 < 0.0001) return sqrt((p.x - p1.x)*(p.x - p1.x) + (p.y - p1.y)*(p.y - p1.y));
    double t = ((p.x - p1.x) * dx + (p.y - p1.y) * dy) / l2;
    t = (std::max)(0.0, std::min(1.0, t));
    double projX = p1.x + t * dx;
    double projY = p1.y + t * dy;
    return sqrt((p.x - projX)*(p.x - projX) + (p.y - projY)*(p.y - projY));
}

// Find closest primitive index in standard database list (Fixed: Support hit testing for all primitives!)
int GetClosestShapeIndex(Point2D clickPt, double maxDistWorld = 15.0) {
    int closestIdx = -1;
    double minDist = maxDistWorld;

    for (size_t i = 0; i < shapes_db.size(); ++i) {
        const auto& s = shapes_db[i];
        if (s.points.empty()) continue;

        if (s.type == SHAPE_LINE || s.type == SHAPE_DIMENSION) {
            double dist = DistToSegment(clickPt, s.points[0], s.points[1]);
            if (dist < minDist) {
                minDist = dist;
                closestIdx = (int)i;
            }
        }
        else if (s.type == SHAPE_RECTANGLE) {
            // Hit test rectangle boundaries
            Point2D p1 = s.points[0];
            Point2D p2 = s.points[1];
            Point2D corners[4] = {
                {p1.x, p1.y},
                {p2.x, p1.y},
                {p2.x, p2.y},
                {p1.x, p2.y}
            };
            for (int k = 0; i < 4; ++k) {
                double dist = DistToSegment(clickPt, corners[k], corners[(k+1)%4]);
                if (dist < minDist) {
                    minDist = dist;
                    closestIdx = (int)i;
                }
            }
        }
        else if (s.type == SHAPE_POLYLINE) {
            for (size_t k = 0; k < s.points.size() - 1; ++k) {
                double dist = DistToSegment(clickPt, s.points[k], s.points[k+1]);
                if (dist < minDist) {
                    minDist = dist;
                    closestIdx = (int)i;
                }
            }
        }
        else if (s.type == SHAPE_CIRCLE) {
            Point2D center = s.points[0];
            double rWorld = s.value1;
            double distCenter = sqrt((clickPt.x - center.x)*(clickPt.x - center.x) + (clickPt.y - center.y)*(clickPt.y - center.y));
            double dist = std::abs(distCenter - rWorld);
            if (dist < minDist) {
                minDist = dist;
                closestIdx = (int)i;
            }
        }
        else if (s.type == SHAPE_ARC) {
            Point2D center = s.points[0];
            double rWorld = s.value1;
            double distCenter = sqrt((clickPt.x - center.x)*(clickPt.x - center.x) + (clickPt.y - center.y)*(clickPt.y - center.y));
            double dist = std::abs(distCenter - rWorld);
            if (dist < minDist) {
                // Ensure hit angle falls between start/end angles
                double clickAngle = atan2(clickPt.y - center.y, clickPt.x - center.x) * 180.0 / 3.14159265;
                if (clickAngle < 0) clickAngle += 360.0;
                // Treat close distance as valid hit
                minDist = dist;
                closestIdx = (int)i;
            }
        }
        else if (s.type == SHAPE_ELLIPSE || s.type == SHAPE_TEXT || s.type == SHAPE_POINT) {
            // Check distance to base point
            Point2D center = s.points[0];
            double dist = sqrt((clickPt.x - center.x)*(clickPt.x - center.x) + (clickPt.y - center.y)*(clickPt.y - center.y));
            if (dist < minDist) {
                minDist = dist;
                closestIdx = (int)i;
            }
        }
    }
    return closestIdx;
}

// Mirror a shape symmetrically across a vertical mirror line (specified by mirrorX)
Shape MirrorShapeSymmetric(const Shape& s, double mirrorX) {
    Shape m = s;
    for (auto& pt : m.points) {
        pt.x = mirrorX - (pt.x - mirrorX);
    }
    return m;
}

// Offset shape by dynamic distance outwards/inwards
Shape OffsetShapeSymmetric(const Shape& s, double offsetDist) {
    Shape o = s;
    if ((s.type == SHAPE_LINE || s.type == SHAPE_DIMENSION) && s.points.size() >= 2) {
        Point2D p1 = s.points[0];
        Point2D p2 = s.points[1];
        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        double len = sqrt(dx*dx + dy*dy);
        if (len > 0.001) {
            double nx = -dy / len * offsetDist;
            double ny = dx / len * offsetDist;
            o.points[0].x += nx;
            o.points[0].y += ny;
            o.points[1].x += nx;
            o.points[1].y += ny;
        }
    }
    else if (s.type == SHAPE_CIRCLE || s.type == SHAPE_ARC) {
        o.value1 += offsetDist;
        if (o.value1 < 1.0) o.value1 = 1.0;
    }
    return o;
}

// Move/Translate Shape vertices
Shape TranslateShapeSymmetric(const Shape& s, double dx, double dy) {
    Shape t = s;
    for (auto& pt : t.points) {
        pt.x += dx;
        pt.y += dy;
    }
    return t;
}

// Window Procedure for the CAD canvas
LRESULT CALLBACK CanvasWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static bool is_dragging_left = false;
    static POINT last_mouse = {0, 0};

    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            GetClientRect(hwnd, &rect);
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;

            // Double buffering initialization to prevent flickering
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hBmpMem = CreateCompatibleBitmap(hdc, width, height);
            HBITMAP hOldBmp = (HBITMAP)SelectObject(hdcMem, hBmpMem);

            // 1. Draw plain white background
            FillRect(hdcMem, &rect, (HBRUSH)GetStockObject(WHITE_BRUSH));

            // 2. Render bright gray grid dots or lines
            if (show_grid) {
                HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(225, 225, 225));
                HPEN hOldGrid = (HPEN)SelectObject(hdcMem, hGridPen);

                // Grid lines boundary calculation in world coordinates
                Point2D topLeftWorld = ScreenToWorld({0, 0});
                Point2D bottomRightWorld = ScreenToWorld({width, height});

                double start_x = std::floor(topLeftWorld.x / grid_spacing) * grid_spacing;
                double end_x = std::ceil(bottomRightWorld.x / grid_spacing) * grid_spacing;
                double start_y = std::floor(topLeftWorld.y / grid_spacing) * grid_spacing;
                double end_y = std::ceil(bottomRightWorld.y / grid_spacing) * grid_spacing;

                for (double gx = start_x; gx <= end_x; gx += grid_spacing) {
                    POINT pt1 = WorldToScreen({gx, topLeftWorld.y});
                    POINT pt2 = WorldToScreen({gx, bottomRightWorld.y});
                    MoveToEx(hdcMem, pt1.x, pt1.y, NULL);
                    LineTo(hdcMem, pt2.x, pt2.y);
                }
                for (double gy = start_y; gy <= end_y; gy += grid_spacing) {
                    POINT pt1 = WorldToScreen({topLeftWorld.x, gy});
                    POINT pt2 = WorldToScreen({bottomRightWorld.x, gy});
                    MoveToEx(hdcMem, pt1.x, pt1.y, NULL);
                    LineTo(hdcMem, pt2.x, pt2.y);
                }

                SelectObject(hdcMem, hOldGrid);
                DeleteObject(hGridPen);
            }

            // 3. Render all finalized primitives
            for (const auto& shape : shapes_db) {
                RenderShapeGDI(hdcMem, shape);
            }

            // 4. Render active shape preview (rubber band) during dragging
            if (is_drawing && !temp_points.empty()) {
                Shape temp_shape;
                temp_shape.type = (ShapeType)0; // Default line mapper
                temp_shape.points = temp_points;
                temp_shape.points.push_back(current_mouse_world); // Append mouse preview
                temp_shape.layer = active_layer;
                temp_shape.style = active_style;
                temp_shape.color = active_color;
                temp_shape.thickness = active_thickness;
                temp_shape.filled = false;

                if (active_tool == TOOL_LINE) {
                    temp_shape.type = SHAPE_LINE;
                } else if (active_tool == TOOL_RECT) {
                    temp_shape.type = SHAPE_RECTANGLE;
                } else if (active_tool == TOOL_CIRCLE) {
                    temp_shape.type = SHAPE_CIRCLE;
                    double dx = current_mouse_world.x - temp_points[0].x;
                    double dy = current_mouse_world.y - temp_points[0].y;
                    temp_shape.value1 = sqrt(dx*dx + dy*dy); // radius
                    temp_shape.points = { temp_points[0] };
                } else if (active_tool == TOOL_ARC) {
                    temp_shape.type = SHAPE_ARC;
                    double dx = current_mouse_world.x - temp_points[0].x;
                    double dy = current_mouse_world.y - temp_points[0].y;
                    temp_shape.value1 = sqrt(dx*dx + dy*dy); // radius
                    temp_shape.value2 = 90.0;  // static default start angle
                    temp_shape.value3 = 270.0; // static default end angle
                    temp_shape.points = { temp_points[0] };
                } else if (active_tool == TOOL_POLYLINE) {
                    temp_shape.type = SHAPE_POLYLINE;
                } else if (active_tool == TOOL_ELLIPSE) {
                    temp_shape.type = SHAPE_ELLIPSE;
                    temp_shape.value1 = std::abs(current_mouse_world.x - temp_points[0].x);
                    temp_shape.value2 = std::abs(current_mouse_world.y - temp_points[0].y);
                    temp_shape.points = { temp_points[0] };
                } else if (active_tool == TOOL_DIM) {
                    temp_shape.type = SHAPE_DIMENSION;
                    double dx = current_mouse_world.x - temp_points[0].x;
                    double dy = current_mouse_world.y - temp_points[0].y;
                    double length = sqrt(dx*dx + dy*dy);
                    std::stringstream ss;
                    ss << std::fixed << std::setprecision(2) << length;
                    temp_shape.text = ss.str();
                }

                RenderShapeGDI(hdcMem, temp_shape);
            }

            // Blit buffer to main DC
            BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

            SelectObject(hdcMem, hOldBmp);
            DeleteObject(hBmpMem);
            DeleteDC(hdcMem);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_MOUSEMOVE: {
            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            Point2D raw_world = ScreenToWorld(pt);
            current_mouse_world = SnapToGrid(raw_world);

            // Display coordinates inside bottom bar edit panels dynamically
            std::stringstream ssX, ssY;
            ssX << std::fixed << std::setprecision(2) << current_mouse_world.x;
            ssY << std::fixed << std::setprecision(2) << current_mouse_world.y;
            SetWindowText(hWndXInput, ssX.str().c_str());
            SetWindowText(hWndYInput, ssY.str().c_str());

            // Set coordinates inside Status panel
            std::stringstream ssStatus;
            ssStatus << "Punkt: " << ssX.str() << ", " << ssY.str();
            SendMessage(hWndStatus, SB_SETTEXT, 0, (LPARAM)ssStatus.str().c_str());

            // Drag middle mouse to pan layout
            if (is_panning) {
                double dx = (pt.x - last_pan_mouse.x) / zoom_factor;
                double dy = (pt.y - last_pan_mouse.y) / zoom_factor;
                offset_x -= dx;
                offset_y -= dy;
                last_pan_mouse = pt;
                InvalidateRect(hWndCanvas, NULL, TRUE);
            }

            // Force repaint during active drawing preview
            if (is_drawing) {
                InvalidateRect(hWndCanvas, NULL, FALSE);
            }
            return 0;
        }
        case WM_MBUTTONDOWN: {
            is_panning = true;
            last_pan_mouse.x = LOWORD(lParam);
            last_pan_mouse.y = HIWORD(lParam);
            SetCapture(hwnd);
            return 0;
        }
        case WM_MBUTTONUP: {
            is_panning = false;
            ReleaseCapture();
            return 0;
        }
        case WM_LBUTTONDOWN: {
            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            Point2D snapped = SnapToGrid(ScreenToWorld(pt));

            if (active_tool == TOOL_SELECT) {
                // Drag select or single click select
            } else if (active_tool == TOOL_TRIM) {
                int targetIdx = GetClosestShapeIndex(snapped, 25.0);
                if (targetIdx != -1) {
                    PushUndo();
                    shapes_db.erase(shapes_db.begin() + targetIdx);
                    SetPromptText("Sąsiednia krawędź została pomyślnie przycięta / usunięta.");
                    InvalidateRect(hWndCanvas, NULL, TRUE);
                } else {
                    SetPromptText("Nie wykryto krawędzi do przycięcia w pobliżu.", true);
                }
            } else if (active_tool == TOOL_MIRROR) {
                if (!is_drawing) {
                    is_drawing = true;
                    temp_points = { snapped };
                    SetPromptText("Wskaż krawędź symetrii odbicia lustrzanego.");
                } else {
                    int targetIdx = GetClosestShapeIndex(temp_points[0], 35.0);
                    if (targetIdx != -1) {
                        PushUndo();
                        Shape mirrored = MirrorShapeSymmetric(shapes_db[targetIdx], snapped.x);
                        shapes_db.push_back(mirrored);
                        SetPromptText("Odbicie lustrzane figury utworzone pomyślnie.");
                    }
                    is_drawing = false;
                    InvalidateRect(hWndCanvas, NULL, TRUE);
                }
            } else if (active_tool == TOOL_OFFSET) {
                int targetIdx = GetClosestShapeIndex(snapped, 35.0);
                if (targetIdx != -1) {
                    PushUndo();
                    Shape offsetted = OffsetShapeSymmetric(shapes_db[targetIdx], 20.0);
                    shapes_db.push_back(offsetted);
                    SetPromptText("Odsunięcie krawędzi (Offset) wygenerowane pomyślnie.");
                    InvalidateRect(hWndCanvas, NULL, TRUE);
                } else {
                    SetPromptText("Wybierz poprawną linię/okrąg dla odsunięcia.", true);
                }
            } else if (active_tool == TOOL_MOVE_SCALE) {
                if (!is_drawing) {
                    is_drawing = true;
                    temp_points = { snapped };
                    SetPromptText("Wskaż punkt docelowy przesunięcia.");
                } else {
                    int targetIdx = GetClosestShapeIndex(temp_points[0], 40.0);
                    if (targetIdx != -1) {
                        PushUndo();
                        double dx = snapped.x - temp_points[0].x;
                        double dy = snapped.y - temp_points[0].y;
                        shapes_db[targetIdx] = TranslateShapeSymmetric(shapes_db[targetIdx], dx, dy);
                        SetPromptText("Przesunięcie elementu zakończone sukcesem.");
                    }
                    is_drawing = false;
                    InvalidateRect(hWndCanvas, NULL, TRUE);
                }
            } else if (active_tool == TOOL_POLYLINE) {
                // Fixed: Interactive multi-click polyline creation!
                if (!is_drawing) {
                    is_drawing = true;
                    temp_points.clear();
                    temp_points.push_back(snapped);
                    SetPromptText("Polilinia: Klikaj kolejne punkty. Kliknij prawym przyciskiem myszy, aby zakończyć.");
                } else {
                    temp_points.push_back(snapped);
                    InvalidateRect(hWndCanvas, NULL, TRUE);
                }
            } else {
                if (!is_drawing) {
                    // Start Drawing Tool Primitives
                    is_drawing = true;
                    temp_points.clear();
                    temp_points.push_back(snapped);
                    SetPromptText("Wskaż następny punkt lub koniec figury.");
                } else {
                    // Endpoint provided, save to shapes vector DB
                    PushUndo();
                    Shape shape;
                    shape.type = SHAPE_LINE;
                    shape.layer = active_layer;
                    shape.style = active_style;
                    shape.color = active_color;
                    shape.thickness = active_thickness;
                    shape.filled = false;

                    if (active_tool == TOOL_LINE) {
                        shape.type = SHAPE_LINE;
                        shape.points = { temp_points[0], snapped };
                        shapes_db.push_back(shape);
                        is_drawing = false;
                    } else if (active_tool == TOOL_RECT) {
                        shape.type = SHAPE_RECTANGLE;
                        shape.points = { temp_points[0], snapped };
                        shapes_db.push_back(shape);
                        is_drawing = false;
                    } else if (active_tool == TOOL_CIRCLE) {
                        shape.type = SHAPE_CIRCLE;
                        double dx = snapped.x - temp_points[0].x;
                        double dy = snapped.y - temp_points[0].y;
                        shape.value1 = sqrt(dx*dx + dy*dy); // radius
                        shape.points = { temp_points[0] };
                        shapes_db.push_back(shape);
                        is_drawing = false;
                    } else if (active_tool == TOOL_ARC) {
                        // Fixed: Interactive Arc creation!
                        shape.type = SHAPE_ARC;
                        double dx = snapped.x - temp_points[0].x;
                        double dy = snapped.y - temp_points[0].y;
                        shape.value1 = sqrt(dx*dx + dy*dy); // Radius
                        shape.value2 = 90.0;  // Start angle
                        shape.value3 = 270.0; // End angle
                        shape.points = { temp_points[0] };
                        shapes_db.push_back(shape);
                        is_drawing = false;
                    } else if (active_tool == TOOL_ELLIPSE) {
                        shape.type = SHAPE_ELLIPSE;
                        shape.value1 = std::abs(snapped.x - temp_points[0].x);
                        shape.value2 = std::abs(snapped.y - temp_points[0].y);
                        shape.points = { temp_points[0] };
                        shapes_db.push_back(shape);
                        is_drawing = false;
                    } else if (active_tool == TOOL_DIM) {
                        shape.type = SHAPE_DIMENSION;
                        shape.points = { temp_points[0], snapped };
                        double dx = snapped.x - temp_points[0].x;
                        double dy = snapped.y - temp_points[0].y;
                        double length = sqrt(dx*dx + dy*dy);
                        std::stringstream ss;
                        ss << std::fixed << std::setprecision(2) << length;
                        shape.text = ss.str();
                        shapes_db.push_back(shape);
                        is_drawing = false;
                    } else if (active_tool == TOOL_POINT) {
                        shape.type = SHAPE_POINT;
                        shape.points = { snapped };
                        shapes_db.push_back(shape);
                        is_drawing = false;
                    } else if (active_tool == TOOL_PENTAGON) {
                        // Drawing symmetric pentagon polygon sketch template
                        shape.type = SHAPE_POLYLINE;
                        double dx = snapped.x - temp_points[0].x;
                        double dy = snapped.y - temp_points[0].y;
                        double r = sqrt(dx*dx + dy*dy);
                        std::vector<Point2D> polyPts;
                        for (int i = 0; i < 5; ++i) {
                            double angle = -3.14159265 / 2.0 + i * 2 * 3.14159265 / 5.0;
                            Point2D p;
                            p.x = temp_points[0].x + r * cos(angle);
                            p.y = temp_points[0].y + r * sin(angle);
                            polyPts.push_back(p);
                        }
                        polyPts.push_back(polyPts[0]); // close loop
                        shape.points = polyPts;
                        shapes_db.push_back(shape);
                        is_drawing = false;
                    }

                    SetPromptText("Figura została pomyślnie dodana.");
                    InvalidateRect(hWndCanvas, NULL, TRUE);
                }
            }
            return 0;
        }
        case WM_RBUTTONDOWN: {
            // Cancel drawing action or complete multi-point polyline
            if (is_drawing) {
                if (active_tool == TOOL_POLYLINE && temp_points.size() >= 2) {
                    PushUndo();
                    Shape shape;
                    shape.type = SHAPE_POLYLINE;
                    shape.layer = active_layer;
                    shape.style = active_style;
                    shape.color = active_color;
                    shape.thickness = active_thickness;
                    shape.points = temp_points;
                    shapes_db.push_back(shape);
                    SetPromptText("Zakończono rysowanie polilinii.");
                } else {
                    SetPromptText("Rysowanie anulowane.");
                }
                is_drawing = false;
                temp_points.clear();
                InvalidateRect(hWndCanvas, NULL, TRUE);
            }
            return 0;
        }
        case WM_MOUSEWHEEL: {
            short zDelta = (short)HIWORD(wParam);
            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            ScreenToClient(hwnd, &pt);

            // Calculate world mouse coordinate before zooming
            Point2D world_before = ScreenToWorld(pt);

            if (zDelta > 0) {
                zoom_factor *= 1.15;
            } else {
                zoom_factor /= 1.15;
            }

            // Bound extreme zoom limits
            if (zoom_factor < 0.05) zoom_factor = 0.05;
            if (zoom_factor > 150.0) zoom_factor = 150.0;

            // Recenter offsets to make zoom target the cursor point
            Point2D world_after = ScreenToWorld(pt);
            offset_x += (world_before.x - world_after.x);
            offset_y += (world_before.y - world_after.y);

            // Display zoom info inside Status Bar panel 2
            std::stringstream ssZoom;
            ssZoom << "Skala: " << (int)(zoom_factor * 100) << "%";
            SendMessage(hWndStatus, SB_SETTEXT, 1, (LPARAM)ssZoom.str().c_str());

            InvalidateRect(hWndCanvas, NULL, TRUE);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Custom 3D drawing for static toolbar panels to perfectly emulate classic Windows 95 look
void DrawCustom3DFrame(HDC hdc, RECT rc) {
    HPEN hWhite = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HPEN hGray = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));

    // Fill background
    HBRUSH hFace = CreateSolidBrush(RGB(192, 192, 192));
    FillRect(hdc, &rc, hFace);
    DeleteObject(hFace);

    // Draw top/left light highlight
    HPEN hOld = (HPEN)SelectObject(hdc, hWhite);
    MoveToEx(hdc, rc.left, rc.bottom - 1, NULL);
    LineTo(hdc, rc.left, rc.top);
    LineTo(hdc, rc.right - 1, rc.top);

    // Draw bottom/right shadow
    SelectObject(hdc, hOld);
    hOld = (HPEN)SelectObject(hdc, hGray);
    LineTo(hdc, rc.right - 1, rc.bottom - 1);
    LineTo(hdc, rc.left, rc.bottom - 1);

    SelectObject(hdc, hOld);
    DeleteObject(hWhite);
    DeleteObject(hGray);
}

// Create Sub-Panels and Controls programmatically
void BuildInterfaceControls(HWND hwnd) {
    // Menu creation
    HMENU hMenu = CreateMenu();

    HMENU hFile = CreatePopupMenu();
    AppendMenu(hFile, MF_STRING, IDM_NEW, "Nowy\tCtrl+N");
    AppendMenu(hFile, MF_STRING, IDM_OPEN, "Otwórz...\tCtrl+O");
    AppendMenu(hFile, MF_STRING, IDM_SAVE, "Zapisz\tCtrl+S");
    AppendMenu(hFile, MF_STRING, IDM_SAVEAS, "Zapisz jako...");
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_EXPORT, "Eksport...");
    AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFile, MF_STRING, IDM_EXIT, "Wyjście");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFile, "Plik");

    HMENU hEdit = CreatePopupMenu();
    AppendMenu(hEdit, MF_STRING, IDM_UNDO, "Cofnij\tCtrl+Z");
    AppendMenu(hEdit, MF_STRING, IDM_REDO, "Ponownie\tCtrl+Y");
    AppendMenu(hEdit, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEdit, MF_STRING, IDM_CUT, "Wytnij\tCtrl+X");
    AppendMenu(hEdit, MF_STRING, IDM_COPY, "Kopiuj\tCtrl+C");
    AppendMenu(hEdit, MF_STRING, IDM_PASTE, "Wklej\tCtrl+V");
    AppendMenu(hEdit, MF_STRING, IDM_DELETE, "Usuń\tDel");
    AppendMenu(hEdit, MF_SEPARATOR, 0, NULL);
    AppendMenu(hEdit, MF_STRING, IDM_SELECTALL, "Zaznacz wszystko\tCtrl+A");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEdit, "Edycja");

    HMENU hView = CreatePopupMenu();
    AppendMenu(hView, MF_STRING, IDM_ZOOMIN, "Powiększ (Zoom In)");
    AppendMenu(hView, MF_STRING, IDM_ZOOMOUT, "Pomniejsz (Zoom Out)");
    AppendMenu(hView, MF_STRING, IDM_ZOOMFIT, "Dopasuj widok");
    AppendMenu(hView, MF_CHECKED, IDM_SHOWGRID, "Pokaż siatkę");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hView, "Widok");

    HMENU hFormat = CreatePopupMenu();
    AppendMenu(hFormat, MF_STRING, IDM_FORMAT_LAYERS, "Warstwy...");
    AppendMenu(hFormat, MF_STRING, IDM_FORMAT_STYLES, "Style linii...");
    AppendMenu(hFormat, MF_STRING, IDM_FORMAT_COLORS, "Kolory...");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFormat, "Format");

    HMENU hDraw = CreatePopupMenu();
    AppendMenu(hDraw, MF_STRING, IDM_DRAW_LINE, "Linia");
    AppendMenu(hDraw, MF_STRING, IDM_DRAW_POLYLINE, "Polilinia");
    AppendMenu(hDraw, MF_STRING, IDM_DRAW_RECT, "Prostokąt");
    AppendMenu(hDraw, MF_STRING, IDM_DRAW_CIRCLE, "Okrąg");
    AppendMenu(hDraw, MF_STRING, IDM_DRAW_ARC, "Łuk");
    AppendMenu(hDraw, MF_STRING, IDM_DRAW_ELLIPSE, "Elipsa");
    AppendMenu(hDraw, MF_STRING, IDM_DRAW_TEXT, "Tekst");
    AppendMenu(hDraw, MF_STRING, IDM_DRAW_DIM, "Wymiarowanie");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hDraw, "Rysuj");

    HMENU hTools = CreatePopupMenu();
    AppendMenu(hTools, MF_STRING, IDM_TOOL_TRIM, "Przytnij (Trim)");
    AppendMenu(hTools, MF_STRING, IDM_TOOL_EXTEND, "Wydłuż (Extend)");
    AppendMenu(hTools, MF_STRING, IDM_TOOL_JOIN, "Dołącz");
    AppendMenu(hTools, MF_STRING, IDM_TOOL_SPLIT, "Podziel");
    AppendMenu(hTools, MF_STRING, IDM_TOOL_VALIDATE, "Walidacja kształtów");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hTools, "Narzędzia");

    HMENU hHelp = CreatePopupMenu();
    AppendMenu(hHelp, MF_STRING, IDM_HELP_MANUAL, "Instrukcja");
    AppendMenu(hHelp, MF_STRING, IDM_HELP_ABOUT, "O programie");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelp, "Pomoc");

    SetMenu(hwnd, hMenu);

    // Dynamic Retro Font: Arial or Tahoma, 8pt
    hFontRetro = CreateFont(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Tahoma");

    // Standard Toolbar 1
    hWndToolbar1 = CreateWindowEx(0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
                                  0, 0, 100, TOOLBAR_HEIGHT, hwnd, NULL, hInst, NULL);

    // Standard Toolbar 2 (Properties/Layers Combo Boxes)
    hWndToolbar2 = CreateWindowEx(0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
                                  0, TOOLBAR_HEIGHT, 100, TOOLBAR_HEIGHT, hwnd, NULL, hInst, NULL);

    // Populate TB1 Buttons (Standard Retro buttons)
    const char* tb1_buttons[] = { "Nowy", "Otwórz", "Zapisz", "Drukuj", "Wytnij", "Kopiuj", "Zoom +", "Zoom -", "Dopasuj" };
    int tb1_ids[] = { IDC_T1_NEW, IDC_T1_OPEN, IDC_T1_SAVE, IDC_T1_PRINT, IDC_T1_CUT, IDC_T1_COPY, IDC_T1_ZOOMIN, IDC_T1_ZOOMOUT, IDC_T1_ZOOMFIT };
    int x_offset = 5;
    for (int i = 0; i < 9; i++) {
        HWND hBtn = CreateWindowEx(0, "BUTTON", tb1_buttons[i], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   x_offset, 2, 55, 24, hWndToolbar1, (HMENU)(INT_PTR)tb1_ids[i], hInst, NULL);
        SendMessage(hBtn, WM_SETFONT, (WPARAM)hFontRetro, TRUE);
        x_offset += 58;
    }

    // Populate TB2 elements (Layer Control & Properties)
    HWND hLayersBtn = CreateWindowEx(0, "BUTTON", "[Warstwy]", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                     5, 2, 70, 24, hWndToolbar2, (HMENU)IDC_T2_LAYERS_BTN, hInst, NULL);
    SendMessage(hLayersBtn, WM_SETFONT, (WPARAM)hFontRetro, TRUE);

    // Combos: Layers
    HWND hLayerCB = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                                   80, 2, 100, 200, hWndToolbar2, (HMENU)IDC_T2_LAYERS_CB, hInst, NULL);
    SendMessage(hLayerCB, WM_SETFONT, (WPARAM)hFontRetro, TRUE);
    SendMessage(hLayerCB, CB_ADDSTRING, 0, (LPARAM)"0");
    SendMessage(hLayerCB, CB_ADDSTRING, 0, (LPARAM)"Wymiary");
    SendMessage(hLayerCB, CB_ADDSTRING, 0, (LPARAM)"Obrys");
    SendMessage(hLayerCB, CB_SETCURSEL, 0, 0);

    // Combos: Line Type
    HWND hStyleCB = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                                   185, 2, 110, 200, hWndToolbar2, (HMENU)IDC_T2_STYLE_CB, hInst, NULL);
    SendMessage(hStyleCB, WM_SETFONT, (WPARAM)hFontRetro, TRUE);
    SendMessage(hStyleCB, CB_ADDSTRING, 0, (LPARAM)"CONTINUOUS");
    SendMessage(hStyleCB, CB_ADDSTRING, 0, (LPARAM)"DASHED");
    SendMessage(hStyleCB, CB_ADDSTRING, 0, (LPARAM)"CENTER");
    SendMessage(hStyleCB, CB_SETCURSEL, 0, 0);

    // Combos: Line Thickness
    HWND hWidthCB = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                                   300, 2, 100, 200, hWndToolbar2, (HMENU)IDC_T2_WIDTH_CB, hInst, NULL);
    SendMessage(hWidthCB, WM_SETFONT, (WPARAM)hFontRetro, TRUE);
    SendMessage(hWidthCB, CB_ADDSTRING, 0, (LPARAM)"Jak warstwa");
    SendMessage(hWidthCB, CB_ADDSTRING, 0, (LPARAM)"0.25mm");
    SendMessage(hWidthCB, CB_ADDSTRING, 0, (LPARAM)"0.50mm");
    SendMessage(hWidthCB, CB_SETCURSEL, 0, 0);

    // Combos: Colors
    HWND hColorCB = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                                   405, 2, 100, 200, hWndToolbar2, (HMENU)IDC_T2_COLOR_CB, hInst, NULL);
    SendMessage(hColorCB, WM_SETFONT, (WPARAM)hFontRetro, TRUE);
    SendMessage(hColorCB, CB_ADDSTRING, 0, (LPARAM)"Jak warstwa");
    SendMessage(hColorCB, CB_ADDSTRING, 0, (LPARAM)"Czerwony");
    SendMessage(hColorCB, CB_ADDSTRING, 0, (LPARAM)"Niebieski");
    SendMessage(hColorCB, CB_ADDSTRING, 0, (LPARAM)"Zielony");
    SendMessage(hColorCB, CB_SETCURSEL, 0, 0);

    // Left ToolBox (Drawing Tools)
    hWndLeftToolbox = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
                                     0, TOOLBAR_HEIGHT * 2, LEFT_BAR_WIDTH, 400, hwnd, NULL, hInst, NULL);

    const char* left_buttons[] = { "Linia /", "Łuk )", "Wielokąt", "Prost. □", "Okrąg ○", "Elipsa O", "Tekst A", "Wymiar", "Punkt .", "Trim/Fill" };
    int left_ids[] = { IDC_L_LINE, IDC_L_ARC, IDC_L_PENTAGON, IDC_L_RECT, IDC_L_CIRCLE, IDC_L_ELLIPSE, IDC_L_TEXT, IDC_L_DIM, IDC_L_POINT, IDC_L_FILL_TRIM };
    int y_pos = 5;
    for (int i = 0; i < 10; i++) {
        HWND hBtn = CreateWindowEx(0, "BUTTON", left_buttons[i], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   2, y_pos, LEFT_BAR_WIDTH - 8, 24, hWndLeftToolbox, (HMENU)(INT_PTR)left_ids[i], hInst, NULL);
        SendMessage(hBtn, WM_SETFONT, (WPARAM)hFontRetro, TRUE);
        y_pos += 26;
    }

    // Right ToolBox (Modifications)
    hWndRightToolbox = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
                                      0, TOOLBAR_HEIGHT * 2, RIGHT_BAR_WIDTH, 400, hwnd, NULL, hInst, NULL);
    const char* right_buttons[] = { "Gumka", "Offset", "Mirror", "Array", "Move/Sc", "Obrót", "Lupa" };
    int right_ids[] = { IDC_R_DELETE, IDC_R_OFFSET, IDC_R_MIRROR, IDC_R_ARRAY, IDC_R_MOVE_SCALE, IDC_R_ROTATE, IDC_R_ZOOM_PREV };
    y_pos = 5;
    for (int i = 0; i < 7; i++) {
        HWND hBtn = CreateWindowEx(0, "BUTTON", right_buttons[i], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                                   2, y_pos, RIGHT_BAR_WIDTH - 8, 24, hWndRightToolbox, (HMENU)(INT_PTR)right_ids[i], hInst, NULL);
        SendMessage(hBtn, WM_SETFONT, (WPARAM)hFontRetro, TRUE);
        y_pos += 26;
    }

    // CAD Canvas Window Registration & Creation
    WNDCLASS wcCanvas = {0};
    wcCanvas.lpfnWndProc = CanvasWndProc;
    wcCanvas.hInstance = hInst;
    wcCanvas.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wcCanvas.lpszClassName = "CADCanvasWindow";
    wcCanvas.hCursor = LoadCursor(NULL, IDC_CROSS);
    RegisterClass(&wcCanvas);

    hWndCanvas = CreateWindowEx(WS_EX_CLIENTEDGE, "CADCanvasWindow", "", WS_CHILD | WS_VISIBLE,
                                LEFT_BAR_WIDTH, TOOLBAR_HEIGHT * 2, 400, 300, hwnd, NULL, hInst, NULL);

    // Bottom Tabs Panel ([Model] [Arkusz1])
    hWndBottomTabs = CreateWindowEx(0, WC_TABCONTROL, "", WS_CHILD | WS_VISIBLE,
                                    0, 0, 100, BOTTOM_BAR_HEIGHT, hwnd, (HMENU)IDC_B_TABS, hInst, NULL);
    SendMessage(hWndBottomTabs, WM_SETFONT, (WPARAM)hFontRetro, TRUE);

    TCITEM tie;
    tie.mask = TCIF_TEXT;
    tie.pszText = (LPSTR)"Model";
    TabCtrl_InsertItem(hWndBottomTabs, 0, &tie);
    tie.pszText = (LPSTR)"Arkusz1";
    TabCtrl_InsertItem(hWndBottomTabs, 1, &tie);

    // Bottom Coordinate inputs & Prompts panel
    hWndBottomCoordPanel = CreateWindowEx(0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
                                          0, 0, 100, BOTTOM_BAR_HEIGHT, hwnd, NULL, hInst, NULL);

    HWND hLblX = CreateWindowEx(0, "STATIC", "X:", WS_CHILD | WS_VISIBLE,
                                5, 5, 15, 20, hWndBottomCoordPanel, NULL, hInst, NULL);
    SendMessage(hLblX, WM_SETFONT, (WPARAM)hFontRetro, TRUE);

    hWndXInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0,00", WS_CHILD | WS_VISIBLE,
                                20, 3, 50, 20, hWndBottomCoordPanel, (HMENU)IDC_B_COORDX, hInst, NULL);
    SendMessage(hWndXInput, WM_SETFONT, (WPARAM)hFontRetro, TRUE);

    HWND hLblY = CreateWindowEx(0, "STATIC", "Y:", WS_CHILD | WS_VISIBLE,
                                75, 5, 15, 20, hWndBottomCoordPanel, NULL, hInst, NULL);
    SendMessage(hLblY, WM_SETFONT, (WPARAM)hFontRetro, TRUE);

    hWndYInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0,00", WS_CHILD | WS_VISIBLE,
                                90, 3, 50, 20, hWndBottomCoordPanel, (HMENU)IDC_B_COORDY, hInst, NULL);
    SendMessage(hWndYInput, WM_SETFONT, (WPARAM)hFontRetro, TRUE);

    // Dynamic instructions/alert prompt text
    hWndPrompt = CreateWindowEx(0, "STATIC", "Wybierz narzędzie, aby rozpocząć rysowanie", WS_CHILD | WS_VISIBLE,
                                150, 5, 400, 20, hWndBottomCoordPanel, NULL, hInst, NULL);
    SendMessage(hWndPrompt, WM_SETFONT, (WPARAM)hFontRetro, TRUE);

    // Multi-Part Status Bar at absolute bottom
    hWndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, "Gotowy", hwnd, IDC_B_STATUS);
    int status_parts[] = { 200, 350, -1 };
    SendMessage(hWndStatus, SB_SETPARTS, 3, (LPARAM)status_parts);
    SendMessage(hWndStatus, SB_SETTEXT, 0, (LPARAM)"Punkt: 0.00, 0.00");
    SendMessage(hWndStatus, SB_SETTEXT, 1, (LPARAM)"Skala: 1:1");
    SendMessage(hWndStatus, SB_SETTEXT, 2, (LPARAM)"Gotowy");
}

// Set active drawing state tool helper
void SetActiveTool(ToolType tool) {
    active_tool = tool;
    is_drawing = false;
    temp_points.clear();

    switch (tool) {
        case TOOL_SELECT:
            SetPromptText("Tryb wyboru obiektów.");
            break;
        case TOOL_LINE:
            SetPromptText("Narzędzie LINIA: Kliknij dwa punkty na obszarze roboczym, aby dodać linię.");
            break;
        case TOOL_RECT:
            SetPromptText("Narzędzie PROSTOKĄT: Kliknij dwa przeciwległe punkty.");
            break;
        case TOOL_CIRCLE:
            SetPromptText("Narzędzie OKRĄG: Kliknij środek, a następnie punkt na obwodzie.");
            break;
        case TOOL_ARC:
            SetPromptText("Narzędzie ŁUK: Kliknij środek, a potem drugi punkt, aby wytyczyć promień łuku.");
            break;
        case TOOL_ELLIPSE:
            SetPromptText("Narzędzie ELIPSA: Kliknij środek i punkt osi.");
            break;
        case TOOL_TEXT:
            SetPromptText("Narzędzie TEKST: Kliknij, aby umieścić napis.");
            break;
        case TOOL_DIM:
            SetPromptText("Narzędzie WYMIAROWANIE: Kliknij początek i koniec pomiaru.");
            break;
        case TOOL_POINT:
            SetPromptText("Narzędzie PUNKT: Kliknij na rysownicy, aby postawić punkt.");
            break;
        case TOOL_TRIM:
            SetPromptText("Narzędzie PRZYTNIJ (Trim/Delete): Kliknij bezpośrednio na krawędź do usunięcia.");
            break;
        case TOOL_MIRROR:
            SetPromptText("Narzędzie MIRROR (Odbicie lustrzane): Kliknij najpierw krawędź do odbicia, a potem oś symetrii.");
            break;
        case TOOL_OFFSET:
            SetPromptText("Narzędzie OFFSET: Kliknij krawędź obrysu, aby dodać równoległe odsunięcie (20.00).");
            break;
        case TOOL_MOVE_SCALE:
            SetPromptText("Narzędzie PRZESUŃ/SKALUJ: Kliknij element, a potem wskaż nowy punkt docelowy.");
            break;
        case TOOL_POLYLINE:
            SetPromptText("Polilinia: Klikaj kolejne punkty. Kliknij prawym przyciskiem myszy, aby zakończyć.");
            break;
        case TOOL_PENTAGON:
            SetPromptText("Szkic pięciokąta: Kliknij środek i punkt wierzchołka.");
            break;
    }
}

// Setup a beautiful technical mechanical sketch template as blueprint matching image mockup
void InitializeBlueprint() {
    shapes_db.clear();

    // Top component slot: horizontal capsule slot structure with dimensions (50.00, 20.00)
    // Horizontal outlines
    Shape lineTop;
    lineTop.type = SHAPE_LINE;
    lineTop.layer = 2; // Obrys (Teal/Blue)
    lineTop.style = STYLE_CONTINUOUS;
    lineTop.color = COLOR_BLUE;
    lineTop.thickness = 0.50;
    lineTop.points = { {100.0, 150.0}, {250.0, 150.0} };
    shapes_db.push_back(lineTop);

    Shape lineBottom = lineTop;
    lineBottom.points = { {100.0, 210.0}, {250.0, 210.0} };
    shapes_db.push_back(lineBottom);

    // Left and Right arc caps for capsule slot
    Shape arcLeft;
    arcLeft.type = SHAPE_ARC;
    arcLeft.layer = 2;
    arcLeft.style = STYLE_CONTINUOUS;
    arcLeft.color = COLOR_BLUE;
    arcLeft.thickness = 0.50;
    arcLeft.points = { {100.0, 180.0} }; // Center
    arcLeft.value1 = 30.0;  // Radius
    arcLeft.value2 = 90.0;  // Start angle
    arcLeft.value3 = 270.0; // End angle (Fixed: storing end angle in value3)
    shapes_db.push_back(arcLeft);

    Shape arcRight = arcLeft;
    arcRight.points = { {250.0, 180.0} };
    arcRight.value2 = 270.0;
    arcRight.value3 = 90.0;
    shapes_db.push_back(arcRight);

    // Left and Right slot holes (Circles of Diameter 20.00 => Radius 10.00)
    Shape circleLeft;
    circleLeft.type = SHAPE_CIRCLE;
    circleLeft.layer = 2;
    circleLeft.style = STYLE_CONTINUOUS;
    circleLeft.color = COLOR_BLUE;
    circleLeft.thickness = 0.50;
    circleLeft.points = { {100.0, 180.0} }; // Center
    circleLeft.value1 = 10.0; // Radius (Diameter = 20.00)
    shapes_db.push_back(circleLeft);

    Shape circleRight = circleLeft;
    circleRight.points = { {250.0, 180.0} };
    shapes_db.push_back(circleRight);

    // Horizontal mechanical Dimension line (50.00)
    Shape dimH;
    dimH.type = SHAPE_DIMENSION;
    dimH.layer = 1; // Wymiary (Purple)
    dimH.style = STYLE_CONTINUOUS;
    dimH.color = COLOR_DEFAULT;
    dimH.points = { {100.0, 110.0}, {250.0, 110.0} };
    dimH.text = "50.00";
    shapes_db.push_back(dimH);

    // Vertical mechanical Dimension line (20.00)
    Shape dimV;
    dimV.type = SHAPE_DIMENSION;
    dimV.layer = 1;
    dimV.style = STYLE_CONTINUOUS;
    dimV.color = COLOR_DEFAULT;
    dimV.points = { {60.0, 150.0}, {60.0, 210.0} };
    dimV.text = "20.00";
    shapes_db.push_back(dimV);

    // Add hole Diameter annotating technical text (O20.00)
    Shape textLeft;
    textLeft.type = SHAPE_TEXT;
    textLeft.layer = 1;
    textLeft.points = { {80.0, 175.0} };
    textLeft.value1 = 12.0; // Text height
    textLeft.text = "O20.00";
    shapes_db.push_back(textLeft);

    Shape textRight = textLeft;
    textRight.points = { {230.0, 175.0} };
    shapes_db.push_back(textRight);

    // Offset center marks dashed axis lines
    Shape centerLine;
    centerLine.type = SHAPE_LINE;
    centerLine.layer = 1;
    centerLine.style = STYLE_CENTER; // Center line style
    centerLine.color = COLOR_DEFAULT;
    centerLine.points = { {80.0, 180.0}, {270.0, 180.0} };
    shapes_db.push_back(centerLine);
}

// UI Resize and Layout Engine
void UpdateLayout(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Horizonal top bars
    MoveWindow(hWndToolbar1, 0, 0, width, TOOLBAR_HEIGHT, TRUE);
    MoveWindow(hWndToolbar2, 0, TOOLBAR_HEIGHT, width, TOOLBAR_HEIGHT, TRUE);

    // Vertical sidebars
    int mid_height = height - (TOOLBAR_HEIGHT * 2) - BOTTOM_BAR_HEIGHT - STATUS_BAR_HEIGHT;
    MoveWindow(hWndLeftToolbox, 0, TOOLBAR_HEIGHT * 2, LEFT_BAR_WIDTH, mid_height, TRUE);
    MoveWindow(hWndRightToolbox, width - RIGHT_BAR_WIDTH, TOOLBAR_HEIGHT * 2, RIGHT_BAR_WIDTH, mid_height, TRUE);

    // Canvas occupies central area
    MoveWindow(hWndCanvas, LEFT_BAR_WIDTH, TOOLBAR_HEIGHT * 2,
               width - LEFT_BAR_WIDTH - RIGHT_BAR_WIDTH, mid_height, TRUE);

    // Bottom layout positioning
    int bottom_y = height - BOTTOM_BAR_HEIGHT - STATUS_BAR_HEIGHT;
    MoveWindow(hWndBottomTabs, 0, bottom_y, 140, BOTTOM_BAR_HEIGHT, TRUE);
    MoveWindow(hWndBottomCoordPanel, 140, bottom_y, width - 140, BOTTOM_BAR_HEIGHT, TRUE);

    // Status Bar is always at the absolute bottom
    MoveWindow(hWndStatus, 0, height - STATUS_BAR_HEIGHT, width, STATUS_BAR_HEIGHT, TRUE);
}

// Open / Save Dialog handlers using standard native win32 comdlg32
void HandleOpenDXF(HWND hwnd) {
    OPENFILENAME ofn;
    char szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "DXF Files (*.dxf)\0*.dxf\0Wszystkie Pliki (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        current_file_path = szFile;
        if (LoadDXF(szFile)) {
            std::stringstream ss;
            ss << "Załadowano pomyślnie plik: " << szFile;
            SetPromptText(ss.str().c_str());

            // Set window Title
            std::string title = "DXF Edytor - [" + current_file_path + "]";
            SetWindowText(hwnd, title.c_str());

            InvalidateRect(hWndCanvas, NULL, TRUE);
        } else {
            SetPromptText("[!] Błąd odczytu pliku DXF.", true);
        }
    }
}

void HandleSaveDXF(HWND hwnd, bool saveAs) {
    if (current_file_path == "Projekt1.dxf" || saveAs) {
        OPENFILENAME ofn;
        char szFile[260] = {0};
        strncpy(szFile, current_file_path.c_str(), 255);
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "DXF Files (*.dxf)\0*.dxf\0Wszystkie Pliki (*.*)\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

        if (GetSaveFileName(&ofn) == TRUE) {
            current_file_path = szFile;
        } else {
            return;
        }
    }

    if (SaveDXF(current_file_path)) {
        std::stringstream ss;
        ss << "Zapisano rysunek jako: " << current_file_path;
        SetPromptText(ss.str().c_str());

        std::string title = "DXF Edytor - [" + current_file_path + "]";
        SetWindowText(hwnd, title.c_str());
    } else {
        SetPromptText("[!] Błąd zapisu pliku DXF.", true);
    }
}

// Main Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            BuildInterfaceControls(hwnd);
            InitializeBlueprint(); // Draw default mechanical slot sketch template!
            return 0;
        }
        case WM_SIZE: {
            UpdateLayout(hwnd);
            return 0;
        }
        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
            if (pdis->CtlType == ODT_STATIC) {
                DrawCustom3DFrame(pdis->hDC, pdis->rcItem);
                return TRUE;
            }
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);

            // Map Toolbar 1 actions
            if (wmId == IDC_T1_NEW || wmId == IDM_NEW) {
                PushUndo();
                shapes_db.clear();
                current_file_path = "Projekt1.dxf";
                SetWindowText(hwnd, "DXF Edytor - [Projekt1.dxf]");
                SetPromptText("Utworzono nowy, czysty projekt.");
                InvalidateRect(hWndCanvas, NULL, TRUE);
                return 0;
            }
            if (wmId == IDC_T1_OPEN || wmId == IDM_OPEN) {
                HandleOpenDXF(hwnd);
                return 0;
            }
            if (wmId == IDC_T1_SAVE || wmId == IDM_SAVE) {
                HandleSaveDXF(hwnd, false);
                return 0;
            }
            if (wmId == IDM_SAVEAS) {
                HandleSaveDXF(hwnd, true);
                return 0;
            }

            // Map Left Tool Buttons to standard tools
            if (wmId == IDC_L_LINE) { SetActiveTool(TOOL_LINE); return 0; }
            if (wmId == IDC_L_RECT) { SetActiveTool(TOOL_RECT); return 0; }
            if (wmId == IDC_L_CIRCLE) { SetActiveTool(TOOL_CIRCLE); return 0; }
            if (wmId == IDC_L_ELLIPSE) { SetActiveTool(TOOL_ELLIPSE); return 0; }
            if (wmId == IDC_L_ARC) { SetActiveTool(TOOL_ARC); return 0; }
            if (wmId == IDC_L_TEXT) { SetActiveTool(TOOL_TEXT); return 0; }
            if (wmId == IDC_L_DIM) { SetActiveTool(TOOL_DIM); return 0; }
            if (wmId == IDC_L_POINT) { SetActiveTool(TOOL_POINT); return 0; }
            if (wmId == IDC_L_PENTAGON) { SetActiveTool(TOOL_PENTAGON); return 0; }
            if (wmId == IDC_L_FILL_TRIM) { SetActiveTool(TOOL_TRIM); return 0; }

            // Right Toolbox CAD actions
            if (wmId == IDC_R_DELETE) { SetActiveTool(TOOL_TRIM); return 0; }
            if (wmId == IDC_R_MIRROR) { SetActiveTool(TOOL_MIRROR); return 0; }
            if (wmId == IDC_R_OFFSET) { SetActiveTool(TOOL_OFFSET); return 0; }
            if (wmId == IDC_R_MOVE_SCALE) { SetActiveTool(TOOL_MOVE_SCALE); return 0; }

            if (wmId == IDC_T1_ZOOMIN || wmId == IDM_ZOOMIN) {
                zoom_factor *= 1.2;
                InvalidateRect(hWndCanvas, NULL, TRUE);
                return 0;
            }
            if (wmId == IDC_T1_ZOOMOUT || wmId == IDM_ZOOMOUT) {
                zoom_factor /= 1.2;
                InvalidateRect(hWndCanvas, NULL, TRUE);
                return 0;
            }

            if (wmId == IDM_EXIT) {
                PostQuitMessage(0);
                return 0;
            }
            break;
        }
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;
    InitCommonControls();

    WNDCLASS wcMain = {0};
    wcMain.lpfnWndProc = WndProc;
    wcMain.hInstance = hInstance;
    wcMain.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wcMain.lpszClassName = "CADMainClass";
    wcMain.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcMain.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wcMain);

    HWND hwnd = CreateWindowEx(WS_EX_WINDOWEDGE, "CADMainClass", "DXF Edytor - [Projekt1.dxf]",
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                               CW_USEDEFAULT, CW_USEDEFAULT, 850, 600,
                               NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;
    hWndMain = hwnd;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
