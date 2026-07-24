#include <cstdlib>
#include <filesystem>

#include "paths.hh"

using namespace VSKS;

static std::string home()
{
    const char* home = std::getenv("HOME");

    return home ? home : "/root";
}

std::vector<VSCodeInstallation> VSKS::discover_installs()
{
    std::vector<VSCodeInstallation> result;

    struct Candidate
    {
        std::string label;
        std::string config_rel;
        std::string app_name;
    };

    // clanker gemenated vector (Pls dont hararss me)
    std::vector<Candidate> candidates = {
        {
            "VSCode (deb/rpm)",
            ".config/Code",
            "Code Safe Storage"
        },
        {
            "VSCode (snap)",
            "snap/code/current/.config/Code",
            "Code Safe Storage"
        },
        {
            "VSCode (flatpak)",
            ".var/app/com.visualstudio.code/config/Code",
            "Code Safe Storage"
        },
        {
            "VSCode Insiders (deb/rpm)",
            ".config/Code - Insiders",
            "Code - Insiders Safe Storage"
        },
        {
            "VSCode Insiders (snap)",
            "snap/code-insiders/current/.config/Code - Insiders",
            "Code - Insiders Safe Storage"
        },
        {
            "VSCode Insiders (flatpak)",
            ".var/app/com.visualstudio.code-insiders/config/Code - Insiders",
            "Code - Insiders Safe Storage"
        },
        {
            "VSCodium",
            ".config/VSCodium",
            "VSCodium Safe Storage"
        },
        {
            "VSCodium (flatpak)",
            ".var/app/com.vscodium.codium/config/VSCodium",
            "VSCodium Safe Storage"
        }
    };

    for (const auto& candidate : candidates)
    {
        std::filesystem::path config_dir = std::filesystem::path(home()) / candidate.config_rel;
        std::filesystem::path database = config_dir / "User" / "globalStorage" / "state.vscdb";

        if (std::filesystem::exists(database))
        {
            result.push_back(
                {
                    candidate.label,
                    database,
                    config_dir / "User" / "globalStorage",
                    candidate.app_name
                }
            );
        }
    }

    return result;
}

std::optional<VSCodeInstallation> VSKS::best_install()
{
    auto installs = discover_installs();

    if (installs.empty())
    {
        return std::nullopt;
    }

    return installs[0];
}