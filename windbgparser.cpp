#include "windbgparser.h"

using namespace std;

WinDbgParser WinDbgParser::m_instance=WinDbgParser();

WinDbgParser::WinDbgParser()
{
    corrVarTypes["UCHAR"] = "BYTE";
    corrVarTypes["PPTR64 VOID"] = "PPVOID";
    corrVarTypes["UINT4B"] = "ULONG";
    corrVarTypes["PUINT4B"] = "PULONG";
    corrVarTypes["UINT2B"] = "USHORT";
    corrVarTypes["PUINT2B"] = "PUSHORT";
    corrVarTypes["UINT8B"] = "ULONGLONG";
}

WinDbgParser& WinDbgParser::getInstance()
{
    return m_instance;
}

//Example:
// ntdll!_PEB
vector<string> WinDbgParser::parseFirstLine(string strFirstLine)
{
    vector<string> result;
    result.resize(2);
    string lTrimed = strFirstLine.substr(strFirstLine.find_first_not_of(' '));

    size_t posExcl = lTrimed.find_first_of('!');

    result[0] = lTrimed.substr(0, posExcl);
    result[1] = lTrimed.substr(posExcl + 1);

    return result;
}

string upperCase(string s)
{
    std::string majuscule = s;
    for (int i=0; i<majuscule.length(); ++i)
    {
      majuscule[i] = ::toupper(majuscule[i]);
    }

    return majuscule;
}

string str_ltrim(string s)
{
    return s.substr(s.find_first_not_of(' '));
}

string WinDbgParser::detectTypeFromStructIfNecessary(string sNotTrimed)
{
    string s, rawName, typePtrName, result;

    size_t tmpPosFirstNotSpace = sNotTrimed.find_first_not_of(' ');
    s = sNotTrimed.substr(tmpPosFirstNotSpace);

    size_t posSpace = s.find_first_of(' ');
    if(posSpace != string::npos)
    {
        rawName = s.substr(posSpace + 1);
        string tmp = s.substr(0, posSpace);
        size_t posPtr = tmp.find("Ptr");
        if(posPtr != string::npos)
            typePtrName = tmp.substr(posPtr + 3);
    }
    else
        rawName = s;

    //Si pointeur
    if(!typePtrName.empty())
    {
        string tmp = "P" + str_ltrim(upperCase((rawName[0] == '_' ? rawName.substr(1) : rawName)));

        if(corrVarTypes.find(tmp) != corrVarTypes.end())
        {
            result = corrVarTypes[tmp];
        }
        else
        {
            result = tmp;
        }

        result += " /* " + s + " */";
    }
    else
    {
        string tmp = str_ltrim(upperCase(s));
        if(corrVarTypes.find(tmp) != corrVarTypes.end())
        {
            result = corrVarTypes[tmp];
        }
        else
        {
            result = tmp[0] == '_' ? tmp.substr(1) : tmp;
        }
    }

    return result;
}

string WinDbgParser::parseType(vector<string> lastParsedContentLine, string strUnparsedType, string* additionnalStr)
{
    string result;
    string strUnparsedTypeWithoutEndl = strUnparsedType;
    size_t posLastEndl = strUnparsedType.find_last_of('\n');
    if(posLastEndl != string::npos)
        strUnparsedTypeWithoutEndl = strUnparsedType.substr(0, posLastEndl);

    if(strUnparsedTypeWithoutEndl.empty())
    {
        *additionnalStr = ";";
        return "";
    }

    if(strUnparsedTypeWithoutEndl.substr(0, strUnparsedTypeWithoutEndl.find_first_of(' ')) == "Pos" && !lastParsedContentLine.empty())
    {
        //Cas d'une partie d'une union
        size_t posComma = strUnparsedTypeWithoutEndl.find_first_of(',');

        //on récupère le type de la ligne d'avant, et on set le bitPos
        result = lastParsedContentLine[0];

        size_t posEndBit = strUnparsedTypeWithoutEndl.substr(posComma + 2).find_first_of(' ');
        *additionnalStr += ":" + strUnparsedTypeWithoutEndl.substr(posComma + 2, posEndBit) + ";";
    }
    else if(strUnparsedTypeWithoutEndl[0] == '[')
    {
        size_t posClosingBracket = strUnparsedTypeWithoutEndl.find_first_of(']');
        result = detectTypeFromStructIfNecessary(strUnparsedTypeWithoutEndl.substr(posClosingBracket + 1));
        *additionnalStr += strUnparsedTypeWithoutEndl.substr(0, posClosingBracket+1) + ";";
    }
    else
    {
        result = detectTypeFromStructIfNecessary(strUnparsedTypeWithoutEndl);
        *additionnalStr = ";";
    }

    return result;
}

//Examples:
//   +0x000 InheritedAddressSpace : UChar
//or
//   +0x004 Padding0         : [4] UChar
//or
//   +0x003 SkipPatchingUser32Forwarders : Pos 3, 1 Bit
vector<string> WinDbgParser::parseContentLine(vector<string> lastParsedContentLine, string strContentLine)
{
    string offsetValue, fieldName, typeName;
    vector<string> result;
    string lTrimed = strContentLine.substr(strContentLine.find_first_not_of(' '));

    size_t posFirstSpace = lTrimed.find_first_of(' ');
    offsetValue = lTrimed.substr(0, posFirstSpace);

    size_t posSecondSpace = lTrimed.substr(posFirstSpace+1).find_first_of(' ') + posFirstSpace;
    fieldName = lTrimed.substr(posFirstSpace+1, posSecondSpace - posFirstSpace + 1);

    size_t posStartType = lTrimed.find_first_of(':') + 1;

    string buf = lTrimed.substr(posStartType);
    string additionnalStr;
    typeName = parseType(lastParsedContentLine, buf.substr(buf.find_first_not_of(' ')), &additionnalStr);


    result.push_back(typeName);
    result.push_back(fieldName);
    result.push_back(offsetValue);
    result.push_back(additionnalStr);

    return result;
}


WinDbgParser::~WinDbgParser(){}

