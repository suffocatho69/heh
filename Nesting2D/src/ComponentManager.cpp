#include "ComponentManager.h"
#include "NestingEngine.h"
#include <set>
#include <cmath>
#include <algorithm> // For std::sort

ComponentManager::ComponentManager() {
}

void ComponentManager::addComponent(const Component& comp) {
    components.push_back(comp);
}

void ComponentManager::removeComponent(int index) {
    if (index >= 0 && index < static_cast<int>(components.size())) {
        components.erase(components.begin() + index);
    }
}

void ComponentManager::clearAllComponents() {
    components.clear();
}

const std::vector<Component>& ComponentManager::getComponents() const {
    return components;
}

std::vector<Component>& ComponentManager::getWritableComponents() {
    return components;
}

NestingStats ComponentManager::calculateStats(const std::vector<SheetLayout>& sheets, int totalOriginalUnits) {
    NestingStats stats;
    stats.totalSheets = static_cast<int>(sheets.size());

    if (sheets.empty()) {
        stats.materialUtilization = 0.0;
        stats.uniqueLayouts = 0;
        stats.totalPlacedCount = 0;
        stats.totalUnplacedCount = totalOriginalUnits;
        return stats;
    }

    double totalUtilizationSum = 0;
    int placedCount = 0;

    // We identify "unique layouts" based on the patterns of parts (names, posX, posY, rotation) in sheets.
    // For a lightweight estimate, let's build a signature string for each sheet's packed configuration.
    std::set<std::string> uniqueSignatures;

    for (const auto& sheet : sheets) {
        totalUtilizationSum += sheet.materialUtilization;
        placedCount += static_cast<int>(sheet.placedComponents.size());

        // Generate signature
        std::string sig = "";
        // Sort sheet components briefly by position to make layout signature position-independent
        std::vector<Component> sortedPlaced = sheet.placedComponents;
        std::sort(sortedPlaced.begin(), sortedPlaced.end(), [](const Component& a, const Component& b) {
            if (a.posX != b.posX) return a.posX < b.posX;
            return a.posY < b.posY;
        });

        for (const auto& c : sortedPlaced) {
            sig += c.name + ":" + std::to_string(std::round(c.posX)) + ","
                + std::to_string(std::round(c.posY)) + ","
                + std::to_string(c.rotated) + ";";
        }
        uniqueSignatures.insert(sig);
    }

    // Recalculate average material utilization based on true Shoelace area
    double totalSheetArea = sheets.size() * sheets[0].width * sheets[0].height;
    double totalPartsRealArea = 0.0;
    for (const auto& sheet : sheets) {
        for (const auto& c : sheet.placedComponents) {
            totalPartsRealArea += c.calculateShoelaceArea();
        }
    }
    if (totalSheetArea > 0.0) {
        stats.materialUtilization = (totalPartsRealArea / totalSheetArea) * 100.0;
    } else {
        stats.materialUtilization = 0.0;
    }

    stats.uniqueLayouts = static_cast<int>(uniqueSignatures.size());
    stats.totalPlacedCount = placedCount;
    stats.totalUnplacedCount = (totalOriginalUnits > placedCount) ? (totalOriginalUnits - placedCount) : 0;

    return stats;
}
