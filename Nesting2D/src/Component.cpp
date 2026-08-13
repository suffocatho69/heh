#include "Component.h"
#include <cmath>

Component::Component()
    : name(""), width(0), height(0), quantity(0), placed(false), posX(0), posY(0), rotated(false), sheetIndex(-1) {
}

Component::Component(const std::string& name, double w, double h, int qty)
    : name(name), width(w), height(h), quantity(qty), placed(false), posX(0), posY(0), rotated(false), sheetIndex(-1) {
    generateDefaultRectangleGeometry();
}

void Component::generateDefaultRectangleGeometry() {
    geometry.clear();

    // Add four lines enclosing the rectangle [0, 0] to [width, height]
    GeoEntity top, right, bottom, left;

    top.type = GeoEntity::LINE;
    top.x1 = 0; top.y1 = height;
    top.x2 = width; top.y2 = height;

    right.type = GeoEntity::LINE;
    right.x1 = width; right.y1 = height;
    right.x2 = width; right.y2 = 0;

    bottom.type = GeoEntity::LINE;
    bottom.x1 = width; bottom.y1 = 0;
    bottom.x2 = 0; bottom.y2 = 0;

    left.type = GeoEntity::LINE;
    left.x1 = 0; left.y1 = 0;
    left.x2 = 0; left.y2 = height;

    geometry.push_back(top);
    geometry.push_back(right);
    geometry.push_back(bottom);
    geometry.push_back(left);
}

double Component::getEffectiveWidth() const {
    return rotated ? height : width;
}

double Component::getEffectiveHeight() const {
    return rotated ? width : height;
}

std::pair<double, double> Component::localToGlobal(double lx, double ly) const {
    double gx = posX;
    double gy = posY;

    if (rotated) {
        // Rotated 90 degrees counter-clockwise:
        // New width is old height, new height is old width.
        // Rotation about local origin (0,0) and then translated to (posX, posY).
        // Standard 2D 90 deg rotation of point (lx, ly):
        // x' = -ly, y' = lx. Since we want to keep inside the positive bounding box of (height, width):
        // x_trans = height - ly
        // y_trans = lx
        gx += (height - ly);
        gy += lx;
    } else {
        gx += lx;
        gy += ly;
    }

    return std::make_pair(gx, gy);
}
