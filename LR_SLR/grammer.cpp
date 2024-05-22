/*
 * @Author: wengx00 wengx86@163.com
 * @Date: 2023-12-16 22:04:11
 * Copyright (c) 2024 by wengx00, All Rights Reserved.
 */
#include "grammer.h"
#include <iostream>
#include <queue>
#include <sstream>

using namespace std;

Grammer::Grammer(string input) {
    vector<string> lines;
    int from = 0, i = 0;
    for (i = 0; i < input.size(); ++i) {
        if (input[i] == '\n') {
            // 换行分割语句
            lines.push_back(input.substr(from, i - from));
            from = i + 1;
        }
    }
    if (from != i)
        lines.push_back(input.substr(from));
    if (lines.empty()) {
        error = "未输入任何文法";
        return;
    }
    map<int, int> action;
    bool isStartToken = true;
    for (int i = 0; i < lines.size(); ++i) {
        string line = lines[i];
        if (line.size() == 0) continue;
        int begin = 0;
        int end = line.size() - 1;
        // 找到非空子串
        while (line[begin] == ' ') {
            begin += 1;
        }
        while (line[end] == ' ') {
            end -= 1;
        }
        if (end - begin <= 0) {
            continue;
        }
        if (line[begin] == '{' && line[end] == '}') {
            // 语义规则
            bool isValue = false;
            string curIndex;
            string curValue;
            for (int j = begin + 1; j <= end; ++j) {
                if (line[j] == ' ') continue;
                if (line[j] == ':' && !isValue) {
                    isValue = true;
                    continue;
                }
                if (line[j] == ',' || line[j] == '}') {
                    if (curIndex.size() == 0 && curValue.size() == 0) {
                        continue;
                    }
                    if ((!isValue || !curIndex.size() || !curValue.size())) {
                        error = "语义规则输入有误";
                        return;
                    }
                    action[stoi(curIndex)] = stoi(curValue);
                    curIndex.clear();
                    curValue.clear();
                    isValue = false;
                    continue;
                }
                if (!isValue) {
                    curIndex.push_back(line[j]);
                }
                else {
                    curValue.push_back(line[j]);
                }
            }
            continue;
        }
        string key;
        string token;
        vector<string> raws;
        bool behind = false;
        for (int j = begin; j <= end; ++j) {
            if (line[j] == ' ') {
                // 分词
                if (behind && token.size()) {
                    raws.push_back(token);
                    token.clear();
                }
                continue;
            }
            if (line[j] == '-' && j < end - 1 && line[j + 1] == '>') {
                // ->
                behind = true;
                j++;
                continue;
            }
            if (line[j] == '|') {
                if (!behind) {
                    error = "|符号不能出现在左值";
                    return;
                }
                // ｜需要分割
                if (token.size()) {
                    raws.push_back(token);
                    token.clear();
                }
                formula[key].push_back(raws);
                raws.clear();
                continue;
            }
            // Common Symbol
            if (!behind) {
                key += line[j];
                continue;
            }
            token.push_back(line[j]);
            if (j == end) {
                raws.push_back(token);
                token.clear();
            }
        }
        if (!behind) {
            error = "文法输入有误";
            return;
        }
        if (!raws.empty()) {
            formula[key].push_back(raws);
        }
        if (isStartToken) {
            start = key;
            isStartToken = false;
        }
        // 映射语义规则
        actions[key].push_back(action);
        action.clear();
    }
    // 拓广文法
    // formula[start + '\''].push_back(vector<string>(1, start));
    // start = start + '\'';

    // 构建非终结符号集
    for (auto it = formula.begin(); it != formula.end(); ++it) {
        notEnd.insert(it->first);
    }

    // 构建终结符号集合
    for (auto& p : formula) {
        for (auto& raws : p.second) {
            for (auto& raw : raws) {
                if (!notEnd.count(raw) && !endSet.count(raw)) endSet.insert(raw);
            }
        }
    }

    // 初始化First集合元素
    initFirst();
    // 初始化Follow集合元素
    initFollow();
    // 构建DFA
    initRelation();
    // 判断是否SLR
    initIsSLR();
}

set<string> Grammer::getFirst(string key) {
    if (!notEnd.count(key)) {
        // 是终结节点
        set<string> res;
        res.insert(key);
        return res;
    }
    // 非终结节点，返回其First集
    return first[key];
}

set<string> Grammer::getFollow(string key) { return follow[key]; }

