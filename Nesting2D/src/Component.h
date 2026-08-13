#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Structure representing geometric entities (extracted from DXF or generated as rectangles)
struct GeoEntity {
    enum Type { LINE, CIRCLE, ARC, POLYLINE };
    Type type = LINE;

    // Coordinates
    double x1 = 0.0;
    double y1 = 0.0;
    double x2 = 0.0;
    double y2 = 0.0;
    double radius = 0.0;
    double start_angle = 0.0; // in degrees
    double end_angle = 0.0;   // in degrees

    // For polyline
    std::vector<std::pair<double, double>> points;
};

class Component {
public:
    Component();
    Component(const std::string& name, double w, double h, int qty = 1);

    // Core attributes
    std::string name;
    double width;  // Dimension along X (unrotated)
    double height; // Dimension along Y (unrotated)
    int quantity;

    // Placement details (assigned by NestingEngine)
    bool placed;
    double posX;
    double posY;
    bool rotated; // true if rotated by 90 degrees (width and height swap roles during packing)
    int sheetIndex; // index of the plate/sheet this component is placed on

    // Geometric geometry path entities (relative to components local coordinate system (0,0) to (width, height))
    std::vector<GeoEntity> geometry;

    // Helper to generate default rectangular geometry path if no DXF was loaded
    void generateDefaultRectangleGeometry();

    // Get the effective bounding box width and height based on rotation
    double getEffectiveWidth() const;
    double getEffectiveHeight() const;

    // Transform a local coordinate (x, y) to global plate coordinates based on placement position and rotation
    std::pair<double, double> localToGlobal(double lx, double ly) const;
};

#endif // COMPONENT_H
