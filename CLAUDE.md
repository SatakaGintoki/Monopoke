# CLAUDE.md - 地产兽域 (Monopoke) 项目指令

## 项目信息
- **项目名称**: 地产兽域 (Monopoke)
- **类型**: 课程实训项目（2024-2025学年）
- **截止日期**: 2026-04-24
- **技术栈**: C++17 + SFML 2.5+ + nlohmann/json
- **构建工具**: CMake 3.16+
- **平台**: Windows 11 (PC)
- **游戏类型**: 2D 回合制大富翁 + 精灵捕捉养成对战
- **玩家人数**: 2人本地同屏对战
- **设计文档**: `地产兽域_游戏设计提案.md`

---

## 整体架构设计

### 架构模式

采用 **游戏状态机 + 系统分层** 架构：

```
┌─────────────────────────────────────────────────────┐
│                    Game (主控)                        │
│              GameLoop + StateManager                 │
└──────────────────────┬──────────────────────────────┘
                       │
        ┌──────────────┼──────────────────┐
        ▼              ▼                  ▼
   ┌─────────┐   ┌──────────┐     ┌───────────┐
   │ States  │   │ Systems  │     │ Managers  │
   │ (状态层) │   │ (逻辑层)  │     │ (管理层)  │
   └─────────┘   └──────────┘     └───────────┘
        │              │                  │
        ▼              ▼                  ▼
   MenuState      BattleSystem     ResourceManager
   PlayState      EconomySystem    AudioManager
   BattleState    ShopSystem       SaveManager
   ShopState      MonsterSystem    ConfigManager
   ResultState    BoardSystem
                  AISystem
```

### 游戏状态机

游戏在任意时刻处于以下某个状态，状态之间通过事件驱动切换：

```
                    ┌──────────┐
                    │ MenuState│ ← 游戏启动入口
                    └────┬─────┘
                         │ 开始游戏
                         ▼
                    ┌──────────┐
              ┌────►│ PlayState│◄────┐  ← 棋盘主界面（掷骰、移动、格子触发）
              │     └──┬───┬───┘     │
              │        │   │         │
     战斗结束  │  踩野怪/│   │踩商店格  │ 购买完成
              │  对手格 │   │         │
              │        ▼   ▼         │
              │  ┌────────┐ ┌───────┐│
              └──┤Battle  │ │ Shop  ├┘
                 │State   │ │ State │
                 └────────┘ └───────┘
                      │
                      ▼ (游戏结束)
                 ┌──────────┐
                 │ResultState│ → 返回 MenuState
                 └──────────┘
```

额外的子状态（在 PlayState 内部处理，不单独建状态类）：
- **EventPopup**: 事件格弹窗（随机事件结果展示）
- **CampPopup**: 营地格弹窗（选择治疗/放生）
- **PropertyPopup**: 空地购买确认弹窗
- **GuardPopup**: 驻守精灵派遣弹窗

---

### 项目目录结构

