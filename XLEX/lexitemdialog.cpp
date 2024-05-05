#include "lexitemdialog.h"
#include "./ui_lexitemdialog.h"
#include <queue>
#include "mdfa.hpp"

LexItemDialog::LexItemDialog(QWidget* parent, QString lex) :
    QDialog(parent),
    ui(new Ui::LexItemDialog),
    lex(lex) {
    ui->setupUi(this);
    this->setWindowTitle(lex);
    init();
}

LexItemDialog::~LexItemDialog() {
    delete ui;
}

void LexItemDialog::init() {
    Nfa nfa(lex.toStdString());
    Dfa dfa(nfa);
    MDfa mdfa(dfa);
    this->generateNfaTable(nfa);
    this->generateDfaTable(dfa);
    this->generateMDfaTable();
}

void LexItemDialog::generateNfaTable(Nfa& nfa) {
    NfaGraph nfaGraph = nfa.getGraph();

    std::set<char> symbols = nfa.getSymbols();
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
    
    QTableWidget *nfaTable = ui->nfaTable;

    nfaTable->setColumnCount(symbols.size() + 1);
    nfaTable->setRowCount(transfers.size());
    // 设置nfaTable Header
    QStringList header;
    header << "状态";
    for (char symbol : symbols) {
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

void LexItemDialog::generateDfaTable(Dfa& dfa) {
    std::vector<DfaNode*> nodes = dfa.getNodes();
    std::set<char> symbols = dfa.getNfa().getSymbols();
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

}
