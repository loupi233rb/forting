#pragma once

#include "type.h"
#include "fortingcore.h"

#include <ctime>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace Forting
{
struct ParseError : std::runtime_error {
    int line = 1;
    int col = 1;
    ParseError(int l, int c, const std::string& msg);
};

class SyntaxParser {
public:
    SyntaxParser();
    ~SyntaxParser();

    SyntaxParser(const SyntaxParser&) = delete;
    SyntaxParser& operator=(const SyntaxParser&) = delete;

    SyntaxParser(SyntaxParser&&) noexcept;
    SyntaxParser& operator=(SyntaxParser&&) noexcept;

    void loadFromFile(const std::string& path);
    void loadFromString(const std::string& code);

    // Outer vector size = unit count; inner vector size = files.size()
    std::vector<action> run(const File& file) const;

    std::size_t unitCount() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}