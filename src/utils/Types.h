#pragma once

#include <string>

// ── 精灵属性类型 ──────────────────────────────────────────
enum class MonsterType {
    Normal,
    Fire,
    Water,
    Grass,
    Electric,
    Ice,
    Dragon,
    Dark,
    Ghost,
    Psychic
};

// ── 格子类型 ──────────────────────────────────────────────
enum class TileType {
    Start,      // 起点（路过发500工资）
    Empty,      // 空地（可购买）
    Wild,       // 野怪格（遇怪/捕捉）
    Shop,       // 商店格（打开商店）
    Event,      // 事件格（随机好事/坏事）
    Camp,       // 营地格（治疗/放生）
    Battle      // 对战格（强制PK）
};

// ── 地皮设施等级 ──────────────────────────────────────────
enum class PropertyLevel {
    Empty = 0,  // 空地
    Hut   = 1,  // 小屋
    Shop  = 2,  // 商店
    Gym   = 3   // 道馆
};

// ── 技能类别 ──────────────────────────────────────────────
enum class SkillCategory { Physical, Special };

// ── 稀有度 ────────────────────────────────────────────────
enum class Rarity { Common, Rare, Legendary };

// ── 战斗类型 ──────────────────────────────────────────────
enum class BattleType { Wild, Guardian, PvP };

// ── 战斗结果 ──────────────────────────────────────────────
enum class BattleResult { Ongoing, Win, Lose, Escaped, Captured };

// ── 游戏模式 ──────────────────────────────────────────────
enum class GameMode { PvP, PvE };

// ── 游戏结果 ──────────────────────────────────────────────
enum class GameOverReason { Bankrupt, MonstersExhausted, MaxLaps };

// ── 道具类型 ──────────────────────────────────────────────
enum class ItemType {
    Pokeball,
    SuperBall,
    Heal50,
    Heal100,
    Antidote,
    Revive,
    FireStone,
    WaterStone,
    GrassStone,
    ThunderStone,
    IceStone
};

// ── 事件类型 ──────────────────────────────────────────────
enum class EventType {
    GoodLuck,       // 财运亨通：得钱
    BadLuck,        // 破财消灾：扣钱
    MonsterHurt,    // 精灵受伤
    MonsterHeal,    // 精灵痊愈
    FreeCapture,    // 免费捕捉
    MonsterLost     // 精灵走失
};

// ── 玩家ID常量 ────────────────────────────────────────────
constexpr int kPlayer1Id = 0;
constexpr int kPlayer2Id = 1;

// ── 游戏常量 ──────────────────────────────────────────────
constexpr int kInitGold       = 1000;   // 初始金币
constexpr int kLapReward      = 500;    // 走完一圈奖励
constexpr int kBoardSize      = 20;     // 棋盘格子数
constexpr int kMaxMonsters    = 6;      // 每玩家最多精灵数
constexpr int kMaxLevel       = 20;     // 精灵最大等级
constexpr int kDiceMin        = 1;      // 骰子最小值
constexpr int kDiceMax        = 6;      // 骰子最大值

// ── 地皮价格常量 ──────────────────────────────────────────
constexpr int kPropertyBasePrices[4] = { 0, 300, 600, 1200 };   // 各等级基础价格
constexpr int kPropertyTolls[4]      = { 0, 100, 200, 400 };    // 各等级基础过路费
constexpr float kPropertyMultipliers[4] = { 1.0f, 1.0f, 1.5f, 2.0f }; // 等级系数

// ── 捕捉概率 ──────────────────────────────────────────────
constexpr float kCaptureBaseRate = 0.5f;       // 满HP基础捕捉率
constexpr float kCaptureHpBonus   = 0.005f;     // 每损失1%HP增加0.5%
constexpr float kCaptureBallBonus = 0.2f;       // 超级球加成
