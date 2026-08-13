#include <iostream>
#include <cassert>
#include <string>
#include "src/Component.h"
#include "src/DXFReader.h"
#include "src/NestingEngine.h"
#include "src/ComponentManager.h"
#include "src/NCGenerator.h"

void runComponentTests() {
    std::cout << "[TEST] Running Component class tests...\n";

    // Create an unrotated component
    Component comp1("PartA", 100.0, 50.0, 2);
    assert(comp1.name == "PartA");
    assert(comp1.width == 100.0);
    assert(comp1.height == 50.0);
    assert(comp1.quantity == 2);
    assert(comp1.getEffectiveWidth() == 100.0);
    assert(comp1.getEffectiveHeight() == 50.0);

    // Translate coordinates
    comp1.posX = 10.0;
    comp1.posY = 20.0;
    auto globalPt = comp1.localToGlobal(10.0, 5.0);
    assert(globalPt.first == 20.0);
    assert(globalPt.second == 25.0);

    // Test with rotation
    comp1.rotated = true;
    assert(comp1.getEffectiveWidth() == 50.0);
    assert(comp1.getEffectiveHeight() == 100.0);

    // rotated: height (50.0) - ly, posX + height - ly, posY + lx
    globalPt = comp1.localToGlobal(10.0, 5.0); // lx=10, ly=5
    // height - ly = 50 - 5 = 45; posX + 45 = 10 + 45 = 55
    // posY + lx = 20 + 10 = 30
    assert(globalPt.first == 55.0);
    assert(globalPt.second == 30.0);

    std::cout << "[TEST] Component tests passed successfully!\n";
}

void runDXFReaderTests() {
    std::cout << "[TEST] Running DXFReader mock tests...\n";

    // Standard simplified mock DXF content representing a square from (10, 10) to (110, 110)
    std::string mockDXF =
        "0\nSECTION\n"
        "2\nENTITIES\n"
        "0\nLINE\n"
        "10\n10.0\n20\n10.0\n11\n110.0\n21\n10.0\n"
        "0\nLINE\n"
        "10\n110.0\n20\n10.0\n11\n110.0\n21\n110.0\n"
        "0\nLINE\n"
        "10\n110.0\n20\n110.0\n11\n10.0\n21\n110.0\n"
        "0\nLINE\n"
        "10\n10.0\n20\n110.0\n11\n10.0\n21\n10.0\n"
        "0\nEOF\n";

    Component comp;
    bool success = DXFReader::loadDXFFromString(mockDXF, comp);
    assert(success);
    assert(comp.width == 100.0);
    assert(comp.height == 100.0);
    assert(comp.geometry.size() == 4);

    // First line should be mapped from [0, 0] to [100, 0] since [10, 10] was minimum bounds
    assert(comp.geometry[0].x1 == 0.0);
    assert(comp.geometry[0].y1 == 0.0);
    assert(comp.geometry[0].x2 == 100.0);
    assert(comp.geometry[0].y2 == 0.0);

    std::cout << "[TEST] DXFReader tests passed successfully!\n";
}

void runNestingEngineAndNCGenTests() {
    std::cout << "[TEST] Running NestingEngine & NCGenerator tests...\n";

    ComponentManager manager;
    // Add multiple parts
    manager.addComponent(Component("Part_Large", 400.0, 300.0, 4)); // total 4 units
    manager.addComponent(Component("Part_Medium", 200.0, 150.0, 6)); // total 6 units
    manager.addComponent(Component("Part_Small", 80.0, 80.0, 10)); // total 10 units

    NestingParams params;
    params.sheetWidth = 1000.0;
    params.sheetHeight = 600.0;
    params.margin = 15.0;
    params.spacing = 10.0;
    params.allowRot0 = true;
    params.allowRot90 = true;
    params.allowRot180 = false;
    params.allowRot270 = false;

    // Run nesting
    auto sheets = NestingEngine::performNesting(manager.getComponents(), params);

    assert(!sheets.empty());
    std::cout << "[INFO] Nesting complete. Total Sheets used: " << sheets.size() << "\n";

    // Print stats
    int totalUnitsExpected = 4 + 6 + 10;
    auto stats = ComponentManager::calculateStats(sheets, totalUnitsExpected);
    std::cout << "[INFO] Material Utilization: " << stats.materialUtilization << "%\n";
    std::cout << "[INFO] Unique Layouts: " << stats.uniqueLayouts << "\n";
    std::cout << "[INFO] Total Placed Count: " << stats.totalPlacedCount << "\n";
    std::cout << "[INFO] Total Unplaced Count: " << stats.totalUnplacedCount << "\n";

    assert(stats.totalPlacedCount + stats.totalUnplacedCount == totalUnitsExpected);

    // Verify coordinates do not exceed plate bounds
    for (size_t s = 0; s < sheets.size(); ++s) {
        const auto& sheet = sheets[s];
        std::cout << "  Sheet #" << (s + 1) << " has " << sheet.placedComponents.size()
                  << " parts, utilization = " << sheet.materialUtilization << "%\n";

        for (const auto& comp : sheet.placedComponents) {
            double ew = comp.getEffectiveWidth();
            double eh = comp.getEffectiveHeight();
            assert(comp.posX >= params.margin);
            assert(comp.posY >= params.margin);
            assert(comp.posX + ew <= params.sheetWidth - params.margin);
            assert(comp.posY + eh <= params.sheetHeight - params.margin);
        }
    }

    // Generate NC G-Code
    NCParams nc;
    nc.spindleSpeed = 15000;
    nc.cuttingFeed = 2000;
    nc.plungeFeed = 600;
    nc.safeZ = 5.0;
    nc.cutDepth = -3.5;

    std::string gcode = NCGenerator::generateGCode(sheets, nc);
    assert(!gcode.empty());
    assert(gcode.find("G21") != std::string::npos);
    assert(gcode.find("S15000") != std::string::npos);
    assert(gcode.find("F2000") != std::string::npos);
    assert(gcode.find("Z5.000") != std::string::npos);
    assert(gcode.find("Z-3.500") != std::string::npos);

    std::cout << "[TEST] Nesting and NC Generator tests passed successfully!\n";
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << "  RETRO 2D NESTING SYSTEM - UNIT TEST SUITE (C++14)\n";
    std::cout << "=========================================================\n";

    runComponentTests();
    runDXFReaderTests();
    runNestingEngineAndNCGenTests();

    std::cout << "\n[SUCCESS] ALL UNIT TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
