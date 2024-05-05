#ifndef LEXITEMDIALOG_H
#define LEXITEMDIALOG_H

#include <QDialog>
#include "nfa.hpp"
#include "dfa.hpp"

namespace Ui {
class LexItemDialog;
}

class LexItemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LexItemDialog(QWidget *parent = nullptr, QString lex = "状态转换图");
    ~LexItemDialog();

private:
    Ui::LexItemDialog *ui;

    QString lex;

    void init();
    // 生成NFA图
    void generateNfaTable(Nfa&);
    // 生成DFA图
    void generateDfaTable(Dfa&);
    // 生成MDFA图
    void generateMDfaTable();
};

#endif // LEXITEMDIALOG_H