void Grammer::initFirst() {
    bool shouldUpdate = true;
    while (shouldUpdate) {
        shouldUpdate = false;

        for (auto& p : formula) {
            string key = p.first;                   // 非终结符
            vector<vector<string>> raws = p.second; // 产生式右侧
            for (const auto& raw : raws) {
                int cur = 0;
                for (; cur < raw.size(); ++cur) {
                    auto firstOfCur = getFirst(raw[cur]); // 当前元素的First集合

                    // 遍历当前First
                    for (auto& el : firstOfCur) {
                        // 除了EPSILON外，新增的元素都加入key的First
                        if (el != EPSILON && !first[key].count(el)) {
                            first[key].insert(el);
                            shouldUpdate = true;
                        }
                    }

                    // EPSILON不在cur的First，可以退出推导式右侧的遍历
                    if (!firstOfCur.count(EPSILON)) {
                        break;
                    }
                }
                // 右侧所有元素First都包含EPSILON，则key的First也应该包含EPSILON
                if (cur == raw.size() && !first[key].count(EPSILON)) {
                    first[key].insert(EPSILON);
                    shouldUpdate = true;
                }
            }
        }
    }
}

void Grammer::initFollow() {
    bool shouldUpdate = true;
    // start的Follow为END_FLAG
    follow[start].insert(END_FLAG);
    while (shouldUpdate) {
        shouldUpdate = false;

        for (auto& p : formula) {
            string key = p.first;
            vector<vector<string>> raws = p.second;
            // 遍历每一个推导式右侧
            for (const auto& raw : raws) {
                // 遍历每一个非终结符号
                for (int i = 0; i < raw.size(); ++i) {
                    if (!notEnd.count(raw[i]))
                        continue;
                    // 末尾
                    if (i == raw.size() - 1) {
                        for (const auto& el : getFollow(key)) {
                            if (!follow[raw[i]].count(el)) {
                                follow[raw[i]].insert(el);
                                shouldUpdate = true;
                            }
                        }
                        continue;
                    }
                    // 非末尾，获取后续元素的First集合
                    set<string> firstOfBehind;
                    int cur = i + 1;
                    for (; cur < raw.size(); ++cur) {
                        set<string> firstOfCur = getFirst(raw[cur]);
                        for (const auto& el : firstOfCur) {
                            if (el != EPSILON)
                                firstOfBehind.insert(el);
                        }
                        if (!firstOfCur.count(EPSILON)) {
                            // 不含EPSILON，First终止
                            break;
                        }
                    }
                    if (cur == raw.size()) {
                        // 每个元素的First都包含Epsilon
                        firstOfBehind.insert(EPSILON);
                    }
                    // 更新Follow集合
                    for (const auto& el : firstOfBehind) {
                        if (el != EPSILON && !follow[raw[i]].count(el)) {
                            follow[raw[i]].insert(el);
                            shouldUpdate = true;
                        }
                    }
                    if (firstOfBehind.count(EPSILON)) {
                        follow[raw[i]].erase(EPSILON);
                        // 包含EPSILON，Follow集合包含产生式左侧的Follow集合
                        for (const auto& el : getFollow(key)) {
                            if (!follow[raw[i]].count(el)) {
                                follow[raw[i]].insert(el);
                                shouldUpdate = true;
                            }
                        }
                    }
                }
            }
        }
    }
}

void Grammer::extend(vector<Item>& nodes) {
    for (int i = 0; i < nodes.size(); ++i) {
        Item& node = nodes[i];
        if (node.type == ItemType::BACKWARD)
            continue; // 跳过规约节点
        string cur = formula[node.key][node.rawsIndex][node.rawIndex]; // 指示的符号
        if (!notEnd.count(cur))
            continue; // 终结字符不可扩展
        vector<vector<string>>& rawsOfCur = formula[cur]; // 非终结字符为Key的推导式
        for (int j = 0; j < rawsOfCur.size(); ++j) {
            int rawOffset = 0;
            for (; rawOffset < rawsOfCur[j].size(); ++rawOffset) {
                // 寻找到非空字符
                if (rawsOfCur[j][rawOffset] != EPSILON)
                    break;
            }
            // 新增节点，指示了Key对应的第i个推导式的第rawOffset个字符
            Item instance(cur,
                rawOffset == 0 ? ItemType::FORWARD : ItemType::BACKWARD, j,
                rawOffset);
            // 无重复则扩展state指示的dfa节点
            if (!count(nodes.begin() /*+ i*/, nodes.end(), instance)) {
                nodes.push_back(instance);
            }
        }
    }
}

void Grammer::extend(int state) {
    extend(dfa[state]);
}

