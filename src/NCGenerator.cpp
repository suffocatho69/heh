#include "NCGenerator.h"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Classifies each geometry entity to a loop index of the component (hole index, or -1 for outer contour)
static int getEntityLoopIndex(const GeoEntity& ent, const Component& comp) {
    auto getStartPt = [](const GeoEntity& next) -> Point {
        if (next.type == GeoEntity::LINE) {
            return Point(next.x1, next.y1);
        } else if (next.type == GeoEntity::ARC) {
            double sa_rad = next.start_angle * M_PI / 180.0;
            return Point(next.x1 + next.radius * std::cos(sa_rad), next.y1 + next.radius * std::sin(sa_rad));
        } else if (next.type == GeoEntity::CIRCLE) {
            return Point(next.x1 + next.radius, next.y1);
        } else if (next.type == GeoEntity::POLYLINE || next.type == GeoEntity::LWPOLYLINE) {
            if (!next.points.empty()) {
                return next.points.front();
            }
        }
        return Point(0, 0);
    };

    Point pStart = getStartPt(ent);

    auto dist = [](Point a, Point b) {
        return std::sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));
    };

    for (size_t h = 0; h < comp.innerContours.size(); ++h) {
        for (const auto& pt : comp.innerContours[h]) {
            if (dist(pStart, pt) < 1.5) {
                return static_cast<int>(h);
            }
        }
    }

    return -1; // Outer contour
}

// Chains entities inside a loop group to minimize tool lifts and make continuous passes
static std::vector<GeoEntity> chainEntities(const std::vector<GeoEntity>& entities) {
    if (entities.empty()) return entities;
    std::vector<GeoEntity> input = entities;
    std::vector<GeoEntity> chained;

    chained.push_back(input.front());
    input.erase(input.begin());

    auto dist = [](double x1, double y1, double x2, double y2) {
        return std::sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
    };

    while (!input.empty()) {
        const auto& last = chained.back();
        double lastX = 0, lastY = 0;
        if (last.type == GeoEntity::LINE || last.type == GeoEntity::ARC) {
            if (last.type == GeoEntity::ARC) {
                double ea_rad = last.end_angle * M_PI / 180.0;
                lastX = last.x1 + last.radius * std::cos(ea_rad);
                lastY = last.y1 + last.radius * std::sin(ea_rad);
            } else {
                lastX = last.x2;
                lastY = last.y2;
            }
        } else if (last.type == GeoEntity::CIRCLE) {
            lastX = last.x1 + last.radius;
            lastY = last.y1;
        } else if (last.type == GeoEntity::POLYLINE || last.type == GeoEntity::LWPOLYLINE) {
            if (!last.points.empty()) {
                lastX = last.points.back().x;
                lastY = last.points.back().y;
            }
        }

        size_t bestIdx = 0;
        double minDist = 1e30;
        bool reverseNext = false;

        for (size_t i = 0; i < input.size(); ++i) {
            const auto& next = input[i];
            double nextStartX = 0, nextStartY = 0;
            double nextEndX = 0, nextEndY = 0;

            if (next.type == GeoEntity::LINE || next.type == GeoEntity::ARC) {
                if (next.type == GeoEntity::ARC) {
                    double sa_rad = next.start_angle * M_PI / 180.0;
                    double ea_rad = next.end_angle * M_PI / 180.0;
                    nextStartX = next.x1 + next.radius * std::cos(sa_rad);
                    nextStartY = next.y1 + next.radius * std::sin(sa_rad);
                    nextEndX = next.x1 + next.radius * std::cos(ea_rad);
                    nextEndY = next.y1 + next.radius * std::sin(ea_rad);
                } else {
                    nextStartX = next.x1; nextStartY = next.y1;
                    nextEndX = next.x2; nextEndY = next.y2;
                }
            } else if (next.type == GeoEntity::CIRCLE) {
                nextStartX = next.x1 + next.radius; nextStartY = next.y1;
                nextEndX = nextStartX; nextEndY = nextStartY;
            } else if (next.type == GeoEntity::POLYLINE || next.type == GeoEntity::LWPOLYLINE) {
                if (!next.points.empty()) {
                    nextStartX = next.points.front().x;
                    nextStartY = next.points.front().y;
                    nextEndX = next.points.back().x;
                    nextEndY = next.points.back().y;
                }
            }

            double dNormal = dist(lastX, lastY, nextStartX, nextStartY);
            double dReverse = dist(lastX, lastY, nextEndX, nextEndY);

            if (dNormal < minDist) {
                minDist = dNormal;
                bestIdx = i;
                reverseNext = false;
            }
            if (dReverse < minDist && (next.type == GeoEntity::LINE || next.type == GeoEntity::ARC || next.type == GeoEntity::POLYLINE)) {
                minDist = dReverse;
                bestIdx = i;
                reverseNext = true;
            }
        }

        GeoEntity nextEnt = input[bestIdx];
        input.erase(input.begin() + bestIdx);

        if (reverseNext) {
            if (nextEnt.type == GeoEntity::LINE) {
                std::swap(nextEnt.x1, nextEnt.x2);
                std::swap(nextEnt.y1, nextEnt.y2);
            } else if (nextEnt.type == GeoEntity::ARC) {
                std::swap(nextEnt.start_angle, nextEnt.end_angle);
                nextEnt.ccw = !nextEnt.ccw;
            } else if (nextEnt.type == GeoEntity::POLYLINE || nextEnt.type == GeoEntity::LWPOLYLINE) {
                std::reverse(nextEnt.points.begin(), nextEnt.points.end());
            }
        }

        chained.push_back(nextEnt);
    }

    return chained;
}

