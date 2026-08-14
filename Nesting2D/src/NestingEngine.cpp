#include "NestingEngine.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>

bool NestingEngine::intersect(
    double x1, double y1, double w1, double h1,
    double x2, double y2, double w2, double h2,
    double spacing
) {
    return !(x1 + w1 + spacing <= x2 ||
             x2 + w2 + spacing <= x1 ||
             y1 + h1 + spacing <= y2 ||
             y2 + h2 + spacing <= y1);
}

bool NestingEngine::isPointInPolygon(Point p, const std::vector<Point>& poly) {
    bool inside = false;
    size_t j = poly.size() - 1;
    for (size_t i = 0; i < poly.size(); ++i) {
        if (((poly[i].y > p.y) != (poly[j].y > p.y)) &&
            (p.x < (poly[j].x - poly[i].x) * (p.y - poly[i].y) / (poly[j].y - poly[i].y) + poly[i].x)) {
            inside = !inside;
        }
        j = i;
    }
    return inside;
}

double NestingEngine::pointToSegmentDistance(Point p, Point s1, Point s2) {
    double dx = s2.x - s1.x;
    double dy = s2.y - s1.y;
    double lenSq = dx * dx + dy * dy;
    if (lenSq < 1e-9) {
        return std::sqrt((p.x - s1.x)*(p.x - s1.x) + (p.y - s1.y)*(p.y - s1.y));
    }
    double t = ((p.x - s1.x) * dx + (p.y - s1.y) * dy) / lenSq;
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    double closestX = s1.x + t * dx;
    double closestY = s1.y + t * dy;
    return std::sqrt((p.x - closestX)*(p.x - closestX) + (p.y - closestY)*(p.y - closestY));
}

double NestingEngine::segmentToSegmentDistance(Point p1, Point p2, Point q1, Point q2) {
    if (std::max(p1.x, p2.x) < std::min(q1.x, q2.x) || std::max(q1.x, q2.x) < std::min(p1.x, p2.x) ||
        std::max(p1.y, p2.y) < std::min(q1.y, q2.y) || std::max(q1.y, q2.y) < std::min(p1.y, p2.y)) {
        double dist = pointToSegmentDistance(q1, p1, p2);
        dist = std::min(dist, pointToSegmentDistance(q2, p1, p2));
        dist = std::min(dist, pointToSegmentDistance(p1, q1, q2));
        dist = std::min(dist, pointToSegmentDistance(p2, q1, q2));
        return dist;
    }

    double d1 = ((p2.x - p1.x)*(q1.y - p1.y) - (p2.y - p1.y)*(q1.x - p1.x));
    double d2 = ((p2.x - p1.x)*(q2.y - p1.y) - (p2.y - p1.y)*(q2.x - p1.x));
    double d3 = ((q2.x - q1.x)*(p1.y - q1.y) - (q2.y - q1.y)*(p1.x - q1.x));
    double d4 = ((q2.x - q1.x)*(p2.y - q1.y) - (q2.y - q1.y)*(p2.x - q1.x));

    if (((d1 > 0 && d2 < 0) || (d1 < 0 && d2 > 0)) &&
        ((d3 > 0 && d4 < 0) || (d3 < 0 && d4 > 0))) {
        return 0.0;
    }

    double dist = pointToSegmentDistance(q1, p1, p2);
    dist = std::min(dist, pointToSegmentDistance(q2, p1, p2));
    dist = std::min(dist, pointToSegmentDistance(p1, q1, q2));
    dist = std::min(dist, pointToSegmentDistance(p2, q1, q2));
    return dist;
}

