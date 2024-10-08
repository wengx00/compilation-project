/*
 * @Author: 翁行
 * @Date: 2023-12-31 23:00:20
 * @LastEditTime: 2024-05-31 00:45:54
 * @FilePath: /XLEX/include/mdfa.hpp
 * @Description: DFA最小化
 * Copyright 2024 (c) 翁行, All Rights Reserved.
 */

#ifndef _MDFA_HPP
#define _MDFA_HPP

#include <iostream>
#include "dfa.hpp"
#include <cstring>
#include <string>
#include <queue>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <stack>
#include <sstream>

 // MDFA节点
struct MDfaNode {
    int state;
    bool isEnd;
    set<DfaNode*> dfaNodes; // 持有的DFA结点
    map<char, int> transfer;

    // 绑定DFA节点到MDFA中
    void bindDfaNodes(set<DfaNode*> dfaNodes) {
        this->dfaNodes = dfaNodes;
        for (DfaNode* node : dfaNodes) {
            if (node->isEnd) {
                isEnd = true;
                break;
            }
        }
    }

    MDfaNode(int state) : state(state), isEnd(false) {}

    bool operator== (const MDfaNode& node) {
        if (node.dfaNodes.size() != dfaNodes.size()) return false;
        for (DfaNode* dfaNode : node.dfaNodes)
            if (!dfaNodes.count(dfaNode)) return false;
        return true;
    }
};

// MDFA All in one
class MDfa {

private:
    Dfa& dfa;
    vector<MDfaNode*> nodes;

    void minimize() { // 最小化
        set<char> symbols = dfa.getNfa().getSymbols(); // 转移符号
        set<DfaNode*> left, right; // 两个拆分
        vector<set<DfaNode*>> completed; // 已完成拆分
        vector<set<DfaNode*>> prepared; // 待拆分
        map<int, set<DfaNode*>> destination; // 转移目的地，做暂存使用
        // 根据是否终结节点初始化
        for (DfaNode* node : dfa.getNodes()) {
            if (node->isEnd) right.insert(node);
            else left.insert(node);
        }
        completed.push_back(left);
        completed.push_back(right);
        for (const char symbol : symbols) {
            prepared = completed;
            completed.clear();
            while (prepared.size()) {
                if (prepared.empty()) break;
                destination.clear();
                set<DfaNode*> cur = prepared.front();
                if (cur.empty()) {
                    prepared.erase(prepared.begin(), prepared.begin() + 1); // 把cur出队
                    continue; // 空集合
                }
                if (cur.size() == 1) {
                    completed.push_back(cur); // 长度为1，无需拆分
                    prepared.erase(prepared.begin(), prepared.begin() + 1);
                    continue;
                }
                for (DfaNode* node : cur) {
                    if (!node->transfers.count(symbol)) { // 不存在该转移
                        destination[-1].insert(node);
                        continue;
                    }
                    int target = node->transfers[symbol]; // 下一个DFA状态
                    for (int i = 0; i < prepared.size(); ++i) {
                        bool matched = false;
                        for (DfaNode* state : prepared[i]) {
                            if (state->state == target) { // 移进的目标是这个集合
                                matched = true;
                                destination[i].insert(node);
                                break;
                            }
                        }
                        if (matched) break;
                    }
                    for (int i = 0; i < completed.size(); ++i) {
                        bool matched = false;
                        for (DfaNode* state : completed[i]) {
                            if (state->state == target) { // 移进的目标是这个集合
                                matched = true;
                                destination[prepared.size() + i].insert(node);
                                break;
                            }
                        }
                        if (matched) break;
                    }
                }
                prepared.erase(prepared.begin(), prepared.begin() + 1); // 把cur出队
                // 找内奸
                if (destination.size() > 1) { // 有内奸
                    for (auto& p : destination)
                        prepared.push_back(p.second);
                    continue;
                }
                // 没有内奸
                completed.push_back(cur);
            }
        }
        // 根据划分结果生成MDFA结点
        MDfaNode* startNode = new MDfaNode(0);
        set<DfaNode*> divideOfStartNode;
        for (set<DfaNode*> divide : completed) {
            for (DfaNode* node : divide) {
                if (node->state == 0) {
                    divideOfStartNode = divide;
                    break;
                }
            }
            if (divideOfStartNode.size()) break; // 已经找到起始划分
        }
        startNode->bindDfaNodes(divideOfStartNode); // 绑定划分
        nodes.push_back(startNode);
        for (int i = 0; i < nodes.size(); ++i) { // 生成MDFA结点
            MDfaNode* cur = nodes[i];
            for (const char symbol : symbols) {
                set<int> next = forward(cur->dfaNodes, symbol); // 获取转移目标
                if (next.empty()) continue;
                for (set<DfaNode*> divide : completed) { // 寻找和目标等价的划分
                    bool matched = false;
                    for (DfaNode* node : divide) {
                        if (next.count(node->state)) {
                            matched = true;
                            break;
                        }
                    }
                    if (!matched) continue; // 不匹配
                    MDfaNode* instance = new MDfaNode(nodes.size());
                    instance->bindDfaNodes(divide);
                    for (MDfaNode* exist : nodes) { // 寻找是否已经存在该结点
                        if (*exist == *instance) {
                            instance = exist;
                            break;
                        }
                    }
                    if (instance->state == nodes.size()) // 不存在的结点
                        nodes.push_back(instance);
                    cur->transfer[symbol] = instance->state;
                    break;
                }
            }
        }
    }

