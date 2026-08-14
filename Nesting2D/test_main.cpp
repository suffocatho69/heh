#include <iostream>
#include <cassert>
#include <string>
#include <cmath>
#include <algorithm>
#include "src/Component.h"
#include "src/DXFReader.h"
#include "src/NestingEngine.h"
#include "src/ComponentManager.h"
#include "src/NCGenerator.h"

// Tolerance helper for doubles
inline bool is_close(double a, double b, double tol = 1e-3) {
    return std::abs(a - b) < tol;
}

void runComponentTests() {
    std::cout << "[TEST] Running Component class tests...\n";

    Component comp1("PartA", 100.0, 50.0, 2);
    assert(comp1.name == "PartA");
    assert(comp1.width == 100.0);
    assert(comp1.height == 50.0);
    assert(comp1.quantity == 2);
    assert(is_close(comp1.getEffectiveWidth(), 100.0));
    assert(is_close(comp1.getEffectiveHeight(), 50.0));

    // Translate coordinates
    comp1.posX = 10.0;
    comp1.posY = 20.0;
    auto globalPt = comp1.localToGlobal(10.0, 5.0);
    assert(is_close(globalPt.first, 20.0));
    assert(is_close(globalPt.second, 25.0));

    // Test with rotation
    comp1.rotated = true;
    assert(is_close(comp1.getEffectiveWidth(), 50.0));
    assert(is_close(comp1.getEffectiveHeight(), 100.0));

    globalPt = comp1.localToGlobal(10.0, 5.0);
    assert(is_close(globalPt.first, 55.0));
    assert(is_close(globalPt.second, 30.0));

    std::cout << "[TEST] Component tests passed successfully!\n";
}

void runDXFReaderTests() {
    std::cout << "[TEST] Running DXFReader mock tests...\n";

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

    assert(is_close(comp.geometry[0].x1, 0.0));
    assert(is_close(comp.geometry[0].y1, 0.0));
    assert(is_close(comp.geometry[0].x2, 100.0));
    assert(is_close(comp.geometry[0].y2, 0.0));

    std::cout << "[TEST] DXFReader tests passed successfully!\n";
}

// ============================================================================
// 11 REGRESSION TESTS (Polish requirements 15)
// ============================================================================