bool NestingEngine::checkCollision(const Component& comp1, const Component& comp2, double spacing) {
    double w1 = comp1.getEffectiveWidth();
    double h1 = comp1.getEffectiveHeight();
    double w2 = comp2.getEffectiveWidth();
    double h2 = comp2.getEffectiveHeight();

    // Bounding box pre-filter
    if (comp1.posX + w1 + spacing <= comp2.posX ||
        comp2.posX + w2 + spacing <= comp1.posX ||
        comp1.posY + h1 + spacing <= comp2.posY ||
        comp2.posY + h2 + spacing <= comp1.posY) {
        return false;
    }

    auto poly1 = comp1.getFlattenedOuterPolygon();
    auto poly2 = comp2.getFlattenedOuterPolygon();

    if (poly1.empty() || poly2.empty()) return true;

    // Convert outer contours to global coordinates
    std::vector<Point> g_outer1;
    for (const auto& p : poly1) {
        auto gp = comp1.localToGlobal(p.x, p.y);
        g_outer1.push_back(Point(gp.first, gp.second));
    }

    std::vector<Point> g_outer2;
    for (const auto& p : poly2) {
        auto gp = comp2.localToGlobal(p.x, p.y);
        g_outer2.push_back(Point(gp.first, gp.second));
    }

    // Convert inner contours (holes) to global coordinates
    std::vector<std::vector<Point>> g_inners1;
    for (const auto& inner : comp1.innerContours) {
        std::vector<Point> g_inner;
        for (const auto& p : inner) {
            auto gp = comp1.localToGlobal(p.x, p.y);
            g_inner.push_back(Point(gp.first, gp.second));
        }
        g_inners1.push_back(g_inner);
    }

    std::vector<std::vector<Point>> g_inners2;
    for (const auto& inner : comp2.innerContours) {
        std::vector<Point> g_inner;
        for (const auto& p : inner) {
            auto gp = comp2.localToGlobal(p.x, p.y);
            g_inner.push_back(Point(gp.first, gp.second));
        }
        g_inners2.push_back(g_inner);
    }

    // Helper lambda: check if two global polygons collide
    auto polygonsCollide = [](const std::vector<Point>& A, const std::vector<Point>& B, double sp) {
        for (size_t i = 0; i < A.size() - 1; ++i) {
            for (size_t j = 0; j < B.size() - 1; ++j) {
                double d = segmentToSegmentDistance(A[i], A[i+1], B[j], B[j+1]);
                if (d < sp - 1e-4) {
                    return true;
                }
            }
        }
        return false;
    };

    // Check if comp2 is nested inside one of the holes of comp1 (innerContours / shape-in-shape support)
    bool nestedInHole1 = false;
    for (const auto& h : g_inners1) {
        if (isPointInPolygon(g_outer2[0], h)) {
            // comp2 must not collide with the hole boundaries
            if (!polygonsCollide(g_outer2, h, spacing)) {
                // And must not cross or be outside the hole (all vertices must be inside)
                bool allInside = true;
                for (const auto& pt : g_outer2) {
                    if (!isPointInPolygon(pt, h)) {
                        allInside = false;
                        break;
                    }
                }
                if (allInside) {
                    nestedInHole1 = true;
                    break;
                }
            }
        }
    }

    // Check if comp1 is nested inside one of the holes of comp2
    bool nestedInHole2 = false;
    for (const auto& h : g_inners2) {
        if (isPointInPolygon(g_outer1[0], h)) {
            // comp1 must not collide with the hole boundaries
            if (!polygonsCollide(g_outer1, h, spacing)) {
                bool allInside = true;
                for (const auto& pt : g_outer1) {
                    if (!isPointInPolygon(pt, h)) {
                        allInside = false;
                        break;
                    }
                }
                if (allInside) {
                    nestedInHole2 = true;
                    break;
                }
            }
        }
    }

    // If either is nested in a hole, there is no collision!
    if (nestedInHole1 || nestedInHole2) {
        return false;
    }

    // Otherwise, check for normal outer contour collision
    if (polygonsCollide(g_outer1, g_outer2, spacing)) {
        return true;
    }

    if (isPointInPolygon(g_outer1[0], g_outer2) || isPointInPolygon(g_outer2[0], g_outer1)) {
        return true;
    }

    return false;
}

