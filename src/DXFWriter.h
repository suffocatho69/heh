#ifndef DXFWRITER_H
#define DXFWRITER_H

#include <string>
#include <vector>
#include "NestingEngine.h"

class DXFWriter {
public:
    // Exports the nested plate sheet layout as a layered multi-entity CAD DXF file
    static bool exportSheetLayout(
        const std::string& filepath,
        const std::vector<SheetLayout>& sheets
    );
};

#endif // DXFWRITER_H
