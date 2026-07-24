#include <iostream>
#include <cstring>

#include "cli.hh"
#include "database.hh"
#include "paths.hh"
#include "io.hh"
#include "repl.hh"

#define VSKS_VERSION "1.0.1"

namespace VSKS
{
    static void print_help()
    {
        std::cout <<
            "VSKS: VSCode Secrets Explorer\n\n"
            "Usage:\n"
            "  vsks [OPTIONS] <COMMAND> [ARGS...]\n\n"
            "Options:\n"
            "  -d, --database <PATH>     Force path to state.vscdb\n"
            "  -k, --key <PATH>          Force path to globalStorage dir\n"
            "      --json                Output as JSON\n"
            "  -q, --quiet               Suppress non-error output\n"
            "  -y, --yes                 Skip confirmation prompts\n"
            "  -v, --version             Show version\n"
            "  -h, --help                Show this help\n\n"
            "Commands:\n"
            "  -l, --list [EXT-ID]                             List extensions or secrets\n"
            "      --extensions                                List all extensions with secrets\n"
            "      --keys <EXT-ID>                             List keys for an extension\n"
            "      --search <QUERY>                            Search extensions/keys\n"
            "      --exists <EXT-ID> <KEY>                     Check if secret exists\n"
            "  -r, --rm <EXT-ID> <KEY>                         Remove a secret\n"
            "  -g, --get <EXT-ID> <KEY>                        Get a secret value\n"
            "  -s, --set <EXT-ID> <KEY> <VALUE>                Set a secret value\n"
            "      --copy <SE> <SK> <DE> <DK>                  Copy a secret\n"
            "      --move <SE> <SK> <DE> <DK>                  Move a secret\n"
            "      --clear <EXT-ID>                            Remove all secrets for extension\n"
            "  -i, --import <PATH>                             Import secrets from JSON file\n"
            "  -e, --export <PATH>                             Export secrets to JSON file\n"
            "\n(no command) → start interactive REPL\n";
    }


    static std::string next_arg(
        int argc,
        char** argv,
        int& i
    ) {
        if (i + 1 < argc)
        {
            return argv[++i];
        }

        return {};
    }

    CLIOptions parse_args(
        int argc,
        char** argv
    ) {
        CLIOptions opts;

        for (int i = 1; i < argc; ++i)
        {
            std::string a = argv[i];

            if (a == "-d" || a == "--database")
            {
                opts.database_path = next_arg(argc, argv, i);
            }

            else if (a == "-k" || a == "--key")
            {
                opts.key_path = next_arg(argc, argv, i);
            }

            else if (a == "--json")
            {
                opts.json = true;
            }

            else if (a == "-q" || a == "--quiet")
            {
                opts.quiet = true;
            }

            else if (a == "-y" || a == "--yes")
            {
                opts.yes = true;
            }

            else if (a == "-v" || a == "--version")
            {
                opts.version = true;
            }

            else if (a == "-h" || a == "--help")
            {
                opts.help = true;
            }

            else if (a == "-l" || a == "--list")
            {
                opts.list = true;

                if (i + 1 < argc && argv[i + 1][0] != '-')
                {
                    opts.arg1 = argv[++i];
                }
            }

            else if (a == "--extensions")
            {
                opts.extensions = true;
            }

            else if (a == "--keys")
            {
                opts.keys = true;
                opts.arg1 = next_arg(argc, argv, i);
            }

            else if (a == "--search")
            {
                opts.search_cmd = true;
                opts.arg1 = next_arg(argc, argv, i);
            }

            else if (a == "--exists")
            {
                opts.exists_Cmd = true;
                opts.arg1 = next_arg(argc, argv, i);
                opts.arg2 = next_arg(argc, argv, i);
            }

            else if (a == "-r" || a == "--rm")
            {
                opts.rm = true;
                opts.arg1 = next_arg(argc, argv, i);
                opts.arg2 = next_arg(argc, argv, i);
            }

            else if (a == "-g" || a == "--get")
            {
                opts.get = true;
                opts.arg1 = next_arg(argc, argv, i);
                opts.arg2 = next_arg(argc, argv, i);
            }

            else if (a == "-s" || a == "--set")
            {
                opts.set_cmd = true;
                opts.arg1 = next_arg(argc, argv, i);
                opts.arg2 = next_arg(argc, argv, i);
                opts.arg3 = next_arg(argc, argv, i);
            }

            else if (a == "--copy")
            {
                opts.copy_cmd = true;
                opts.arg1 = next_arg(argc, argv, i);
                opts.arg2 = next_arg(argc, argv, i);
                opts.arg3 = next_arg(argc, argv, i);
                opts.arg4 = next_arg(argc, argv, i);
            }

            else if (a == "--move")
            {
                opts.move_cmd = true;
                opts.arg1 = next_arg(argc, argv, i);
                opts.arg2 = next_arg(argc, argv, i);
                opts.arg3 = next_arg(argc, argv, i);
                opts.arg4 = next_arg(argc, argv, i);
            }

            else if (a == "--clear")
            {
                opts.clear_cmd = true;
                opts.arg1 = next_arg(argc, argv, i);
            }

            else if (a == "-i" || a == "--import")
            {
                opts.import_cmd = true;
                opts.arg1 = next_arg(argc, argv, i);
            }

            else if (a == "-e" || a == "--export")
            {
                opts.export_cmd = true;
                opts.arg1 = next_arg(argc, argv, i);
            }
        }

        bool any_cmd =
            opts.list       || opts.extensions || opts.keys       ||
            opts.search_cmd || opts.exists_Cmd || opts.rm         ||
            opts.get        || opts.set_cmd    || opts.copy_cmd   ||
            opts.move_cmd   || opts.clear_cmd  || opts.import_cmd ||
            opts.export_cmd || opts.version    || opts.help;

        if (!any_cmd)
        {
            opts.repl = true;
        }

        return opts;
    }

