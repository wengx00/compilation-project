/*
 * @Author: 翁行
 * @Date: 2024-05-02 22:43:26
 * @LastEditTime: 2024-05-23 21:58:56
 * @FilePath: /XLEX/main.cpp
 * @Description: 入口文件
 * Copyright 2024 (c) 翁行, All Rights Reserved.
 */

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