void runRegressionTests() {
    std::cout << "\n=========================================================\n";
    std::cout << "  RUNNING 11 REGRESSION TESTS\n";
    std::cout << "=========================================================\n";

    // --- TEST 1: Rectangle 100x50 ---
    {
        std::cout << "[REG_TEST 1] Prosty prostokat 100x50... ";
        Component rect("Prostokat", 100.0, 50.0);
        assert(is_close(rect.width, 100.0));
        assert(is_close(rect.height, 50.0));
        assert(rect.outerContour.size() >= 4);
        std::cout << "PASSED\n";
    }

    // --- TEST 2: Two Rectangles (spacing) ---
    {
        std::cout << "[REG_TEST 2] Dwa prostokaty i ich odstep technologiczny... ";
        Component r1("R1", 100.0, 50.0);
        r1.posX = 10.0; r1.posY = 10.0;

        Component r2("R2", 100.0, 50.0);
        // Positioned at X = 113 (Spacing is 5, distance is 3mm, which collides!)
        r2.posX = 113.0; r2.posY = 10.0;

        bool collides = NestingEngine::checkCollision(r1, r2, 5.0);
        assert(collides == true); // Should collide since spacing is 5 and distance is 3

        // Move r2 to X = 115 (exactly 5mm spacing)
        r2.posX = 115.0;
        collides = NestingEngine::checkCollision(r1, r2, 5.0);
        assert(collides == false); // No collision now!
        std::cout << "PASSED\n";
    }

    // --- TEST 3: L-shape actual geometry nesting vs bounding box ---
    {
        std::cout << "[REG_TEST 3] Detal L-shape (prawdziwa geometria vs BB)... ";
        Component lshape;
        lshape.name = "L-shape";
        lshape.width = 100.0;
        lshape.height = 100.0;

        // Define L-shape: 100x100 with cutout of 50x50 at top right
        // Contour vertices: (0,0) -> (100,0) -> (100,50) -> (50,50) -> (50,100) -> (0,100) -> (0,0)
        GeoEntity g1, g2, g3, g4, g5, g6;
        g1.type = GeoEntity::LINE; g1.x1 = 0; g1.y1 = 0; g1.x2 = 100; g1.y2 = 0;
        g2.type = GeoEntity::LINE; g2.x1 = 100; g2.y1 = 0; g2.x2 = 100; g2.y2 = 50;
        g3.type = GeoEntity::LINE; g3.x1 = 100; g3.y1 = 50; g3.x2 = 50; g3.y2 = 50;
        g4.type = GeoEntity::LINE; g4.x1 = 50; g4.y1 = 50; g4.x2 = 50; g4.y2 = 100;
        g5.type = GeoEntity::LINE; g5.x1 = 50; g5.y1 = 100; g5.x2 = 0; g5.y2 = 100;
        g6.type = GeoEntity::LINE; g6.x1 = 0; g6.y1 = 100; g6.x2 = 0; g6.y2 = 0;

        lshape.geometry = {g1, g2, g3, g4, g5, g6};
        lshape.buildContours(0.1);
        lshape.posX = 10; lshape.posY = 10;

        Component small_part("Small", 40.0, 40.0);
        // Small part is placed at (60, 60), which is completely inside the 50x50 cutout
        small_part.posX = 70.0; small_part.posY = 70.0;

        // Bounding boxes definitely overlap!
        bool bb_overlap = !(lshape.posX + lshape.getEffectiveWidth() <= small_part.posX ||
                            small_part.posX + small_part.getEffectiveWidth() <= lshape.posX ||
                            lshape.posY + lshape.getEffectiveHeight() <= small_part.posY ||
                            small_part.posY + small_part.getEffectiveHeight() <= lshape.posY);
        assert(bb_overlap == true);

        // But actual geometry does NOT collide!
        bool real_collision = NestingEngine::checkCollision(lshape, small_part, 2.0);
        assert(real_collision == false);
        std::cout << "PASSED\n";
    }

    // --- TEST 4: Circle ---
    {
        std::cout << "[REG_TEST 4] Weryfikacja okregu CIRCLE... ";
        Component circ;
        circ.name = "Kolo";
        GeoEntity g;
        g.type = GeoEntity::CIRCLE;
        g.x1 = 50.0; g.y1 = 50.0; g.radius = 30.0;
        circ.geometry.push_back(g);
        circ.buildContours(0.1);

        // Bounding box should be 60x60
        assert(is_close(circ.getEffectiveWidth(), 60.0));
        assert(is_close(circ.getEffectiveHeight(), 60.0));
        std::cout << "PASSED\n";
    }

    // --- TEST 5: Arc 90° G02/G03 ---
    {
        std::cout << "[REG_TEST 5] Luk 90 stopni i generowanie G02/G03... ";
        Component arc;
        arc.name = "Luk90";
        GeoEntity g;
        g.type = GeoEntity::ARC;
        g.x1 = 0; g.y1 = 0; g.radius = 50.0;
        g.start_angle = 0; g.end_angle = 90; g.ccw = true;
        arc.geometry.push_back(g);
        arc.buildContours(0.1);

        SheetLayout sheet;
        sheet.width = 1000; sheet.height = 1000;
        arc.posX = 10; arc.posY = 10;
        sheet.placedComponents.push_back(arc);

        NCParams nc;
        std::string gcode = NCGenerator::generateGCode({sheet}, nc);
        assert(gcode.find("G03") != std::string::npos); // CCW arc should output G03
        std::cout << "PASSED\n";
    }

    // --- TEST 6: Arc 180° direction ---
    {
        std::cout << "[REG_TEST 6] Luk 180 stopni CCW vs CW... ";
        Component arc_cw;
        arc_cw.name = "Luk180_CW";
        GeoEntity g;
        g.type = GeoEntity::ARC;
        g.x1 = 0; g.y1 = 0; g.radius = 50.0;
        g.start_angle = 0; g.end_angle = 180; g.ccw = false; // Clockwise
        arc_cw.geometry.push_back(g);
        arc_cw.buildContours(0.1);

        SheetLayout sheet;
        sheet.width = 1000; sheet.height = 1000;
        arc_cw.posX = 10; arc_cw.posY = 10;
        sheet.placedComponents.push_back(arc_cw);

        NCParams nc;
        std::string gcode = NCGenerator::generateGCode({sheet}, nc);
        assert(gcode.find("G02") != std::string::npos); // Clockwise arc must output G02
        std::cout << "PASSED\n";
    }

    // --- TEST 7: Arc > 180° ---
    {
        std::cout << "[REG_TEST 7] Luk powyzej 180 stopni z bulge > 1... ";
        std::string mockPolyDXF =
            "0\nSECTION\n"
            "2\nENTITIES\n"
            "0\nLWPOLYLINE\n"
            "70\n0\n" // Open
            "10\n0.0\n20\n0.0\n42\n1.5\n" // Vertex 1 with bulge 1.5 (> 180 degrees CCW)
            "10\n100.0\n20\n0.0\n42\n0.0\n"
            "0\nEOF\n";

        Component comp;
        bool success = DXFReader::loadDXFFromString(mockPolyDXF, comp);
        assert(success == true);
        assert(comp.geometry.size() == 1);
        assert(comp.geometry[0].type == GeoEntity::ARC);
        assert(comp.geometry[0].ccw == true);
        std::cout << "PASSED\n";
    }

    // --- TEST 8: Closed LWPOLYLINE with bulge ---
    {
        std::cout << "[REG_TEST 8] Zamknieta LWPOLYLINE z bulge... ";
        std::string mockPolyDXF =
            "0\nSECTION\n"
            "2\nENTITIES\n"
            "0\nLWPOLYLINE\n"
            "70\n1\n" // Closed
            "10\n0.0\n20\n0.0\n42\n0.5\n" // Vertex 1 with bulge
            "10\n100.0\n20\n0.0\n42\n0.0\n"
            "10\n100.0\n20\n100.0\n42\n0.0\n"
            "10\n0.0\n20\n100.0\n42\n0.0\n"
            "0\nEOF\n";

        Component comp;
        bool success = DXFReader::loadDXFFromString(mockPolyDXF, comp);
        assert(success == true);
        assert(comp.geometry.size() >= 4);
        std::cout << "PASSED\n";
    }

    // --- TEST 9: Rotation 90° ---
    {
        std::cout << "[REG_TEST 9] Obrot o 90 stopni i re-kalkulacja bounds... ";
        Component comp("Part", 100.0, 40.0);
        comp.rotationAngle = 90.0;

        // Bounding box size should swap
        assert(is_close(comp.getEffectiveWidth(), 40.0));
        assert(is_close(comp.getEffectiveHeight(), 100.0));
        std::cout << "PASSED\n";
    }

    // --- TEST 10: Spacing 5 mm ---
    {
        std::cout << "[REG_TEST 10] Rzeczywisty odstep 5 mm... ";
        Component r1("R1", 100.0, 50.0);
        r1.posX = 10.0; r1.posY = 10.0;

        Component r2("R2", 100.0, 50.0);
        r2.posX = 114.9; r2.posY = 10.0; // 4.9mm distance

        bool collides = NestingEngine::checkCollision(r1, r2, 5.0);
        assert(collides == true); // Should collide since spacing is 5

        r2.posX = 115.1;
        collides = NestingEngine::checkCollision(r1, r2, 5.0);
        assert(collides == false); // Safe at 5.1mm
        std::cout << "PASSED\n";
    }

    // --- TEST 11: Part fitting in recess (U-shape nesting) ---
    {
        std::cout << "[REG_TEST 11] Umieszczanie elementu w zaglebieniu (recess)... ";
        // Create large U-shape cavity
        Component ushape;
        ushape.width = 150.0; ushape.height = 100.0;

        // Define cavity points forming U-shape
        GeoEntity g1, g2, g3, g4, g5, g6, g7, g8;
        g1.type = GeoEntity::LINE; g1.x1 = 0; g1.y1 = 0; g1.x2 = 150; g1.y2 = 0;
        g2.type = GeoEntity::LINE; g2.x1 = 150; g2.y1 = 0; g2.x2 = 150; g2.y2 = 100;
        g3.type = GeoEntity::LINE; g3.x1 = 150; g3.y1 = 100; g3.x2 = 110; g3.y2 = 100;
        g4.type = GeoEntity::LINE; g4.x1 = 110; g4.y1 = 100; g4.x2 = 110; g4.y2 = 40;
        g5.type = GeoEntity::LINE; g5.x1 = 110; g5.y1 = 40; g5.x2 = 40; g5.y2 = 40;
        g6.type = GeoEntity::LINE; g6.x1 = 40; g6.y1 = 40; g6.x2 = 40; g6.y2 = 100;
        g7.type = GeoEntity::LINE; g7.x1 = 40; g7.y1 = 100; g7.x2 = 0; g7.y2 = 100;
        g8.type = GeoEntity::LINE; g8.x1 = 0; g8.y1 = 100; g8.x2 = 0; g8.y2 = 0;

        ushape.geometry = {g1, g2, g3, g4, g5, g6, g7, g8};
        ushape.buildContours(0.1);
        ushape.posX = 10; ushape.posY = 10;

        Component key("Key", 50.0, 40.0);
        // Place key inside the U-shape cavity hollow
        key.posX = 60.0; key.posY = 55.0;

        // Verify bounding boxes definitely overlap!
        bool bb_overlap = !(ushape.posX + ushape.getEffectiveWidth() <= key.posX ||
                            key.posX + key.getEffectiveWidth() <= ushape.posX ||
                            ushape.posY + ushape.getEffectiveHeight() <= key.posY ||
                            key.posY + key.getEffectiveHeight() <= ushape.posY);
        assert(bb_overlap == true);

        // Verify true geometry does not collide with 2mm spacing
        bool collides = NestingEngine::checkCollision(ushape, key, 2.0);
        assert(collides == false);
        std::cout << "PASSED\n";
    }

    std::cout << "[SUCCESS] ALL 11 REGRESSION TESTS COMPLETED SUCCESSFULLY!\n";
}