void Grammer::initRelation() {
    // 初始节点 => start指示的推导式的第一条的第一个符号
    vector<Item> beginState;
    beginState.push_back(Item(start, ItemType::FORWARD, 0, 0));
    dfa.push_back(beginState);
    isSLR = true; // 暂时先是
    // 遍历每一个DFA节点
    for (int cur = 0; cur < dfa.size(); ++cur) {
        cout << "Current State" << cur << '\n';
        extend(cur); // 扩展当前DFA节点(可能右侧项目含有非终结符号)
        // forwards[cur]和backwards[cur]分别记录了移进和规约关系
        // 遍历DFA节点上的每一个项目
        for (int it = 0; it < dfa[cur].size(); ++it) {
            Item& item = dfa[cur][it]; // 取出当前项
            if (item.type == ItemType::BACKWARD) {
                // 规约项
                set<string> followOfItem = getFollow(item.key);
                for (const auto& el : followOfItem) {
                    if (backwards[cur].count(el)) {
                        // 存在交集，非SLR(1)
                        isSLR = false;
                        stringstream ss;
                        ss << "第" << cur << "个节点中规约项目的Follow集合有交集\n";
                        reason += ss.str();
                    }
                    backwards[cur][el] = it;
                }
                continue;
            }
            // 移进项
            string raw = formula[item.key][item.rawsIndex][item.rawIndex];
            // 移进新的节点
            Item instance(item.key, ItemType::FORWARD, item.rawsIndex,
                item.rawIndex + 1);
            if (instance.rawIndex >=
                formula[instance.key][instance.rawsIndex].size()) {
                // 超过了该推导式的结尾 -> 变成规约节点
                instance.type = ItemType::BACKWARD;
            }
            if (forwards[cur].count(raw)) {
                // 已经存在该移进关系
                int target = forwards[cur][raw];
                vector<Item>& next = dfa[target];
                if (!count(next.begin(), next.end(), instance)) {
                    // 如果下一DFA节点中未存在该Instance状态 -> 加入下一DFA节点中
                    dfa[target].push_back(instance);
                }
                continue;
            }
            // 未存在该移进关系
            vector<Item> perhapsNewState{ instance };
            extend(perhapsNewState);
            int target = findState(perhapsNewState);
            if (target == -1) {
                // 该状态不存在于任何DFA节点中 -> 新增一个DFA节点
                dfa.push_back(perhapsNewState);
                target = dfa.size() - 1;
            }
            // 加入移进关系
            forwards[cur][raw] = target;
        }
    }
}

void Grammer::initIsSLR() {
    // DFA图构建完成后 -> 判断移进规约是否冲突
    if (isSLR) {
        stringstream ss;
        for (int cur = 0; cur < dfa.size(); ++cur) {
            set<string> curForwards, curBackwards, duplicates;
            for (auto p : forwards[cur]) {
                curForwards.insert(p.first);
            }
            for (auto p : backwards[cur]) {
                curBackwards.insert(p.first);
            }
            set_intersection(curForwards.begin(), curForwards.end(),
                curBackwards.begin(), curBackwards.end(),
                inserter(duplicates, duplicates.begin()));
            if (!duplicates.empty()) {
                // 交集不空 不为SLR
                isSLR = false;
                ss.str("");
                ss.clear();
                ss << "第" << cur
                    << "个节点的移进项First集合和规约项Follow集合有交集\n";
                reason += ss.str();
            }
        }
    }
}

int Grammer::findState(vector<Item>& current) {
    for (int i = 0; i < dfa.size(); ++i) {
        auto& state = dfa[i];
        // if (current.size() != state.size()) continue;
        bool exist = true;
        // 拿个map存state加快查询速度
        unordered_map<string, int> stateMap;
        for (int j = 0; j < state.size(); ++j) {
            stateMap[state[j].id()] = j;
        }
        for (auto& node : current) {
            if (!stateMap.count(node.id())) {
                exist = false;
                break;
            }
        }
        if (exist) return i;
    }
    return -1;
}

bool Grammer::slr() { return isSLR; }
bool Grammer::bad() { return !error.empty(); }
string Grammer::getReason() { return reason; }
string Grammer::getError() { return error; }

set<string> Grammer::getNotEnd() { return notEnd; }
set<string> Grammer::getEnd() { return endSet; }

string Grammer::getStart() {
    return start;
}

string Grammer::getExtraGrammer() {
    stringstream ss;
    map<string, bool> visited;
    queue<string> ready;
    ready.push(start);
    while (ready.size()) {
        string cur = ready.front();
        ready.pop();
        if (visited[cur])
            continue;
        visited[cur] = true;
        vector<vector<string>> rawsOfCur = formula[cur];
        for (auto& raw : rawsOfCur) {
            ss << cur << " ->";
            for (auto& token : raw) {
                if (notEnd.count(token)) {
                    ready.push(token);
                }
                ss << ' ' << token;
            }
            ss << '\n';
        }
    }
    return ss.str();
}

vector<vector<Item>> Grammer::getDfa() { return dfa; }

map<string, vector<vector<string>>> Grammer::getFormula() {
    return formula;
}

