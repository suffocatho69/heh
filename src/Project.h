#ifndef PROJECT_H
#define PROJECT_H

#include <string>
#include <vector>
#include "Component.h"
#include "NestingEngine.h"
#include "NCGenerator.h"

class Project {
public:
    static bool saveProject(
        const std::string& filepath,
        const std::vector<Component>& components,
        const NestingParams& params,
        const NCParams& nc
    );

    static bool loadProject(
        const std::string& filepath,
        std::vector<Component>& outComponents,
        NestingParams& outParams,
        NCParams& outNC
    );

    static bool saveLastSettings(
        const std::string& filepath,
        const NestingParams& params,
        const NCParams& nc
    );

    static bool loadLastSettings(
        const std::string& filepath,
        NestingParams& outParams,
        NCParams& outNC
    );
};

#endif // PROJECT_H