bool NestingEngine::canPlaceComponent(
    const Component& comp,
    double x, double y,
    const std::vector<Component>& alreadyPlaced,
    const NestingParams& params
) {
    double w = comp.getEffectiveWidth();
    double h = comp.getEffectiveHeight();

    if (x < params.margin || (x + w) > (params.sheetWidth - params.margin)) {
        return false;
    }
    if (y < params.margin || (y + h) > (params.sheetHeight - params.margin)) {
        return false;
    }

    Component trial = comp;
    trial.posX = x;
    trial.posY = y;

    for (const auto& placed : alreadyPlaced) {
        if (intersect(x, y, w, h,
                      placed.posX, placed.posY, placed.getEffectiveWidth(), placed.getEffectiveHeight(),
                      params.spacing)) {
            if (checkCollision(trial, placed, params.spacing)) {
                return false;
            }
        }
    }

    return true;
}

// Inner struct to evaluate nesting solutions
struct NestingSolution {
    std::vector<SheetLayout> sheets;
    int unplacedCount = 0;
    double totalUtilization = 0.0;
    double cncLength = 0.0;
};

// Main Nesting implementation using multiple strategies and selecting the absolute best
std::vector<SheetLayout> NestingEngine::performNesting(
    const std::vector<Component>& originalComponents,
    const NestingParams& params
) {
    // 1. Flatten list to individual units based on their quantities
    std::vector<Component> baseItems;
    for (const auto& c : originalComponents) {
        for (int i = 0; i < c.quantity; ++i) {
            Component unit = c;
            unit.quantity = 1;
            unit.placed = false;
            baseItems.push_back(unit);
        }
    }

    if (baseItems.empty()) {
        return std::vector<SheetLayout>();
    }

    // 2. Define standard multi-strategy sort passes
    std::vector<NestingSolution> solutions;

    // Check allowed rotations
    std::vector<int> allowedRotations;
    if (params.allowRot0) allowedRotations.push_back(0);
    if (params.allowRot90) allowedRotations.push_back(90);
    if (params.allowRot180) allowedRotations.push_back(180);
    if (params.allowRot270) allowedRotations.push_back(270);
    if (allowedRotations.empty()) allowedRotations.push_back(0);

    // Run 7 different sorting strategies
    for (int strategy = 0; strategy < 7; ++strategy) {
        std::vector<Component> items = baseItems;

        if (strategy == 0) {
            // Area Descending
            std::sort(items.begin(), items.end(), [](const Component& a, const Component& b) {
                return (a.width * a.height) > (b.width * b.height);
            });
        } else if (strategy == 1) {
            // Width Descending
            std::sort(items.begin(), items.end(), [](const Component& a, const Component& b) {
                return a.width > b.width;
            });
        } else if (strategy == 2) {
            // Height Descending
            std::sort(items.begin(), items.end(), [](const Component& a, const Component& b) {
                return a.height > b.height;
            });
        } else if (strategy == 3) {
            // Longest Side Descending
            std::sort(items.begin(), items.end(), [](const Component& a, const Component& b) {
                return std::max(a.width, a.height) > std::max(b.width, b.height);
            });
        } else if (strategy == 4) {
            // Shortest Side Descending
            std::sort(items.begin(), items.end(), [](const Component& a, const Component& b) {
                return std::min(a.width, a.height) > std::min(b.width, b.height);
            });
        } else if (strategy == 5) {
            // Best Fit Heuristic Order
            std::sort(items.begin(), items.end(), [](const Component& a, const Component& b) {
                return (a.width * a.height + std::max(a.width, a.height)) > (b.width * b.height + std::max(b.width, b.height));
            });
        } else {
            // Perturbed variations (Deterministic shuffle of area descending)
            std::sort(items.begin(), items.end(), [](const Component& a, const Component& b) {
                return (a.width * a.height) > (b.width * b.height);
            });
            for (size_t i = 0; i < items.size(); i += 2) {
                if (i + 1 < items.size()) {
                    std::swap(items[i], items[i+1]);
                }
            }
        }

        // Run packing for this strategy
        std::vector<SheetLayout> sheets;
        int unplacedCount = 0;

        for (auto& item : items) {
            bool placedSuccessfully = false;

            for (size_t sheetIdx = 0; sheetIdx < sheets.size(); ++sheetIdx) {
                auto& sheet = sheets[sheetIdx];

                double bestX = -1, bestY = -1;
                int bestRotation = 0;
                double bestScore = 1e30;

                // Generate Bottom-Left anchor search points
                std::vector<std::pair<double, double>> scanPoints;
                scanPoints.push_back(std::make_pair(params.margin, params.margin));

                for (const auto& placed : sheet.placedComponents) {
                    scanPoints.push_back(std::make_pair(placed.posX, placed.posY + placed.getEffectiveHeight() + params.spacing));
                    scanPoints.push_back(std::make_pair(placed.posX + placed.getEffectiveWidth() + params.spacing, placed.posY));
                }

                for (const auto& pt : scanPoints) {
                    double tx = pt.first;
                    double ty = pt.second;

                    for (int rot : allowedRotations) {
                        item.rotationAngle = rot;
                        item.rotated = (rot == 90 || rot == 270);

                        if (canPlaceComponent(item, tx, ty, sheet.placedComponents, params)) {
                            double score = ty * 15.0 + tx;
                            if (score < bestScore) {
                                bestScore = score;
                                bestX = tx;
                                bestY = ty;
                                bestRotation = rot;
                            }
                        }
                    }
                }

                if (bestX >= 0) {
                    item.placed = true;
                    item.posX = bestX;
                    item.posY = bestY;
                    item.rotationAngle = bestRotation;
                    item.rotated = (bestRotation == 90 || bestRotation == 270);
                    item.sheetIndex = static_cast<int>(sheetIdx);
                    sheet.placedComponents.push_back(item);
                    placedSuccessfully = true;
                    break;
                }
            }

            if (!placedSuccessfully) {
                SheetLayout newSheet;
                newSheet.width = params.sheetWidth;
                newSheet.height = params.sheetHeight;
                newSheet.materialUtilization = 0;

                double bestX = -1, bestY = -1;
                int bestRotation = 0;
                double bestScore = 1e30;

                for (int rot : allowedRotations) {
                    item.rotationAngle = rot;
                    item.rotated = (rot == 90 || rot == 270);
                    if (canPlaceComponent(item, params.margin, params.margin, newSheet.placedComponents, params)) {
                        double score = rot;
                        if (score < bestScore) {
                            bestScore = score;
                            bestX = params.margin;
                            bestY = params.margin;
                            bestRotation = rot;
                        }
                    }
                }

                if (bestX >= 0) {
                    item.placed = true;
                    item.posX = bestX;
                    item.posY = bestY;
                    item.rotationAngle = bestRotation;
                    item.rotated = (bestRotation == 90 || bestRotation == 270);
                    item.sheetIndex = static_cast<int>(sheets.size());
                    newSheet.placedComponents.push_back(item);
                    sheets.push_back(newSheet);
                } else {
                    unplacedCount++;
                }
            }
        }

        // Calculate material utilization and CNC travel distance
        double totalPartsArea = 0;
        double sheetTotalArea = params.sheetWidth * params.sheetHeight;
        double totalUtilization = 0;
        double cncLength = 0;

        for (auto& sheet : sheets) {
            double partsArea = 0;
            Point lastPos(params.margin, params.margin);
            for (const auto& c : sheet.placedComponents) {
                partsArea += (c.width * c.height);
                cncLength += std::sqrt((c.posX - lastPos.x)*(c.posX - lastPos.x) + (c.posY - lastPos.y)*(c.posY - lastPos.y));
                lastPos = Point(c.posX, c.posY);
            }
            sheet.materialUtilization = (partsArea / sheetTotalArea) * 100.0;
            totalPartsArea += partsArea;
        }

        if (!sheets.empty()) {
            totalUtilization = (totalPartsArea / (sheets.size() * sheetTotalArea)) * 100.0;
        }

        NestingSolution sol;
        sol.sheets = sheets;
        sol.unplacedCount = unplacedCount;
        sol.totalUtilization = totalUtilization;
        sol.cncLength = cncLength;
        solutions.push_back(sol);
    }

    // 3. Selection of the absolute best result based on multi-level priority criteria
    auto bestIt = std::min_element(solutions.begin(), solutions.end(), [](const NestingSolution& a, const NestingSolution& b) {
        // Priority 1: Least unplaced parts
        if (a.unplacedCount != b.unplacedCount) {
            return a.unplacedCount < b.unplacedCount;
        }
        // Priority 2: Least sheets used
        if (a.sheets.size() != b.sheets.size()) {
            return a.sheets.size() < b.sheets.size();
        }
        // Priority 3: Highest material utilization
        if (std::abs(a.totalUtilization - b.totalUtilization) > 1e-2) {
            return a.totalUtilization > b.totalUtilization;
        }
        // Priority 4: Shortest rapid CNC travel paths
        return a.cncLength < b.cncLength;
    });

    return bestIt->sheets;
}

