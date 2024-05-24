# 编译原理课程项目 2: LR_SLR 和语法树生成

> 20212131001 (c) 翁行

## 编译方法

推荐使用 CMake + Ninja 编译

1. 确保已安装 CMake 和 Ninja

2. 运行构建脚本 `scripts/build-debug.sh` 或 `script/build-release.sh`

也可以手动编译

```bash
# 进入项目根目录
cd /path/to/project
# CMake构建
cmake -G Ninja -B build .
# 进入构建目录
cd build
# 执行Ninja
ninja
```

## 运行环境

- macOS 12.0+

- Windows 10/11+

## 输入文法说明

1. 每一行 BNF 前都要有其语义动作，且 BNF 右部不能包含“｜”，使用多行来代替或

   ```
   { 0: -1, 1: 0, 3: 1 }
   if-stmt -> if exp then stmt-sequence end
   { 0: -1, 1: 0, 3: 1, 5: 2 }
   if-stmt -> if exp then stmt-sequence else stmt-sequence end
   ```

2. 空为 EPSILON

   ```
   {}
   statement-list -> EPSILON
   ```

3. 特殊符号需要和项目一的 LEX Label 对应

   ```
   {}
   return-stmt -> return semi
   ```

## 输入待解析 LEX 说明

- 必须使用项目一生成的代码解析程序解析后生成的 LEX 文件

- 项目一使用的分词 YAML 文件必须和 BNF 使用的分词对应
