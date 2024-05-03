#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "QFileDialog"
#include "QMessageBox"
#include "lexitemdialog.h"

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
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

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


void MainWindow::on_parseFileAction_clicked() {
    QString* str = new QString(ui->lexEditor->toPlainText());
    // 根据换行分割
    QStringList list = str->split('\n');
    // 分别生成 Table Column
    QTableWidget* table = ui->tableWidget;
    table->setRowCount(list.size());
    // 列: 序号、正则表达式、
    for (int i = 0; i < list.length(); ++i) {
        QTableWidgetItem* exp = new QTableWidgetItem(list[i]);

        QWidget* actions = new QWidget();
        QLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(5, 5, 5, 5);
        actions->setLayout(layout);

        QPushButton* nfaAction = new QPushButton("NFA->DFA->MDFA");
        nfaAction->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        connect(nfaAction, &QPushButton::clicked, this, [this, i]() {
            this->handleTableItemAction(i);
        });

        layout->addWidget(nfaAction);

        table->setItem(i, 0, exp);
        table->setCellWidget(i, 1, actions);
    }
    this->regexList = list;
}

void MainWindow::handleTableItemAction(int index) {
    QString regex = this->regexList[index];
    qDebug("regex: %s", regex.toStdString().c_str());
    LexItemDialog* dialog = new LexItemDialog(this, regex);
    dialog->show();
}