#include "DXFReader.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper structure for parsed polyline vertex
struct PolylineVertex {
    double x = 0.0;
    double y = 0.0;
    double bulge = 0.0;
};

// Converts a bulge segment to an ARC or sequence of LINEs
static void convertBulgeToArc(double x1, double y1, double x2, double y2, double b, Component& outComponent) {
    if (std::abs(b) < 1e-4) {
        GeoEntity geo;
        geo.type = GeoEntity::LINE;
        geo.x1 = x1; geo.y1 = y1;
        geo.x2 = x2; geo.y2 = y2;
        outComponent.geometry.push_back(geo);
        return;
    }

    double dx = x2 - x1;
    double dy = y2 - y1;
    double L = std::sqrt(dx * dx + dy * dy);
    if (L < 1e-4) return;

    double r = L * (1.0 + b * b) / (4.0 * std::abs(b));
    double a = L * (1.0 - b * b) / (4.0 * std::abs(b));

    double mx = (x1 + x2) / 2.0;
    double my = (y1 + y2) / 2.0;

    double cx = 0, cy = 0;
    if (b > 0) {
        // CCW: Center is to the left
        cx = mx - a * (dy / L);
        cy = my + a * (dx / L);
    } else {
        // CW: Center is to the right
        cx = mx + a * (dy / L);
        cy = my - a * (dx / L);
    }

    double sa = std::atan2(y1 - cy, x1 - cx) * 180.0 / M_PI;
    double ea = std::atan2(y2 - cy, x2 - cx) * 180.0 / M_PI;

    GeoEntity geo;
    geo.type = GeoEntity::ARC;
    geo.x1 = cx;
    geo.y1 = cy;
    geo.radius = r;
    geo.start_angle = sa;
    geo.end_angle = ea;
    geo.ccw = (b > 0);
    outComponent.geometry.push_back(geo);
}

