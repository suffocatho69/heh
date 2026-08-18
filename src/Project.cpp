#include "Project.h"
#include "NumUtil.h"
#include <fstream>
#include <sstream>
#include <iostream>

static std::string trimStr(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

bool Project::saveProject(
    const std::string& filepath,
    const std::vector<Component>& components,
    const NestingParams& params,
    const NCParams& nc
) {
    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    out << "[NESTING2D_PROJECT_V1]\n";

    // Write Nesting Params
    out << "[NESTING_PARAMS]\n";
    out << "sheetWidth=" << NumUtil::formatDouble(params.sheetWidth, 2) << "\n";
    out << "sheetHeight=" << NumUtil::formatDouble(params.sheetHeight, 2) << "\n";
    out << "margin=" << NumUtil::formatDouble(params.margin, 2) << "\n";
    out << "spacing=" << NumUtil::formatDouble(params.spacing, 2) << "\n";
    out << "allowRot0=" << (params.allowRot0 ? 1 : 0) << "\n";
    out << "allowRot90=" << (params.allowRot90 ? 1 : 0) << "\n";
    out << "allowRot180=" << (params.allowRot180 ? 1 : 0) << "\n";
    out << "allowRot270=" << (params.allowRot270 ? 1 : 0) << "\n";
    out << "angleStep=" << NumUtil::formatDouble(params.angleStep, 2) << "\n";

    // Write NC Params
    out << "[NC_PARAMS]\n";
    out << "cuttingFeed=" << NumUtil::formatDouble(nc.cuttingFeed, 2) << "\n";
    out << "plungeFeed=" << NumUtil::formatDouble(nc.plungeFeed, 2) << "\n";
    out << "safeZ=" << NumUtil::formatDouble(nc.safeZ, 2) << "\n";
    out << "cutDepth=" << NumUtil::formatDouble(nc.cutDepth, 2) << "\n";
    out << "spindleSpeed=" << nc.spindleSpeed << "\n";

    // Write Components
    out << "[COMPONENTS]\n";
    out << "count=" << components.size() << "\n";
    for (size_t i = 0; i < components.size(); ++i) {
        const auto& c = components[i];
        out << "BEGIN_COMPONENT\n";
        out << "name=" << c.name << "\n";
        out << "width=" << NumUtil::formatDouble(c.width, 2) << "\n";
        out << "height=" << NumUtil::formatDouble(c.height, 2) << "\n";
        out << "quantity=" << c.quantity << "\n";

        // Outer contour
        out << "outer_size=" << c.outerContour.size() << "\n";
        for (const auto& p : c.outerContour) {
            out << NumUtil::formatDouble(p.x, 3) << " " << NumUtil::formatDouble(p.y, 3) << "\n";
        }

        // Inner contours
        out << "inner_count=" << c.innerContours.size() << "\n";
        for (const auto& inner : c.innerContours) {
            out << "inner_size=" << inner.size() << "\n";
            for (const auto& p : inner) {
                out << NumUtil::formatDouble(p.x, 3) << " " << NumUtil::formatDouble(p.y, 3) << "\n";
            }
        }

        out << "END_COMPONENT\n";
    }

    out.close();
    return true;
}

bool Project::loadProject(
    const std::string& filepath,
    std::vector<Component>& outComponents,
    NestingParams& outParams,
    NCParams& outNC
) {
    std::ifstream in(filepath);
    if (!in.is_open()) return false;

    outComponents.clear();
    std::string line;
    std::string currentSection = "";

    while (std::getline(in, line)) {
        line = trimStr(line);
        if (line.empty() || line[0] == '#') continue;

        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
            continue;
        }

        size_t eqPos = line.find('=');
        std::string key = (eqPos != std::string::npos) ? trimStr(line.substr(0, eqPos)) : "";
        std::string val = (eqPos != std::string::npos) ? trimStr(line.substr(eqPos + 1)) : "";

        if (currentSection == "NESTING_PARAMS") {
            if (key == "sheetWidth") outParams.sheetWidth = NumUtil::parseDouble(val);
            else if (key == "sheetHeight") outParams.sheetHeight = NumUtil::parseDouble(val);
            else if (key == "margin") outParams.margin = NumUtil::parseDouble(val);
            else if (key == "spacing") outParams.spacing = NumUtil::parseDouble(val);
            else if (key == "allowRot0") outParams.allowRot0 = (NumUtil::parseInt(val) != 0);
            else if (key == "allowRot90") outParams.allowRot90 = (NumUtil::parseInt(val) != 0);
            else if (key == "allowRot180") outParams.allowRot180 = (NumUtil::parseInt(val) != 0);
            else if (key == "allowRot270") outParams.allowRot270 = (NumUtil::parseInt(val) != 0);
            else if (key == "angleStep") outParams.angleStep = NumUtil::parseDouble(val);
        } else if (currentSection == "NC_PARAMS") {
            if (key == "cuttingFeed") outNC.cuttingFeed = NumUtil::parseDouble(val);
            else if (key == "plungeFeed") outNC.plungeFeed = NumUtil::parseDouble(val);
            else if (key == "safeZ") outNC.safeZ = NumUtil::parseDouble(val);
            else if (key == "cutDepth") outNC.cutDepth = NumUtil::parseDouble(val);
            else if (key == "spindleSpeed") outNC.spindleSpeed = NumUtil::parseInt(val);
        } else if (currentSection == "COMPONENTS" && line == "BEGIN_COMPONENT") {
            Component c;
            while (std::getline(in, line)) {
                line = trimStr(line);
                if (line == "END_COMPONENT") break;

                eqPos = line.find('=');
                if (eqPos != std::string::npos) {
                    key = trimStr(line.substr(0, eqPos));
                    val = trimStr(line.substr(eqPos + 1));

                    if (key == "name") c.name = val;
                    else if (key == "width") c.width = NumUtil::parseDouble(val);
                    else if (key == "height") c.height = NumUtil::parseDouble(val);
                    else if (key == "quantity") c.quantity = NumUtil::parseInt(val);
                    else if (key == "outer_size") {
                        int count = NumUtil::parseInt(val);
                        for (int i = 0; i < count; ++i) {
                            if (std::getline(in, line)) {
                                std::stringstream pss(line);
                                std::string px, py;
                                pss >> px >> py;
                                c.outerContour.push_back(Point(NumUtil::parseDouble(px), NumUtil::parseDouble(py)));
                            }
                        }
                    } else if (key == "inner_count") {
                        int innerCount = NumUtil::parseInt(val);
                        for (int ic = 0; innerCount > 0 && ic < innerCount; ++ic) {
                            if (std::getline(in, line)) {
                                line = trimStr(line);
                                size_t ieq = line.find('=');
                                if (ieq != std::string::npos) {
                                    int isize = NumUtil::parseInt(line.substr(ieq + 1));
                                    std::vector<Point> inner;
                                    for (int ip = 0; ip < isize; ++ip) {
                                        if (std::getline(in, line)) {
                                            std::stringstream pss(line);
                                            std::string px, py;
                                            pss >> px >> py;
                                            inner.push_back(Point(NumUtil::parseDouble(px), NumUtil::parseDouble(py)));
                                        }
                                    }
                                    c.innerContours.push_back(inner);
                                }
                            }
                        }
                    }
                }
            }
            outComponents.push_back(c);
        }
    }

    in.close();
    return true;
}

bool Project::saveLastSettings(
    const std::string& filepath,
    const NestingParams& params,
    const NCParams& nc
) {
    std::vector<Component> emptyComps;
    return saveProject(filepath, emptyComps, params, nc);
}

bool Project::loadLastSettings(
    const std::string& filepath,
    NestingParams& outParams,
    NCParams& outNC
) {
    std::vector<Component> dummyComps;
    return loadProject(filepath, dummyComps, outParams, outNC);
}