bool NestingEngine::validateNesting(
    const std::vector<SheetLayout>& sheets,
    const NestingParams& params,
    std::string& outErrorMessage
) {
    if (sheets.empty()) {
        outErrorMessage = "Brak wygenerowanego rozkroju płyt!";
        return false;
    }

    auto dist = [](Point a, Point b) {
        return std::sqrt((a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y));
    };

    for (size_t s = 0; s < sheets.size(); ++s) {
        const auto& sheet = sheets[s];

        for (size_t i = 0; i < sheet.placedComponents.size(); ++i) {
            const auto& comp = sheet.placedComponents[i];
            double w = comp.getEffectiveWidth();
            double h = comp.getEffectiveHeight();

            // 1. Fits on sheet
            if (comp.posX < params.margin || comp.posY < params.margin ||
                comp.posX + w > params.sheetWidth - params.margin ||
                comp.posY + h > params.sheetHeight - params.margin) {
                outErrorMessage = "Detal '" + comp.name + "' na płycie #" + std::to_string(s + 1) + " wykracza poza granice płyty (uwzględniając marginesy)!";
                return false;
            }

            auto poly = comp.getFlattenedOuterPolygon();
            if (poly.size() < 3) {
                outErrorMessage = "Detal '" + comp.name + "' posiada niepoprawną lub pustą geometrię konturu!";
                return false;
            }

            // 2. Closure check
            if (dist(poly.front(), poly.back()) > 0.5) {
                outErrorMessage = "Kontur detalu '" + comp.name + "' nie jest poprawnie zamknięty (odstęp wynosi " + std::to_string(dist(poly.front(), poly.back())) + " mm)!";
                return false;
            }

            // 3. Zero-length segments and self-intersection checks
            for (size_t k = 0; k < poly.size() - 1; ++k) {
                double len = dist(poly[k], poly[k+1]);
                if (len < 1e-4) {
                    outErrorMessage = "Detal '" + comp.name + "' zawiera segment o zerowej długości (segment #" + std::to_string(k) + ")!";
                    return false;
                }

                // Check self-intersection against other non-adjacent edges
                for (size_t m = k + 2; m < poly.size() - 1; ++m) {
                    if (k == 0 && m == poly.size() - 2) continue; // adjacent at start/end

                    double d = segmentToSegmentDistance(poly[k], poly[k+1], poly[m], poly[m+1]);
                    if (d < 1e-4) {
                        outErrorMessage = "Kontur detalu '" + comp.name + "' krzyżuje się sam ze sobą!";
                        return false;
                    }
                }
            }

            // 4. Collision check against all other components on the same sheet
            for (size_t j = i + 1; j < sheet.placedComponents.size(); ++j) {
                if (checkCollision(comp, sheet.placedComponents[j], params.spacing)) {
                    outErrorMessage = "Wykryto kolizję między detalem '" + comp.name + "' a detalem '" + sheet.placedComponents[j].name + "' na płycie #" + std::to_string(s + 1) + "!";
                    return false;
                }
            }
        }
    }

    return true;
}
