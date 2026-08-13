#ifndef NESTINGENGINE_H
#define NESTINGENGINE_H

#include <vector>
#include "Component.h"

struct SheetLayout {
    double width;
    double height;
    std::vector<Component> placedComponents;
    double materialUtilization; // usage percentage: 0 to 100
};

struct NestingParams {
    double sheetWidth = 3000.0;
    double sheetHeight = 1500.0;
    double margin = 10.0;       // Margin from sheet border
    double spacing = 5.0;       // Distance between adjacent components

    // Custom checkboxes for individual rotation angles
    bool allowRot0 = true;
    bool allowRot90 = true;
    bool allowRot180 = false;
    bool allowRot270 = false;
};

class NestingEngine {
public:
    // Packs components into sheets. Modifies the placement properties in originalComponents.
    // Returns a list of sheets layouts containing the packed parts.
    static std::vector<SheetLayout> performNesting(
        const std::vector<Component>& originalComponents,
        const NestingParams& params
    );

    // Runs pre-NC validation of nested sheet layouts, checking for collisions, overlaps, out-of-bounds, self-intersections, and closure.
    static bool validateNesting(
        const std::vector<SheetLayout>& sheets,
        const NestingParams& params,
        std::string& outErrorMessage
    );

    // Checks if two components collide, accounting for true geometry contours and technology spacing
    static bool checkCollision(const Component& comp1, const Component& comp2, double spacing);

    // Ray-casting point-in-polygon helper
    static bool isPointInPolygon(Point p, const std::vector<Point>& poly);

    // Minimum distance from point to segment
    static double pointToSegmentDistance(Point p, Point s1, Point s2);

    // Minimum distance between two segments
    static double segmentToSegmentDistance(Point p1, Point p2, Point q1, Point q2);

private:
    // Checks if a component can be placed at (x, y) on the sheet with specific dimensions
    static bool canPlaceComponent(
        const Component& comp,
        double x, double y,
        const std::vector<Component>& alreadyPlaced,
        const NestingParams& params
    );

    // Overlap helper for two placed 2D axis-aligned rectangles with extra spacing
    static bool intersect(
        double x1, double y1, double w1, double h1,
        double x2, double y2, double w2, double h2,
        double spacing
    );
};

#endif // NESTINGENGINE_H
