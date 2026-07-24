# VSKS <img src="assets/icon.png" width="128" align="right">

A reworked and Linux focused [**VSCode Secrets Explorer** (VSCSE)](https://github.com/xNasuni/vscse/) fork, adding more features, better compatability with multiple editions of VSCode, and reliable stability.

### Context

Visual Studio Code's extension API can store persistent encrypted values that can't easily be edited or viewed.

There *are* scenarios where modifying these values would be useful, like changing locally stored extension data.

### CLI and REPL Usage

```powershell
vsks --database / -d <PATH>                         -- force a path to VSCDB file.
vsks --key / -k <PATH>                              -- force a path to a local state file.
vsks --list / -l <EXTENSION-ID>                     -- list all secrets for a specific extension.
vsks --extensions                                   -- list all extensions with stored secrets.
vsks --keys <EXTENSION-ID>                          -- list only keys for an extension.
vsks --search <QUERY>                               -- search extensions and secret keys.
vsks --exists <EXTENSION-ID> <KEY>                  -- check whether a secret exists.
vsks --rm / -r <EXTENSION-ID> <KEY>                 -- remove a secret.
vsks --get / -g <EXTENSION-ID> <KEY>                -- fetch and decrypt a secret.
vsks --set / -s <EXTENSION-ID> <KEY> <VALUE>        -- store and encrypt a secret.
vsks --copy <SRC-EXT> <SRC-KEY> <DST-EXT> <DST-KEY> -- copy a secret.
vsks --move <SRC-EXT> <SRC-KEY> <DST-EXT> <DST-KEY> -- move a secret.
vsks --clear <EXTENSION-ID>                         -- remove all secrets for an extension.
vsks --import / -i <PATH>                           -- import secrets from a file.
vsks --export / -e <PATH>                           -- export secrets to a file.
vsks --json                                         -- output results as JSON.
vsks --quiet / -q                                   -- suppress non-error output.
vsks --yes / -y                                     -- skip confirmation prompts.
vsks --version / -v                                 -- show program version.
vsks --help / -h                                    -- show this help message.
```

```powershell
exit                                         -- exit the program.
help                                         -- show available commands.
ls [EXTENSION-ID]                            -- list extensions or secrets.
extensions                                   -- list all extensions.
keys <EXTENSION-ID>                          -- list keys for an extension.
search <QUERY>                               -- search extensions and keys.
exists <EXTENSION-ID> <KEY>                  -- check whether a secret exists.
rm <EXTENSION-ID> <KEY>                      -- remove a secret.
get <EXTENSION-ID> <KEY>                     -- get a secret value.
set <EXTENSION-ID> <KEY> <VALUE>             -- set a secret value.
copy <SRC-EXT> <SRC-KEY> <DST-EXT> <DST-KEY> -- copy a secret.
move <SRC-EXT> <SRC-KEY> <DST-EXT> <DST-KEY> -- move a secret.
clear <EXTENSION-ID>                         -- remove all secrets for an extension.
```