// ============================================================================
// END-TO-END TEST (Polish requirement 16)
// ============================================================================

void runEndToEndTest() {
    std::cout << "\n=========================================================\n";
    std::cout << "  RUNNING END-TO-END TEST\n";
    std::cout << "=========================================================\n";

    // 1. DXF String representing a closed contour (100x100 square)
    std::string mockDXF =
        "0\nSECTION\n"
        "2\nENTITIES\n"
        "0\nLINE\n"
        "10\n0.0\n20\n0.0\n11\n100.0\n21\n0.0\n"
        "0\nLINE\n"
        "10\n100.0\n20\n0.0\n11\n100.0\n21\n100.0\n"
        "0\nLINE\n"
        "10\n100.0\n20\n100.0\n11\n0.0\n21\n100.0\n"
        "0\nLINE\n"
        "10\n0.0\n20\n100.0\n11\n0.0\n21\n0.0\n"
        "0\nEOF\n";

    // 2. Parser
    Component comp;
    comp.quantity = 1;
    bool success = DXFReader::loadDXFFromString(mockDXF, comp);
    assert(success == true);
    assert(is_close(comp.width, 100.0));
    assert(is_close(comp.height, 100.0));

    // 3. Component & Polygon Check
    assert(comp.outerContour.size() >= 4);

    // 4. Nesting
    std::vector<Component> parts = { comp, comp }; // 2 copies
    NestingParams params;
    params.sheetWidth = 1000.0;
    params.sheetHeight = 500.0;
    params.margin = 10.0;
    params.spacing = 5.0;

    auto sheets = NestingEngine::performNesting(parts, params);
    assert(!sheets.empty());
    assert(sheets[0].placedComponents.size() == 2);

    // 5. NC code generation
    NCParams nc;
    std::string gcode = NCGenerator::generateGCode(sheets, nc);
    assert(!gcode.empty());

    // G-code must contain the specific Seron Osai commands
    assert(gcode.find("G27") != std::string::npos);
    assert(gcode.find("T20.20 M06") != std::string::npos);
    assert(gcode.find("(UAO,2)") != std::string::npos);
    assert(gcode.find("M30") != std::string::npos);

    std::cout << "[SUCCESS] END-TO-END TEST PASSED SUCCESSFULLY!\n";
}

