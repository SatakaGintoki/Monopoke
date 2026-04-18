#pragma once

#include <string>
#include <vector>

class Player;
class Board;
class Monster;

class SaveManager {
public:
    static SaveManager& get();

    // 保存游戏状态到文件
    bool save_game(const std::string& path = "saves/autosave.json");

    // 加载游戏状态
    bool load_game(const std::string& path = "saves/autosave.json");

    // 获取当前存档信息
    std::string get_save_info(const std::string& path = "saves/autosave.json") const;

    // 是否有存档
    bool has_save(const std::string& path = "saves/autosave.json") const;

private:
    SaveManager() = default;

    std::string last_save_path_;
};
