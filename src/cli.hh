#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace VSKS
{
    struct CLIOptions
    {
        std::optional<std::filesystem::path> database_path;
        std::optional<std::filesystem::path> key_path;

        bool list = false;
        bool extensions = false;
        bool keys = false;
        bool search_cmd = false;
        bool exists_Cmd = false;
        bool rm = false;
        bool get = false;
        bool set_cmd = false;
        bool copy_cmd = false;
        bool move_cmd = false;
        bool clear_cmd = false;
        bool import_cmd = false;
        bool export_cmd = false;
        bool repl = false;
        bool version = false;
        bool help = false;
        
        // flags
        bool json = false;
        bool quiet = false;
        bool yes = false;

        std::string arg1, arg2, arg3, arg4;
    };

    CLIOptions parse_args(
        int argc,
        char** argv
    );

    int run_cli(
        const CLIOptions& options
    );
}