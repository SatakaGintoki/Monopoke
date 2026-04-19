#pragma once

#include <string>
#include <vector>

class Player;
class Board;
class Monster;

class SaveManager {
public:
    static SaveManager& get();

    static constexpr const char* kAutosavePath = "saves/autosave.json";

    // 保存完整对局（棋盘、双玩家、回合）
    bool save_play_state(const Player& p1, const Player& p2, const Board& board,
                         int current_player, int turn_count, int game_mode = 0,
                         const std::string& path = kAutosavePath);

    // 从文件恢复对局
    bool load_play_state(Player& p1, Player& p2, Board& board,
                         int& current_player, int& turn_count, int& game_mode,
                         const std::string& path = kAutosavePath);

    // 兼容旧接口：写入完整状态需使用 save_play_state
    bool save_game(const std::string& path = kAutosavePath);

    // 若文件存在且可解析为有效存档则 true
    bool load_game(const std::string& path = kAutosavePath);

    std::string get_save_info(const std::string& path = kAutosavePath) const;

    bool has_save(const std::string& path = kAutosavePath) const;

private:
    SaveManager() = default;

    std::string last_save_path_;
};