```
Monopoke/
├── CMakeLists.txt                  # 构建配置
├── CLAUDE.md                       # 项目指令（本文件）
├── 地产兽域_游戏设计提案.md           # 设计文档
│
├── src/
│   ├── main.cpp                    # 程序入口
│   │
│   ├── core/                       # 核心框架层
│   │   ├── Game.h/cpp              # 游戏主类（初始化、主循环、退出）
│   │   ├── StateManager.h/cpp      # 状态机管理器（push/pop/switch状态）
│   │   ├── GameState.h             # 状态基类（纯虚接口）
│   │   └── EventBus.h/cpp          # 事件总线（模块间解耦通信）
│   │
│   ├── states/                     # 游戏状态层
│   │   ├── MenuState.h/cpp         # 主菜单状态
│   │   ├── PlayState.h/cpp         # 棋盘游戏主状态（最复杂）
│   │   ├── BattleState.h/cpp       # 战斗状态
│   │   ├── ShopState.h/cpp         # 商店状态
│   │   └── ResultState.h/cpp       # 结算状态
│   │
│   ├── entity/                     # 实体数据层
│   │   ├── Player.h/cpp            # 玩家（金币、位置、精灵列表、地皮列表）
│   │   ├── Monster.h/cpp           # 精灵（属性、技能、等级、经验、HP）
│   │   ├── Skill.h/cpp             # 技能（名称、属性、威力、PP、命中率）
│   │   ├── Tile.h/cpp              # 格子（类型、位置、归属、等级、驻守精灵）
│   │   ├── Board.h/cpp             # 棋盘（格子数组、环形路径）
│   │   └── Item.h/cpp              # 道具（精灵球、伤药、进化石等）
│   │
│   ├── system/                     # 游戏逻辑系统层
│   │   ├── BattleSystem.h/cpp      # 战斗系统（伤害计算、属性克制、回合管理）
│   │   ├── EconomySystem.h/cpp     # 经济系统（金币收支、地皮买卖、过路费）
│   │   ├── MonsterSystem.h/cpp     # 精灵系统（捕捉、升级、进化、养成）
│   │   ├── ShopSystem.h/cpp        # 商店系统（商品列表、购买逻辑）
│   │   ├── BoardSystem.h/cpp       # 棋盘系统（掷骰、移动、格子触发分发）
│   │   ├── EventSystem.h/cpp       # 事件系统（随机事件池、概率抽取）
│   │   └── AISystem.h/cpp          # AI系统（简单AI / 策略AI）
│   │
│   ├── ui/                         # UI渲染层
│   │   ├── UIManager.h/cpp         # UI管理器（弹窗栈、HUD）
│   │   ├── Button.h/cpp            # 按钮组件
│   │   ├── Panel.h/cpp             # 面板组件
│   │   ├── HUD.h/cpp               # 顶部信息栏（金币、回合数、玩家信息）
│   │   ├── BoardRenderer.h/cpp     # 棋盘渲染（格子绘制、棋子动画）
│   │   ├── BattleRenderer.h/cpp    # 战斗界面渲染
│   │   └── PopupRenderer.h/cpp     # 弹窗渲染（事件/营地/购买确认）
│   │
│   └── utils/                      # 工具层
│       ├── ConfigManager.h/cpp     # JSON配置读取（精灵表、技能表、地图表）
│       ├── ResourceManager.h/cpp   # 资源管理（纹理、字体、音效缓存）
│       ├── AudioManager.h/cpp      # 音频管理（BGM切换、音效播放）
│       ├── SaveManager.h/cpp       # 存档管理（JSON序列化/反序列化）
│       ├── Random.h/cpp            # 随机数工具（骰子、概率判定）
│       └── Types.h                 # 全局枚举和类型定义
│
├── assets/
│   ├── data/
│   │   ├── monsters.json           # 精灵数据表（10只精灵的完整属性）
│   │   ├── skills.json             # 技能数据表（10个技能）
│   │   ├── items.json              # 道具数据表
│   │   ├── tiles.json              # 地图格子配置（20格布局）
│   │   ├── events.json             # 随机事件池
│   │   └── shop.json               # 商店商品配置
│   ├── fonts/                      # 字体文件（需支持中文）
│   ├── textures/                   # 纹理资源（精灵、格子、UI）
│   └── sounds/                     # 音效和BGM
│
└── saves/                          # 存档目录
    └── autosave.json
```

---

### 核心类设计

#### 1. Game 主类（单例）

```
Game
├── sf::RenderWindow window_        # SFML窗口
├── StateManager state_manager_     # 状态机
├── ResourceManager resource_mgr_   # 资源管理
├── AudioManager audio_mgr_         # 音频管理
├── ConfigManager config_mgr_       # 配置管理
│
├── run()                           # 主循环入口
├── handle_events()                 # 处理SFML事件
├── update(float dt)                # 更新当前状态
└── render()                        # 渲染当前状态
```

#### 2. GameState 状态基类

