#include "NCGenerator.h"
#include <sstream>
#include <iomanip>
#include <cmath>

std::string NCGenerator::generateGCode(
    const std::vector<SheetLayout>& sheets,
    const NCParams& params
) {
    std::stringstream ss;

    // Set standard double formatting
    ss << std::fixed << std::setprecision(3);

    // G-code Header
    ss << "; =========================================================\n";
    ss << "; RETRO 2D NESTING G-CODE GENERATOR\n";
    ss << "; Generated G-code for " << sheets.size() << " plates/sheets\n";
    ss << "; =========================================================\n";
    ss << "G21 ; Set units to millimeters\n";
    ss << "G90 ; Absolute coordinates positioning\n";
    ss << "G17 ; XY Plane selection\n";
    ss << "G00 Z" << params.safeZ << " ; Lift cutter to safe Z clearance height\n";
    ss << "M03 S" << (int)params.spindleSpeed << " ; Start spindle clockwise at " << (int)params.spindleSpeed << " RPM\n\n";

    int sheetCounter = 1;
    for (const auto& sheet : sheets) {
        ss << "; ---------------------------------------------------------\n";
        ss << "; PLATE / SHEET " << sheetCounter << " (Size: " << sheet.width << "x" << sheet.height << " mm)\n";
        ss << "; ---------------------------------------------------------\n";

        if (sheet.placedComponents.empty()) {
            ss << "; (This sheet has no nested components)\n\n";
            sheetCounter++;
            continue;
        }

        for (const auto& comp : sheet.placedComponents) {
            ss << "\n; Part: " << comp.name << " placed at (" << comp.posX << ", " << comp.posY
               << ") Rotated=" << (comp.rotated ? "YES" : "NO") << "\n";

            for (const auto& geo : comp.geometry) {
                if (geo.type == GeoEntity::LINE) {
                    // Translate local line endpoints to global coordinates
                    auto startPt = comp.localToGlobal(geo.x1, geo.y1);
                    auto endPt = comp.localToGlobal(geo.x2, geo.y2);

                    // 1. Rapid move to start point (above stock)
                    ss << "G00 X" << startPt.first << " Y" << startPt.second << "\n";

                    // 2. Plunge down into stock
                    ss << "G01 Z" << params.cutDepth << " F" << params.plungeFeed << "\n";

                    // 3. Cut to end point
                    ss << "G01 X" << endPt.first << " Y" << endPt.second << " F" << params.cuttingFeed << "\n";

                    // 4. Retract back to safety height
                    ss << "G00 Z" << params.safeZ << "\n";

                } else if (geo.type == GeoEntity::CIRCLE) {
                    // Circle around center (geo.x1, geo.y1)
                    auto centerPt = comp.localToGlobal(geo.x1, geo.y1);
                    double r = geo.radius;

                    // Start cut at (centerX + radius, centerY)
                    double startX = centerPt.first + r;
                    double startY = centerPt.second;

                    // 1. Rapid to start
                    ss << "G00 X" << startX << " Y" << startY << "\n";

                    // 2. Plunge depth
                    ss << "G01 Z" << params.cutDepth << " F" << params.plungeFeed << "\n";

                    // 3. Cut full circle clockwise (G02)
                    // In relative center offsets: I = -r (center lies -r along X), J = 0
                    ss << "G02 X" << startX << " Y" << startY << " I" << -r << " J0.000 F" << params.cuttingFeed << "\n";

                    // 4. Retract
                    ss << "G00 Z" << params.safeZ << "\n";

                } else if (geo.type == GeoEntity::ARC) {
                    // Arc: Center (geo.x1, geo.y1), radius, start_angle, end_angle
                    double r = geo.radius;

                    // Local start and end points on arc
                    // In a rotated component, we must also rotate the start/end angles or just rotate local positions!
                    // Let's compute local endpoints first, and then translate them globally:
                    double sa_rad = geo.start_angle * M_PI / 180.0;
                    double ea_rad = geo.end_angle * M_PI / 180.0;

                    double localStartX = geo.x1 + r * std::cos(sa_rad);
                    double localStartY = geo.y1 + r * std::sin(sa_rad);
                    double localEndX = geo.x1 + r * std::cos(ea_rad);
                    double localEndY = geo.y1 + r * std::sin(ea_rad);

                    auto startPt = comp.localToGlobal(localStartX, localStartY);
                    auto endPt = comp.localToGlobal(localEndX, localEndY);
                    auto globalCenterPt = comp.localToGlobal(geo.x1, geo.y1);

                    // Relative offsets to center
                    double i_offset = globalCenterPt.first - startPt.first;
                    double j_offset = globalCenterPt.second - startPt.second;

                    // 1. Rapid to start
                    ss << "G00 X" << startPt.first << " Y" << startPt.second << "\n";

                    // 2. Plunge down
                    ss << "G01 Z" << params.cutDepth << " F" << params.plungeFeed << "\n";

                    // 3. Cut arc. Determine direction: CCW (G03) or CW (G02)
                    ss << "G03 X" << endPt.first << " Y" << endPt.second
                       << " I" << i_offset << " J" << j_offset << " F" << params.cuttingFeed << "\n";

                    // 4. Retract
                    ss << "G00 Z" << params.safeZ << "\n";
                }
            }
        }
        sheetCounter++;
    }

    // G-code Footer
    ss << "\n; End of programs and shutdown commands\n";
    ss << "M05 ; Turn off spindle motor\n";
    ss << "G00 X0.000 Y0.000 ; Return home\n";
    ss << "M30 ; Program end and reset\n";

    return ss.str();
}
