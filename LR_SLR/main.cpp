/*
 * @Author: 20212131001 翁行
 * @Date: 2023-12-16 13:50:54
 * @LastEditTime: 2024-05-23 13:35:51
 * @FilePath: /LR_SLR/main.cpp
 * @Description: 入口函数
 * Copyright (c) 2024 by wengx00, All Rights Reserved.
 */
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    w.setWindowTitle("LR_SLR (c) 20212131001");
    return a.exec();
}
