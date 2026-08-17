#include "Geometry.h"
#include <cmath>
#include <algorithm>

namespace Geometry {

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

std::vector<Point> computeNFP(const std::vector<Point>& polyA_stationary, const std::vector<Point>& polyB_moving) {
    std::vector<Point> nfpPoints;
    if (polyA_stationary.empty() || polyB_moving.empty()) return nfpPoints;

    // 1. Sliding vertices of B along edges of A
    size_t nA = polyA_stationary.size();
    for (size_t i = 0; i < nA; ++i) {
        Point a1 = polyA_stationary[i];
        Point a2 = polyA_stationary[(i + 1) % nA];

        double dx = a2.x - a1.x;
        double dy = a2.y - a1.y;

        for (const auto& bj : polyB_moving) {
            double samples[3] = { 0.0, 0.5, 1.0 };
            for (double t : samples) {
                double edgeX = a1.x + t * dx;
                double edgeY = a1.y + t * dy;
                nfpPoints.push_back(Point(edgeX - bj.x, edgeY - bj.y));
            }
        }
    }

    // 2. Sliding edges of B along vertices of A
    size_t nB = polyB_moving.size();
    for (const auto& ai : polyA_stationary) {
        for (size_t j = 0; j < nB; ++j) {
            Point b1 = polyB_moving[j];
            Point b2 = polyB_moving[(j + 1) % nB];

            double dx = b2.x - b1.x;
            double dy = b2.y - b1.y;

            double samples[3] = { 0.0, 0.5, 1.0 };
            for (double t : samples) {
                double edgeX = b1.x + t * dx;
                double edgeY = b1.y + t * dy;
                nfpPoints.push_back(Point(ai.x - edgeX, ai.y - edgeY));
            }
        }
    }

    return nfpPoints;
}

} // namespace Geometry
