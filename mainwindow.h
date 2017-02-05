#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QPushButton>
#include <QTextEdit>
#include <QGridLayout>
#include <memory>

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow();

public slots:
    void processConversion();

private:
    void addSymbols(std::list<std::vector<std::string>>& content);
    void toOutput(std::list<std::vector<std::string>>& content, std::stringstream& outputSs);
    std::unique_ptr<QTextEdit> m_qTextToConvert;
    std::unique_ptr<QTextEdit>  m_qTextConverted;
    std::unique_ptr<QPushButton> m_bConvert;

    QGridLayout m_layout;
};

#endif // MAINWINDOW_H
