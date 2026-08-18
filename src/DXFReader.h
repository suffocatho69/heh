#ifndef DXFREADER_H
#define DXFREADER_H

#include <string>
#include <vector>
#include "Component.h"

class DXFReader {
public:
    // Reads a DXF file, extracts line/arc/circle entities, calculates its bounding box,
    // normalizes coordinates to [0, 0] base, and outputs a Component object.
    // Returns true on success, false on error.
    static bool loadDXF(const std::string& filepath, Component& outComponent);

    // Reads a DXF content from an in-memory string (useful for testing/simulation).
    static bool loadDXFFromString(const std::string& content, Component& outComponent);
};

#endif // DXFREADER_H
