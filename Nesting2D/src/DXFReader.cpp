#include "DXFReader.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper structure for parsing
struct RawDXFEntity {
    std::string type;
    double x1 = 0, y1 = 0, x2 = 0, y2 = 0;
    double radius = 0;
    double start_angle = 0, end_angle = 0;
};

// Internal parsing implementation
static bool parseDXFStream(std::istream& in, Component& outComponent) {
    std::string line;
    std::string groupCode;
    std::string value;

    std::vector<RawDXFEntity> rawEntities;
    std::string currentSection = "";
    std::string currentEntity = "";
    RawDXFEntity tempEntity;

    auto trim = [](std::string& s) {
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
    };

    while (std::getline(in, groupCode)) {
        trim(groupCode);
        if (!std::getline(in, value)) break;
        trim(value);

        if (groupCode == "0") {
            // If we have an existing entity, save it before starting a new one
            if (currentEntity == "LINE" || currentEntity == "ARC" || currentEntity == "CIRCLE") {
                tempEntity.type = currentEntity;
                rawEntities.push_back(tempEntity);
            }

            currentEntity = value;
            tempEntity = RawDXFEntity(); // Reset temp

            if (value == "EOF") {
                break;
            }
        } else {
            // Collect properties for the current entity
            if (currentEntity == "LINE") {
                if (groupCode == "10") tempEntity.x1 = std::stod(value);
                else if (groupCode == "20") tempEntity.y1 = std::stod(value);
                else if (groupCode == "11") tempEntity.x2 = std::stod(value);
                else if (groupCode == "21") tempEntity.y2 = std::stod(value);
            } else if (currentEntity == "ARC") {
                if (groupCode == "10") tempEntity.x1 = std::stod(value); // center x
                else if (groupCode == "20") tempEntity.y1 = std::stod(value); // center y
                else if (groupCode == "40") tempEntity.radius = std::stod(value);
                else if (groupCode == "50") tempEntity.start_angle = std::stod(value);
                else if (groupCode == "51") tempEntity.end_angle = std::stod(value);
            } else if (currentEntity == "CIRCLE") {
                if (groupCode == "10") tempEntity.x1 = std::stod(value); // center x
                else if (groupCode == "20") tempEntity.y1 = std::stod(value); // center y
                else if (groupCode == "40") tempEntity.radius = std::stod(value);
            }
        }
    }

    // Save the last entity
    if (currentEntity == "LINE" || currentEntity == "ARC" || currentEntity == "CIRCLE") {
        tempEntity.type = currentEntity;
        rawEntities.push_back(tempEntity);
    }

    if (rawEntities.empty()) {
        // Return a default mock rectangle component if empty/invalid DXF
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
    for (const auto& ent : rawEntities) {
        if (ent.type == "LINE") {
            updateBounds(ent.x1, ent.y1);
            updateBounds(ent.x2, ent.y2);
        } else if (ent.type == "CIRCLE") {
            updateBounds(ent.x1 - ent.radius, ent.y1 - ent.radius);
            updateBounds(ent.x1 + ent.radius, ent.y1 + ent.radius);
        } else if (ent.type == "ARC") {
            // Simplified bounding box of ARC: center +/- radius
            // To be robust, let's include the center, start, end, and quadrant points
            double sa_rad = ent.start_angle * M_PI / 180.0;
            double ea_rad = ent.end_angle * M_PI / 180.0;
            updateBounds(ent.x1 + ent.radius * std::cos(sa_rad), ent.y1 + ent.radius * std::sin(sa_rad));
            updateBounds(ent.x1 + ent.radius * std::cos(ea_rad), ent.y1 + ent.radius * std::sin(ea_rad));

            // Check if quadrants are crossed by the arc
            double angles[] = { 0, 90, 180, 270 };
            for (double angle : angles) {
                // Determine if angle lies in arc angle range [start, end]
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
        }
    }

    // Check if bounds are valid
    if (minX > maxX || minY > maxY) {
        return false;
    }

    double dWidth = maxX - minX;
    double dHeight = maxY - minY;

    // Guard against infinites or zeroes
    if (dWidth <= 0) dWidth = 10;
    if (dHeight <= 0) dHeight = 10;

    outComponent.width = dWidth;
    outComponent.height = dHeight;
    outComponent.geometry.clear();

    // Map raw entities to normalized GeoEntities
    for (const auto& ent : rawEntities) {
        GeoEntity geo;
        if (ent.type == "LINE") {
            geo.type = GeoEntity::LINE;
            geo.x1 = ent.x1 - minX;
            geo.y1 = ent.y1 - minY;
            geo.x2 = ent.x2 - minX;
            geo.y2 = ent.y2 - minY;
            outComponent.geometry.push_back(geo);
        } else if (ent.type == "CIRCLE") {
            geo.type = GeoEntity::CIRCLE;
            geo.x1 = ent.x1 - minX;
            geo.y1 = ent.y1 - minY;
            geo.radius = ent.radius;
            outComponent.geometry.push_back(geo);
        } else if (ent.type == "ARC") {
            geo.type = GeoEntity::ARC;
            geo.x1 = ent.x1 - minX;
            geo.y1 = ent.y1 - minY;
            geo.radius = ent.radius;
            geo.start_angle = ent.start_angle;
            geo.end_angle = ent.end_angle;
            outComponent.geometry.push_back(geo);
        }
    }

    return true;
}

bool DXFReader::loadDXF(const std::string& filepath, Component& outComponent) {
    std::ifstream infile(filepath);
    if (!infile.is_open()) {
        return false;
    }

    // Set a default name based on the file path
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
