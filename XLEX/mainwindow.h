/*
 * @Author: 翁行
 * @Date: 2024-05-07 13:07:22
 * Copyright 2024 (c) 翁行, All Rights Reserved.
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_importFileAction_clicked();

    void on_parseFileAction_clicked();

private:
    Ui::MainWindow* ui;
    std::map<std::string, std::string> reservedMap;

    // 当前的正则表达式
    std::string regex;
    QMap<std::string, std::string> reserved;
    QMap<std::string, std::string> op;
    std::string identifier;
    std::string number;
    std::string letter;
    std::string digit;
    std::string comment;

    // 初始化UI
    void init();
};
#endif // MAINWINDOW_H
