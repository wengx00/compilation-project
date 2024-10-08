/*
 * @Author: 翁行
 * @Date: 2023-12-31 16:20:40
 * @LastEditTime: 2024-05-31 00:46:22
 * @FilePath: /XLEX/include/dfa.hpp
 * @Description: NFA图转DFA图
 * Copyright 2024 (c) 翁行, All Rights Reserved.
 */
#ifndef _DFA_HPP
#define _DFA_HPP

#include <iostream>
#include "nfa.hpp"
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <stack>


using namespace std;

// DFA节点
struct DfaNode {
    int state;
    bool isEnd = false;
    map<char, int> transfers; // 转移
    set<NfaNode*> nfaNodes;
    DfaNode() : state(0) {}
    DfaNode(int state) : state(state) {}
    DfaNode(const DfaNode& node) : state(node.state), transfers(node.transfers), nfaNodes(node.nfaNodes) {}

    // 绑定NFA集合到DFA节点中
    void bindNfaNodes(set<NfaNode*> nodes) {
        nfaNodes = nodes;
        for (NfaNode* node : nodes) {
            if (node->isEnd) {
                isEnd = true;
                return;
            }
        }
    }

    bool operator== (const DfaNode& node) {
        if (nfaNodes.size() != node.nfaNodes.size()) return false;
        for (NfaNode* nfaNode : nfaNodes) {
            if (!node.nfaNodes.count(nfaNode)) return false;
        }
        return true;
    }
};

// DFA All in one
class Dfa {
private:
    // 生成DFA图
    void generate() {
        NfaGraph nfaGraph = nfa.getGraph();
        set<char> symbols = nfa.getSymbols();
        DfaNode* start = new DfaNode();
        start->bindNfaNodes(epsilonClosure(nfaGraph.start)); // 将NFA节点列表绑定进DFA状态中
        nodes.push_back(start); // 加入初始节点
        for (int i = 0; i < nodes.size(); ++i) {
            DfaNode* cur = nodes[i];
            for (char symbol : symbols) {
                set<NfaNode*> nfaNodesOfSymbol;
                for (NfaNode* next : cur->nfaNodes) {
                    set<NfaNode*> nfaNodesOfNext = forward(next, symbol);
                    nfaNodesOfSymbol.insert(nfaNodesOfNext.begin(), nfaNodesOfNext.end());
                }
                if (nfaNodesOfSymbol.empty()) continue;
                // 判断是否新增
                DfaNode* instance = new DfaNode(nodes.size());
                instance->bindNfaNodes(nfaNodesOfSymbol);
                // 子集构造法
                for (DfaNode* exist : nodes) {
                    if (*exist == *instance) {
                        instance = exist;
                        break;
                    }
                }
                if (instance->state == nodes.size()) { // 需要新增节点
                    nodes.push_back(instance);
                }
                // 加入转移关系
                cur->transfers[symbol] = instance->state;
            }
        }
    }

    // 以symbol步进
    set<NfaNode*> forward(NfaNode* source, char symbol) {
        set<NfaNode*> result;
        for (NfaNode* next : source->transfers[symbol]) {
            result.insert(next);
            // 同时要加入其EPSILON闭包
            set<NfaNode*> closureOfNext = epsilonClosure(next);
            result.insert(closureOfNext.begin(), closureOfNext.end());
        }
        return result;
    }

    // NFA节点的EPSILON闭包
    set<NfaNode*> epsilonClosure(NfaNode* source) {
        set<NfaNode*> closure;
        stack<NfaNode*> prepared; // DFS栈
        map<int, int> visited;
        prepared.push(source);
        while (prepared.size()) {
            NfaNode* cur = prepared.top();
            prepared.pop();
            if (visited[cur->state]) continue;
            closure.insert(cur);
            visited[cur->state] = 1;
            for (NfaNode* next : cur->transfers[EPSILON])
                prepared.push(next);
        }
        return closure;
    }

    vector<DfaNode*> nodes;
    Nfa& nfa;
public:
    Dfa(Nfa& nfa) : nfa(nfa) {
        generate();
    }
    // 获取原始NFA
    Nfa getNfa() {
        return nfa;
    }
    // 获取DFA节点列表
    vector<DfaNode*> getNodes() {
        return nodes;
    }
};

#endif