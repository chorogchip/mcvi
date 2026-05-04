#pragma once

#include <map>
#include <string>
#include <string_view>

namespace mcvi {

class BlockAliases {
public:
    void set(char alias, std::string block_name);
    const std::string* get(char alias) const;
    const std::map<char, std::string>& entries() const;

    std::string describe(char alias) const;
    std::string describe_all() const;
    std::string describe_empty() const;

private:
    std::map<char, std::string> aliases_;
};

bool is_block_alias_char(char ch);
bool handle_block_alias_command(BlockAliases& aliases, std::string_view argument, std::string& message);

} // namespace mcvi
