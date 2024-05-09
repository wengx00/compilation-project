/*
 * @Author: 翁行
 * @Date: 2024-05-09 23:01:36
 * Copyright 2024 (c) 翁行, All Rights Reserved.
 */
#ifndef CODEPREVIEWER_H
#define CODEPREVIEWER_H

#include <QDialog>

namespace Ui {
    class CodePreviewer;
}

class CodePreviewer : public QDialog {
    Q_OBJECT

public:
    explicit CodePreviewer(QString code, QWidget* parent = nullptr);
    ~CodePreviewer();

private slots:
    void on_saveCode_clicked();

private:
    Ui::CodePreviewer* ui;

    QString code;
};

#endif // CODEPREVIEWER_H
