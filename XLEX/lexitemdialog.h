/*
 * @Author: 翁行
 * @Date: 2024-05-07 13:15:04
 * Copyright 2024 (c) 翁行, All Rights Reserved.
 */
#ifndef LEXITEMDIALOG_H
#define LEXITEMDIALOG_H

#include <QDialog>
#include "nfa.hpp"
#include "dfa.hpp"
#include "mdfa.hpp"
#include <yaml-cpp/yaml.h>

namespace Ui {
    class LexItemDialog;
}

class LexItemDialog : public QDialog {
    Q_OBJECT

public:
    explicit LexItemDialog(YAML::Node& doc, QWidget* parent = nullptr);
    ~LexItemDialog();

private slots:
    void on_codeGenerate_clicked();

private:
    Ui::LexItemDialog* ui;

    std::string regex;
    std::map<std::string, std::string> reserved;
    std::map<std::string, std::string> op;
    std::string identifier;
    std::string number;
    std::string letter;
    std::string digit;
    std::string comment;

    Nfa* nfa;
    Dfa* dfa;
    MDfa* mdfa;

    void init(YAML::Node&);
    // 生成NFA图
    void generateNfaTable();
    // 生成DFA图
    void generateDfaTable();
    // 生成MDFA图
    void generateMDfaTable();

    // 代码生成
    QString codeGenerate();
};

#endif // LEXITEMDIALOG_H
