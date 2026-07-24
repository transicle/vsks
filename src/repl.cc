#include <iostream>
#include <sstream>
#include <vector>

#include "io.hh"
#include "repl.hh"

namespace VSKS
{
    static std::vector<std::string> split_args(
        const std::string& line
    ) {
        std::vector<std::string> args;
        std::istringstream ss(line); // just googled this, im so confused what the fortnite this is
        std::string token;

        bool in_quote = false;
        char quote_char = 0; // counting quote chars

        std::string current_char;

        for (char c : line)
        {
            if (in_quote)
            {
                if (c == quote_char)
                {
                    in_quote = false;
                }

                else
                {
                    current_char += c;
                }
            }

            else if (c == '"' || c == '\'')
            {
                in_quote = true;
                quote_char = c;
            }

            else if (c == ' ' || c == '\t')
            {
                if (!current_char.empty())
                {
                    args.push_back(current_char);
                    current_char.clear();
                }
            }

            else
            {
                current_char += c;
            }
        }

        if (!current_char.empty())
        {
            args.push_back(current_char);
        }

        return args;
    }

    static void print_help() {
        std::cout <<
            "exit                                          -- exit\n"
            "help                                          -- show this help\n"
            "ls [EXTENSION-ID]                             -- list extensions or secrets\n"
            "extensions                                    -- list all extensions\n"
            "keys <EXTENSION-ID>                           -- list keys for extension\n"
            "search <QUERY>                                -- search extensions and keys\n"
            "exists <EXTENSION-ID> <KEY>                   -- check if secret exists\n"
            "rm <EXTENSION-ID> <KEY>                       -- remove a secret\n"
            "get <EXTENSION-ID> <KEY>                      -- get a secret value\n"
            "set <EXTENSION-ID> <KEY> <VALUE>              -- set a secret value\n"
            "copy <SRC-EXT> <SRC-KEY> <DST-EXT> <DST-KEY>  -- copy a secret\n"
            "move <SRC-EXT> <SRC-KEY> <DST-EXT> <DST-KEY>  -- move a secret\n"
            "clear <EXTENSION-ID>                          -- remove all secrets for extension\n";
    }

    void run_repl(
        Database& database,
        bool json_mode,
        bool quiet
    ) {
        if (!quiet)
        {
            std::cout << "VSKS REPL, type 'help' for commands, 'exit' to quit.\n";
        }

        std::string line;

        while (true)
        {
            if (!quiet)
            {
                std::cout << "VSKS >> ";
            }

            if (!std::getline(std::cin, line))
            {
                break;
            }

            auto args = split_args(line);
            if (args.empty())
            {
                continue;
            }

            const auto& cmd = args[0];
            if (cmd == "exit" || cmd == "quit")
            {
                break;
            }
            
            else if (cmd == "help")
            {
                print_help();
            }

            else if (cmd == "ls")
            {
                if (args.size() >= 2)
                {
                    auto secrets = database.list_secrets(args[1]);
                    print_secrets(secrets, json_mode);
                }

                else
                {
                    auto extensions = database.list_extensions();
                    print_list(extensions, json_mode);
                }
            }

            else if (cmd == "keys")
            {
                if (args.size() < 2)
                {
                    std::cerr << "Usage: keys <EXTENSION-ID>\n";
                    continue;
                }

                print_list(database.list_keys(args[1]), json_mode);
            }

            else if (cmd == "search")
            {
                if (args.size() < 2)
                {
                    std::cerr << "Usage: search <QUERY>\n";
                    continue;
                }

                print_secrets(database.search(args[1]), json_mode);
            }

            else if (cmd == "exists")
            {
                if (args.size() < 3)
                {
                    std::cerr << "Usage: exists <EXTENSION-ID> <KEY>\n";
                    continue;
                }

                bool extension = database.exists(args[1], args[2]);

                if (json_mode)
                {
                    std::cout << (extension ? "true" : "false") << "\n";
                }

                else
                {
                    std::cout << (extension ? "exists" : "not found") << "\n";
                }
            }

            else if (cmd == "get")
            {
                if (args.size() < 3)
                {
                    std::cerr << "Usage: get <EXTENSION-ID> <KEY>\n";
                    continue;
                }

                auto value = database.get(args[1], args[2]);
                if (value)
                {
                    print_value(*value, json_mode);
                }

                else
                {
                    std::cerr << "not found\n";
                }
            }

            else if (cmd == "set")
            {
                if (args.size() < 4)
                {
                    std::cerr << "Usage: set <EXTENSION-ID> <KEY> <VALUE>\n";
                    continue;
                }

                database.set(args[1], args[2], args[3]);

                if (!quiet)
                {
                    std::cout << "ok, set\n";
                }
            }

            else if (cmd == "rm")
            {
                if (args.size() < 3)
                {
                    std::cerr << "Usage: rm <EXTENSION-ID> <KEY>\n";
                    continue;
                }

                bool ok = database.remove(args[1], args[2]);

                if (!quiet)
                {
                    std::cout << (ok ? "removed" : "not found") << "\n";
                }
            }

            else if (cmd == "copy")
            {
                if (args.size() < 5)
                {
                    std::cerr << "Usage: copy <SE> <SK> <DE> <DK>\n";
                    continue;
                }

                bool ok = database.copy(args[1], args[2], args[3], args[4]);

                if (!quiet)
                {
                    std::cout << (ok ? "copied" : "source not found");
                }
            }

            else if (cmd == "move")
            {
                if (args.size() < 5)
                {
                    std::cerr << "Usage: move <SE> <SK> <DE> <DK>\n";
                    continue;
                }

                bool ok = database.move(args[1], args[2], args[3], args[4]);

                if (!quiet)
                {
                    std::cout << (ok ? "moved" : "source not found");
                }
            }

            else if (cmd == "clear")
            {
                if (args.size() < 2)
                {
                    std::cerr << "Usage: clear <EXTENSION-ID>\n";
                    continue;
                }

                int amt = database.clear(args[1]);
                if (!quiet)
                {
                    std::cout << "removed " << amt << "secret(s)\n";
                }
            }

            else
            {
                std::cerr << "unknown command: " << cmd << " (try 'help')\n";
            }
        }
    }
}