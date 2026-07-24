#pragma once

#include <string>

#include "database.hh"

namespace VSKS
{
    void run_repl(
        Database& database,
        bool json_mode,
        bool quiet
    );
}