#include <sqlite3.h>

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>

#include "database.hh"
#include "crypto.hh"
#include "io.hh"

namespace VSKS
{
    static std::string make_db_key(
        const std::string& extension_id,
        const std::string& key
    ) {
        return R"(secret://{"extensionId":")"
            + extension_id
            + R"(","key":")"
            + key
            + R"("})";
    }

    static bool parse_db_key(
        const std::string& db_key,
        std::string& extension_id,
        std::string& key
    ) {
        static constexpr std::string_view PREFIX = "secret://";

        if (!db_key.starts_with(PREFIX))
        {
            return false;
        }

        std::string json = db_key.substr(PREFIX.size());

        auto find_value = [&](const std::string& field)
        {
            std::string needle = "\"" + field + "\":\"";

            auto position = json.find(needle);
            if (position == std::string::npos)
            {
                return std::string();
            }

            position += needle.size();

            auto end = json.find('"', position);
            if (end == std::string::npos)
            {
                return std::string();
            }

            return json.substr(position, end - position);
        };

        extension_id = find_value("extensionId");
        key = find_value("key");

        return !extension_id.empty();
    }

    static Bytes parse_value_blob(
        const std::string& json_value
    ) {
        auto position = json_value.find("\"data\":[");
        if (position == std::string::npos)
        {
            return Bytes(
                json_value.begin(),
                json_value.end()
            );
        }

        position += 8;

        auto end = json_value.find(']', position);
        if (end == std::string::npos)
        {
            return {};
        }

        Bytes result;

        std::istringstream stream(json_value.substr(position, end - position));
        std::string token;

        while (std::getline(stream, token, ','))
        {
            try
            {
                result.push_back(
                    static_cast<uint8_t>(std::stoi(token))
                );
            }

            catch (...) { }
        }

        return result;
    }

    static std::string encode_value_blob(
        const Bytes& data
    ) {
        std::string result = R"({"type":"Buffer","data":[)";

        for (size_t i = 0; i < data.size(); ++i)
        {
            if (i)
            {
                result += ',';
            }

            result += std::to_string(data[i]);
        }

        result += "]}";

        return result;
    }

    struct Database::Impl
    {
        sqlite3* db = nullptr;
        std::string app_name;

        explicit Impl(
            const std::filesystem::path& path,
            const std::string& application_name
        )
            : app_name(application_name)
        {
            if (sqlite3_open(
                    path.c_str(),
                    &db
                ) != SQLITE_OK)
            {
                throw std::runtime_error(
                    std::string("Cannot open database: ")
                    + sqlite3_errmsg(db)
                );
            }

            sqlite3_exec(
                db,
                "PRAGMA journal_mode=WAL;",
                nullptr,
                nullptr,
                nullptr
            );
        }

        ~Impl()
        {
            if (db)
            {
                sqlite3_close(db);
            }
        }


        template<typename Callback>
        void query(
            const std::string& sql,
            const std::vector<std::string>& binds,
            Callback&& callback
        ) const {
            sqlite3_stmt* statement = nullptr;

            if (sqlite3_prepare_v2(
                    db,
                    sql.c_str(),
                    -1,
                    &statement,
                    nullptr
                ) != SQLITE_OK)
            {
                throw std::runtime_error(
                    std::string("SQL error: ")
                    + sqlite3_errmsg(db)
                );
            }

            for (size_t i = 0; i < binds.size(); ++i)
            {
                sqlite3_bind_text(
                    statement,
                    static_cast<int>(i + 1),
                    binds[i].c_str(),
                    -1,
                    SQLITE_TRANSIENT
                );
            }

            while (sqlite3_step(statement) == SQLITE_ROW)
            {
                callback(statement);
            }

            sqlite3_finalize(statement);
        }


        void exec(
            const std::string& sql,
            const std::vector<std::string>& binds = {}
        ) {
            sqlite3_stmt* statement = nullptr;

            if (sqlite3_prepare_v2(
                    db,
                    sql.c_str(),
                    -1,
                    &statement,
                    nullptr
                ) != SQLITE_OK)
            {
                throw std::runtime_error(
                    std::string("SQL error: ")
                    + sqlite3_errmsg(db)
                );
            }

            for (size_t i = 0; i < binds.size(); ++i)
            {
                sqlite3_bind_text(
                    statement,
                    static_cast<int>(i + 1),
                    binds[i].c_str(),
                    -1,
                    SQLITE_TRANSIENT
                );
            }

            sqlite3_step(statement);

            sqlite3_finalize(statement);
        }


        std::optional<std::string> decrypt_value(
            const std::string& raw_value
        ) const {
            Bytes blob = parse_value_blob(raw_value);

            return vscode_decrypt(
                blob,
                app_name
            );
        }


        std::string encrypt_value(
            const std::string& plaintext
        ) const {
            Bytes encrypted = vscode_encrypt(
                plaintext,
                app_name
            );

            return encode_value_blob(
                encrypted
            );
        }
    };

    Database::Database(
        const std::filesystem::path& path,
        const std::string& app_name
    )
        : impl_(new Impl(path, app_name))
    { }

    Database::~Database()
    {
        delete impl_;
    }


    std::vector<std::string> Database::list_extensions() const
    {
        std::set<std::string> extensions;

        impl_->query(
            "SELECT key FROM ItemTable WHERE key LIKE 'secret://%'",
            {},
            [&](sqlite3_stmt* statement)
            {
                std::string db_key =
                    reinterpret_cast<const char*>(
                        sqlite3_column_text(
                            statement,
                            0
                        )
                    );

                std::string extension_id;
                std::string key;

                if (parse_db_key(
                        db_key,
                        extension_id,
                        key
                    ))
                {
                    extensions.insert(extension_id);
                }
            }
        );

        return std::vector<std::string>(
            extensions.begin(),
            extensions.end()
        );
    }


    std::vector<Secret> Database::list_secrets(
        const std::string& extension_id
    ) const {
        std::vector<Secret> secrets;

        impl_->query(
            "SELECT key, value FROM ItemTable WHERE key LIKE 'secret://%'",
            {},
            [&](sqlite3_stmt* statement)
            {
                std::string db_key =
                    reinterpret_cast<const char*>(
                        sqlite3_column_text(
                            statement,
                            0
                        )
                    );

                std::string db_value =
                    reinterpret_cast<const char*>(
                        sqlite3_column_text(
                            statement,
                            1
                        )
                    );

                std::string found_extension;
                std::string key;

                if (!parse_db_key(
                        db_key,
                        found_extension,
                        key
                    ))
                {
                    return;
                }

                if (!extension_id.empty()
                    && found_extension != extension_id)
                {
                    return;
                }

                auto decrypted = impl_->decrypt_value(
                    db_value
                );

                secrets.push_back(
                    {
                        found_extension,
                        key,
                        decrypted.value_or(
                            "<decryption failed>"
                        )
                    }
                );
            }
        );

        return secrets;
    }

    std::vector<std::string> Database::list_keys(
        const std::string& extension_id
    ) const {
        std::vector<std::string> keys;

        impl_->query(
            "SELECT key FROM ItemTable WHERE key LIKE 'secret://%'",
            {},
            [&](sqlite3_stmt* statement)
            {
                std::string db_key =
                    reinterpret_cast<const char*>(
                        sqlite3_column_text(
                            statement,
                            0
                        )
                    );

                std::string found_extension;
                std::string key;

                if (parse_db_key(
                        db_key,
                        found_extension,
                        key
                    )
                    && found_extension == extension_id)
                {
                    keys.push_back(key);
                }
            }
        );

        return keys;
    }


    std::vector<Secret> Database::search(
        const std::string& query
    ) const {
        std::vector<Secret> results;

        auto secrets = list_secrets("");

        for (auto& secret : secrets)
        {
            if (secret.extension_id.find(query) != std::string::npos
                || secret.key.find(query) != std::string::npos)
            {
                results.push_back(secret);
            }
        }

        return results;
    }


    bool Database::exists(
        const std::string& extension_id,
        const std::string& key
    ) const {
        bool found = false;

        impl_->query(
            "SELECT 1 FROM ItemTable WHERE key=? LIMIT 1",
            {
                make_db_key(
                    extension_id,
                    key
                )
            },
            [&](sqlite3_stmt*)
            {
                found = true;
            }
        );

        return found;
    }


    std::optional<std::string> Database::get(
        const std::string& extension_id,
        const std::string& key
    ) const {
        std::optional<std::string> result;

        impl_->query(
            "SELECT value FROM ItemTable WHERE key=? LIMIT 1",
            {
                make_db_key(
                    extension_id,
                    key
                )
            },
            [&](sqlite3_stmt* statement)
            {
                std::string raw =
                    reinterpret_cast<const char*>(
                        sqlite3_column_text(
                            statement,
                            0
                        )
                    );

                result = impl_->decrypt_value(
                    raw
                );
            }
        );

        return result;
    }


    bool Database::set(
        const std::string& extension_id,
        const std::string& key,
        const std::string& value
    ) {
        std::string db_key = make_db_key(
            extension_id,
            key
        );

        std::string db_value = impl_->encrypt_value(
            value
        );

        impl_->exec(
            "INSERT OR REPLACE INTO ItemTable(key, value) VALUES(?, ?)",
            {
                db_key,
                db_value
            }
        );

        return true;
    }


    bool Database::remove(
        const std::string& extension_id,
        const std::string& key
    ) {
        if (!exists(
                extension_id,
                key
            ))
        {
            return false;
        }

        impl_->exec(
            "DELETE FROM ItemTable WHERE key=?",
            {
                make_db_key(
                    extension_id,
                    key
                )
            }
        );

        return true;
    }


    int Database::clear(
        const std::string& extension_id
    ) {
        auto keys = list_keys(
            extension_id
        );

        for (const auto& key : keys)
        {
            impl_->exec(
                "DELETE FROM ItemTable WHERE key=?",
                {
                    make_db_key(
                        extension_id,
                        key
                    )
                }
            );
        }

        return static_cast<int>(
            keys.size()
        );
    }


    bool Database::copy(
        const std::string& source_extension,
        const std::string& source_key,

        const std::string& destination_extension,
        const std::string& destination_key
    ) {
        auto value = get(
            source_extension,
            source_key
        );

        if (!value)
        {
            return false;
        }

        return set(
            destination_extension,
            destination_key,
            *value
        );
    }


    bool Database::move(
        const std::string& source_extension,
        const std::string& source_key,

        const std::string& destination_extension,
        const std::string& destination_key
    ) {
        if (!copy(
                source_extension,
                source_key,
                destination_extension,
                destination_key
            ))
        {
            return false;
        }

        remove(
            source_extension,
            source_key
        );

        return true;
    }

    bool Database::export_to(
        const std::filesystem::path& out
    ) const {
        auto secrets = list_secrets("");

        std::ofstream file(
            out
        );

        if (!file)
        {
            return false;
        }

        file << "[\n";

        for (size_t i = 0; i < secrets.size(); ++i)
        {
            const auto& secret = secrets[i];

            file
                << "  {\"extensionId\":\""
                << json_escape(
                    secret.extension_id
                )
                << "\",\"key\":\""
                << json_escape(
                    secret.key
                )
                << "\",\"value\":\""
                << json_escape(
                    secret.value
                )
                << "\"}";

            if (i + 1 < secrets.size())
            {
                file << ",";
            }

            file << "\n";
        }

        file << "]\n";

        return true;
    }


    int Database::import_from(
        const std::filesystem::path& input
    ) {
        std::ifstream file(
            input
        );

        if (!file)
        {
            return -1;
        }

        std::string content(
            (
                std::istreambuf_iterator<char>(
                    file
                )
            ),
            std::istreambuf_iterator<char>()
        );

        int count = 0;
        size_t position = 0;

        while ((position = content.find(
                    '{',
                    position
                )) != std::string::npos)
        {
            auto end = content.find(
                '}',
                position
            );

            if (end == std::string::npos)
            {
                break;
            }

            std::string object = content.substr(
                position,
                end - position + 1
            );

            position = end + 1;

            auto find_value = [&](const std::string& field)
            {
                std::string needle =
                    "\""
                    + field
                    + "\":\"";

                auto start = object.find(
                    needle
                );

                if (start == std::string::npos)
                {
                    return std::string();
                }

                start += needle.size();

                auto end = object.find(
                    '"',
                    start
                );

                if (end == std::string::npos)
                {
                    return std::string();
                }

                return object.substr(
                    start,
                    end - start
                );
            };

            std::string extension_id = find_value(
                "extensionId"
            );

            std::string key = find_value(
                "key"
            );

            std::string value = find_value(
                "value"
            );

            if (extension_id.empty()
                || key.empty())
            {
                continue;
            }

            set(
                extension_id,
                key,
                value
            );

            ++count;
        }

        return count;
    }
}