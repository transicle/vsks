#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace VSKS
{
    struct VSCodeInstallation // tl;dr: does stuff
    {
        std::string label;
        std::filesystem::path database_path;
        std::filesystem::path state_dir;
        std::string app_name;
    };

    std::vector<VSCodeInstallation> discover_installs();

    // @brief picks the goodest installation. (first found in preference order)
    std::optional<VSCodeInstallation> best_install();
}