std::string NCGenerator::generateGCode(
    const std::vector<SheetLayout>& sheets,
    const NCParams& params
) {
    std::stringstream ss;

    // Set standard double formatting with 3 decimal places
    ss << std::fixed << std::setprecision(3);

    // Seron Osai specific comments and setup
    ss << ";POSTPROCESOR DLA PLOTERA FREZUJACEGO SERON Z STEROWANIEM OSAI\n";
    ss << ";ZESPOL RETRO NESTINGATOR3000\n";
    ss << ";LICZBA PLYT: " << sheets.size() << "\n";
    ss << "G27\n";
    ss << "G90\n";
    ss << "G17\n";

    // Tool definition and spindle start
    ss << "T20.20 M06\n";
    ss << "S" << (int)params.spindleSpeed << " M03\n";
    ss << "(UAO,2)\n\n";

    int sheetCounter = 1;
    for (const auto& sheet : sheets) {
        ss << ";KONTUR PLYTY #" << sheetCounter << " (" << (int)sheet.width << "x" << (int)sheet.height << ")\n";

        if (sheet.placedComponents.empty()) {
            ss << "; (Pusta plyta)\n\n";
            sheetCounter++;
            continue;
        }

        for (const auto& comp : sheet.placedComponents) {
            ss << ";DETAL: " << comp.name << " POS: (" << comp.posX << "," << comp.posY << ") OBR: " << comp.rotationAngle << "\n";

            // Define loop sequence: cut holes first (0 to N-1), then outer contour (-1)
            std::vector<int> loopIndices;
            for (size_t h = 0; h < comp.innerContours.size(); ++h) {
                loopIndices.push_back(static_cast<int>(h));
            }
            loopIndices.push_back(-1); // Outer contour

            for (int loopIdx : loopIndices) {
                // Gather entities in this loop group
                std::vector<GeoEntity> group;
                for (const auto& ent : comp.geometry) {
                    if (getEntityLoopIndex(ent, comp) == loopIdx) {
                        group.push_back(ent);
                    }
                }

                if (group.empty()) continue;

                // Chain entities to minimize tool lifts and guarantee continuous cuts
                std::vector<GeoEntity> chained = chainEntities(group);

                // Multi-pass calculations
                double targetDepth = -std::abs(params.cutDepth);
                double maxStep = std::abs(params.maxPassDepth);
                if (maxStep < 0.1) maxStep = 3.0;
                int passesCount = static_cast<int>(std::ceil(std::abs(targetDepth) / maxStep));
                if (passesCount < 1) passesCount = 1;

                // Loop over Z passes
                for (int pass = 1; pass <= passesCount; ++pass) {
                    double currentZ = (targetDepth / passesCount) * pass;
                    ss << ";  --- PASS #" << pass << " AT Z = " << currentZ << " ---\n";

                    Point currentPos;
                    bool isPlunged = false;
                    double accumLength = 0.0; // Track accumulated cutting length for tabs

                    for (const auto& geo : chained) {
                        // Determine the Z depth for this specific segment (partial depth support)
                        double passZ = currentZ;
                        if (geo.isPartialDepth) {
                            double limitZ = -std::abs(geo.customDepth);
                            if (passZ < limitZ) {
                                passZ = limitZ; // clamp to custom depth
                            }
                        }

                        if (geo.type == GeoEntity::LINE) {
                            auto p1 = comp.localToGlobal(geo.x1, geo.y1);
                            auto p2 = comp.localToGlobal(geo.x2, geo.y2);

                            Point startPt(p1.first, p1.second);
                            Point endPt(p2.first, p2.second);

                            double segLen = std::sqrt((endPt.x - startPt.x)*(endPt.x - startPt.x) + (endPt.y - startPt.y)*(endPt.y - startPt.y));

                            if (isPlunged && std::abs(currentPos.x - startPt.x) < 0.1 && std::abs(currentPos.y - startPt.y) < 0.1) {
                                // Continuous path cutting without lift
                                // Tab (bridge) calculation on outer contour
                                if (loopIdx == -1 && params.useTabs && (accumLength + segLen > params.tabInterval)) {
                                    double remainingToTab = params.tabInterval - accumLength;
                                    double t1 = remainingToTab / segLen;
                                    double t2 = (remainingToTab + params.tabLength) / segLen;
                                    if (t2 > 1.0) t2 = 1.0;

                                    Point tabStart(startPt.x + t1*(endPt.x - startPt.x), startPt.y + t1*(endPt.y - startPt.y));
                                    Point tabEnd(startPt.x + t2*(endPt.x - startPt.x), startPt.y + t2*(endPt.y - startPt.y));

                                    // Cut up to tab start
                                    ss << "G01 X" << tabStart.x << " Y" << tabStart.y << " Z" << passZ << " F" << (int)params.cuttingFeed << ".\n";

                                    // Raise Z for technological tab
                                    double tabZ = passZ + params.tabHeight;
                                    if (tabZ > 0.0) tabZ = 0.0;
                                    ss << "G01 X" << tabStart.x << " Y" << tabStart.y << " Z" << tabZ << " F" << (int)params.plungeFeed << ".\n";
                                    ss << "G01 X" << tabEnd.x << " Y" << tabEnd.y << " Z" << tabZ << " F" << (int)params.cuttingFeed << ".\n";

                                    // Plunge back
                                    ss << "G01 X" << tabEnd.x << " Y" << tabEnd.y << " Z" << passZ << " F" << (int)params.plungeFeed << ".\n";
                                    ss << "G01 X" << endPt.x << " Y" << endPt.y << " Z" << passZ << " F" << (int)params.cuttingFeed << ".\n";

                                    accumLength = segLen - (remainingToTab + params.tabLength);
                                    if (accumLength < 0.0) accumLength = 0.0;
                                } else {
                                    ss << "G01 X" << endPt.x << " Y" << endPt.y << " Z" << passZ << " F" << (int)params.cuttingFeed << ".\n";
                                    accumLength += segLen;
                                }
                            } else {
                                if (isPlunged) {
                                    ss << "G40\n"; // cancel radius compensation on retract
                                    ss << "G01 X" << currentPos.x << " Y" << currentPos.y << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                                    isPlunged = false;
                                }
                                ss << "G00 X" << startPt.x << " Y" << startPt.y << " Z" << params.safeZ << "\n";

                                // Radius compensation activation
                                if (params.useRadiusComp) {
                                    if (loopIdx == -1) {
                                        ss << "G41\n"; // Outer left
                                    } else {
                                        ss << "G42\n"; // Inner right
                                    }
                                }

                                ss << "G94 G01 X" << startPt.x << " Y" << startPt.y << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                                ss << "G01 X" << startPt.x << " Y" << startPt.y << " Z" << passZ << " F" << (int)params.plungeFeed << ".\n";
                                ss << "G01 X" << endPt.x << " Y" << endPt.y << " Z" << passZ << " F" << (int)params.cuttingFeed << ".\n";
                                isPlunged = true;
                                accumLength += segLen;
                            }
                            currentPos = endPt;
                        } else if (geo.type == GeoEntity::CIRCLE) {
                            if (isPlunged) {
                                ss << "G40\n";
                                ss << "G01 X" << currentPos.x << " Y" << currentPos.y << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                                isPlunged = false;
                            }
                            auto centerPt = comp.localToGlobal(geo.x1, geo.y1);
                            double r = geo.radius;
                            double startX = centerPt.first + r;
                            double startY = centerPt.second;

                            ss << "G00 X" << startX << " Y" << startY << " Z" << params.safeZ << "\n";

                            if (params.useRadiusComp) {
                                ss << "G42\n"; // Circle entities are holes in our system
                            }

                            ss << "G94 G01 X" << startX << " Y" << startY << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                            ss << "G01 X" << startX << " Y" << startY << " Z" << passZ << " F" << (int)params.plungeFeed << ".\n";
                            ss << "G02 X" << startX << " Y" << startY << " I" << -r << " J0.000 F" << (int)params.cuttingFeed << ".\n";
                            ss << "G40\n"; // cancel after loop
                            ss << "G01 X" << startX << " Y" << startY << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                            currentPos = Point(startX, startY);
                        } else if (geo.type == GeoEntity::ARC) {
                            double r = geo.radius;
                            double sa_rad = geo.start_angle * M_PI / 180.0;
                            double ea_rad = geo.end_angle * M_PI / 180.0;

                            double localStartX = geo.x1 + r * std::cos(sa_rad);
                            double localStartY = geo.y1 + r * std::sin(sa_rad);
                            double localEndX = geo.x1 + r * std::cos(ea_rad);
                            double localEndY = geo.y1 + r * std::sin(ea_rad);

                            auto startPt = comp.localToGlobal(localStartX, localStartY);
                            auto endPt = comp.localToGlobal(localEndX, localEndY);
                            auto globalCenterPt = comp.localToGlobal(geo.x1, geo.y1);

                            double i_offset = globalCenterPt.first - startPt.first;
                            double j_offset = globalCenterPt.second - startPt.second;

                            bool actualCCW = geo.ccw;

                            if (isPlunged && std::abs(currentPos.x - startPt.first) < 0.1 && std::abs(currentPos.y - startPt.second) < 0.1) {
                                ss << (actualCCW ? "G03" : "G02") << " X" << endPt.first << " Y" << endPt.second
                                   << " I" << i_offset << " J" << j_offset << " F" << (int)params.cuttingFeed << ".\n";
                            } else {
                                if (isPlunged) {
                                    ss << "G40\n";
                                    ss << "G01 X" << currentPos.x << " Y" << currentPos.y << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                                    isPlunged = false;
                                }
                                ss << "G00 X" << startPt.first << " Y" << startPt.second << " Z" << params.safeZ << "\n";

                                if (params.useRadiusComp) {
                                    if (loopIdx == -1) ss << "G41\n"; else ss << "G42\n";
                                }

                                ss << "G94 G01 X" << startPt.first << " Y" << startPt.second << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                                ss << "G01 X" << startPt.first << " Y" << startPt.second << " Z" << passZ << " F" << (int)params.plungeFeed << ".\n";
                                ss << (actualCCW ? "G03" : "G02") << " X" << endPt.first << " Y" << endPt.second
                                   << " I" << i_offset << " J" << j_offset << " F" << (int)params.cuttingFeed << ".\n";
                                isPlunged = true;
                            }
                            currentPos = Point(endPt.first, endPt.second);
                        } else if (geo.type == GeoEntity::POLYLINE) {
                            if (!geo.points.empty()) {
                                auto p0 = comp.localToGlobal(geo.points[0].x, geo.points[0].y);
                                Point startPt(p0.first, p0.second);

                                if (isPlunged && std::abs(currentPos.x - startPt.x) < 0.1 && std::abs(currentPos.y - startPt.y) < 0.1) {
                                    // Already at start of polyline, keep plunged
                                } else {
                                    if (isPlunged) {
                                        ss << "G40\n";
                                        ss << "G01 X" << currentPos.x << " Y" << currentPos.y << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                                        isPlunged = false;
                                    }
                                    ss << "G00 X" << startPt.x << " Y" << startPt.y << " Z" << params.safeZ << "\n";

                                    if (params.useRadiusComp) {
                                        if (loopIdx == -1) ss << "G41\n"; else ss << "G42\n";
                                    }

                                    ss << "G94 G01 X" << startPt.x << " Y" << startPt.y << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                                    ss << "G01 X" << startPt.x << " Y" << startPt.y << " Z" << passZ << " F" << (int)params.plungeFeed << ".\n";
                                    isPlunged = true;
                                }

                                for (size_t k = 1; k < geo.points.size(); ++k) {
                                    auto pk = comp.localToGlobal(geo.points[k].x, geo.points[k].y);
                                    ss << "G01 X" << pk.first << " Y" << pk.second << " Z" << passZ << " F" << (int)params.cuttingFeed << ".\n";
                                    currentPos = Point(pk.first, pk.second);
                                }
                            }
                        }
                    }

                    if (isPlunged) {
                        ss << "G40\n"; // cancel radius compensation
                        ss << "G01 X" << currentPos.x << " Y" << currentPos.y << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                    }
                }
            }
        }
        sheetCounter++;
    }

    // G-code End commands matching Seron Osai specifications exactly
    ss << "\nM05\nM30\n";

    return ss.str();
}
