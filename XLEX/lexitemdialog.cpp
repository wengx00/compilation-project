#include "lexitemdialog.h"
#include "ui_lexitemdialog.h"
#include "dfa.hpp"
#include "nfa.hpp"
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
    std::string lex = this->lex.toStdString();
    Nfa nfa(lex);
    Dfa dfa(nfa);
    MDfa mdfa(dfa);

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
                col += 1;
            }
        }
    }
}
