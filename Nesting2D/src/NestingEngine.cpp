#include "NestingEngine.h"
#include <algorithm>
#include <cmath>
#include <iostream>

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

    for (const auto& placed : alreadyPlaced) {
        if (intersect(x, y, w, h,
                      placed.posX, placed.posY, placed.getEffectiveWidth(), placed.getEffectiveHeight(),
                      params.spacing)) {
            return false;
        }
    }

    return true;
}

std::vector<SheetLayout> NestingEngine::performNesting(
    const std::vector<Component>& originalComponents,
    const NestingParams& params
) {
    // 1. Flatten list to individual units based on their quantities
    std::vector<Component> itemsToPlace;
    for (const auto& c : originalComponents) {
        for (int i = 0; i < c.quantity; ++i) {
            Component unit = c;
            unit.quantity = 1;
            unit.placed = false;
            itemsToPlace.push_back(unit);
        }
    }

    // 2. Sort components by area in descending order
    std::sort(itemsToPlace.begin(), itemsToPlace.end(), [](const Component& a, const Component& b) {
        return (a.width * a.height) > (b.width * b.height);
    });

    std::vector<SheetLayout> sheets;

    // 3. Define allowed rotation angle list
    std::vector<int> allowedRotations;
    if (params.allowRot0) allowedRotations.push_back(0);
    if (params.allowRot90) allowedRotations.push_back(90);
    if (params.allowRot180) allowedRotations.push_back(180);
    if (params.allowRot270) allowedRotations.push_back(270);

    // Default fallback to 0 if none checked
    if (allowedRotations.empty()) {
        allowedRotations.push_back(0);
    }

    // 4. Nesting Bottom-Left Best-Fit Strategy
    for (auto& item : itemsToPlace) {
        bool placedSuccessfully = false;

        for (size_t sheetIdx = 0; sheetIdx < sheets.size(); ++sheetIdx) {
            auto& sheet = sheets[sheetIdx];

            double bestX = -1, bestY = -1;
            int bestRotation = 0;
            double bestScore = 1e30;

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
                item.rotated = (rot == 90 || rot == 270);
                if (canPlaceComponent(item, params.margin, params.margin, newSheet.placedComponents, params)) {
                    double score = rot; // prefer lesser rotation if equal
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
                item.rotated = (bestRotation == 90 || bestRotation == 270);
                item.sheetIndex = static_cast<int>(sheets.size());
                newSheet.placedComponents.push_back(item);
                sheets.push_back(newSheet);
            } else {
                std::cerr << "Warning: Component '" << item.name << "' is too large for sheet size.\n";
            }
        }
    }

    double sheetTotalArea = params.sheetWidth * params.sheetHeight;
    for (auto& sheet : sheets) {
        double partsArea = 0;
        for (const auto& c : sheet.placedComponents) {
            partsArea += (c.width * c.height);
        }
        sheet.materialUtilization = (partsArea / sheetTotalArea) * 100.0;
    }

    return sheets;
}
