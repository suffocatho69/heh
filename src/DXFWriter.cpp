#include "DXFWriter.h"
#include <fstream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool DXFWriter::exportSheetLayout(
    const std::string& filepath,
    const std::vector<SheetLayout>& sheets
) {
    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    // DXF Header
    out << "  0\nSECTION\n  2\nHEADER\n  0\nENDSEC\n";
    out << "  0\nSECTION\n  2\nCLASSES\n  0\nENDSEC\n";
    out << "  0\nSECTION\n  2\nTABLES\n  0\nENDSEC\n";
    out << "  0\nSECTION\n  2\nBLOCKS\n  0\nENDSEC\n";

    // ENTITIES section
    out << "  0\nSECTION\n  2\nENTITIES\n";

    int sheetIdx = 1;
    for (const auto& sheet : sheets) {
        // Draw Sheet border boundary as lines on "SHEET_BORDER" layer
        double w = sheet.width;
        double h = sheet.height;
        double x_offset = (sheetIdx - 1) * (w + 200.0); // side-by-side positioning

        auto writeLine = [&](double x1, double y1, double x2, double y2, const std::string& layer) {
            out << "  0\nLINE\n  8\n" << layer << "\n";
            out << " 10\n" << x1 << "\n 20\n" << y1 << "\n";
            out << " 11\n" << x2 << "\n 21\n" << y2 << "\n";
        };

        writeLine(x_offset, 0.0, x_offset + w, 0.0, "SHEET_BORDER");
        writeLine(x_offset + w, 0.0, x_offset + w, h, "SHEET_BORDER");
        writeLine(x_offset + w, h, x_offset, h, "SHEET_BORDER");
        writeLine(x_offset, h, x_offset, 0.0, "SHEET_BORDER");

        for (const auto& comp : sheet.placedComponents) {
            double angle = comp.rotationAngle;
            if (comp.rotated && angle == 0.0) angle = 90.0;

            // Find normalization offsets matching localToGlobal mapping
            double minLocalX = 1e30, minLocalY = 1e30;
            for (const auto& p : comp.outerContour) {
                Point rp = Component::transformPoint(p, angle, Point(0,0));
                if (rp.x < minLocalX) minLocalX = rp.x;
                if (rp.y < minLocalY) minLocalY = rp.y;
            }

            for (const auto& geo : comp.geometry) {
                if (geo.type == GeoEntity::LINE) {
                    Point p1 = Component::transformPoint(Point(geo.x1, geo.y1), angle, Point(0, 0));
                    Point p2 = Component::transformPoint(Point(geo.x2, geo.y2), angle, Point(0, 0));

                    double gx1 = x_offset + comp.posX + (p1.x - minLocalX);
                    double gy1 = comp.posY + (p1.y - minLocalY);
                    double gx2 = x_offset + comp.posX + (p2.x - minLocalX);
                    double gy2 = comp.posY + (p2.y - minLocalY);

                    writeLine(gx1, gy1, gx2, gy2, "NESTED_PARTS");
                } else if (geo.type == GeoEntity::CIRCLE) {
                    Point center = Component::transformPoint(Point(geo.x1, geo.y1), angle, Point(0, 0));
                    double gcx = x_offset + comp.posX + (center.x - minLocalX);
                    double gcy = comp.posY + (center.y - minLocalY);

                    out << "  0\nCIRCLE\n  8\nNESTED_PARTS\n";
                    out << " 10\n" << gcx << "\n 20\n" << gcy << "\n";
                    out << " 40\n" << geo.radius << "\n";
                } else if (geo.type == GeoEntity::ARC) {
                    Point center = Component::transformPoint(Point(geo.x1, geo.y1), angle, Point(0, 0));
                    double gcx = x_offset + comp.posX + (center.x - minLocalX);
                    double gcy = comp.posY + (center.y - minLocalY);

                    double sa = geo.start_angle + angle;
                    double ea = geo.end_angle + angle;

                    out << "  0\nARC\n  8\nNESTED_PARTS\n";
                    out << " 10\n" << gcx << "\n 20\n" << gcy << "\n";
                    out << " 40\n" << geo.radius << "\n";
                    out << " 50\n" << sa << "\n 51\n" << ea << "\n";
                } else if (geo.type == GeoEntity::POLYLINE || geo.type == GeoEntity::LWPOLYLINE) {
                    if (geo.points.size() >= 2) {
                        for (size_t k = 0; k < geo.points.size() - 1; ++k) {
                            Point p1 = Component::transformPoint(geo.points[k], angle, Point(0, 0));
                            Point p2 = Component::transformPoint(geo.points[k+1], angle, Point(0, 0));

                            double gx1 = x_offset + comp.posX + (p1.x - minLocalX);
                            double gy1 = comp.posY + (p1.y - minLocalY);
                            double gx2 = x_offset + comp.posX + (p2.x - minLocalX);
                            double gy2 = comp.posY + (p2.y - minLocalY);

                            writeLine(gx1, gy1, gx2, gy2, "NESTED_PARTS");
                        }
                    }
                }
            }
        }
        sheetIdx++;
    }

    out << "  0\nENDSEC\n  0\nEOF\n";
    return true;
}