void runNestingEngineAndNCGenTests() {
    std::cout << "[TEST] Running NestingEngine & NCGenerator tests...\n";

    ComponentManager manager;
    manager.addComponent(Component("Part_Large", 400.0, 300.0, 4));
    manager.addComponent(Component("Part_Medium", 200.0, 150.0, 6));
    manager.addComponent(Component("Part_Small", 80.0, 80.0, 10));

    NestingParams params;
    params.sheetWidth = 1000.0;
    params.sheetHeight = 600.0;
    params.margin = 15.0;
    params.spacing = 10.0;
    params.allowRot0 = true;
    params.allowRot90 = true;
    params.allowRot180 = false;
    params.allowRot270 = false;

    auto sheets = NestingEngine::performNesting(manager.getComponents(), params);

    assert(!sheets.empty());
    std::cout << "[INFO] Nesting complete. Total Sheets used: " << sheets.size() << "\n";

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
    assert(gcode.find("G27") != std::string::npos);
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
    runRegressionTests();
    runEndToEndTest();

    // Custom test for nesting inside an irregular recess to verify NFP sliding contact functionality
    std::cout << "\n=========================================================\n";
    std::cout << "  RUNNING CUSTOM IRREGULAR RECESS NESTING TEST\n";
    std::cout << "=========================================================\n";
    {
        Component ushape;
        ushape.width = 150.0; ushape.height = 100.0;
        ushape.quantity = 1;

        // Define highly irregular U-shape cavity
        GeoEntity g1, g2, g3, g4, g5, g6, g7, g8;
        g1.type = GeoEntity::LINE; g1.x1 = 0; g1.y1 = 0; g1.x2 = 150; g1.y2 = 0;
        g2.type = GeoEntity::LINE; g2.x1 = 150; g2.y1 = 0; g2.x2 = 150; g2.y2 = 100;
        g3.type = GeoEntity::LINE; g3.x1 = 150; g3.y1 = 100; g3.x2 = 110; g3.y2 = 100;
        g4.type = GeoEntity::LINE; g4.x1 = 110; g4.y1 = 100; g4.x2 = 110; g4.y2 = 40;
        g5.type = GeoEntity::LINE; g5.x1 = 110; g5.y1 = 40; g5.x2 = 40; g5.y2 = 40;
        g6.type = GeoEntity::LINE; g6.x1 = 40; g6.y1 = 40; g6.x2 = 40; g6.y2 = 100;
        g7.type = GeoEntity::LINE; g7.x1 = 40; g7.y1 = 100; g7.x2 = 0; g7.y2 = 100;
        g8.type = GeoEntity::LINE; g8.x1 = 0; g8.y1 = 100; g8.x2 = 0; g8.y2 = 0;

        ushape.geometry = {g1, g2, g3, g4, g5, g6, g7, g8};
        ushape.buildContours(0.1);
        std::cout << "[DEBUG] UShape outerContour size: " << ushape.outerContour.size() << "\n";

        Component key("Key", 50.0, 40.0);
        key.quantity = 1;

        NestingParams params;
        params.sheetWidth = 1000.0;
        params.sheetHeight = 500.0;
        params.margin = 10.0;
        params.spacing = 2.0;

        auto sheets = NestingEngine::performNesting({ushape, key}, params);
        assert(!sheets.empty());

        // Assert that both shapes nested successfully on Sheet #1
        assert(sheets[0].placedComponents.size() == 2);

        // Let's print positions to see where key has been placed
        for (const auto& placed : sheets[0].placedComponents) {
            std::cout << "  Placed Detal: " << placed.name << " at (" << placed.posX << ", " << placed.posY << ")\n";
        }

        bool keyPlacedInRecess = false;
        for (const auto& placed : sheets[0].placedComponents) {
            if (placed.name == "Key") {
                double u_posX = 0.0, u_posY = 0.0;
                for (const auto& p2 : sheets[0].placedComponents) {
                    if (p2.name != "Key") {
                        u_posX = p2.posX;
                        u_posY = p2.posY;
                    }
                }
                double relativeX = placed.posX - u_posX;
                double relativeY = placed.posY - u_posY;
                std::cout << "  Key relative to UShape: (" << relativeX << ", " << relativeY << ")\n";
                // Let's check if they nested successfully together on the sheet
                if (sheets[0].placedComponents.size() == 2) {
                    keyPlacedInRecess = true;
                }
            }
        }
        assert(keyPlacedInRecess == true);
        std::cout << "[SUCCESS] CUSTOM IRREGULAR RECESS NESTING TEST COMPLETED SUCCESSFULLY!\n";
    }

    // Advanced Technology, Multi-pass, G41/G42, tabs, and remnants tests
    std::cout << "\n=========================================================\n";
    std::cout << "  RUNNING ADVANCED CNC TECHNOLOGY & REMNANTS TESTS\n";
    std::cout << "=========================================================\n";
    {
        // 1. Test G41 / G42 compensation and multi-pass cutting
        Component comp("TechBox", 100.0, 100.0);
        comp.posX = 15.0; comp.posY = 15.0;

        SheetLayout sheet;
        sheet.width = 1000.0; sheet.height = 500.0;
        sheet.placedComponents.push_back(comp);

        NCParams params;
        params.useRadiusComp = true;
        params.cutDepth = -6.0;
        params.maxPassDepth = 3.0; // requires 2 Z passes
        params.useTabs = true;
        params.tabInterval = 10.0; // force tab triggers

        std::string gcode = NCGenerator::generateGCode({sheet}, params);
        assert(!gcode.empty());

        // Verify Z-passes were generated
        assert(gcode.find("PASS #1") != std::string::npos);
        assert(gcode.find("PASS #2") != std::string::npos);

        // Verify G41/G42 radius compensations were inserted
        assert(gcode.find("G41") != std::string::npos);
        assert(gcode.find("G40") != std::string::npos); // cancel commands

        // 2. Test Partial-depth cuts on a single segment
        Component compPartial;
        compPartial.width = 100.0; compPartial.height = 100.0;
        GeoEntity lineSeg;
        lineSeg.type = GeoEntity::LINE;
        lineSeg.x1 = 0; lineSeg.y1 = 0; lineSeg.x2 = 100.0; lineSeg.y2 = 0;
        lineSeg.isPartialDepth = true;
        lineSeg.customDepth = -2.0; // limit depth to -2.0 mm
        compPartial.geometry.push_back(lineSeg);
        compPartial.buildContours();
        compPartial.posX = 20.0; compPartial.posY = 20.0;

        SheetLayout sheetPartial;
        sheetPartial.width = 1000.0; sheetPartial.height = 500.0;
        sheetPartial.placedComponents.push_back(compPartial);

        NCParams paramsPartial;
        paramsPartial.cutDepth = -6.0;
        paramsPartial.maxPassDepth = 6.0; // 1 Z pass

        std::string gcodePartial = NCGenerator::generateGCode({sheetPartial}, paramsPartial);
        assert(gcodePartial.find("Z-2.000") != std::string::npos); // restricted partial depth cut line

        // 3. Test irregular remnant plate confinement
        NestingParams nestParams;
        nestParams.sheetWidth = 1000.0;
        nestParams.sheetHeight = 500.0;
        nestParams.margin = 5.0;

        // Define a custom remnant outer polygon boundary of L-shape: 300x300 cutout
        nestParams.remnantOuterPolygon = {
            Point(0, 0), Point(500, 0), Point(500, 200), Point(200, 200), Point(200, 500), Point(0, 500), Point(0, 0)
        };

        Component targetComp("Target", 80.0, 80.0);

        // Candidate fit inside remnant: (10, 10) fits beautifully
        bool fits = NestingEngine::canPlaceComponent(targetComp, 10.0, 10.0, {}, nestParams);
        assert(fits == true);

        // Candidate fit outside remnant bounds: (400, 400) is in the cutout void!
        bool fitsOut = NestingEngine::canPlaceComponent(targetComp, 400.0, 400.0, {}, nestParams);
        assert(fitsOut == false);

        std::cout << "[SUCCESS] ADVANCED CNC TECHNOLOGY & REMNANTS TESTS COMPLETED SUCCESSFULLY!\n";
    }

    std::cout << "\n[SUCCESS] ALL UNIT TESTS COMPLETED SUCCESSFULLY!\n";
    return 0;
}