    // 以symbol步进的结果集合
    set<int> forward(set<DfaNode*> source, char symbol) {
        set<int> result;
        for (DfaNode* node : source) {
            if (!node->transfers.count(symbol)) continue; // 当前DFA结点上不存在该转移关系
            result.insert(node->transfers[symbol]);
        }
        return result;
    }

public:
    MDfa(Dfa& dfa) : dfa(dfa) {
        minimize();
    };

    // 获取MDFA节点列表
    vector<MDfaNode*> getNodes() {
        return nodes;
    }

    // Legacy：生成C++分析程序。该项目已改用其他方法
    string lex() {
        stringstream ss;
        ss << "#include <iostream>" << '\n';
        ss << "#include <string>" << '\n';
        ss << "#include <cstring>" << '\n' << '\n';
        ss << "using namespace std;" << '\n' << '\n';
        ss << "int main() {" << '\n';
        ss << "     string input;" << '\n';
        ss << "     cin >> input;" << '\n';
        ss << "     int currentState = 0;" << '\n';
        ss << "     for (int i = 0; i < input.size(); ++i) {" << '\n';
        ss << "         char id = input[i];" << '\n';
        ss << "         switch(currentState) {" << '\n';
        for (MDfaNode* node : nodes) {
            ss << "         case " << node->state << ":" << '\n';
            ss << "             switch (id) {" << '\n';
            for (auto& p : node->transfer) {
                ss << "             case '" << p.first << "':" << '\n';
                ss << "                 currentState = " << p.second << ";" << '\n';
                ss << "                 break;" << '\n';
            }
            ss << "             default:" << '\n';
            if (node->transfer.count(ANY)) {
                ss << "                 currentState = " << node->transfer[ANY] << ";" << '\n';
                ss << "                 break;" << '\n';
            }
            else {
                ss << "                 cout << \"Error: Invalid input character.\" << '\\n';" << '\n';
                ss << "                 return 1;" << '\n';
            }
            ss << "             }" << '\n';
            ss << "             break;" << '\n';
        }
        ss << "         }" << '\n';
        ss << "     }" << '\n';
        ss << "     switch (currentState) {" << '\n';
        for (MDfaNode* node : nodes) {
            if (!node->isEnd) continue;
            ss << "     case " << node->state << ":" << '\n';
            ss << "         cout << \"Accepted.\" << '\\n';" << '\n';
            ss << "         break;" << '\n';
        }
        ss << "     default:" << '\n';
        ss << "         cout << \"Not Accepted.\" << '\\n';" << '\n';
        ss << "     }" << '\n';
        ss << "     return 0;" << '\n';
        ss << "}" << '\n';
        return ss.str();
    }

    Dfa& getDfa() {
        return dfa;
    }
};

#endif