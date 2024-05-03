#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "constants.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_importFileAction_clicked();

    void on_parseFileAction_clicked();

    void handleTableItemAction(int);


private:
    Ui::MainWindow *ui;

    // 当前的正则表达式列表
    QStringList regexList;
    
    // 初始化UI
    void init();
};
#endif // MAINWINDOW_H
