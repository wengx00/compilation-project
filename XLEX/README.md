<!--
 * @Author: 翁行
 * @Date: 2024-05-10 13:37:44
 * Copyright 2024 (c) 翁行, All Rights Reserved.
-->

# 编译原理项目 - 任务一

> @Author: 20212131001 翁行

本项目基于 macOS 14.4 开发，可以运行在 macOS 12.0+、Windows 10/11、Ubuntu 等操作系统中。

项目使用 CMake 和 Ninja 编译，并且依赖 Qt 6.0+ 的 SDK

为了抹平不同操作系统的编译产物差异，项目推荐使用 Ninja 配合 CMake 来编译。

当然，你也可以在 macOS / Linux 下使用 CMake + Make，Windows 下使用 CMake + msbuild 来编译。

## 依赖

本项目使用了开源库 `yaml-cpp` 来解析 YAML 文件（输入正则表达式）

开源地址：https://github.com/jbeder/yaml-cpp/releases/tag/0.8.0

可以在本项目目录下的 `yaml-cpp-0.8.0` 中看到。

在编译运行项目代码前，需要先将 yaml-cpp-0.8.0 编译成静态库

1. 进入 `yaml-cpp-0.8.0` 目录

2. 执行 CMake 构建

   ```bash
   cmake build -B build -G Ninja .
   ```

3. 进入 `yaml-cpp-0.8.0/build` 目录

4. 执行 Ninja 构建

   ```bash
   ninja
   ```

成功后，即可看到 `yaml-cpp-0.8.0/build/libyaml-cpp.a` 静态库，将其移动到 `lib/yaml-cpp/` 目录下即可。

## 目录说明

- `include`

  - `yaml-cpp` 用于读取 YAML 文件的开源库头文件

  - 其他为项目核心的 `.hpp` ｜ `.h` 文件，覆盖 NFA -> DFA -> MDFA 的核心逻辑

- `lib`

  - `yaml-cpp` yaml-cpp 编译后的静态库地址

- `scripts` 编译/构建脚本

- `yaml-cpp` 开源 YAML 库的源码，可用于编译成静态库放入 `lib/yaml-cpp`

## 编译运行

### 1. 使用 Visual Studio Code

如果使用 Visual Studio Code 进行 Debug / 开发，可以直接配置项目目录下的 `scripts` 目录的 `build-debug.sh` 为运行任务，也就是说 `.vscode/tasks.json` 的配置可以如下：

```json
{
	"version": "2.0.0",
	"tasks": [
		{
			"label": "build-debug",
			"type": "shell",
			"command": "./scripts/build-debug.sh"
		}
	]
}
```

配置后，按 `F5` 或通过 Debug 面板启动即可编译运行和打断点调试。

### 2. 使用脚本编译

进入 `/scripts` 目录，运行：

```bash
./build-release.sh
```

即可编译运行 Release 版本

以上的 Bash 等价于：

```bash
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -B ../build ../
cd ../build
ninja
```

即使用 CMake + Ninja 构建 Release 版本的可执行程序。

### 3. 使用 Qt Creator

直接在 Qt Creator 中打开该项目，然后选择对应的编译环境 Build -> Run 也可以一键启动
