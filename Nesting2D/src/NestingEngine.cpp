#include "NestingEngine.h"
#include <algorithm>
#include <cmath>
#include <iostream>

bool NestingEngine::intersect(
    double x1, double y1, double w1, double h1,
    double x2, double y2, double w2, double h2,
    double spacing
) {
    // Check overlap with spacing buffer around both rects.
    // Box 1 boundaries including buffer spacing:
    // Left: x1 - spacing, Right: x1 + w1 + spacing, Bottom: y1 - spacing, Top: y1 + h1 + spacing
    // But a simpler way is to just expand one rectangle by the spacing:
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

    // Check boundary constraints including margins
    if (x < params.margin || (x + w) > (params.sheetWidth - params.margin)) {
        return false;
    }
    if (y < params.margin || (y + h) > (params.sheetHeight - params.margin)) {
        return false;
    }

    // Check intersection with already placed components on this sheet
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
            unit.quantity = 1; // single instance
            unit.placed = false;
            itemsToPlace.push_back(unit);
        }
    }

    // 2. Sort components by area in descending order
    std::sort(itemsToPlace.begin(), itemsToPlace.end(), [](const Component& a, const Component& b) {
        return (a.width * a.height) > (b.width * b.height);
    });

    std::vector<SheetLayout> sheets;

    // 3. Best-Fit / First-Fit Bin Packing strategy
    for (auto& item : itemsToPlace) {
        bool placedSuccessfully = false;

        // Try placing on existing sheets first
        for (size_t sheetIdx = 0; sheetIdx < sheets.size(); ++sheetIdx) {
            auto& sheet = sheets[sheetIdx];

            // Scan the sheet to find the best (or first) valid location using a bottom-left strategy
            double bestX = -1, bestY = -1;
            bool rotNeeded = false;
            double bestScore = 1e30; // Min-Y, then Min-X to achieve standard bottom-left nesting packing

            // To scan efficiently, we can check potential "corner" or anchoring points.
            // These points are combinations of (margin, margin) and coordinates of already placed items.
            std::vector<std::pair<double, double>> scanPoints;
            scanPoints.push_back(std::make_pair(params.margin, params.margin));

            for (const auto& placed : sheet.placedComponents) {
                // Point top-left of placed rect
                scanPoints.push_back(std::make_pair(placed.posX, placed.posY + placed.getEffectiveHeight() + params.spacing));
                // Point bottom-right of placed rect
                scanPoints.push_back(std::make_pair(placed.posX + placed.getEffectiveWidth() + params.spacing, placed.posY));
            }

            // Test scan points
            for (const auto& pt : scanPoints) {
                double tx = pt.first;
                double ty = pt.second;

                // Test unrotated orientation
                item.rotated = false;
                if (canPlaceComponent(item, tx, ty, sheet.placedComponents, params)) {
                    // Score = Y + X (bottom-left)
                    double score = ty * 10.0 + tx;
                    if (score < bestScore) {
                        bestScore = score;
                        bestX = tx;
                        bestY = ty;
                        rotNeeded = false;
                    }
                }

                // Test rotated orientation (if permitted)
                if (params.allowRotation) {
                    item.rotated = true;
                    if (canPlaceComponent(item, tx, ty, sheet.placedComponents, params)) {
                        double score = ty * 10.0 + tx;
                        if (score < bestScore) {
                            bestScore = score;
                            bestX = tx;
                            bestY = ty;
                            rotNeeded = true;
                        }
                    }
                }
            }

            if (bestX >= 0) {
                // Found a valid position on this sheet!
                item.placed = true;
                item.posX = bestX;
                item.posY = bestY;
                item.rotated = rotNeeded;
                item.sheetIndex = static_cast<int>(sheetIdx);
                sheet.placedComponents.push_back(item);
                placedSuccessfully = true;
                break;
            }
        }

        // If we couldn't place on existing sheets, create a new sheet
        if (!placedSuccessfully) {
            SheetLayout newSheet;
            newSheet.width = params.sheetWidth;
            newSheet.height = params.sheetHeight;
            newSheet.materialUtilization = 0;

            // Try to place at the very bottom-left: (margin, margin)
            item.rotated = false;
            bool fitsUnrotated = canPlaceComponent(item, params.margin, params.margin, newSheet.placedComponents, params);

            bool fitsRotated = false;
            if (params.allowRotation) {
                item.rotated = true;
                fitsRotated = canPlaceComponent(item, params.margin, params.margin, newSheet.placedComponents, params);
            }

            if (fitsUnrotated || fitsRotated) {
                if (fitsUnrotated && fitsRotated) {
                    // Pick the orientation with lesser height to keep the packing dense at the bottom
                    if (item.width <= item.height) {
                        item.rotated = false;
                    } else {
                        item.rotated = true;
                    }
                } else if (fitsUnrotated) {
                    item.rotated = false;
                } else {
                    item.rotated = true;
                }

                item.placed = true;
                item.posX = params.margin;
                item.posY = params.margin;
                item.sheetIndex = static_cast<int>(sheets.size());
                newSheet.placedComponents.push_back(item);
                sheets.push_back(newSheet);
            } else {
                // If the single component cannot fit even a blank sheet, we leave it unplaced
                std::cerr << "Warning: Component '" << item.name << "' is too large for sheet size "
                          << params.sheetWidth << "x" << params.sheetHeight << " (including margins).\n";
            }
        }
    }

    // 4. Calculate material utilization statistics for each sheet
    double sheetTotalArea = params.sheetWidth * params.sheetHeight;
    for (auto& sheet : sheets) {
        double partsArea = 0;
        for (const auto& c : sheet.placedComponents) {
            partsArea += (c.width * c.height); // original area
        }
        sheet.materialUtilization = (partsArea / sheetTotalArea) * 100.0;
    }

    return sheets;
}
