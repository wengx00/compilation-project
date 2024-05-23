/*
 * @Author: 翁行
 * @Date: 2024-05-07 13:14:38
 * @LastEditTime: 2024-05-23 23:03:46
 * @FilePath: /XLEX/lexitemdialog.cpp
 * @Description: 生成NFA、DFA、MDFA表的UI
 * Copyright 2024 (c) 翁行, All Rights Reserved.
 */

#include "lexitemdialog.h"
#include "codepreviewer.h"
#include "./ui_lexitemdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <sstream>
#include <fstream>

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

// 读取YAML输入->YAML解析->NFA->DFA->MDFA
void LexItemDialog::init(YAML::Node& doc) {
    // 所有保留字
    for (auto it = doc["RESERVED"].begin(); it != doc["RESERVED"].end(); ++it) {
        for (auto item : it->second) {
            reserved[item.as<std::string>()] = it->first.as<std::string>();
        }
    }
    // 所有OP
    for (auto it = doc["OP"].begin(); it != doc["OP"].end(); ++it) {
        op[it->second.as<std::string>()] = it->first.as<std::string>();
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
    for (auto it = op.begin(); it != op.end(); ++it) {
        tokens.push_back(it->first);
    }
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

    // map里的key要把转译字符删掉
    std::map<std::string, std::string> reserved;
    std::map<std::string, std::string> op;
    for (auto& it : this->reserved) {
        reserved[_removeEscape(it.first)] = it.second;
    }
    for (auto& it : this->op) {
        op[_removeEscape(it.first)] = it.second;
    }
    this->reserved = reserved;
    this->op = op;
}

// 渲染NFA表
void LexItemDialog::generateNfaTable() {
    NfaGraph nfaGraph = nfa->getGraph();

    std::set<char> symbols = nfa->getSymbols();
    symbols.insert(EPSILON);
    std::vector<std::map<char, std::string>> transfers(nfaGraph.end->state + 1);
    std::vector<int> visited(nfaGraph.end->state + 1);
    std::queue<NfaNode*> queue;

    // BFS生成一张二维表
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
    nfaTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);


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

// 生成DFA表
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
    dfaTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    for (int i = 0; i < nodes.size(); ++i) {
        DfaNode* cur = nodes[i];
        // 状态编号
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

// 生成最小化DFA表
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
    mdfaTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    for (int i = 0; i < nodes.size(); ++i) {
        MDfaNode* cur = nodes[i];
        // 状态编号
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

// 代码生成
QString LexItemDialog::codeGenerate() {
    std::vector<MDfaNode*> nodes = mdfa->getNodes();
    QString code;
    // 库文件
    code +=
        "#include <iostream>\n"
        "#include <string>\n"
        "#include <cstring>\n"
        "#include <vector>\n"
        "#include <map>\n"
        "#include <sstream>\n"
        "#include <fstream>\n\n";
    // namespace
    code += "using namespace std;\n\n";

    // RESERVED 关键字
    code += "map<string, string> reserved = {\n";
    for (auto& it : reserved) {
        code += "\t{\"" + it.first + "\", \"" + it.second + "\"},\n";
    }
    code += "};\n";

    // OP 运算符
    code += "map<string, string> op = {\n";
    for (auto& it : op) {
        code += "\t{\"" + it.first + "\", \"" + it.second + "\"},\n";
    }
    code += "};\n";

    // 工具方法：是否数字Token
    code +=
        "bool isNumber(string token) {\n"
        "\tif (token.size() < 1) return false;\n"
        "\tif (token[0] >= '0' && token[0] <= '9') return true;\n"
        "\treturn false;\n"
        "}\n";

    // 工具方法：是否注释Token
    code +=
        "bool isComment(string token) {\n"
        "\tif (token.size() < 2) return false;\n"
        "\tif (token[0] >= 'a' && token[0] <= 'z' || token[0] >= 'A' && token[0] <= 'Z') return false;\n"
        "\treturn true;\n"
        "}\n";

    // 工具方法：string转小写
    code +=
        "string toLowerCase(string token) {\n"
        "\tstring ret = \"\";\n"
        "\tfor (auto c : token) ret += tolower(c);\n"
        "\treturn ret;\n"
        "}\n";

    // 处理Token的方法
    code +=
        "void handleToken(string token, ofstream& os) {\n"
        "\tif (token.size() == 0) return;\n"
        "\tstring label;\n"
        "\tif (reserved.count(token)) label = reserved[token];\n"
        "\telse if (op.count(token)) label = op[token];\n"
        "\telse if (isNumber(token)) label = \"NUMBER\";\n"
        "\telse if (isComment(token)) label = \"COMMENT\";\n"
        "\telse label = \"IDENTIFIER\";\n"
        "\tcout << label << \" : \" << token << '\\n';\n"
        "\tos << label << \" : \" << token << '\\n';\n"
        "}\n";

    // 主函数头
    code += "int main(int argc, char* argv[]) {\n";
    // 入参校验
    code +=
        "\tif (argc != 3) {\n"
        "\t\t cout << \"Error: Invalid input. Require input file path on agrv[1] and output file path on agrv[2]. \" << '\\n';\n"
        "\t\t return 1;\n"
        "\t}\n";
    // 源代码文件path
    code += "\tstring path = argv[1];\n";
    // 输出文件path
    code += "\tstring outputPath = argv[2];\n";
    // 打开文件
    code +=
        "\tifstream ifs(path);\n"
        "\tofstream os(outputPath);\n"
        "\tif (!ifs || !ifs.is_open()) {\n"
        "\t\tcout << \"Error: Cannot open input file. \" << '\\n';\n"
        "\t\treturn 1;\n"
        "\t}\n"
        "\tif (!os || !os.is_open()) {\n"
        "\t\tcout << \"Error: Cannot open output file. \" << '\\n';\n"
        "\t\treturn 1;\n"
        "\t}\n";
    // 读取源代码
    code +=
        "\tstringstream ss;\n"
        "\tss << ifs.rdbuf();\n"
        "\tstring code = ss.str();\n"
        "\tstring token = \"\";\n"
        "\tss.clear();\n"
        "\tss.str(\"\");\n";
    // 初始状态
    code += "\tint currentState = 0;\n";
    // 循环遍历
    code +=
        "\tfor (int i = 0; i < code.size(); ++i) {\n"
        "\t\tchar id = code[i];\n"
        "\t\tswitch(currentState) {\n";
    for (MDfaNode* node : nodes) {
        code +=
            "\t\t\tcase " + QString::number(node->state) + ":\n"
            "\t\t\t\tswitch (id) {\n";
        for (auto& p : node->transfer) {
            if (p.first == ANY) continue;
            code +=
                "\t\t\t\t\tcase '" + QString(p.first) + "':\n"
                "\t\t\t\t\t\tcurrentState = " + QString::number(p.second) + ";\n"
                "\t\t\t\t\t\ttoken += id;\n"
                "\t\t\t\t\t\tbreak;\n";
        }
        if (node->transfer.count(ANY)) {
            code +=
                "\t\t\t\t\tdefault:\n"
                "\t\t\t\t\tif (id == '\\n') {\n"
                "\t\t\t\t\t\tif (token.size() > 0) {\n"
                "\t\t\t\t\t\t\thandleToken(token, os);\n"
                "\t\t\t\t\t\t\ttoken = \"\";\n"
                "\t\t\t\t\t\t}\n"
                "\t\t\t\t\t\tcurrentState = 0;\n"
                "\t\t\t\t\t}\n"
                "\t\t\t\t\telse {\n"
                "\t\t\t\t\tcurrentState = " + QString::number(node->transfer[ANY]) + ";\n"
                "\t\t\t\t\ttoken += id;\n"
                "\t\t\t\t\t}\n"
                "\t\t\t\t\tbreak;\n";
        }
        else {
            if (node->isEnd) {
                // 拿到一个分词，重新开始
                code +=
                    "\t\t\t\t\tdefault:\n"
                    "\t\t\t\t\t\tif (token.size() > 0) {\n"
                    "\t\t\t\t\t\t\thandleToken(token, os);\n"
                    "\t\t\t\t\t\t\ttoken = \"\";\n"
                    "\t\t\t\t\t\t\tif (id != '\\n' && id != ' ' && id != '\\t') {\n"
                    "\t\t\t\t\t\t\t\ti--;\n"
                    "\t\t\t\t\t\t\t}\n"
                    "\t\t\t\t\t\t}\n"
                    "\t\t\t\t\t\tcurrentState = 0;\n";
            }
            else {
                // 其他情况为错误情形
                code +=
                    "\t\t\t\t\tdefault:\n"
                    "\t\t\t\t\tif (id == '\\n' || id == ' ' || id == '\\t') {\n"
                    "\t\t\t\t\t\tif (token.size() == 0) {\n"
                    "\t\t\t\t\t\t\tbreak;\n"
                    "\t\t\t\t\t\t}\n"
                    "\t\t\t\t\t}\n"
                    "\t\t\t\t\t\tos << \"Error: Invalid input character. \" << '\\n';\n"
                    "\t\t\t\t\t\tcout << \"Error: Invalid input character. \" << '\\n';\n"
                    "\t\t\t\t\t\treturn 1;\n";
            }
        }
        code +=
            "\t\t\t\t}\n"
            "\t\t\tbreak;\n";
    }
    code +=
        "\t\t}\n"
        "\t}\n";

    // 读取完毕，根据最终状态取到最后的分词
    code += "\tswitch(currentState) {\n";
    for (auto node : nodes) {
        if (node->isEnd) {
            code +=
                "\t\tcase " + QString::number(node->state) + ":\n";
        }
    }
    code +=
        "\t\t\tif (token.size() > 0) {\n"
        "\t\t\t\thandleToken(token, os);\n"
        "\t\t\t}\n"
        "\t\t\tbreak;\n";
    // 其他情况为错误情形
    code +=
        "\t\tdefault:\n"
        "\t\t\tcout << \"Error: Invalid input. \" << '\\n';\n"
        "\t\t\treturn 1;"
        "\t}\n";
    // 主函数尾
    code +=
        "\tcout << \"Success.\" << '\\n';\n"
        "\treturn 0;\n"
        "}\n";
    return code;
}

// 触发生成代码
void LexItemDialog::on_codeGenerate_clicked() {
    QString code = codeGenerate();
    CodePreviewer* codePreviewer = new CodePreviewer(code, this);
    codePreviewer->show();
}

