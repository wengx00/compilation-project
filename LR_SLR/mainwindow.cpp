/*
 * @Author: wengx00 wengx86@163.com
 * @Date: 2023-12-17 00:49:57
 * Copyright (c) 2024 by wengx00, All Rights Reserved.
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), currentGrammer(nullptr) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
    if (currentGrammer) delete currentGrammer;
}

void MainWindow::renderBasicInfo() {
    if (!currentGrammer) {
        QMessageBox::information(this, "提示", "请先点击解析文法");
        return;
    }
    Grammer& grammer = *currentGrammer;
    QString error = QString::fromStdString(grammer.getError());
    if (error.isEmpty()) error = "未发现错误";
    ui->syntaxError->setPlainText(error);
    QString followSet, firstSet;
    // 渲染非终结节点的Follow集合和First集合
    std::set<std::string> notEnd = grammer.getNotEnd();
    for (std::string token : notEnd) {
        firstSet += token + ": ";
        followSet += token + ": ";
        std::set<std::string> firstOfToken = grammer.getFirst(token);
        std::set<std::string> followOfToken = grammer.getFollow(token);
        for (auto it = firstOfToken.begin(); it != firstOfToken.end();) {
            firstSet += *it;
            if (++it != firstOfToken.end()) firstSet += ", ";
        }
        for (auto it = followOfToken.begin(); it != followOfToken.end();) {
            followSet += *it;
            if (++it != followOfToken.end()) followSet += ", ";
        }
        firstSet += "\n";
        followSet += "\n";
    }
    ui->firstSet->setPlainText(firstSet);
    ui->followSet->setPlainText(followSet);

    // 渲染拓广文法
    QString extraGrammer = QString::fromStdString(grammer.getExtraGrammer());
    ui->extraGrammer->setPlainText(extraGrammer);
}

void MainWindow::renderDfaTable() {
    if (!currentGrammer) {
        QMessageBox::information(this, "提示", "请先点击解析文法");
        return;
    }
    Grammer& grammer = *currentGrammer;
    std::vector<std::vector<Item> > dfa = grammer.getDfa();
    std::set<std::string> endSet = grammer.getEnd();
    std::set<std::string> notEndSet = grammer.getNotEnd();
    std::string startToken = grammer.getStart();
    auto* table = ui->dfa;
    table->setColumnCount(1 + endSet.size() + notEndSet.size());
    table->setRowCount(dfa.size());
    QStringList header;
    header << "状态" << "状态内文法";
    for (auto& token : notEndSet) {
        if (token == startToken) continue;
        header << QString::fromStdString(token);
    }
    for (auto& token : endSet) {
        header << QString::fromStdString(token);
    }
    table->setHorizontalHeaderLabels(header);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    // 文法
    auto formula = grammer.getFormula();
    // 迭代生成cell
    for (int state = 0; state < (int)dfa.size(); ++state) {
        QTableWidgetItem* id = new QTableWidgetItem(); // 状态编号
        QTableWidgetItem* inner = new QTableWidgetItem(); // 状态内文法

        id->setText(QString::fromStdString(std::to_string(state)));
        table->setItem(state, 0, id); // 加入编号列

        QString innerText;
        for (int offset = 0; offset < (int)dfa[state].size(); ++offset) {
            // 遍历节点内部
            Item& cur = dfa[state][offset];
            std::vector<std::string> rawOfCur = formula[cur.key][cur.rawsIndex]; // 那一行文法
            innerText += QString::fromStdString(cur.key) + " -> ";
            for (int tokenOffset = 0; tokenOffset <= (int)rawOfCur.size(); ++tokenOffset) {
                // 构造类似A -> (.a)
                if (tokenOffset == cur.rawIndex) innerText += ".";
                innerText += QString::fromStdString(rawOfCur[tokenOffset]);
            }
            innerText += "\n";
        }
        inner->setText(innerText);
        table->setItem(state, 1, inner);

        int col = 2;
        for (auto& token : notEndSet) {
            if (token == startToken) continue;
            int target = grammer.forward(state, token);
            if (target >= 0) {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setText(QString::fromStdString(std::to_string(target)));
                table->setItem(state, col, item);
            }
            col++;
        }
        for (auto& token : endSet) {
            int target = grammer.forward(state, token);
            if (target >= 0) {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setText(QString::fromStdString(std::to_string(target)));
                table->setItem(state, col, item);
            }
            col++;
        }
    }

}


void MainWindow::renderSlrTable() {
    Grammer& grammer = *currentGrammer;
    std::set<std::string> endSet = grammer.getEnd();
    std::set<std::string> notEndSet = grammer.getNotEnd();
    std::string startToken = grammer.getStart();
    auto dfa = grammer.getDfa();
    auto formula = grammer.getFormula();
    endSet.insert(END_FLAG);

    auto* table = ui->slr;
    table->setColumnCount(endSet.size() + notEndSet.size());
    table->setRowCount(dfa.size());
    QStringList header;
    header << "状态";
    for (auto& token : notEndSet) {
        if (token == startToken) continue;
        header << QString::fromStdString(token);
    }
    for (auto& token : endSet) {
        header << QString::fromStdString(token);
    }
    table->setHorizontalHeaderLabels(header);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    // Cell
    for (int state = 0; state < (int)dfa.size(); ++state) {
        QTableWidgetItem* id = new QTableWidgetItem(); // 状态编号
        id->setText(QString::number(state));
        table->setItem(state, 0, id);

        int column = 1;
        for (auto& token : notEndSet) {
            if (token == startToken) continue;
            int target = grammer.forward(state, token);
            if (target > -1) {
                QTableWidgetItem* end = new QTableWidgetItem(); // 状态编号
                end->setText("s" + QString::number(target));
                table->setItem(state, column, end);
            }
            else if ((target = grammer.backward(state, token)) > -1) {
                QTableWidgetItem* end = new QTableWidgetItem(); // 状态编号
                Item& node = dfa[state][target];
                if (node.key == startToken) {
                    end->setText("ACCEPT");
                }
                else {
                    QString endText = "r(" + QString::fromStdString(node.key) + "->";
                    for (auto& token : formula[node.key][node.rawsIndex]) {
                        endText += QString::fromStdString(token);
                    }
                    endText += ")";
                    end->setText(endText);
                }
                table->setItem(state, column, end);
            }
            column++;
        }
        for (auto& token : endSet) {
            int target = grammer.forward(state, token);
            if (target > -1) {
                QTableWidgetItem* end = new QTableWidgetItem(); // 状态编号
                end->setText("s" + QString::number(target));
                table->setItem(state, column, end);
            }
            else if ((target = grammer.backward(state, token)) > -1) {
                QTableWidgetItem* end = new QTableWidgetItem(); // 状态编号
                Item& node = dfa[state][target];
                if (node.key == startToken) {
                    end->setText("ACCEPT");
                }
                else {
                    QString endText = "r(" + QString::fromStdString(node.key) + "->";
                    for (auto& token : formula[node.key][node.rawsIndex]) {
                        endText += QString::fromStdString(token);
                    }
                    endText += ")";
                    end->setText(endText);
                }
                table->setItem(state, column, end);
            }
            column++;
        }

    }
}

void MainWindow::on_toParseGrammer_clicked() {
    std::string grammerStr = ui->grammer->toPlainText().toStdString();
    Grammer* grammer = new Grammer(grammerStr);
    currentGrammer = grammer;
    renderBasicInfo();
    if (!grammer->bad()) {
        renderDfaTable();
        renderSlrTable();
    }
    ui->resultTab->setCurrentIndex(0);
}

void MainWindow::on_chooseFile_clicked() {
    // 获取文件名
    QString fileName{ QFileDialog::getOpenFileName(
        this, "打开源文件", ".", "文本文件(*.txt, *)") };
    // 构造文件对象
    QFile file{ fileName };

    if (fileName.isEmpty())
        return;

    // 读取全部内容
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->grammer->setText(file.readAll());
        QMessageBox::information(this, "提示", "读取成功");
        file.close();
    }

    // 文件打开失败
    else
        QMessageBox::information(this, "提示", "读取失败");
}

void MainWindow::on_saveFile_clicked() {
    QString tempPath = QDir::homePath();
    QString savePath = QFileDialog::getSaveFileName(this, tr("SaveSourceCode"), tempPath, "TEXT(*.txt)");
    if (savePath.isEmpty())
        return;
    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(this, "提示", "文件保存失败");
        return;
    }
    // 写文件
    QString content = ui->grammer->toPlainText().toUtf8();
    QTextStream out(&file);
    out << content;
    file.close();
    QMessageBox::information(this, "提示", "文件保存成功");
}

void MainWindow::on_toParseStatement_clicked() {
    QString statement = ui->statement->toPlainText();
    if (statement.isEmpty()) {
        QMessageBox::information(this, "提示", "请输入待解析语句");
        return;
    }
    if (!currentGrammer) {
        QMessageBox::information(this, "提示", "请先解析文法后再解析语句");
        return;
    }
    qDebug() << "待解析语句: " << statement;
    Grammer& grammer = *currentGrammer;
    ParsedResult result = grammer.parse(statement.toStdString());
    ui->treeWidget->clear();
    if (result.error.size() == 0) {
        traverseTree(result.root, 0);
        QMessageBox::information(this, "提示", "语法树解析成功");
    }
    else {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, QString::fromStdString(result.error));
        QMessageBox::warning(this, "提示", "语法树解析失败");
    }
    ui->resultTab->setCurrentIndex(1);
}


// 遍历树
void MainWindow::traverseTree(TreeNode* tree, QTreeWidgetItem* p) {
    if (tree == NULL) return;
    QTreeWidgetItem* item;
    if (p == 0)
        item = new QTreeWidgetItem(ui->treeWidget);
    else
        item = new QTreeWidgetItem(p);
    QString text = QString::fromStdString(tree->label);
    if (tree->value.size()) {
        text += " : " + QString::fromStdString(tree->value);
    }
    item->setText(0, text);
    item->setForeground(0, Qt::black);
    for (TreeNode* child : tree->children) {
        traverseTree(child, item);
    }
}

