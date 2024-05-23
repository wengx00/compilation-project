/*
 * @Author: 20212131001 翁行
 * @Date: 2024-05-23 13:19:39
 * @LastEditTime: 2024-05-23 13:48:41
 * @FilePath: /LR_SLR/mainwindow.h
 * @Description: QT主窗口头文件
 * Copyright (c) 2024 by wengx00, All Rights Reserved.
 *
 * 版本历史详见.cpp文件
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include "grammer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_toParseGrammer_clicked();

    void on_chooseFile_clicked();

    void on_saveFile_clicked();

    void on_toParseStatement_clicked();

    void on_importLex_clicked();

private:
    Ui::MainWindow* ui;
    void renderBasicInfo();
    void renderDfaTable();
    void renderSlrTable();
    void traverseTree(TreeNode*, QTreeWidgetItem*);
    Grammer* currentGrammer;
};
#endif // MAINWINDOW_H
