/*
 * @Author: 翁行
 * @Date: 2024-05-07 13:14:38
 * Copyright 2024 (c) 翁行, All Rights Reserved.
 */
#include "lexitemdialog.h"
#include "./ui_lexitemdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <sstream>

LexItemDialog::LexItemDialog(YAML::Node& doc, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::LexItemDialog) {
    ui->setupUi(this);
    this->setWindowTitle("状态转换图");
    init(doc);
}

LexItemDialog::~LexItemDialog() {
    delete ui;
    if (nfa) delete nfa;
    if (dfa) delete dfa;
    if (mdfa) delete mdfa;
}

void LexItemDialog::init(YAML::Node& doc) {
    // 所有保留字
    for (auto it = doc["RESERVED"].begin(); it != doc["RESERVED"].end(); ++it) {
        reserved[it->first.as<std::string>()] = it->second.as<std::string>();
    }
    // 所有OP
    for (auto it = doc["OP"].begin(); it != doc["OP"].end(); ++it) {
        op[it->first.as<std::string>()] = it->second.as<std::string>();
    }
    digit = "";
    for (int i = 0; i < doc["DIGIT"].size(); ++i) {
        digit += doc["DIGIT"][i].as<std::string>();
        if (i != doc["DIGIT"].size() - 1) digit += "|";
    }
    letter = "";
    for (int i = 0; i < doc["LETTER"].size(); ++i) {
        letter += doc["LETTER"][i].as<std::string>();
        if (i != doc["LETTER"].size() - 1) letter += "|";
    }

    qDebug("[LETTER] %s", letter.c_str());
    qDebug("[DIGIT] %s", digit.c_str());

    identifier = doc["IDENTIFIER"].as<std::string>();
    number = doc["NUMBER"].as<std::string>();
    comment = doc["COMMENT"].as<std::string>();


    // 替换 identifier 里的 digit、number 和 letter
    _replaceAll(number, "DIGIT", "(" + digit + ")");
    _replaceAll(identifier, "NUMBER", number);
    _replaceAll(identifier, "LETTER", "(" + letter + ")");
    _replaceAll(identifier, "DIGIT", "(" + digit + ")");

    qDebug("[IDENTIFIER] %s", identifier.c_str());
    qDebug("[NUMBER] %s", number.c_str());
    qDebug("[COMMENT] %s", comment.c_str());

    // 拼接总的正则表达式
    std::vector<std::string> tokens;
    if (identifier.size() > 0) {
        tokens.push_back(identifier);
    }
    if (number.size() > 0) {
        tokens.push_back(number);
    }
    if (comment.size() > 0) {
        tokens.push_back(comment);
    }
    regex = "";
    for (int it = 0; it < tokens.size(); ++it) {
        regex += tokens[it];
        if (it != tokens.size() - 1) regex += "|";
    }

    qDebug("[REGEX] %s", regex.c_str());

    nfa = new Nfa(regex);
    dfa = new Dfa(*nfa);
    mdfa = new MDfa(*dfa);

    // NFA -> DFA -> MDFA
    this->generateNfaTable();
    this->generateDfaTable();
    this->generateMDfaTable();
}

void LexItemDialog::generateNfaTable() {
    NfaGraph nfaGraph = nfa->getGraph();

    std::set<char> symbols = nfa->getSymbols();
    symbols.insert(EPSILON);
    std::vector<std::map<char, std::string>> transfers(nfaGraph.end->state + 1);
    std::vector<int> visited(nfaGraph.end->state + 1);
    std::queue<NfaNode*> queue;

    queue.push(nfaGraph.start);
    while (queue.size()) {
        NfaNode* cur = queue.front();
        queue.pop();
        if (visited[cur->state]) {
            continue;
        }
        visited[cur->state] = 1;
        for (auto& p : cur->transfers) {
            std::string target = "";
            for (NfaNode* next : p.second) {
                if (!visited[next->state]) {
                    queue.push(next);
                    target += to_string(next->state) + ", ";
                }
            }
            target = target.substr(0, target.size() - 2);
            transfers[cur->state][p.first] = target;
        }
    }

    QTableWidget* nfaTable = ui->nfaTable;

    nfaTable->setColumnCount(symbols.size() + 1);
    nfaTable->setRowCount(transfers.size());
    // 设置nfaTable Header
    QStringList header;
    header << "状态";
    for (char symbol : symbols) {
        if (symbol == EPSILON) {
            header << "EPSILON";
            continue;
        }
        header << QString(1, symbol);
    }
    nfaTable->setHorizontalHeaderLabels(header);
    nfaTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 设置nfaTable Content
    for (int i = 0; i < transfers.size(); ++i) {
        nfaTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        int col = 1;
        for (char symbol : symbols) {
            if (transfers[i].count(symbol)) {
                nfaTable->setItem(i, col, new QTableWidgetItem(QString(transfers[i][symbol].c_str())));
            }
            col += 1;
        }
    }
}

void LexItemDialog::generateDfaTable() {
    std::vector<DfaNode*> nodes = dfa->getNodes();
    std::set<char> symbols = nfa->getSymbols();
    QTableWidget* dfaTable = ui->dfaTable;
    dfaTable->setColumnCount(symbols.size() + 1);
    dfaTable->setRowCount(nodes.size());

    QStringList header;
    header << "状态";
    for (char symbol : symbols) {
        header << QString(1, symbol);
    }
    dfaTable->setHorizontalHeaderLabels(header);
    dfaTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int i = 0; i < nodes.size(); ++i) {
        DfaNode* cur = nodes[i];
        dfaTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        int col = 1;
        for (char symbol : symbols) {
            if (cur->transfers.count(symbol)) {
                dfaTable->setItem(i, col, new QTableWidgetItem(QString::number(cur->transfers[symbol])));
            }
            col++;
        }
    }
}

void LexItemDialog::generateMDfaTable() {
    std::set<char> symbols = nfa->getSymbols();
    std::vector<MDfaNode*> nodes = mdfa->getNodes();

    QTableWidget* mdfaTable = ui->mdfaTable;
    mdfaTable->setColumnCount(symbols.size() + 1);
    mdfaTable->setRowCount(nodes.size());

    QStringList header;
    header << "状态";
    for (char symbol : symbols) {
        header << QString(1, symbol);
    }
    mdfaTable->setHorizontalHeaderLabels(header);
    mdfaTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int i = 0; i < nodes.size(); ++i) {
        MDfaNode* cur = nodes[i];
        mdfaTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        int col = 1;
        for (char symbol : symbols) {
            if (cur->transfer.count(symbol)) {
                mdfaTable->setItem(i, col, new QTableWidgetItem(QString::number(cur->transfer[symbol])));
            }
            col++;
        }
    }
}

void LexItemDialog::on_codeGenerate_clicked() {
    QString filename = QFileDialog::getSaveFileName(this, "保存文件", ".", "C++源文件(*.cpp)");
    QFile file{ filename };
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out{ &file };
        out << QString::fromStdString(mdfa->lex());
        file.close();
        QMessageBox::information(this, "提示", "生成成功");
        return;
    }

    QMessageBox::information(this, "提示", "文件保存失败");
}

