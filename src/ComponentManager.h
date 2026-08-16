#ifndef COMPONENTMANAGER_H
#define COMPONENTMANAGER_H

#include <vector>
#include "Component.h"
#include "NestingEngine.h" // Needed for SheetLayout declaration

struct NestingStats {
    double materialUtilization = 0; // Average utilization %
    int totalSheets = 0;
    int uniqueLayouts = 0;
    int totalPlacedCount = 0;
    int totalUnplacedCount = 0;
};

class ComponentManager {
public:
    ComponentManager();

    // Add components manually or via DXF
    void addComponent(const Component& comp);
    void removeComponent(int index);
    void clearAllComponents();

    // Accessors
    const std::vector<Component>& getComponents() const;
    std::vector<Component>& getWritableComponents();

    // Calculates summary statistics based on current sheets
    static NestingStats calculateStats(const std::vector<SheetLayout>& sheets, int originalCount);

private:
    std::vector<Component> components;
};

#endif // COMPONENTMANAGER_H