int Grammer::forward(int state, string key) {
    if (forwards[state].count(key)) {
        return forwards[state][key];
    }
    return -1;
}

int Grammer::backward(int state, string key) {
    if (backwards[state].count(key)) {
        return backwards[state][key];
    }
    return -1;
}

ParsedResult Grammer::parse(string input) {
    // input 是lex文件 LABEL : VALUE
    queue<pair<string, string>> lex;
    string label;
    string value;
    bool isLabel = true;
    for (char c : input) {
        if (c == ' ') {
            continue;
        }
        if (c == '\n') {
            if (label.size()) {
                lex.push({ label, value });
            }
            label.clear();
            value.clear();
            isLabel = true;
            continue;
        }
        if (c == ':' && isLabel) {
            isLabel = false;
            continue;
        }
        if (isLabel) {
            label += c;
        }
        else {
            value += c;
        }
    }
    if (label.size() && !isLabel) {
        lex.push({ label, value });
    }
    ParsedResult result;
    string output;
    vector<int> stash;
    // 在工作区的TreeNode
    vector<TreeNode*> workspace;
    lex.push({ END_FLAG, END_FLAG });
    int state = 0; // 当前DFA状态编号
    int count = 0;
    stringstream ss;
    for (;;) {
        ss.str("");
        ss.clear();
        map<string, int>& curForwards = forwards[state]; // 当前所有可移进状态
        map<string, int>& curBackwards = backwards[state]; // 当前所有可规约状态

        auto pair = lex.front(); // 当前输入的字符
        string token = pair.first;
        string value = pair.second;
        stash.push_back(state); // 当前状态入栈

        if (curForwards.count(token)) {
            // 找到了移进关系
            lex.pop();
            ++count;
            int next = curForwards[token]; // 下一个状态
            ss << "在状态" << state << "通过" << token << "移进到状态" << next;
            state = next;
            output += token;
            result.outputs.push_back(output);
            TreeNode* current = new TreeNode;
            current->label = token;
            current->value = value;
            workspace.push_back(current);
            result.routes.push_back(ss.str());
            continue;
        }
        if (curBackwards.count(token)) {
            // 找到了规约关系
            int target = curBackwards[token];
            ss << "在状态" << state << "通过" << token << "规约到状态" << target;
            Item& item = dfa[state][target];
            vector<string>& raws = formula[item.key][item.rawsIndex];
            // 生成语法树节点
            map<int, int> action = actions[item.key][item.rawsIndex];
            result.routes.push_back(ss.str());
            TreeNode* current = new TreeNode;
            current->label = item.key;
            int offset = workspace.size() - raws.size();
            if (offset < 0) {
                result.error = "语法树解析错误";
                return result;
            }
            for (int i = 0; i < raws.size(); i++) {
                int index = raws.size() - 1 - i;
                if (!action.count(index) || action[index] < -1) {
                    continue;
                }
                if (action[index] == -1) {
                    // 作为树根
                    if (workspace[index + offset]->children.size() > 0 && current->children.size() > 0) {
                        result.error = "语法树解析错误";
                        return result;
                    }
                    if (workspace[index + offset]->children.size() == 0) {
                        for (auto child : current->children) {
                            if (child) {
                                child->parent = workspace[index + offset];
                            }
                        }
                        workspace[index + offset]->children = current->children;
                    }
                    current = workspace[index + offset];
                    continue;
                }
                TreeNode* child = workspace[index + offset];
                if (action[index] >= current->children.size()) {
                    // 扩容
                    current->children.resize(action[index] + 1);
                }
                current->children[action[index]] = child;
                child->parent = current;
            }
            // 出栈
            workspace.erase(workspace.begin() + offset, workspace.end());
            // 将新生成的节点入栈
            workspace.push_back(current);

            if (item.key == start) {
                // 接收
                result.accept = true;
                result.outputs.push_back(start);
                break;
            }
            int useful = 0;
            for (int i = 0; i < raws.size(); ++i) {
                // 找到不是EPSILON的大小
                if (raws[i] != EPSILON) useful++;
            }
            if (useful > 0) {
                output.erase(output.end() - useful, output.end());
                stash.erase(stash.end() - useful, stash.end());
            }
            int next = forwards[stash[stash.size() - 1]][item.key];
            state = next;
            output += item.key;
            result.outputs.push_back(output);
            continue;
        }
        // 找不到关系，出错
        ss << "在状态" << state << "上找不到" << token << "对应的移进/规约关系";
        result.error = ss.str();
        break;
    }
    if (workspace.size() != 1) {
        error = "语法树解析错误";
        return result;
    }
    result.root = workspace[0];
    workspace.pop_back();
    return result;
}
