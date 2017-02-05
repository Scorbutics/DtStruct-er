#include <sstream>
#include <unordered_map>
#include "mainwindow.h"
#include "windbgparser.h"

using namespace std;

MainWindow::MainWindow()
{
    m_qTextToConvert = unique_ptr<QTextEdit>(new QTextEdit(this));
    m_qTextConverted = unique_ptr<QTextEdit>(new QTextEdit(this));
    m_bConvert = unique_ptr<QPushButton>(new QPushButton("Convert", this));

    m_layout.addWidget(&(*m_qTextToConvert), 0, 0);
    m_layout.addWidget(&(*m_qTextConverted), 0, 1);
    m_layout.addWidget(&(*m_bConvert), 1, 0);

    this->setLayout(&m_layout);
    this->show();

    QObject::connect(&(*m_bConvert), SIGNAL(clicked()), this, SLOT(processConversion()));
}

void MainWindow::toOutput(list<vector<string>>& content, stringstream& outputSs)
{

    for(vector<string>& vLine : content)
    {
        if(!vLine[2].empty())
            outputSs << ("\t/* " + vLine[2] + " */ ");
        outputSs << ("\t" + vLine[0] + " " + vLine[1]);

        //Contenu additionnel (ex : pour un tableau ex : "[4]" ou encore pour des bits ex :": 1")
        //à défaut, le ';' au moins
        if(vLine.size() > 3)
            outputSs << vLine[3];

        outputSs << endl;
    }
}

void MainWindow::addSymbols(list<vector<string>>& content)
{
    bool inSymbolUnion = false, inSymbolStruct = false;
    string lastAddressOffset;
    int index = 0;

    map<int, string> toBeAdded;

    for(vector<string>& vLine : content)
    {

        //On trouve une union (deux adresses d'offset successives égales)
        if(!lastAddressOffset.empty() && lastAddressOffset == vLine[2] && !inSymbolUnion)
        {
            inSymbolUnion = true;

            //On ajoute à la ligne courante, + le nombre de lignes déjà rajoutées (pour éviter le décalage)
            toBeAdded[index - 1 + toBeAdded.size()] = "union";
            toBeAdded[index - 1 + toBeAdded.size()] = "{";
        }
        else if((lastAddressOffset.empty() || lastAddressOffset != vLine[2]) && inSymbolUnion)
        {
            inSymbolUnion = false;
            toBeAdded[index + toBeAdded.size()] = "};";
        }

        lastAddressOffset = vLine[2];


        //Cas d'une portion d'octet (avec les ':')
        if(vLine.size() > 3 && vLine[3][0] == ':' && !inSymbolStruct)
        {
            inSymbolStruct = true;

            toBeAdded[index + toBeAdded.size()] = "struct";
            toBeAdded[index + toBeAdded.size()] = "{";
        }
        else if(vLine[3][0] != ':' && inSymbolStruct)
        {
            inSymbolStruct = false;

            toBeAdded[index + toBeAdded.size()] = "};\n";
        }

        index++;
    }

    for(auto it = toBeAdded.begin(); it != toBeAdded.end(); it++)
    {
        unsigned int i = 0;
        for(auto itContent = content.begin(); itContent != content.end(); i++, itContent++)
        {
            if(it->first == i)
            {
                vector<string> vectS;
                vectS.push_back(it->second);
                vectS.push_back("");
                vectS.push_back("");
                content.insert(itContent, vectS);
            }
        }
    }
}

void MainWindow::processConversion()
{
    string strToConvert = m_qTextToConvert->toPlainText().toUtf8().constData();

    string line, structName;
    stringstream ss(strToConvert);
    stringstream outputSs;
    list<vector<string>> content;
    bool resultGetLine;

    //First line conversion here, with named module and named struct
    do {
        resultGetLine = getline(ss, line);
    }while(line.empty() && resultGetLine);

    const vector<string> v = WinDbgParser::getInstance().parseFirstLine(line);

    //Let's setup the string with the module and the struct name
    outputSs << ("//module name : " + v[0] + "\nstruct " + v[1] + "\n{");

    structName = v[1];

    vector<string> vLine;
    while(getline(ss, line))
    {
        vLine = WinDbgParser::getInstance().parseContentLine(vLine, line);
        content.push_back(vLine);
    }

    addSymbols(content);

    toOutput(content, outputSs);

    outputSs << "\n};";

    //Ajout du typedef si l'on a bien une structure de nom commençant par '_'
    if(structName.find_first_of('_') == 0)
        outputSs << ("\ntypedef struct " + structName + " " + structName.substr(1) + ";");

    m_qTextConverted->setText(QString(outputSs.str().c_str()));
}