```
GameState (abstract)
├── virtual init()                  # 进入状态时初始化
├── virtual cleanup()               # 离开状态时清理
├── virtual handle_event(sf::Event) # 处理输入
├── virtual update(float dt)        # 逻辑更新
└── virtual render(sf::RenderWindow&) # 渲染
```

#### 3. Player 玩家

```
Player
├── std::string name_               # 玩家名
├── int gold_                       # 金币（初始1000）
├── int position_                   # 棋盘位置（0-19）
├── std::vector<Monster> monsters_  # 精灵队伍（最多6只）
├── std::vector<int> properties_    # 拥有的地皮索引
├── std::vector<Item> inventory_    # 背包道具
├── int laps_completed_             # 已走圈数（用于发工资）
│
├── add_gold(int) / spend_gold(int)
├── add_monster(Monster) / remove_monster(int)
├── has_alive_monsters() -> bool    # 是否还有存活精灵
└── is_bankrupt() -> bool           # 是否破产
```

#### 4. Monster 精灵

```
Monster
├── int id_                         # 精灵ID
├── std::string name_               # 名称
├── MonsterType type_               # 属性（火/水/草/电/冰/龙/恶）
├── int level_                      # 等级（1-20）
├── int exp_ / int exp_to_next_     # 经验值
├── int current_hp_ / int max_hp_   # 当前/最大HP
├── int atk_, def_, spd_            # 攻击、防御、速度
├── int sp_atk_, sp_def_            # 特攻、特防
├── std::vector<Skill> skills_      # 技能列表（2-4个）
├── Rarity rarity_                  # 稀有度
├── int evolve_level_               # 进化等级（0=不可进化）
├── int evolve_to_id_               # 进化目标ID
│
├── take_damage(int) -> bool        # 受伤，返回是否存活
├── gain_exp(int) -> bool           # 获得经验，返回是否升级
├── level_up()                      # 升级（属性+5%）
├── try_evolve() -> bool            # 尝试进化
├── heal(int)                       # 治疗
└── is_alive() -> bool
```

#### 5. Tile 格子

```
Tile
├── int index_                      # 格子索引（0-19）
├── TileType type_                  # 格子类型（7种）
├── int owner_id_                   # 归属玩家（-1=无主）
├── int property_level_             # 设施等级（0-3）
├── int price_                      # 购买价格
├── int guardian_monster_id_        # 驻守精灵在玩家队伍中的索引（-1=无）
│
├── get_toll() -> int               # 计算过路费
├── upgrade_cost() -> int           # 升级费用
└── can_upgrade() -> bool
```

#### 6. BattleSystem 战斗系统

```
BattleSystem
├── Monster* attacker_              # 攻击方精灵
├── Monster* defender_              # 防御方精灵
├── BattleType battle_type_         # 战斗类型（野怪/驻守/玩家对战）
│
├── start_battle(Monster&, Monster&, BattleType)
├── execute_turn(Skill&) -> BattleResult
├── calculate_damage(Monster&, Monster&, Skill&) -> int
├── get_type_effectiveness(MonsterType, MonsterType) -> float
├── try_capture(Monster&, int ball_type) -> bool  # 捕捉判定
├── try_escape(Monster&, Monster&) -> bool        # 逃跑判定
└── get_battle_result() -> BattleResult
```

#### 7. BoardSystem 棋盘系统

```
BoardSystem
├── Board board_                    # 棋盘数据
├── int current_player_             # 当前回合玩家
│
├── roll_dice() -> int              # 掷骰子（1-6）
├── move_player(Player&, int steps) # 移动玩家
├── trigger_tile(Player&, Tile&)    # 触发格子效果（分发到各子系统）
├── check_lap(Player&)              # 检查是否走完一圈（发工资）
├── next_turn()                     # 切换回合
└── check_game_over() -> int        # 检查胜负（返回赢家ID或-1）
```

---

### 全局枚举定义 (Types.h)

