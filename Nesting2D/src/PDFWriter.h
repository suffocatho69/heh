#ifndef PDFWRITER_H
#define PDFWRITER_H

#include <string>
#include <vector>
#include "NestingEngine.h"
#include "NCGenerator.h"

class PDFWriter {
public:
    // Standalone lightweight PDF generator. Assembles standard-compliant PDF document from scratch.
    static bool generatePDFReport(
        const std::string& filepath,
        const std::vector<SheetLayout>& sheets,
        const NestingParams& params,
        const NCParams& nc,
        const std::string& toolName,
        double lastNestingTime
    );
};

#endif // PDFWRITER_H
