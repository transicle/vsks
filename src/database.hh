#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace VSKS {

    struct Secret {
        std::string extension_id;
        std::string key;
        std::string value; // decrypted
    };

    class Database
    {
    public:
        explicit Database(
            const std::filesystem::path& path,
            const std::string& app_name
        );

        ~Database();

        Database(
            const Database&
        ) = delete;
        
        Database& operator=(
            const Database&
        ) = delete;

        std::vector<std::string> list_extensions() const;

        std::vector<Secret> list_secrets(
            const std::string& extension_id
        ) const;

        std::vector<std::string> list_keys(
            const std::string& extension_id
        ) const;

        std::vector<Secret> search(
            const std::string& query
        ) const;

        bool exists(
            const std::string& extension_id,
            const std::string& key
        ) const;

        std::optional<std::string> get(
            const std::string& extension_id,
            const std::string& key
        ) const;

        bool set(
            const std::string& extension_id,
            const std::string& key,
            const std::string& value
        );

        bool remove(
            const std::string& extension_id,
            const std::string& key
        );

        int clear(
            const std::string& extension_id
        );

        bool copy(
            const std::string& src_ext,
            const std::string& src_key,

            const std::string& dst_ext,
            const std::string& dst_key
        );

        bool move(
            const std::string& src_ext,
            const std::string& src_key,

            const std::string& dst_ext,
            const std::string& dst_key
        );

        bool export_to(
            const std::filesystem::path& out
        ) const;

        int import_from(
            const std::filesystem::path& input
        );

    private:
        struct Impl;
        Impl* impl_;
    };

}