static bool parseDXFStream(std::istream& in, Component& outComponent) {
    std::string line;
    std::string groupCode;
    std::string value;

    auto trim = [](std::string& s) {
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
    };

    std::string currentEntity = "";

    // LWPOLYLINE parsing state
    std::vector<PolylineVertex> lwVertices;
    bool lwClosed = false;

    // POLYLINE parsing state
    bool inPolyline = false;
    bool polyClosed = false;
    std::vector<PolylineVertex> polyVertices;

    // SPLINE parsing state
    std::vector<Point> splinePoints;

    // ELLIPSE parsing state
    double ellCx = 0, ellCy = 0;
    double ellMx = 0, ellMy = 0; // major axis endpoints relative to center
    double ellRatio = 1.0;

    // Temporary variables for LINE, ARC, CIRCLE
    double tx1 = 0, ty1 = 0, tx2 = 0, ty2 = 0;
    double tRad = 0, tSa = 0, tEa = 0;

    auto finalizeCurrentEntity = [&]() {
        if (currentEntity == "LINE") {
            GeoEntity geo;
            geo.type = GeoEntity::LINE;
            geo.x1 = tx1; geo.y1 = ty1;
            geo.x2 = tx2; geo.y2 = ty2;
            outComponent.geometry.push_back(geo);
        } else if (currentEntity == "ARC") {
            GeoEntity geo;
            geo.type = GeoEntity::ARC;
            geo.x1 = tx1; geo.y1 = ty1; // center
            geo.radius = tRad;
            geo.start_angle = tSa;
            geo.end_angle = tEa;
            geo.ccw = true;
            outComponent.geometry.push_back(geo);
        } else if (currentEntity == "CIRCLE") {
            GeoEntity geo;
            geo.type = GeoEntity::CIRCLE;
            geo.x1 = tx1; geo.y1 = ty1; // center
            geo.radius = tRad;
            outComponent.geometry.push_back(geo);
        } else if (currentEntity == "LWPOLYLINE") {
            // Reconstruct the vertices
            if (!lwVertices.empty()) {
                for (size_t i = 0; i < lwVertices.size() - 1; ++i) {
                    convertBulgeToArc(lwVertices[i].x, lwVertices[i].y, lwVertices[i+1].x, lwVertices[i+1].y, lwVertices[i].bulge, outComponent);
                }
                if (lwClosed) {
                    convertBulgeToArc(lwVertices.back().x, lwVertices.back().y, lwVertices.front().x, lwVertices.front().y, lwVertices.back().bulge, outComponent);
                }
            }
            lwVertices.clear();
        } else if (currentEntity == "SPLINE") {
            if (splinePoints.size() >= 2) {
                GeoEntity geo;
                geo.type = GeoEntity::POLYLINE;
                for (const auto& p : splinePoints) {
                    geo.points.push_back(p);
                }
                outComponent.geometry.push_back(geo);
            }
            splinePoints.clear();
        } else if (currentEntity == "ELLIPSE") {
            // Generate standard ellipse approximation (36 segments)
            GeoEntity geo;
            geo.type = GeoEntity::POLYLINE;
            for (int i = 0; i <= 36; ++i) {
                double phi = (2.0 * M_PI * i) / 36.0;
                double px = ellCx + std::cos(phi) * ellMx - std::sin(phi) * ellMy * ellRatio;
                double py = ellCy + std::sin(phi) * ellMx + std::cos(phi) * ellMy * ellRatio;
                geo.points.push_back(Point(px, py));
            }
            outComponent.geometry.push_back(geo);
        }
    };

    while (std::getline(in, groupCode)) {
        trim(groupCode);
        if (!std::getline(in, value)) break;
        trim(value);

        if (groupCode == "0") {
            finalizeCurrentEntity();

            std::string prevEntity = currentEntity;
            currentEntity = value;

            // Reset temp variables
            tx1 = ty1 = tx2 = ty2 = tRad = tSa = tEa = 0;

            if (currentEntity == "POLYLINE") {
                inPolyline = true;
                polyClosed = false;
                polyVertices.clear();
            } else if (currentEntity == "VERTEX" && inPolyline) {
                PolylineVertex v;
                polyVertices.push_back(v);
            } else if (currentEntity == "SEQEND" && inPolyline) {
                // Finalize polyline
                if (!polyVertices.empty()) {
                    for (size_t i = 0; i < polyVertices.size() - 1; ++i) {
                        convertBulgeToArc(polyVertices[i].x, polyVertices[i].y, polyVertices[i+1].x, polyVertices[i+1].y, polyVertices[i].bulge, outComponent);
                    }
                    if (polyClosed) {
                        convertBulgeToArc(polyVertices.back().x, polyVertices.back().y, polyVertices.front().x, polyVertices.front().y, polyVertices.back().bulge, outComponent);
                    }
                }
                inPolyline = false;
                polyVertices.clear();
            } else if (value == "EOF") {
                break;
            }
        } else {
            // Parse group codes based on active entity
            if (currentEntity == "LINE") {
                if (groupCode == "10") tx1 = std::stod(value);
                else if (groupCode == "20") ty1 = std::stod(value);
                else if (groupCode == "11") tx2 = std::stod(value);
                else if (groupCode == "21") ty2 = std::stod(value);
            } else if (currentEntity == "ARC") {
                if (groupCode == "10") tx1 = std::stod(value);
                else if (groupCode == "20") ty1 = std::stod(value);
                else if (groupCode == "40") tRad = std::stod(value);
                else if (groupCode == "50") tSa = std::stod(value);
                else if (groupCode == "51") tEa = std::stod(value);
            } else if (currentEntity == "CIRCLE") {
                if (groupCode == "10") tx1 = std::stod(value);
                else if (groupCode == "20") ty1 = std::stod(value);
                else if (groupCode == "40") tRad = std::stod(value);
            } else if (currentEntity == "LWPOLYLINE") {
                if (groupCode == "10") {
                    PolylineVertex v;
                    v.x = std::stod(value);
                    lwVertices.push_back(v);
                } else if (groupCode == "20" && !lwVertices.empty()) {
                    lwVertices.back().y = std::stod(value);
                } else if (groupCode == "42" && !lwVertices.empty()) {
                    lwVertices.back().bulge = std::stod(value);
                } else if (groupCode == "70") {
                    lwClosed = (std::stoi(value) & 1);
                }
            } else if (currentEntity == "POLYLINE") {
                if (groupCode == "70") {
                    polyClosed = (std::stoi(value) & 1);
                }
            } else if (currentEntity == "VERTEX" && inPolyline && !polyVertices.empty()) {
                if (groupCode == "10") polyVertices.back().x = std::stod(value);
                else if (groupCode == "20") polyVertices.back().y = std::stod(value);
                else if (groupCode == "42") polyVertices.back().bulge = std::stod(value);
            } else if (currentEntity == "SPLINE") {
                if (groupCode == "10") {
                    Point p;
                    p.x = std::stod(value);
                    splinePoints.push_back(p);
                } else if (groupCode == "20" && !splinePoints.empty()) {
                    splinePoints.back().y = std::stod(value);
                }
            } else if (currentEntity == "ELLIPSE") {
                if (groupCode == "10") ellCx = std::stod(value);
                else if (groupCode == "20") ellCy = std::stod(value);
                else if (groupCode == "11") ellMx = std::stod(value);
                else if (groupCode == "21") ellMy = std::stod(value);
                else if (groupCode == "40") ellRatio = std::stod(value);
            }
        }
    }

    finalizeCurrentEntity();

    if (outComponent.geometry.empty()) {
        return false;
    }

    // Now, determine the bounding box to normalize coordinates and calculate width & height.
    double minX = 1e30, minY = 1e30;
    double maxX = -1e30, maxY = -1e30;

    auto updateBounds = [&](double x, double y) {
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
    };

    // Calculate bounding box based on entity vertices
    for (const auto& ent : outComponent.geometry) {
        if (ent.type == GeoEntity::LINE) {
            updateBounds(ent.x1, ent.y1);
            updateBounds(ent.x2, ent.y2);
        } else if (ent.type == GeoEntity::CIRCLE) {
            updateBounds(ent.x1 - ent.radius, ent.y1 - ent.radius);
            updateBounds(ent.x1 + ent.radius, ent.y1 + ent.radius);
        } else if (ent.type == GeoEntity::ARC) {
            double sa_rad = ent.start_angle * M_PI / 180.0;
            double ea_rad = ent.end_angle * M_PI / 180.0;
            updateBounds(ent.x1 + ent.radius * std::cos(sa_rad), ent.y1 + ent.radius * std::sin(sa_rad));
            updateBounds(ent.x1 + ent.radius * std::cos(ea_rad), ent.y1 + ent.radius * std::sin(ea_rad));

            double angles[] = { 0, 90, 180, 270 };
            for (double angle : angles) {
                double s = ent.start_angle;
                double e = ent.end_angle;
                bool inRange = false;
                if (s <= e) {
                    inRange = (angle >= s && angle <= e);
                } else {
                    inRange = (angle >= s || angle <= e);
                }
                if (inRange) {
                    double r = angle * M_PI / 180.0;
                    updateBounds(ent.x1 + ent.radius * std::cos(r), ent.y1 + ent.radius * std::sin(r));
                }
            }
        } else if (ent.type == GeoEntity::POLYLINE) {
            for (const auto& p : ent.points) {
                updateBounds(p.x, p.y);
            }
        }
    }

    if (minX > maxX || minY > maxY) {
        return false;
    }

    double dWidth = maxX - minX;
    double dHeight = maxY - minY;

    if (dWidth <= 0) dWidth = 10;
    if (dHeight <= 0) dHeight = 10;

    outComponent.width = dWidth;
    outComponent.height = dHeight;

    // Shift all geometries to start at 0, 0
    for (auto& ent : outComponent.geometry) {
        if (ent.type == GeoEntity::LINE || ent.type == GeoEntity::ARC || ent.type == GeoEntity::CIRCLE) {
            ent.x1 -= minX;
            ent.y1 -= minY;
            ent.x2 -= minX;
            ent.y2 -= minY;
        } else if (ent.type == GeoEntity::POLYLINE) {
            for (auto& p : ent.points) {
                p.x -= minX;
                p.y -= minY;
            }
        }
    }

    // Build the polygonal contours of the component
    outComponent.buildContours(0.1);

    return true;
}

bool DXFReader::loadDXF(const std::string& filepath, Component& outComponent) {
    std::ifstream infile(filepath);
    if (!infile.is_open()) {
        return false;
    }

    size_t lastSlash = filepath.find_last_of("\\/");
    std::string name = (lastSlash == std::string::npos) ? filepath : filepath.substr(lastSlash + 1);
    outComponent.name = name;

    return parseDXFStream(infile, outComponent);
}

bool DXFReader::loadDXFFromString(const std::string& content, Component& outComponent) {
    std::istringstream stream(content);
    outComponent.name = "dxf_imported";
    return parseDXFStream(stream, outComponent);
}
