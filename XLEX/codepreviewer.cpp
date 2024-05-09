/*
 * @Author: 翁行
 * @Date: 2024-05-09 23:13:07
 * Copyright 2024 (c) 翁行, All Rights Reserved.
 */
#include "codepreviewer.h"
#include "ui_codepreviewer.h"
#include <QFileDialog>
#include <QMessageBox>

CodePreviewer::CodePreviewer(QString code, QWidget* parent) :
    QDialog(parent),
    code(code),
    ui(new Ui::CodePreviewer) {
    ui->setupUi(this);
    ui->previewer->setPlainText(code);
}

CodePreviewer::~CodePreviewer() {
    delete ui;
}

void CodePreviewer::on_saveCode_clicked() {
    QString filename = QFileDialog::getSaveFileName(this, "保存文件", ".", "C++源文件(*.cpp)");
    QFile file{ filename };
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out{ &file };
        out << code;
        file.close();
        QMessageBox::information(this, "提示", "生成成功");
        return;
    }

    QMessageBox::information(this, "提示", "文件保存失败");
}

