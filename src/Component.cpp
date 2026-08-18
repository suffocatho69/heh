#include "Component.h"
#include <cmath>
#include <algorithm>
#include <iostream>

Component::Component()
    : name(""), width(0), height(0), quantity(0), placed(false), posX(0), posY(0),
      rotationAngle(0.0), rotated(false), sheetIndex(-1) {
}

Component::Component(const std::string& name, double w, double h, int qty)
    : name(name), width(w), height(h), quantity(qty), placed(false), posX(0), posY(0),
      rotationAngle(0.0), rotated(false), sheetIndex(-1) {
    generateDefaultRectangleGeometry();
}

void Component::generateDefaultRectangleGeometry() {
    geometry.clear();

    // Add four lines enclosing the rectangle [0, 0] to [width, height]
    GeoEntity top, right, bottom, left;

    top.type = GeoEntity::LINE;
    top.x1 = 0; top.y1 = height;
    top.x2 = width; top.y2 = height;

    right.type = GeoEntity::LINE;
    right.x1 = width; right.y1 = height;
    right.x2 = width; right.y2 = 0;

    bottom.type = GeoEntity::LINE;
    bottom.x1 = width; bottom.y1 = 0;
    bottom.x2 = 0; bottom.y2 = 0;

    left.type = GeoEntity::LINE;
    left.x1 = 0; left.y1 = 0;
    left.x2 = 0; left.y2 = height;

    geometry.push_back(top);
    geometry.push_back(right);
    geometry.push_back(bottom);
    geometry.push_back(left);

    buildContours(0.1);
}

// Rotates a point around origin (0, 0)
Point Component::transformPoint(Point p, double angle_deg, Point origin) {
    double dx = p.x - origin.x;
    double dy = p.y - origin.y;

    double rx = 0;
    double ry = 0;

    // Normalize angle to [0, 360)
    double angle = angle_deg;
    while (angle < 0) angle += 360.0;
    while (angle >= 360.0) angle -= 360.0;

    if (std::abs(angle - 0.0) < 1e-4) {
        rx = dx;
        ry = dy;
    } else if (std::abs(angle - 90.0) < 1e-4) {
        rx = -dy;
        ry = dx;
    } else if (std::abs(angle - 180.0) < 1e-4) {
        rx = -dx;
        ry = -dy;
    } else if (std::abs(angle - 270.0) < 1e-4) {
        rx = dy;
        ry = -dx;
    } else {
        double rad = angle * M_PI / 180.0;
        double cosA = std::cos(rad);
        double sinA = std::sin(rad);
        rx = dx * cosA - dy * sinA;
        ry = dx * sinA + dy * cosA;
    }

    return Point(rx + origin.x, ry + origin.y);
}

// Flatten circle into segments
static std::vector<Point> flattenCircle(double cx, double cy, double r, double tolerance) {
    std::vector<Point> pts;
    if (r <= 0) return pts;

    // Calculate number of segments needed
    int num_segs = 32;
    if (tolerance > 0) {
        double val = 1.0 - (tolerance / r);
        if (val > -1.0 && val < 1.0) {
            double angle = std::acos(val);
            num_segs = static_cast<int>(std::ceil(2.0 * M_PI / angle));
        }
    }
    if (num_segs < 12) num_segs = 12;
    if (num_segs > 180) num_segs = 180;

    // Round up to nearest multiple of 4 to ensure vertices on cardinal directions
    num_segs = ((num_segs + 3) / 4) * 4;

    for (int i = 0; i <= num_segs; ++i) {
        double a = (2.0 * M_PI * i) / num_segs;
        pts.push_back(Point(cx + r * std::cos(a), cy + r * std::sin(a)));
    }
    return pts;
}

