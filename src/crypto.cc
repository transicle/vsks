#include <openssl/evp.h>
#include <openssl/rand.h>

#include <libsecret/secret.h>

#include <cstring>
#include <stdexcept>

#include "crypto.hh"

// clanker gemenated portion of namespace

// i pinky promise i looked over all of it!!

namespace VSKS
{
    static constexpr const char* PBKDF2_SALT = "saltysalt";
    static constexpr const char* PBKDF2_PEANUTS = "peanuts";
    static constexpr int PBKDF2_ITER = 1;
    static constexpr int KEY_LEN = 16;
    static constexpr int BLOCK_SIZE = 16;

    Bytes derive_key(
        const std::string& password
    ) {
        Bytes key(KEY_LEN);

        if (PKCS5_PBKDF2_HMAC_SHA1(
                password.c_str(),
                static_cast<int>(password.size()),
                reinterpret_cast<const uint8_t*>(PBKDF2_SALT),
                static_cast<int>(std::strlen(PBKDF2_SALT)),
                PBKDF2_ITER,
                KEY_LEN,
                key.data()
            ) != 1)
        {
            throw std::runtime_error("PBKDF2 failed");
        }

        return key;
    }

    std::optional<std::string> decrypt(
        const Bytes& ciphertext,
        const Bytes& key
    ) {
        if (key.size() != KEY_LEN)
        {
            return std::nullopt;
        }

        if (ciphertext.empty())
        {
            return std::nullopt;
        }

        uint8_t iv[BLOCK_SIZE];
        std::memset(iv, 0x20, BLOCK_SIZE);

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
        {
            return std::nullopt;
        }

        std::optional<std::string> result;

        do
        {
            if (EVP_DecryptInit_ex(
                    ctx,
                    EVP_aes_128_cbc(),
                    nullptr,
                    key.data(),
                    iv
                ) != 1)
            {
                break;
            }

            EVP_CIPHER_CTX_set_padding(ctx, 0);

            std::string plaintext(ciphertext.size() + BLOCK_SIZE, '\0');
            int out_len1 = 0;
            int out_len2 = 0;

            if (EVP_DecryptUpdate(
                    ctx,
                    reinterpret_cast<uint8_t*>(plaintext.data()),
                    &out_len1,
                    ciphertext.data(),
                    static_cast<int>(ciphertext.size())
                ) != 1)
            {
                break;
            }

            if (EVP_DecryptFinal_ex(
                    ctx,
                    reinterpret_cast<uint8_t*>(plaintext.data()) + out_len1,
                    &out_len2
                ) != 1)
            {
                break;
            }

            plaintext.resize(out_len1 + out_len2);

            if (!plaintext.empty())
            {
                uint8_t pad = static_cast<uint8_t>(plaintext.back());

                if (pad > 0 && pad <= BLOCK_SIZE)
                {
                    plaintext.resize(plaintext.size() - pad);
                }
            }

            result = std::move(plaintext);
        }

        while (false);

        EVP_CIPHER_CTX_free(ctx);

        return result;
    }

    Bytes encrypt(
        const std::string& plaintext,
        const Bytes& key,
        std::string_view prefix
    ) {
        size_t pad_len = BLOCK_SIZE - (plaintext.size() % BLOCK_SIZE);
        std::string padded = plaintext + std::string(pad_len, static_cast<char>(pad_len));

        uint8_t iv[BLOCK_SIZE];
        std::memset(iv, 0x20, BLOCK_SIZE);

        Bytes ciphertext(padded.size());

        int out_len1 = 0;
        int out_len2 = 0;

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx)
        {
            throw std::runtime_error("EVP_CIPHER_CTX_new failed");
        }

        EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key.data(), iv);
        EVP_CIPHER_CTX_set_padding(ctx, 0);
        EVP_EncryptUpdate(
            ctx,
            ciphertext.data(),
            &out_len1,
            reinterpret_cast<const uint8_t*>(padded.data()),
            static_cast<int>(padded.size())
        );

        EVP_EncryptFinal_ex(ctx, ciphertext.data() + out_len1, &out_len2);
        EVP_CIPHER_CTX_free(ctx);

        ciphertext.resize(out_len1 + out_len2);

        Bytes result;
        for (char c : prefix)
        {
            result.push_back(static_cast<uint8_t>(c));
        }

        result.insert(result.end(), ciphertext.begin(), ciphertext.end());

        return result;
    }


    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wmissing-field-initializers"

    static const SecretSchema VSCODE_SCHEMA =
    {
        "chrome_libsecret_os_crypt_password_v2",
        SECRET_SCHEMA_DONT_MATCH_NAME,
        {
            { "application", SECRET_SCHEMA_ATTRIBUTE_STRING },
            { nullptr,       SECRET_SCHEMA_ATTRIBUTE_STRING },
        }
    };

    #pragma GCC diagnostic pop

    std::optional<std::string> get_password(
        const std::string& app_name
    ) {
        GError* err = nullptr;

        gchar* pw = secret_password_lookup_sync(
            &VSCODE_SCHEMA,
            nullptr,
            &err,
            "application",
            app_name.c_str(),
            nullptr
        );

        if (err)
        {
            g_error_free(err);
            return std::nullopt;
        }

        if (!pw)
        {
            return std::nullopt;
        }

        std::string result(pw);
        secret_password_free(pw);

        return result;
    }

    bool set_password(
        const std::string& app_name,
        const std::string& password
    ) {
        GError* err = nullptr;

        bool ok = secret_password_store_sync(
            &VSCODE_SCHEMA,
            SECRET_COLLECTION_DEFAULT,
            app_name.c_str(),
            password.c_str(),
            nullptr,
            &err,
            "application",
            app_name.c_str(),
            nullptr
        );

        if (err)
        {
            g_error_free(err);
            return false;
        }

        return ok;
    }

    std::optional<std::string> vscode_decrypt(
        const Bytes& raw,
        const std::string& app_name
    ) {
        if (raw.size() < 3)
        {
            return std::nullopt;
        }

        std::string_view sv(
            reinterpret_cast<const char*>(raw.data()),
            raw.size()
        );

        Bytes key;
        Bytes body;

        if (sv.starts_with(PREFIX_V11))
        {
            auto kring = get_password(app_name);
            std::string pw = kring.value_or(std::string(PBKDF2_PEANUTS));

            key  = derive_key(pw);
            body = Bytes(raw.begin() + 3, raw.end());
        }

        else if (sv.starts_with(PREFIX_V10))
        {
            key  = derive_key(PBKDF2_PEANUTS);
            body = Bytes(raw.begin() + 3, raw.end());
        }

        else
        {
            return std::string(sv);
        }

        return decrypt(body, key);
    }

    Bytes vscode_encrypt(
        const std::string& plaintext,
        const std::string& app_name
    ) {
        auto kring = get_password(app_name);

        if (kring)
        {
            Bytes key = derive_key(*kring);
            return encrypt(plaintext, key, PREFIX_V11);
        }

        Bytes key = derive_key(PBKDF2_PEANUTS);
        return encrypt(plaintext, key, PREFIX_V10);
    }
}