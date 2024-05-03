#ifndef LEXITEMDIALOG_H
#define LEXITEMDIALOG_H

#include <QDialog>

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
};

#endif // LEXITEMDIALOG_H
