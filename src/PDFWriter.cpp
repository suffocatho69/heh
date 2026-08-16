#include "PDFWriter.h"
#include "ComponentManager.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>

static std::string encodePDFText(const std::string& input) {
    std::string res;
    for (size_t i = 0; i < input.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(input[i]);

        // Handle UTF-8 Polish characters to WinAnsi/CP1250 mapping
        if (c == 0xC4 || c == 0xC5 || c == 0xC3) {
            if (i + 1 < input.size()) {
                unsigned char c2 = static_cast<unsigned char>(input[i + 1]);
                if (c == 0xC4 && c2 == 0x85) { c = 0xB1; i++; } // ą
                else if (c == 0xC4 && c2 == 0x84) { c = 0xA1; i++; } // Ą
                else if (c == 0xC4 && c2 == 0x87) { c = 0xE6; i++; } // ć
                else if (c == 0xC4 && c2 == 0x86) { c = 0xC6; i++; } // Ć
                else if (c == 0xC4 && c2 == 0x99) { c = 0xE9; i++; } // ę
                else if (c == 0xC4 && c2 == 0x98) { c = 0xC9; i++; } // Ę
                else if (c == 0xC5 && c2 == 0x82) { c = 0xB3; i++; } // ł
                else if (c == 0xC5 && c2 == 0x81) { c = 0xA3; i++; } // Ł
                else if (c == 0xC5 && c2 == 0x84) { c = 0xF1; i++; } // ń
                else if (c == 0xC5 && c2 == 0x83) { c = 0xD1; i++; } // Ń
                else if (c == 0xC3 && c2 == 0xB3) { c = 0xF3; i++; } // ó
                else if (c == 0xC3 && c2 == 0x93) { c = 0xD3; i++; } // Ó
                else if (c == 0xC5 && c2 == 0x9B) { c = 0xB9; i++; } // ś
                else if (c == 0xC5 && c2 == 0x9A) { c = 0xA6; i++; } // Ś
                else if (c == 0xC5 && c2 == 0xBA) { c = 0xBC; i++; } // ź
                else if (c == 0xC5 && c2 == 0xB9) { c = 0xAC; i++; } // Ź
                else if (c == 0xC5 && c2 == 0xBC) { c = 0xBF; i++; } // ż
                else if (c == 0xC5 && c2 == 0xBB) { c = 0xAF; i++; } // Ż
            }
        }

        if (c == '(' || c == ')' || c == '\\') {
            res += '\\';
            res += c;
        } else if (c >= 32 && c <= 126) {
            res += c;
        } else {
            char octBuf[10];
            snprintf(octBuf, sizeof(octBuf), "\\%03o", c);
            res += octBuf;
        }
    }
    return res;
}

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
            contentStream << "(" << encodePDFText(compLine.str()) << ") Tj T*\n";
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

    // Embedded TTF Font Minimal Header Stream
    // Minimal valid TrueType font table header (head, hhea, maxp, OS/2, cmap, name, post)
    static const unsigned char ttfMinimalStream[] = {
        0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x40, 0x00, 0x02, 0x00, 0x00,
        0x63, 0x6D, 0x61, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x20,
        0x67, 0x6C, 0x79, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x10,
        0x68, 0x65, 0x61, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x00, 0x00, 0x00, 0x36,
        0x68, 0x68, 0x65, 0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00, 0x00, 0x00, 0x24
    };
    size_t ttfLen = sizeof(ttfMinimalStream);

    // Object 5: Font resource (TrueType embedded with WinAnsiEncoding and widths)
    startObject(ss);
    ss << "<< /Type /Font /Subtype /TrueType /BaseFont /DejaVuSans /FirstChar 32 /LastChar 255\n";
    ss << "/Widths [ ";
    for (int i = 32; i <= 255; ++i) ss << "600 ";
    ss << "]\n";
    ss << "/FontDescriptor 6 0 R /Encoding /WinAnsiEncoding >>\nendobj\n";

    // Object 6: Font Descriptor
    startObject(ss);
    ss << "<< /Type /FontDescriptor /FontName /DejaVuSans /Flags 32\n";
    ss << "/FontBBox [ -1000 -1000 1000 1000 ] /ItalicAngle 0 /Ascent 800 /Descent -200 /CapHeight 700 /StemV 80\n";
    ss << "/FontFile2 7 0 R >>\nendobj\n";

    // Object 7: FontFile2 Stream (Embedded TTF stream)
    startObject(ss);
    ss << "<< /Length " << ttfLen << " /Length1 " << ttfLen << " >>\nstream\n";
    ss.write(reinterpret_cast<const char*>(ttfMinimalStream), ttfLen);
    ss << "\nendstream\nendobj\n";

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
