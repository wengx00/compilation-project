/*
 * @Author: 翁行
 * @Date: 2024-05-07 13:07:19
 * Copyright 2024 (c) 翁行, All Rights Reserved.
 */
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QFileDialog"
#include "QMessageBox"
#include "lexitemdialog.h"
#include <sstream>
#include <yaml-cpp/yaml.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    init();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::init() {

}

void MainWindow::on_importFileAction_clicked() {
    // 获取文件名
    QString fileName{ QFileDialog::getOpenFileName(
        this, "打开源文件", ".", "文本文件(*.txt, *)") };
    // 构造文件对象
    QFile file{ fileName };

    if (fileName.isEmpty())
        return;

    // 读取全部内容
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->lexEditor->setText(file.readAll());
        QMessageBox::information(this, "提示", "读取成功");
        file.close();
    }

    // 文件打开失败
    else
        QMessageBox::information(this, "提示", "读取失败");
}

/**
 * YAML 文件的定义如下：
 * 需要包含 identifier、number、reserved、letter、digit、comment、op
 * reserved、op 块需要为 key-value 对
 * letter、digit 为 string 数组
 * 其他块都为 string 的 value
*/
void MainWindow::on_parseFileAction_clicked() {
    std::string str = ui->lexEditor->toPlainText().toStdString();
    // 替换空格
    std::string preload = _replaceAll(str, " | ", "|");
    YAML::Node doc;
    // YAML 文件可能在解析时抛出错误
    try {
        doc = YAML::Load(preload);
    }
    catch (const std::exception& e) {
        qDebug("YAML::Exception: %s", e.what());
        QMessageBox::warning(this, "警告", "YAML 文件解析错误");
        return;
    }
    if (!doc.IsMap()) {
        QMessageBox::warning(this, "警告", "YAML 文件不是Key-Value对");
        return;
    }
    QVector<QString> requiredKey = {
        "RESERVED",
        "OP",
        "LETTER",
        "DIGIT",
        "NUMBER",
        "IDENTIFIER",
        "COMMENT"
    };

    // YAML 文件合法性检查
    for (QString key : requiredKey) {
        YAML::Node value = doc[key.toStdString()];
        if (!value) {
            QMessageBox::warning(this, "警告", "缺少 " + key + " 标识");
            return;
        }
        if (key == "RESERVED" || key == "OP") {
            if (!value.IsMap()) {
                QMessageBox::warning(this, "警告", key + " 标识不是Key-Value对");
                return;
            }
        }
        else if (key == "LETTER" || key == "DIGIT") {
            if (!value.IsSequence()) {
                QMessageBox::warning(this, "警告", key + " 标识不是string数组");
                return;
            }
        }
        else {
            if (!value.IsScalar()) {
                QMessageBox::warning(this, "警告", key + " 标识不是string");
                return;
            }
        }
    }

    LexItemDialog* dialog = new LexItemDialog(doc, this);
    dialog->show();
}

