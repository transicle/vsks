#include <iostream>
#include <string>

#include "io.hh"

namespace VSKS
{

// more clanker shlop gahahahahaha
std::string json_escape(
    const std::string& string
) {
    std::string result;

    for (char character : string)
    {
        switch (character)
        {
            case '"':
            {
                result += "\\\"";
                break;
            }

            case '\\':
            {
                result += "\\\\";
                break;
            }

            case '\n':
            {
                result += "\\n";
                break;
            }

            case '\r':
            {
                result += "\\r";
                break;
            }

            case '\t':
            {
                result += "\\t";
                break;
            }

            default:
            {
                result += character;
                break;
            }
        }
    }

    return result;
}

void print_list(
    const std::vector<std::string>& items,
    bool json
) {
    if (json)
    {
        std::cout << "[";

        for (size_t i = 0; i < items.size(); ++i)
        {
            if (i)
            {
                std::cout << ",";
            }

            std::cout
                << "\""
                << json_escape(items[i])
                << "\"";
        }

        std::cout << "]\n";
    }

    else
    {
        for (const auto& item : items)
        {
            std::cout << item << "\n";
        }
    }
}

void print_secrets(
    const std::vector<Secret>& secrets,
    bool json
) {
    if (json)
    {
        std::cout << "[";

        for (size_t i = 0; i < secrets.size(); ++i)
        {
            if (i)
            {
                std::cout << ",";
            }

            const auto& secret = secrets[i];

            std::cout
                << "{\"extensionId\":\""
                << json_escape(secret.extension_id)
                << "\",\"key\":\""
                << json_escape(secret.key)
                << "\",\"value\":\""
                << json_escape(secret.value)
                << "\"}";
        }

        std::cout << "]\n";
    }

    else
    {
        for (const auto& secret : secrets)
        {
            std::cout
                << secret.extension_id
                << "  "
                << secret.key
                << "\n"
                << "  "
                << secret.value
                << "\n";
        }
    }
}

void print_value(
    const std::string& value,
    bool json
) {
    if (json)
    {
        std::cout
            << "{\"value\":\""
            << json_escape(value)
            << "\"}\n";
    }

    else
    {
        std::cout << value << "\n";
    }
}

bool confirm(
    const std::string& prompt,
    bool yes_flag
) {
    if (yes_flag)
    {
        return true;
    }

    std::cerr
        << prompt
        << " [y/N] ";

    std::string line;

    if (!std::getline(std::cin, line))
    {
        return false;
    }

    return
        !line.empty()
        && (
            line[0] == 'y'
            || line[0] == 'Y'
        );
}

}