#include "PDFWriter.h"
#include "ComponentManager.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>

bool PDFWriter::generatePDFReport(
    const std::string& filepath,
    const std::vector<SheetLayout>& sheets,
    const NestingParams& params,
    const NCParams& nc,
    const std::string& toolName,
    double lastNestingTime
) {
    std::ofstream out(filepath, std::ios::binary);
    if (!out.is_open()) return false;

    // We'll track object byte offsets for the xref table
    std::vector<size_t> offsets;
    auto startObject = [&](std::ostream& s) {
        offsets.push_back(s.tellp());
        s << offsets.size() << " 0 obj\n";
    };

    std::stringstream ss;
    ss << "%PDF-1.4\n";

    // Object 1: Catalog
    startObject(ss);
    ss << "<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";

    // Object 2: Pages list (We will have 1 page in our report)
    startObject(ss);
    ss << "<< /Type /Pages /Kids [ 3 0 R ] /Count 1 >>\nendobj\n";

    // Prepare Page content stream first to calculate exact /Length
    std::stringstream contentStream;
    contentStream << "BT\n/F1 14 Tf\n18 TL\n50 800 Td\n";

    // Technical Title
    contentStream << "(=========================================================) Tj T*\n";
    contentStream << "(  RAPORT ROZKROJU NESTINGATOR3000) Tj T*\n";
    contentStream << "(=========================================================) Tj T*\n\n";

    // Plate Parameters
    std::stringstream plateLine;
    plateLine << "Parametry plyty: " << params.sheetWidth << " x " << params.sheetHeight << " mm";
    contentStream << "(" << plateLine.str() << ") Tj T*\n";

    std::stringstream marginLine;
    marginLine << "Margines: " << params.margin << " mm  |  Odstep technologiczny: " << params.spacing << " mm";
    contentStream << "(" << marginLine.str() << ") Tj T*\n";

    std::stringstream ncLine;
    ncLine << "Narzedzie: " << toolName << "  |  Posuw: " << nc.cuttingFeed << " mm/min";
    contentStream << "(" << ncLine.str() << ") Tj T*\n\n";

    // Summary statistics
    contentStream << "(STATYSTYKI ROZKROJU:) Tj T*\n";
    contentStream << "(---------------------------------------------------------) Tj T*\n";

    int totalOriginal = 0;
    for (const auto& s : sheets) {
        totalOriginal += s.placedComponents.size(); // ułożone detale
    }
    NestingStats stats = ComponentManager::calculateStats(sheets, totalOriginal);

    std::stringstream stats1;
    stats1 << "Liczba plyt ogolem: " << stats.totalSheets;
    contentStream << "(" << stats1.str() << ") Tj T*\n";

    std::stringstream stats2;
    stats2 << "Wykorzystanie materialu: " << std::fixed << std::setprecision(2) << stats.materialUtilization << " %";
    contentStream << "(" << stats2.str() << ") Tj T*\n";

    std::stringstream stats3;
    stats3 << "Ulozone detale: " << stats.totalPlacedCount << " / " << totalOriginal;
    contentStream << "(" << stats3.str() << ") Tj T*\n";

    std::stringstream stats4;
    stats4 << "Czas obliczen: " << std::fixed << std::setprecision(3) << lastNestingTime << " s";
    contentStream << "(" << stats4.str() << ") Tj T*\n\n";

    // Placed Sheets details
    contentStream << "(SZCZEGOLY ULOZENIA:) Tj T*\n";
    contentStream << "(---------------------------------------------------------) Tj T*\n";
    for (size_t s = 0; s < sheets.size() && s < 5; ++s) {
        std::stringstream sheetLine;
        sheetLine << "Plyta #" << (s + 1) << " (Wykorzystanie: " << std::fixed << std::setprecision(2) << sheets[s].materialUtilization << " %):";
        contentStream << "(" << sheetLine.str() << ") Tj T*\n";

        int count = 0;
        for (const auto& c : sheets[s].placedComponents) {
            if (count > 5) {
                contentStream << "  (... oraz kolejne detale ...) Tj T*\n";
                break;
            }
            std::stringstream compLine;
            compLine << "  - Detal: " << c.name << " na (" << (int)c.posX << ", " << (int)c.posY << ") obrot: " << c.rotationAngle << " deg";
            contentStream << "(" << compLine.str() << ") Tj T*\n";
            count++;
        }
    }

    contentStream << "ET\n";
    std::string streamData = contentStream.str();

    // Object 3: Page object
    startObject(ss);
    ss << "<< /Type /Page /Parent 2 0 R /Resources << /Font << /F1 5 0 R >> >> /MediaBox [ 0 0 595 842 ] /Contents 4 0 R >>\nendobj\n";

    // Object 4: Content stream object
    startObject(ss);
    ss << "<< /Length " << streamData.length() << " >>\nstream\n" << streamData << "endstream\nendobj\n";

    // Object 5: Font resource
    startObject(ss);
    ss << "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >>\nendobj\n";

    // xref table
    size_t xrefOffset = ss.tellp();
    ss << "xref\n0 " << (offsets.size() + 1) << "\n";
    ss << "0000000000 65535 f \n";
    for (size_t offset : offsets) {
        ss << std::setw(10) << std::setfill('0') << offset << " 00000 n \n";
    }

    // Trailer
    ss << "trailer\n<< /Size " << (offsets.size() + 1) << " /Root 1 0 R >>\n";
    ss << "startxref\n" << xrefOffset << "\n%%EOF\n";

    out << ss.str();
    out.close();
    return true;
}