    int run_cli(
        const CLIOptions& options
    ) {
        if (options.help)
        {
            print_help();
            return 0;
        }

        if (options.version)
        {
            std::cout << "vsks " << VSKS_VERSION << "\n";
            return 0;
        }

        std::filesystem::path db_path;
        std::string app_name = "Code Safe Storage";

        if (options.database_path)
        {
            db_path = *options.database_path;
        }

        else
        {
            auto install = best_install();

            if (!install)
            {
                std::cerr << "vsks: no VSCode installation found. Use -d to specify a database.\n";
                return 1;
            }

            db_path  = install->database_path;
            app_name = install->app_name;

            if (!options.quiet)
            {
                std::cerr
                    << "Using: "
                    << install->label
                    << " ("
                    << db_path.string()
                    << ")\n";
            }
        }

        Database db(db_path, app_name);

        if (options.repl)
        {
            run_repl(db, options.json, options.quiet);
            return 0;
        }

        if (options.list)
        {
            if (options.arg1.empty())
            {
                print_list(db.list_extensions(), options.json);
            }

            else
            {
                print_secrets(db.list_secrets(options.arg1), options.json);
            }
        }

        else if (options.extensions)
        {
            print_list(db.list_extensions(), options.json);
        }

        else if (options.keys)
        {
            if (options.arg1.empty())
            {
                std::cerr << "--keys requires EXTENSION-ID\n";
                return 1;
            }

            print_list(db.list_keys(options.arg1), options.json);
        }

        else if (options.search_cmd)
        {
            if (options.arg1.empty())
            {
                std::cerr << "--search requires QUERY\n";
                return 1;
            }

            print_secrets(db.search(options.arg1), options.json);
        }

        else if (options.exists_Cmd)
        {
            bool e = db.exists(options.arg1, options.arg2);

            if (options.json)
            {
                std::cout << (e ? "true" : "false") << "\n";
            }

            else
            {
                std::cout << (e ? "exists" : "not found") << "\n";
            }

            return e ? 0 : 1;
        }

        else if (options.get)
        {
            auto v = db.get(options.arg1, options.arg2);

            if (!v)
            {
                std::cerr << "not found\n";
                return 1;
            }

            print_value(*v, options.json);
        }

        else if (options.set_cmd)
        {
            db.set(options.arg1, options.arg2, options.arg3);

            if (!options.quiet)
            {
                std::cout << "ok\n";
            }
        }

        else if (options.rm)
        {
            if (!confirm(
                    "Remove secret " + options.arg1 + " / " + options.arg2 + "?",
                    options.yes
                ))
            {
                return 0;
            }

            bool ok = db.remove(options.arg1, options.arg2);

            if (!options.quiet)
            {
                std::cout << (ok ? "removed" : "not found") << "\n";
            }

            return ok ? 0 : 1;
        }

        else if (options.copy_cmd)
        {
            bool ok = db.copy(options.arg1, options.arg2, options.arg3, options.arg4);

            if (!options.quiet)
            {
                std::cout << (ok ? "copied" : "source not found") << "\n";
            }

            return ok ? 0 : 1;
        }

        else if (options.move_cmd)
        {
            if (!confirm("Move secret?", options.yes))
            {
                return 0;
            }

            bool ok = db.move(options.arg1, options.arg2, options.arg3, options.arg4);

            if (!options.quiet)
            {
                std::cout << (ok ? "moved" : "source not found") << "\n";
            }

            return ok ? 0 : 1;
        }

        else if (options.clear_cmd)
        {
            if (!confirm(
                    "Remove ALL secrets for " + options.arg1 + "?",
                    options.yes
                ))
            {
                return 0;
            }

            int n = db.clear(options.arg1);

            if (!options.quiet)
            {
                std::cout << "removed " << n << " secret(s)\n";
            }
        }

        else if (options.import_cmd)
        {
            int n = db.import_from(options.arg1);

            if (n < 0)
            {
                std::cerr << "failed to open " << options.arg1 << "\n";
                return 1;
            }

            if (!options.quiet)
            {
                std::cout << "imported " << n << " secret(s)\n";
            }
        }

        else if (options.export_cmd)
        {
            if (!db.export_to(options.arg1))
            {
                std::cerr << "export failed\n";
                return 1;
            }

            if (!options.quiet)
            {
                std::cout << "exported to " << options.arg1 << "\n";
            }
        }

        return 0;
    }
}