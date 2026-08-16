#ifndef ZIPWRITER_H
#define ZIPWRITER_H

#include <string>
#include <vector>

struct ZIPFileEntry {
    std::string name;
    std::string content;
};

class ZIPWriter {
public:
    // Standalone pure-C++ standard-compliant uncompressed ZIP archiver.
    // Packages multiple files into a fully compatible .zip archive.
    static bool createZIP(
        const std::string& filepath,
        const std::vector<ZIPFileEntry>& entries
    );
};

#endif // ZIPWRITER_H
