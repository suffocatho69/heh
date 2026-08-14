#ifndef COMPONENT_H
#define COMPONENT_H

#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Point structure for polygons
struct Point {
    double x;
    double y;

    Point() : x(0.0), y(0.0) {}
    Point(double x, double y) : x(x), y(y) {}
};

// Structure representing geometric entities (extracted from DXF or generated as rectangles)
struct GeoEntity {
    enum Type { LINE, CIRCLE, ARC, POLYLINE, LWPOLYLINE, SPLINE, ELLIPSE };
    Type type = LINE;

    // Coordinates
    double x1 = 0.0;
    double y1 = 0.0;
    double x2 = 0.0;
    double y2 = 0.0;
    double radius = 0.0;
    double start_angle = 0.0; // in degrees
    double end_angle = 0.0;   // in degrees
    bool ccw = true;          // direction of arc

    // Bulge parameter for polyline segments
    double bulge = 0.0;

    // For polylines, splines, ellipses
    std::vector<Point> points;

    // Technology parameters
    bool isPartialDepth = false;
    double customDepth = 0.0; // custom depth for this specific segment
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
    double rotationAngle; // Rotation angle in degrees (0, 90, 180, 270, etc.)
    bool rotated;         // true if rotated (e.g. 90 or 270 degrees, kept for legacy compatibility)
    int sheetIndex;       // index of the plate/sheet this component is placed on

    // Geometric geometry path entities (relative to components local coordinate system (0,0) to (width, height))
    std::vector<GeoEntity> geometry;

    // True polygonal contours representing actual geometry
    std::vector<Point> outerContour;                 // Flattened polygon of outer perimeter
    std::vector<std::vector<Point>> innerContours;   // Flattened polygons of internal holes

    // Mapping of geometry entities to outer and inner contours
    std::vector<size_t> outerEntityIndices;
    std::vector<std::vector<size_t>> innerEntityIndices;

    // Helper to generate default rectangular geometry path if no DXF was loaded
    void generateDefaultRectangleGeometry();

    // Reconstruct the outer contour and inner contours from the geometry entities
    void buildContours(double tolerance = 0.1);

    // Calculate the true physical area using Shoelace formula (outer area minus inner holes areas)
    double calculateShoelaceArea() const;

    // Get the effective bounding box width and height based on rotationAngle
    double getEffectiveWidth() const;
    double getEffectiveHeight() const;

    // Get the bounding box of the active rotated geometry
    void recalculateBoundingBox();

    // Transform a local coordinate (x, y) to global plate coordinates based on placement position and rotation
    std::pair<double, double> localToGlobal(double lx, double ly) const;

    // Rotates a point around a given origin by angle (in degrees)
    static Point transformPoint(Point p, double angle_deg, Point origin = Point(0, 0));

    // Flatten a single component's geometry to a set of points (outer contour)
    std::vector<Point> getFlattenedOuterPolygon() const;
};

// Global function to flatten component geometry
std::vector<Point> flattenGeometry(
    const Component& component,
    double tolerance
);

#endif // COMPONENT_H
