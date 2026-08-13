#ifndef NCGENERATOR_H
#define NCGENERATOR_H

#include <string>
#include <vector>
#include "NestingEngine.h"

struct NCParams {
    double spindleSpeed = 12000.0; // S command (RPM)
    double cuttingFeed = 1500.0;   // F command (mm/min) for normal cuts
    double plungeFeed = 500.0;     // F command for plunging (Z axis movement)
    double safeZ = 10.0;           // Safe height for rapid travel (G00)
    double cutDepth = -2.0;        // Cutting depth (negative value, Z axis)
};

class NCGenerator {
public:
    // Generates CNC ISO G-code (standard RS-274 dialect) for cutting
    // the nested shapes layout from one or multiple sheet layouts.
    // Returns the G-code content as a string.
    static std::string generateGCode(
        const std::vector<SheetLayout>& sheets,
        const NCParams& params
    );

private:
    // Formats a line of G-code with double values rounded cleanly
    static std::string formatGCodeLine(const std::string& cmd, double x, double y, double z, double feed = 0);
};

#endif // NCGENERATOR_H
