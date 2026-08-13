#include "NCGenerator.h"
#include <sstream>
#include <iomanip>
#include <cmath>

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
            ss << ";DETAL: " << comp.name << " POS: (" << comp.posX << "," << comp.posY << ") OBR: " << (comp.rotated ? "90" : "0") << "\n";

            Point currentPos;
            bool isPlunged = false;

            for (const auto& geo : comp.geometry) {
                if (geo.type == GeoEntity::LINE) {
                    auto p1 = comp.localToGlobal(geo.x1, geo.y1);
                    auto p2 = comp.localToGlobal(geo.x2, geo.y2);

                    Point startPt(p1.first, p1.second);
                    Point endPt(p2.first, p2.second);

                    if (isPlunged && std::abs(currentPos.x - startPt.x) < 0.1 && std::abs(currentPos.y - startPt.y) < 0.1) {
                        // Continuous path cutting without lift
                        ss << "G01 X" << endPt.x << " Y" << endPt.y << " Z" << params.cutDepth << " F" << (int)params.cuttingFeed << ".\n";
                    } else {
                        if (isPlunged) {
                            ss << "G01 X" << currentPos.x << " Y" << currentPos.y << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                            isPlunged = false;
                        }
                        ss << "G00 X" << startPt.x << " Y" << startPt.y << " Z" << params.safeZ << "\n";
                        ss << "G94 G01 X" << startPt.x << " Y" << startPt.y << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                        ss << "G01 X" << startPt.x << " Y" << startPt.y << " Z" << params.cutDepth << " F" << (int)params.plungeFeed << ".\n";
                        ss << "G01 X" << endPt.x << " Y" << endPt.y << " Z" << params.cutDepth << " F" << (int)params.cuttingFeed << ".\n";
                        isPlunged = true;
                    }
                    currentPos = endPt;
                } else if (geo.type == GeoEntity::CIRCLE) {
                    if (isPlunged) {
                        ss << "G01 X" << currentPos.x << " Y" << currentPos.y << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                        isPlunged = false;
                    }
                    auto centerPt = comp.localToGlobal(geo.x1, geo.y1);
                    double r = geo.radius;
                    double startX = centerPt.first + r;
                    double startY = centerPt.second;

                    ss << "G00 X" << startX << " Y" << startY << " Z" << params.safeZ << "\n";
                    ss << "G94 G01 X" << startX << " Y" << startY << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                    ss << "G01 X" << startX << " Y" << startY << " Z" << params.cutDepth << " F" << (int)params.plungeFeed << ".\n";
                    ss << "G02 X" << startX << " Y" << startY << " I" << -r << " J0.000 F" << (int)params.cuttingFeed << ".\n";
                    ss << "G01 X" << startX << " Y" << startY << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                    currentPos = Point(startX, startY);
                } else if (geo.type == GeoEntity::ARC) {
                    if (isPlunged) {
                        ss << "G01 X" << currentPos.x << " Y" << currentPos.y << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                        isPlunged = false;
                    }
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

                    ss << "G00 X" << startPt.first << " Y" << startPt.second << " Z" << params.safeZ << "\n";
                    ss << "G94 G01 X" << startPt.first << " Y" << startPt.second << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                    ss << "G01 X" << startPt.first << " Y" << startPt.second << " Z" << params.cutDepth << " F" << (int)params.plungeFeed << ".\n";

                    // Arc direction dynamically outputs G02 or G03 CCW/CW
                    ss << (geo.ccw ? "G03" : "G02") << " X" << endPt.first << " Y" << endPt.second
                       << " I" << i_offset << " J" << j_offset << " F" << (int)params.cuttingFeed << ".\n";

                    ss << "G01 X" << endPt.first << " Y" << endPt.second << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
                    currentPos = Point(endPt.first, endPt.second);
                }
            }
            if (isPlunged) {
                ss << "G01 X" << currentPos.x << " Y" << currentPos.y << " Z" << params.safeZ << " F" << (int)params.cuttingFeed << ".\n";
            }
        }
        sheetCounter++;
    }

    // G-code End commands matching Seron Osai specifications exactly
    ss << "\nM05\nM30\n";

    return ss.str();
}
