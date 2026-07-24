#pragma once

#include <string>
#include <vector>

#include "database.hh"

namespace VSKS
{
    void print_list(
        const std::vector<std::string>& items,
        bool json
    );

    void print_secrets(
        const std::vector<Secret>& secrets,
        bool json
    );

    void print_value(
        const std::string& value,
        bool json
    );

    // @brief asks for yes/no conf. if `yes_flag` is true, just skip it
    bool confirm(
        const std::string& prompt,
        bool yes_flag
    );

    std::string json_escape(
        const std::string& string
    );
}