// model/CbTemplateRef.cpp -- see CbTemplateRef.h.
//
// Successor of the dialogs' MFC-era OnChangeTemplatedeclaration, which removed
// "const " / "unsigned " ... as substrings (so a concept named is_const lost
// half its name) and split on the first comma (so a default or a nested
// template argument list with a comma broke it). This splits the parameter list
// on top-level commas and takes each parameter's last identifier.
#include "StdAfx.h"
#include "CbTemplateRef.h"
#include "ClassBuilderInclude.h"   // BaseClass::StripTemplateDeclaration

#include <cctype>
#include <string>
#include <vector>

namespace {

bool IsIdentChar(char c) { return std::isalnum((unsigned char)c) || c == '_'; }

std::string Trim(const std::string& s)
{
    size_t b = 0, e = s.size();
    while (b < e && std::isspace((unsigned char)s[b])) ++b;
    while (e > b && std::isspace((unsigned char)s[e - 1])) --e;
    return s.substr(b, e - b);
}

// The reference of one template parameter: its name (the last identifier), with
// "..." kept for a parameter pack.
std::string ParameterReference(const std::string& param)
{
    std::string name;
    for (size_t i = param.size(); i > 0; --i)
    {
        if (IsIdentChar(param[i - 1]))
        {
            size_t end = i;
            while (i > 0 && IsIdentChar(param[i - 1])) --i;
            name = param.substr(i, end - i);
            break;
        }
    }
    if (name.empty())
        return name;
    if (param.find("...") != std::string::npos)
        name += "...";
    return name;
}

} // namespace

std::string Cb_DeriveTemplateReference(const std::string& declaration)
{
    CbString stripped(Trim(declaration).c_str());
    BaseClass::StripTemplateDeclaration(stripped);
    std::string decl = Trim(std::string((const char*)stripped));

    if (decl.compare(0, 8, "template") != 0)
        return std::string();
    decl = Trim(decl.substr(8));
    if (decl.empty() || decl[0] != '<')
        return std::string();

    // The parameter list: up to the '>' that closes the opening '<'.
    std::vector<std::string> params;
    std::string cur;
    int depth = 0;
    for (size_t i = 1; i < decl.size(); ++i)
    {
        const char c = decl[i];
        if (c == '<' || c == '(') ++depth;
        else if (c == ')') --depth;
        else if (c == '>')
        {
            if (depth == 0) break;
            --depth;
        }
        else if (c == ',' && depth == 0)
        {
            params.push_back(cur);
            cur.clear();
            continue;
        }
        cur += c;
    }
    if (!Trim(cur).empty() || !params.empty())
        params.push_back(cur);

    std::string ref = "<";
    bool first = true;
    for (const std::string& p : params)
    {
        const std::string r = ParameterReference(Trim(p));
        if (r.empty())
            continue;
        if (!first) ref += ", ";
        first = false;
        ref += r;
    }
    return ref + ">";
}