// Flatten arc CCW from start_angle to end_angle
static std::vector<Point> flattenArc(double cx, double cy, double r, double start_angle, double end_angle, bool ccw, double tolerance) {
    std::vector<Point> pts;
    if (r <= 0) return pts;

    double s = start_angle;
    double e = end_angle;

    // Normalize angles to [0, 360)
    auto norm = [](double angle) {
        while (angle < 0) angle += 360.0;
        while (angle >= 360.0) angle -= 360.0;
        return angle;
    };

    s = norm(s);
    e = norm(e);

    double diff = 0;
    if (ccw) {
        diff = (e >= s) ? (e - s) : (360.0 - s + e);
    } else {
        diff = (s >= e) ? (s - e) : (360.0 - e + s);
    }

    if (diff == 0) diff = 360.0; // complete circle

    int num_segs = static_cast<int>(std::ceil(diff / 10.0)); // 10 deg steps by default
    if (tolerance > 0) {
        double val = 1.0 - (tolerance / r);
        if (val > -1.0 && val < 1.0) {
            double angle_step = std::acos(val) * 180.0 / M_PI;
            if (angle_step > 0) {
                num_segs = static_cast<int>(std::ceil(diff / angle_step));
            }
        }
    }
    if (num_segs < 4) num_segs = 4;
    if (num_segs > 120) num_segs = 120;

    for (int i = 0; i <= num_segs; ++i) {
        double frac = static_cast<double>(i) / num_segs;
        double a_deg;
        if (ccw) {
            a_deg = s + frac * diff;
        } else {
            a_deg = s - frac * diff;
        }
        double a_rad = norm(a_deg) * M_PI / 180.0;
        pts.push_back(Point(cx + r * std::cos(a_rad), cy + r * std::sin(a_rad)));
    }
    return pts;
}

// Reconstruct outer and inner contours from the geometry entities
void Component::buildContours(double tolerance) {
    outerContour.clear();
    innerContours.clear();

    std::vector<std::vector<Point>> loops;

    struct Segment {
        Point p1, p2;
        std::vector<Point> intermediate; // For curves, includes full path
    };

    std::vector<Segment> segs;

    // Flatten entities to simple segments
    for (const auto& ent : geometry) {
        if (ent.type == GeoEntity::LINE) {
            Segment seg;
            seg.p1 = Point(ent.x1, ent.y1);
            seg.p2 = Point(ent.x2, ent.y2);
            seg.intermediate.push_back(seg.p1);
            seg.intermediate.push_back(seg.p2);
            segs.push_back(seg);
        } else if (ent.type == GeoEntity::CIRCLE) {
            auto circle_pts = flattenCircle(ent.x1, ent.y1, ent.radius, tolerance);
            if (!circle_pts.empty()) {
                loops.push_back(circle_pts);
            }
        } else if (ent.type == GeoEntity::ARC) {
            auto arc_pts = flattenArc(ent.x1, ent.y1, ent.radius, ent.start_angle, ent.end_angle, ent.ccw, tolerance);
            if (arc_pts.size() >= 2) {
                Segment seg;
                seg.p1 = arc_pts.front();
                seg.p2 = arc_pts.back();
                seg.intermediate = arc_pts;
                segs.push_back(seg);
            }
        } else if (ent.type == GeoEntity::POLYLINE || ent.type == GeoEntity::LWPOLYLINE) {
            // Process polyline points
            if (ent.points.size() >= 2) {
                for (size_t i = 0; i < ent.points.size() - 1; ++i) {
                    Segment seg;
                    seg.p1 = ent.points[i];
                    seg.p2 = ent.points[i+1];
                    seg.intermediate.push_back(seg.p1);
                    seg.intermediate.push_back(seg.p2);
                    segs.push_back(seg);
                }
            }
        }
    }

    // Now, chain separate segments together to find closed loops
    const double eps = 0.5; // connection tolerance in mm

    auto dist = [](Point a, Point b) {
        return std::sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
    };

    while (!segs.empty()) {
        std::vector<Point> current_loop;
        // Start with first remaining segment
        Segment current_seg = segs.front();
        segs.erase(segs.begin());

        current_loop = current_seg.intermediate;
        Point last_pt = current_seg.p2;

        bool found_next = true;
        while (found_next) {
            found_next = false;
            for (auto it = segs.begin(); it != segs.end(); ++it) {
                // Match start of next with end of current
                if (dist(last_pt, it->p1) < eps) {
                    current_loop.insert(current_loop.end(), it->intermediate.begin() + 1, it->intermediate.end());
                    last_pt = it->p2;
                    segs.erase(it);
                    found_next = true;
                    break;
                }
                // Match end of next with end of current (reversed)
                else if (dist(last_pt, it->p2) < eps) {
                    std::vector<Point> rev = it->intermediate;
                    std::reverse(rev.begin(), rev.end());
                    current_loop.insert(current_loop.end(), rev.begin() + 1, rev.end());
                    last_pt = it->p1;
                    segs.erase(it);
                    found_next = true;
                    break;
                }
            }
        }

        if (current_loop.size() >= 3) {
            // Check if closed
            if (dist(current_loop.front(), current_loop.back()) < eps) {
                current_loop.back() = current_loop.front(); // force exact closure
            } else {
                // Not closed, but still treat as a path
            }
            loops.push_back(current_loop);
        }
    }

    if (loops.empty()) {
        // Fallback to bounding box contour
        outerContour.push_back(Point(0, 0));
        outerContour.push_back(Point(width, 0));
        outerContour.push_back(Point(width, height));
        outerContour.push_back(Point(0, height));
        outerContour.push_back(Point(0, 0));
        return;
    }

    // Sort loops by 2D polygon area. The largest one is the outer contour!
    auto polyArea = [](const std::vector<Point>& poly) {
        double area = 0.0;
        size_t j = poly.size() - 1;
        for (size_t i = 0; i < poly.size(); ++i) {
            area += (poly[j].x + poly[i].x) * (poly[j].y - poly[i].y);
            j = i;
        }
        return std::abs(area / 2.0);
    };

    std::sort(loops.begin(), loops.end(), [&](const std::vector<Point>& a, const std::vector<Point>& b) {
        return polyArea(a) > polyArea(b);
    });

    outerContour = loops.front();
    for (size_t i = 1; i < loops.size(); ++i) {
        innerContours.push_back(loops[i]);
    }
}

