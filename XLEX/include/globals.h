#ifndef _GLOBALS_H
#define _GLOBALS_H

// EPSILON符号
#define EPSILON '@'
// 连接
#define CONCAT '.'
// 或
#define UNION '|'
// 闭包
#define CLOSURE '*'
// 左括号
#define LBRACKET '('
// 右括号
#define RBRACKET ')'

// 无意义字符，需跳过
#define SKIP_COUNT 3
const char SKIP[SKIP_COUNT] = { ' ', '@', '\n' };

// 保留字
#define RESERVED_COUNT 6
const char RESERVED[RESERVED_COUNT] = {
    EPSILON, CONCAT, UNION, CLOSURE, LBRACKET, RBRACKET
};

inline int _indexOf(const char *source, int count, char target) {
    for (int i = 0; i < count; ++i)
        if (source[i] == target) return i;
    return -1;
}

// 优先级
inline int _privilege(char target) {
    switch (target)
    {
    case CLOSURE:
        return 3;
    case CONCAT:
        return 2;
    case UNION:
        return 1;
    default:
        return 0;
    }
}

// 是否特殊字符
inline bool _reservedSymbol(char target) {
    return _indexOf(RESERVED, RESERVED_COUNT, target) > -1;
}

// 是否无意义字符
inline bool _skip(char target) {
    return _indexOf(SKIP, SKIP_COUNT, target) > -1;
}

#endif