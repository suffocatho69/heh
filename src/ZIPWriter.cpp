#include "ZIPWriter.h"
#include <fstream>
#include <sstream>

static unsigned int calculateCRC32(const std::string& data) {
    unsigned int crc = 0xFFFFFFFF;
    for (char c : data) {
        crc ^= static_cast<unsigned char>(c);
        for (int i = 0; i < 8; ++i) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return ~crc;
}

static void writeInt16(std::ostream& out, unsigned short val) {
    char data[2];
    data[0] = static_cast<char>(val & 0xFF);
    data[1] = static_cast<char>((val >> 8) & 0xFF);
    out.write(data, 2);
}

static void writeInt32(std::ostream& out, unsigned int val) {
    char data[4];
    data[0] = static_cast<char>(val & 0xFF);
    data[1] = static_cast<char>((val >> 8) & 0xFF);
    data[2] = static_cast<char>((val >> 16) & 0xFF);
    data[3] = static_cast<char>((val >> 24) & 0xFF);
    out.write(data, 4);
}

bool ZIPWriter::createZIP(
    const std::string& filepath,
    const std::vector<ZIPFileEntry>& entries
) {
    std::ofstream out(filepath, std::ios::binary);
    if (!out.is_open()) return false;

    struct InternalMetadata {
        unsigned int crc;
        unsigned int size;
        unsigned int localHeaderOffset;
    };

    std::vector<InternalMetadata> meta;
    meta.reserve(entries.size());

    // 1. Write Local File Headers and file data
    for (const auto& entry : entries) {
        InternalMetadata m;
        m.crc = calculateCRC32(entry.content);
        m.size = static_cast<unsigned int>(entry.content.length());
        m.localHeaderOffset = static_cast<unsigned int>(out.tellp());
        meta.push_back(m);

        // Signature: 0x04034b50
        writeInt32(out, 0x04034b50);
        // Version needed: 10 (1.0)
        writeInt16(out, 10);
        // General purpose flag: 0
        writeInt16(out, 0);
        // Compression method: 0 (STORE)
        writeInt16(out, 0);
        // Last mod file time / date: 0
        writeInt16(out, 0);
        writeInt16(out, 0);
        // CRC-32
        writeInt32(out, m.crc);
        // Compressed size
        writeInt32(out, m.size);
        // Uncompressed size
        writeInt32(out, m.size);
        // File name length
        writeInt16(out, static_cast<unsigned short>(entry.name.length()));
        // Extra field length
        writeInt16(out, 0);

        // File name string
        out.write(entry.name.c_str(), entry.name.length());

        // File data bytes
        out.write(entry.content.c_str(), entry.content.length());
    }

    // 2. Write Central Directory Headers
    unsigned int centralDirOffset = static_cast<unsigned int>(out.tellp());

    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& entry = entries[i];
        const auto& m = meta[i];

        // Signature: 0x02014b50
        writeInt32(out, 0x02014b50);
        // Version made by: 10
        writeInt16(out, 10);
        // Version needed: 10
        writeInt16(out, 10);
        // General purpose flag: 0
        writeInt16(out, 0);
        // Compression method: 0
        writeInt16(out, 0);
        // Last mod file time / date: 0
        writeInt16(out, 0);
        writeInt16(out, 0);
        // CRC-32
        writeInt32(out, m.crc);
        // Compressed size
        writeInt32(out, m.size);
        // Uncompressed size
        writeInt32(out, m.size);
        // File name length
        writeInt16(out, static_cast<unsigned short>(entry.name.length()));
        // Extra field length: 0
        writeInt16(out, 0);
        // File comment length: 0
        writeInt16(out, 0);
        // Disk number start: 0
        writeInt16(out, 0);
        // Internal attributes: 0
        writeInt16(out, 0);
        // External attributes: 0
        writeInt32(out, 0);
        // Local header relative offset
        writeInt32(out, m.localHeaderOffset);

        // File name string
        out.write(entry.name.c_str(), entry.name.length());
    }

    unsigned int centralDirSize = static_cast<unsigned int>(out.tellp()) - centralDirOffset;

    // 3. Write End of Central Directory (EOCD)
    // Signature: 0x06054b50
    writeInt32(out, 0x06054b50);
    // Number of this disk: 0
    writeInt16(out, 0);
    // Disk where central directory starts: 0
    writeInt16(out, 0);
    // Number of central directory records on this disk
    writeInt16(out, static_cast<unsigned short>(entries.size()));
    // Total number of central directory records
    writeInt16(out, static_cast<unsigned short>(entries.size()));
    // Size of central directory
    writeInt32(out, centralDirSize);
    // Offset of central directory relative to start of archive
    writeInt32(out, centralDirOffset);
    // Comment length: 0
    writeInt16(out, 0);

    out.close();
    return true;
}
