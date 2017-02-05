#ifndef WINDBGPARSER_H
#define WINDBGPARSER_H

#include <vector>
#include <map>
#include <string>

class WinDbgParser
{
private:
    static WinDbgParser m_instance;
    WinDbgParser();
    ~WinDbgParser();
    std::string parseType(std::vector<std::string> lastParsedContentLine, std::string strUnparsedType, std::string* additionnalStr);
    std::string detectTypeFromStructIfNecessary(std::string s);
    std::map<std::string, std::string> corrVarTypes;

public:
    static WinDbgParser& getInstance();
    std::vector<std::string> parseFirstLine(std::string strFirstLine);
    std::vector<std::string> parseContentLine(std::vector<std::string> lastParsedContentLine, std::string strContentLine);
};

#endif // WINDBGPARSER_H
