#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <vector>
#include "Component.h"

namespace Geometry {
    // Checks if a 2D polygon is strictly convex
    bool isConvex(const std::vector<Point>& poly);

    // Computes outward offset boundary of a polygon by distance d
    std::vector<Point> offsetPolygon(const std::vector<Point>& poly, double distance);

    // Computes No-Fit-Polygon (Minkowski difference A \oplus (-B)) contact boundary vertices.
    // For convex polygons, uses exact polar edge vector convolution sorting.
    std::vector<Point> computeNFP(const std::vector<Point>& polyA_stationary, const std::vector<Point>& polyB_moving);
}

#endif // GEOMETRY_H