static double calculateLoopArea(const std::vector<Point>& poly) {
    if (poly.size() < 3) return 0.0;
    double area = 0.0;
    size_t j = poly.size() - 1;
    for (size_t i = 0; i < poly.size(); ++i) {
        area += (poly[j].x + poly[i].x) * (poly[j].y - poly[i].y);
        j = i;
    }
    return std::abs(area / 2.0);
}

double Component::calculateShoelaceArea() const {
    double outerArea = calculateLoopArea(outerContour);
    double innerAreaTotal = 0.0;
    for (const auto& inner : innerContours) {
        innerAreaTotal += calculateLoopArea(inner);
    }
    double realArea = outerArea - innerAreaTotal;
    return (realArea > 0.0) ? realArea : 0.0;
}

// Bounding box size getters
double Component::getEffectiveWidth() const {
    double minX = 1e30, maxX = -1e30;
    auto pts = getFlattenedOuterPolygon();
    if (pts.empty()) return rotated ? height : width;

    for (const auto& p : pts) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
    }
    return maxX - minX;
}

double Component::getEffectiveHeight() const {
    double minY = 1e30, maxY = -1e30;
    auto pts = getFlattenedOuterPolygon();
    if (pts.empty()) return rotated ? width : height;

    for (const auto& p : pts) {
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }
    return maxY - minY;
}

void Component::recalculateBoundingBox() {
    // Already calculated on the fly by getEffectiveWidth / getEffectiveHeight
}

// Get the actual transformed outer polygon
std::vector<Point> Component::getFlattenedOuterPolygon() const {
    std::vector<Point> transformed;
    if (outerContour.empty()) return transformed;

    double angle = rotationAngle;
    if (rotated && angle == 0.0) {
        angle = 90.0;
    }

    // First, rotate around origin (0, 0)
    double minX = 1e30, minY = 1e30;
    for (const auto& p : outerContour) {
        Point rp = transformPoint(p, angle, Point(0, 0));
        transformed.push_back(rp);
        if (rp.x < minX) minX = rp.x;
        if (rp.y < minY) minY = rp.y;
    }

    // Shift so minimum bounds start at (0, 0)
    for (auto& p : transformed) {
        p.x -= minX;
        p.y -= minY;
    }

    return transformed;
}

std::pair<double, double> Component::localToGlobal(double lx, double ly) const {
    double angle = rotationAngle;
    if (rotated && angle == 0.0) {
        angle = 90.0;
    }

    // Get transformed local points offset so we know how to map lx, ly
    std::vector<Point> rot_local;
    double minX = 1e30, minY = 1e30;
    for (const auto& p : outerContour) {
        Point rp = transformPoint(p, angle, Point(0,0));
        if (rp.x < minX) minX = rp.x;
        if (rp.y < minY) minY = rp.y;
    }

    // Rotate the query point (lx, ly)
    Point rotated_pt = transformPoint(Point(lx, ly), angle, Point(0,0));

    // Shift by minX, minY and translate to posX, posY
    double gx = posX + (rotated_pt.x - minX);
    double gy = posY + (rotated_pt.y - minY);

    return std::make_pair(gx, gy);
}

std::vector<Point> flattenGeometry(
    const Component& component,
    double tolerance
) {
    Component temp = component;
    temp.buildContours(tolerance);
    return temp.getFlattenedOuterPolygon();
}
