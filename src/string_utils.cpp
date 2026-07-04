#include "string_utils.hpp"

std::string ShellSingleQuote(std::string_view value)
{
    std::string out;
    out.reserve(value.size() + 2);
    out.push_back('\'');
    for (char c : value)
    {
        if (c == '\'')
            out += "'\\''"; // close, escaped quote, reopen
        else
            out.push_back(c);
    }
    out.push_back('\'');
    return out;
}