#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace VSKS
{
    using Bytes = std::vector<uint8_t>;

    // tl;dr: v10 = PBKDF2("peanuts", "saltysalt", 1, 16) + AES-128-CBC, IV=16 spaces
    //        v11 = same but key from libsecret keyring

    inline constexpr std::string_view PREFIX_V10 = "v10";
    inline constexpr std::string_view PREFIX_V11 = "v11";

    // @brief derive an AES-128 key from the given pswd using Chromium's exact params:
    //        PBKDF2-SHA1, salt="saltysalt", iterations=1, keylen=16
    Bytes derive_key(
        const std::string& password
    );

    // @brief strip v10/v11 prefix, then AES-128-CBC, return nullopt on failure
    std::optional<std::string> decrypt(
        const Bytes& cipher_text,
        const Bytes& key
    );

    Bytes encrypt(
        const std::string& plain_text,
        const Bytes& key,
        std::string_view prefix
    );

    std::optional<std::string> get_password(
        const std::string& app_name
    );

    bool set_password(
        const std::string& app_name,
        const std::string& password
    );

    std::optional<std::string> vscode_decrypt(
        const Bytes& raw,
        const std::string& app_name
    );

    // @brief encrypts with v11 if keyring available, v10 otherwise
    Bytes vscode_encrypt(
        const std::string& plain_text,
        const std::string& app_name
    );
}