# 地产兽域 (Monopoke)

C++17 + SFML 2.6 的本地棋盘游戏：大富翁式移动、地皮经济、精灵战斗与商店。

## 构建

1. 安装 **SFML 2.6**（Windows 下默认期望路径见 `CMakeLists.txt` 中的 `SFML_ROOT`，可按本机修改）。
2. 在项目根目录执行：

```bash
cmake -B build -G "MinGW Makefiles"
cmake --build build
```

可执行文件输出在 `build/bin/`，构建后会将 `assets/` 与 SFML DLL 复制到该目录。

## 运行

在 `build/bin`（或已复制好 `assets`、`saves` 的工作目录）下运行 `Monopoke.exe`，保证能加载 `assets/data/*.json` 与字体（默认尝试 `C:/Windows/Fonts/msyh.ttc`）。

## 操作摘要

| 场景 | 操作 |
|------|------|
| 主菜单 | 鼠标点击：`单机 vs AI`、`本地双人`、`继续游戏`、`退出` |
| 棋盘 | **空格** 掷骰（单机时为玩家1；双人模式下双方都用空格） |
| 驻守 | 站在**自己已购空地**且未驻守时，**F1–F6** 用对应队伍位精灵驻守 |
| 野怪格 | **C** 捕捉（消耗精灵球）**B** 战斗 **V** 逃跑 |
| 营地 | **F1–F6** 将该精灵回满血；**Backspace** 放生队伍最后一只并获得 50 金（至少保留一只） |
| 对战格 | **F1** 开战 **F2** 跳过；胜利可得 500 金与随机道具 |
| 退出 | **ESC** |

## 存档

自动存档路径：`saves/autosave.json`（每回合切换后写入；新开局也会写入初始状态）。主菜单「继续游戏」从此文件恢复。

## 地图数据

默认从 [`assets/data/tiles.json`](assets/data/tiles.json) 加载 20 格；若文件缺失或格子数不为 20，则回退到内置默认布局。

## SFML 路径

若 CMake 找不到 SFML，请编辑 [`CMakeLists.txt`](CMakeLists.txt) 中的 `SFML_ROOT`，或改用 `-DSFML_DIR=...` 指向 SFML 的 CMake 配置目录。
