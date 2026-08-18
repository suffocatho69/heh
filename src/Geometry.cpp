#include "Geometry.h"
#include <cmath>
#include <algorithm>

namespace Geometry {

bool isConvex(const std::vector<Point>& poly) {
    if (poly.size() < 3) return false;
    bool hasPos = false;
    bool hasNeg = false;
    size_t n = poly.size();
    for (size_t i = 0; i < n; ++i) {
        Point p1 = poly[i];
        Point p2 = poly[(i + 1) % n];
        Point p3 = poly[(i + 2) % n];

        double cross = (p2.x - p1.x) * (p3.y - p2.y) - (p2.y - p1.y) * (p3.x - p2.x);
        if (cross > 1e-7) hasPos = true;
        if (cross < -1e-7) hasNeg = true;
        if (hasPos && hasNeg) return false;
    }
    return true;
}

std::vector<Point> offsetPolygon(const std::vector<Point>& poly, double distance) {
    if (poly.size() < 3 || std::abs(distance) < 1e-6) {
        return poly;
    }

    size_t n = poly.size();
    std::vector<Point> offsetPoly;

    // Helper to calculate line intersection of two 2D lines (p1-p2 and p3-p4)
    auto lineIntersection = [](Point p1, Point p2, Point p3, Point p4, Point& outIntersect) -> bool {
        double A1 = p2.y - p1.y;
        double B1 = p1.x - p2.x;
        double C1 = A1 * p1.x + B1 * p1.y;

        double A2 = p4.y - p3.y;
        double B2 = p3.x - p4.x;
        double C2 = A2 * p3.x + B2 * p3.y;

        double det = A1 * B2 - A2 * B1;
        if (std::abs(det) < 1e-9) return false;

        outIntersect.x = (B2 * C1 - B1 * C2) / det;
        outIntersect.y = (A1 * C2 - A2 * C1) / det;
        return true;
    };

    std::vector<std::pair<Point, Point>> offsetSegments;
    for (size_t i = 0; i < n; ++i) {
        Point p1 = poly[i];
        Point p2 = poly[(i + 1) % n];

        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        double len = std::sqrt(dx * dx + dy * dy);
        if (len < 1e-9) continue;

        // Outward normal
        double nx = -dy / len;
        double ny = dx / len;

        Point op1(p1.x + nx * distance, p1.y + ny * distance);
        Point op2(p2.x + nx * distance, p2.y + ny * distance);
        offsetSegments.push_back({ op1, op2 });
    }

    if (offsetSegments.empty()) return poly;

    size_t m = offsetSegments.size();
    for (size_t i = 0; i < m; ++i) {
        auto seg1 = offsetSegments[i];
        auto seg2 = offsetSegments[(i + 1) % m];

        Point intersectPt;
        if (lineIntersection(seg1.first, seg1.second, seg2.first, seg2.second, intersectPt)) {
            offsetPoly.push_back(intersectPt);
        } else {
            offsetPoly.push_back(seg1.second);
        }
    }

    return offsetPoly;
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct EdgeVector {
    double dx;
    double dy;
    double angle;
};

std::vector<Point> computeNFP(const std::vector<Point>& polyA_stationary, const std::vector<Point>& polyB_moving) {
    if (polyA_stationary.size() < 3 || polyB_moving.size() < 3) {
        return polyA_stationary;
    }

    // 1. Find vertex A with min Y (and min X if tied)
    Point minA = polyA_stationary[0];
    for (const auto& p : polyA_stationary) {
        if (p.y < minA.y || (std::abs(p.y - minA.y) < 1e-6 && p.x < minA.x)) {
            minA = p;
        }
    }

    // 2. Find vertex B with min Y (and min X if tied)
    Point minB = polyB_moving[0];
    for (const auto& p : polyB_moving) {
        if (p.y < minB.y || (std::abs(p.y - minB.y) < 1e-6 && p.x < minB.x)) {
            minB = p;
        }
    }

    // 3. Collect edge vectors of A
    std::vector<EdgeVector> edges;
    size_t nA = polyA_stationary.size();
    for (size_t i = 0; i < nA; ++i) {
        Point p1 = polyA_stationary[i];
        Point p2 = polyA_stationary[(i + 1) % nA];
        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        if (std::hypot(dx, dy) > 1e-6) {
            double ang = std::atan2(dy, dx);
            if (ang < 0) ang += 2.0 * M_PI;
            edges.push_back({ dx, dy, ang });
        }
    }

    // 4. Collect negated edge vectors of B (-B)
    size_t nB = polyB_moving.size();
    for (size_t j = 0; j < nB; ++j) {
        Point p1 = polyB_moving[j];
        Point p2 = polyB_moving[(j + 1) % nB];
        // Edge of -B: (p1 - p2)
        double dx = p1.x - p2.x;
        double dy = p1.y - p2.y;
        if (std::hypot(dx, dy) > 1e-6) {
            double ang = std::atan2(dy, dx);
            if (ang < 0) ang += 2.0 * M_PI;
            edges.push_back({ dx, dy, ang });
        }
    }

    // 5. Sort all edge vectors by polar angle
    std::sort(edges.begin(), edges.end(), [](const EdgeVector& e1, const EdgeVector& e2) {
        return e1.angle < e2.angle;
    });

    // 6. Generate connected closed NFP contour starting at startPt = minA - minB
    std::vector<Point> nfpContour;
    Point curr(minA.x - minB.x, minA.y - minB.y);
    nfpContour.push_back(curr);

    for (const auto& e : edges) {
        curr.x += e.dx;
        curr.y += e.dy;
        nfpContour.push_back(curr);
    }

    return nfpContour;
}

} // namespace Geometry
