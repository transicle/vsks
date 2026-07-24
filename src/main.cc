#include <iostream>

#include "cli.hh"

int main(
    int argc,
    char** argv
)
{
    try
    {
        auto options = VSKS::parse_args(
            argc,
            argv
        );

        return VSKS::run_cli(
            options
        );
    }

    catch (const std::exception& exception)
    {
        std::cerr
            << "VSKS: failure: "
            << exception.what()
            << "\n";

        return 1;
    }
}