```cpp
enum class MonsterType {
    Normal, Fire, Water, Grass, Electric, Ice, Dragon, Dark, Ghost, Psychic
};

enum class TileType {
    Start,      // 起点
    Empty,      // 空地（可购买）
    Wild,       // 野怪格
    Shop,       // 商店格
    Event,      // 事件格
    Camp,       // 营地格
    Battle      // 对战格
};

enum class PropertyLevel {
    Empty = 0,  // 空地
    Hut = 1,    // 小屋
    Shop = 2,   // 商店
    Gym = 3     // 道馆
};

enum class SkillCategory { Physical, Special };

enum class Rarity { Common, Rare, Legendary };

enum class BattleType { Wild, Guardian, PvP };

enum class BattleResult { Ongoing, Win, Lose, Escaped, Captured };

enum class GameMode { PvP, PvE };
```

---

### 数据流设计

#### 回合主流程数据流

```
PlayState::update()
    │
    ├─ 1. 等待玩家点击"掷骰子"
    │      └─ BoardSystem::roll_dice() → int steps
    │
    ├─ 2. 移动棋子
    │      └─ BoardSystem::move_player(player, steps)
    │          └─ BoardSystem::check_lap(player) → 发500工资
    │
    ├─ 3. 触发格子
    │      └─ BoardSystem::trigger_tile(player, tile)
    │          │
    │          ├─ TileType::Empty → EconomySystem::try_purchase(player, tile)
    │          ├─ TileType::Wild  → 切换到 BattleState（野怪战斗）
    │          ├─ TileType::Shop  → 切换到 ShopState
    │          ├─ TileType::Event → EventSystem::trigger_random_event(player)
    │          ├─ TileType::Camp  → 弹出营地选择面板
    │          └─ TileType::Battle→ 切换到 BattleState（玩家对战）
    │
    ├─ 4. 检查胜负
    │      └─ BoardSystem::check_game_over()
    │          └─ 若有赢家 → 切换到 ResultState
    │
    └─ 5. 切换回合
           └─ BoardSystem::next_turn()
               └─ 自动存档 SaveManager::auto_save()
```

#### 战斗流程数据流

```
BattleState::update()
    │
    ├─ 1. 速度比较 → 决定先后手
    │      └─ attacker.spd_ vs defender.spd_
    │
    ├─ 2. 先手方选择行动
    │      ├─ 选择技能 → BattleSystem::execute_turn(skill)
    │      │              └─ calculate_damage() → take_damage()
    │      ├─ 逃跑     → BattleSystem::try_escape()
    │      └─ 捕捉(野怪) → BattleSystem::try_capture()
    │
    ├─ 3. 检查是否结束
    │      └─ defender.is_alive()? → 继续 or 结算
    │
    ├─ 4. 后手方行动（同上）
    │
    └─ 5. 战斗结算
           └─ BattleResult → 发放奖励/扣除金币
               └─ 返回 PlayState
```

---

### 属性克制表（代码实现参考）

```
effectiveness[攻击属性][防御属性] → float

Fire   > Grass  (2.0)    Grass  > Water  (2.0)    Water  > Fire   (2.0)
Electric > Water (2.0)   Ice    > Grass  (2.0)    Ice    > Dragon (2.0)
Dragon > Dragon (0.0 免疫)
Dark   > Ghost  (2.0)    Dark   > Psychic(2.0)

其余组合 → 1.0（正常伤害）
反向克制 → 0.5
```

---

### 关键设计决策

| 决策点 | 选择 | 理由 |
|--------|------|------|
| 架构模式 | 状态机 + 系统分层 | 状态切换清晰，系统间解耦，适合回合制游戏 |
| 图形库 | SFML | 轻量级2D库，API简洁，适合课程项目 |
| 数据驱动 | JSON配置文件 | 精灵/技能/地图数据与代码分离，方便调整数值 |
| 状态管理 | 栈式状态机 | 支持状态叠加（如PlayState上叠BattleState） |
| 弹窗处理 | PlayState内部子状态 | 避免状态类过多，事件/营地/购买弹窗逻辑简单 |
| AI系统 | 策略优先级 | 先检查克制关系，再选最高威力技能，简单有效 |
| 存档格式 | JSON | 可读性好，nlohmann/json库已集成 |

