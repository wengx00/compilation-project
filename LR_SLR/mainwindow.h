/*
 * @Author: wengx00 wengx86@163.com
 * @Date: 2023-12-16 21:46:02
 * Copyright (c) 2024 by wengx00, All Rights Reserved.
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
