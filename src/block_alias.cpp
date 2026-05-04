#include "block_alias.hpp"

#include <cctype>
#include <sstream>
#include <utility>

namespace mcvi {
namespace {

std::string trim(std::string_view text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
        text.remove_prefix(1);
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
        text.remove_suffix(1);
    }
    return std::string(text);
}

std::string remaining(std::istringstream& input) {
    std::string rest;
    std::getline(input, rest);
    return trim(rest);
}

} // namespace

void BlockAliases::set(char alias, std::string block_name) {
    aliases_[alias] = std::move(block_name);
}

const std::string* BlockAliases::get(char alias) const {
    auto it = aliases_.find(alias);
    if (it == aliases_.end()) {
        return nullptr;
    }
    return &it->second;
}

const std::map<char, std::string>& BlockAliases::entries() const {
    return aliases_;
}

std::string BlockAliases::describe(char alias) const {
    const std::string* block_name = get(alias);
    if (block_name == nullptr) {
        return std::string(1, alias) + " is unassigned";
    }
    return std::string(1, alias) + " = " + *block_name;
}

std::string BlockAliases::describe_all() const {
    if (aliases_.empty()) {
        return "No block aliases";
    }

    std::string out;
    for (const auto& [alias, block_name] : aliases_) {
        if (!out.empty()) {
            out += ", ";
        }
        out.push_back(alias);
        out += "=";
        out += block_name;
    }
    return out;
}

std::string BlockAliases::describe_empty() const {
    std::string out = "empty: ";
    bool first = true;
    for (char ch = 'a'; ch <= 'z'; ++ch) {
        if (get(ch) != nullptr) {
            continue;
        }
        if (!first) {
            out += ",";
        }
        out.push_back(ch);
        first = false;
    }
    for (char ch = 'A'; ch <= 'Z'; ++ch) {
        if (get(ch) != nullptr) {
            continue;
        }
        if (!first) {
            out += ",";
        }
        out.push_back(ch);
        first = false;
    }
    if (first) {
        out += "none";
    }
    return out;
}

bool is_block_alias_char(char ch) {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}

bool handle_block_alias_command(BlockAliases& aliases, std::string_view argument, std::string& message) {
    std::istringstream input(trim(argument));
    std::string first;
    input >> first;

    if (first.empty()) {
        message = "Usage: bl {alias} [block], bl all, bl emp";
        return false;
    }
    if (first == "all") {
        message = aliases.describe_all();
        return true;
    }
    if (first == "emp") {
        message = aliases.describe_empty();
        return true;
    }
    if (first.size() != 1 || !is_block_alias_char(first[0])) {
        message = "Block alias must be a-z or A-Z";
        return false;
    }

    char alias = first[0];
    std::string block_name = remaining(input);
    if (block_name.empty()) {
        message = aliases.describe(alias);
        return true;
    }

    aliases.set(alias, block_name);
    message = aliases.describe(alias);
    return true;
}

} // namespace mcvi