---

### 开发里程碑

| 阶段 | 内容 | 预计周期 |
|------|------|----------|
| M1 | 项目骨架：CMake + SFML窗口 + 状态机框架 + 资源管理 | 3天 |
| M2 | 棋盘系统：20格渲染 + 掷骰移动 + 格子触发框架 | 4天 |
| M3 | 精灵系统：Monster类 + JSON数据加载 + 属性计算 | 3天 |
| M4 | 战斗系统：伤害公式 + 属性克制 + 回合流程 + 战斗UI | 5天 |
| M5 | 经济系统：地皮购买/升级 + 过路费 + 驻守机制 | 3天 |
| M6 | 商店系统：商品列表 + 购买逻辑 + 背包管理 | 2天 |
| M7 | 事件/营地：随机事件池 + 营地治疗/放生 | 2天 |
| M8 | AI系统：简单AI对手 + PvE模式 | 3天 |
| M9 | 存档系统：JSON序列化 + 自动存档 + 读档 | 2天 |
| M10 | 打磨：UI美化 + 音效 + 数值平衡 + Bug修复 | 3天 |

---

## 编码规范

### C++ 代码规范
- 使用 C++17 标准
- 命名规范：
  - 类名：`PascalCase`（如 `BattleSystem`）
  - 函数名：`snake_case`（如 `calculate_damage`）
  - 成员变量：`snake_case_`（如 `current_hp_`）
  - 枚举值：`PascalCase`（如 `MonsterType::Fire`）
  - 常量：`kPascalCase`（如 `kMaxLevel`）
- 头文件使用 `#pragma once`
- 智能指针优先（`std::shared_ptr` / `std::unique_ptr`）
- 异常处理：使用 `try-catch`，记录日志
- 每个类一个 `.h` + `.cpp` 文件对

### 提交规范
- 每完成一个里程碑提交一次
- 注释关键逻辑（伤害公式、状态切换、概率计算）
- 保持代码风格一致

---

## 教学说明

### 学习模式（重要！）
作为课程作业项目，开发过程中：
1. **解释每一步做什么** - 不只是生成代码，还要说明为什么
2. **讲解关键概念** - 遇到 C++ 语法、设计模式、SFML用法等会详细解释
3. **多种实现对比** - 重要的实现会分析不同方案的优缺点
4. **回答所有问题** - 任何不懂的地方都可以问

### 通过本项目应掌握的知识
- C++ 面向对象编程（继承、多态、虚函数）
- CMake 构建项目
- SFML 2D游戏开发（窗口、渲染、事件、音频）
- 游戏状态机设计模式
- JSON 数据驱动设计
- 游戏循环与帧率控制
- 伤害公式与数值设计

---

## 交流偏好
- 使用中文交流
- 解释代码时详细一些，帮助学习理解
- 遇到不懂的术语时顺便解释

---

## 技术选型说明

### 为什么选择 SFML？
```
SFML vs 其他方案：
  SFML:
    + 轻量级，API简洁直观
    + 2D渲染、音频、输入一体化
    + C++原生，无额外绑定
    + 社区活跃，文档完善
    + 适合课程项目规模
  SDL2:
    - C风格API，需要更多封装
    + 更底层，灵活性更高
  Raylib:
    + 极简API
    - 生态较小
  结论：SFML 平衡了易用性和功能性，最适合本项目
```

### 为什么用状态机架构？
```
状态机 vs 其他架构：
  状态机:
    + 状态切换清晰（菜单→游戏→战斗→商店）
    + 每个状态独立管理自己的输入/更新/渲染
    + 栈式设计支持状态叠加（战斗覆盖在游戏上）
    + 适合回合制游戏
  ECS (Entity-Component-System):
    - 过于复杂，适合大型项目
    - 学习成本高
  结论：状态机是回合制游戏的经典架构，简单有效
```
