#include "PlayState.h"
#include "../core/Game.h"
#include "../system/MonsterSystem.h"
#include "../system/EventSystem.h"
#include "../system/AISystem.h"
#include "../system/EconomySystem.h"
#include "../utils/SaveManager.h"
#include "../utils/ConfigManager.h"
#include "../ui/RetroUI.h"
#include "json.hpp"
#include "BattleState.h"
#include "ShopState.h"
#include "ResultState.h"
#include <algorithm>
#include <cmath>

namespace {

ItemType parse_item_type_str(const std::string& s) {
    if (s == "Pokeball") {
        return ItemType::Pokeball;
    }
    if (s == "SuperBall") {
        return ItemType::SuperBall;
    }
    if (s == "Heal50") {
        return ItemType::Heal50;
    }
    if (s == "Heal100") {
        return ItemType::Heal100;
    }
    if (s == "Antidote") {
        return ItemType::Antidote;
    }
    if (s == "Revive") {
        return ItemType::Revive;
    }
    if (s == "FireStone") {
        return ItemType::FireStone;
    }
    if (s == "WaterStone") {
        return ItemType::WaterStone;
    }
    if (s == "GrassStone") {
        return ItemType::GrassStone;
    }
    if (s == "ThunderStone") {
        return ItemType::ThunderStone;
    }
    if (s == "IceStone") {
        return ItemType::IceStone;
    }
    return ItemType::Heal50;
}

Item item_from_json(const nlohmann::json& j) {
    Item it;
    it.type = parse_item_type_str(j.value("type", std::string()));
    it.name = j.value("name", std::string(""));
    it.price = j.value("price", 0);
    it.count = 1;
    return it;
}

bool consume_one_ball(Player& p, float& bonus_out) {
    auto& inv = p.inventory();
    for (size_t i = 0; i < inv.size(); ++i) {
        if (inv[i].type == ItemType::SuperBall && inv[i].count > 0) {
            --inv[i].count;
            if (inv[i].count <= 0) {
                inv.erase(inv.begin() + static_cast<std::ptrdiff_t>(i));
            }
            bonus_out = 0.2f;
            return true;
        }
    }
    for (size_t i = 0; i < inv.size(); ++i) {
        if (inv[i].type == ItemType::Pokeball && inv[i].count > 0) {
            --inv[i].count;
            if (inv[i].count <= 0) {
                inv.erase(inv.begin() + static_cast<std::ptrdiff_t>(i));
            }
            bonus_out = 0.f;
            return true;
        }
    }
    return false;
}

} // namespace

PlayState::PlayState(bool load_from_save, GameMode mode)
    : load_from_save_(load_from_save)
    , initial_game_mode_(mode) {}

void PlayState::init() {
    if (!font_.loadFromFile("C:/Windows/Fonts/msyh.ttc")) {
        font_.loadFromFile("C:/Windows/Fonts/arial.ttf");
    }

    board_view_.setSize(1440.f, 960.f);
    ui_view_.setSize(1440.f, 960.f);
    ui_view_.setCenter(720.f, 480.f);

    if (load_from_save_) {
        int cp = 0;
        int tc = 0;
        int gm = 0;
        if (SaveManager::get().load_play_state(players_[0], players_[1], board_, cp, tc, gm,
                                                 SaveManager::kAutosavePath)) {
            current_player_ = cp;
            turn_count_ = tc;
            game_mode_ = (gm == 1) ? GameMode::PvP : GameMode::PvE;
            renderer_.init(board_);
            phase_ = TurnPhase::WaitRoll;
            dice_result_ = 0;
            steps_remaining_ = 0;
            ai_guardian_attempted_ = false;
            message_ = (current_player_ == 0 ? L"\u73a9\u5bb61" : L"\u73a9\u5bb62");
            message_ += L" \u7684\u56de\u5408\uff0c\u6309\u7a7a\u683c\u63b7\u9ab0";
            camera_pos_ = renderer_.tile_center(players_[current_player_].position());
            board_view_.setCenter(camera_pos_);
            return;
        }
    }

    init_new_game();
    camera_pos_ = renderer_.tile_center(players_[current_player_].position());
    board_view_.setCenter(camera_pos_);
}

void PlayState::init_new_game() {
    board_.init_from_json("assets/data/tiles.json");
    if (board_.tiles().size() != static_cast<size_t>(kBoardSize)) {
        board_.init_default();
    }
    players_[0] = Player(kPlayer1Id, "Player1");
    players_[1] = Player(kPlayer2Id, "Player2");

    auto starters = MonsterSystem::get().get_starter_choices();
    if (!starters.empty()) {
        players_[0].add_monster(starters[0]);
        if (starters.size() >= 2) {
            players_[1].add_monster(starters[1]);
        }
    }

    renderer_.init(board_);

    current_player_ = 0;
    turn_count_ = 0;
    game_mode_ = initial_game_mode_;
    phase_ = TurnPhase::WaitRoll;
    dice_result_ = 0;
    steps_remaining_ = 0;
    message_ = L"\u73a9\u5bb61 \u7684\u56de\u5408\uff0c\u6309\u7a7a\u683c\u63b7\u9ab0";

    ai_guardian_attempted_ = false;
    SaveManager::get().save_play_state(players_[0], players_[1], board_, current_player_, turn_count_, 0,
                                        SaveManager::kAutosavePath);
}

void PlayState::resume() {
    GameState::resume();
    BattleExitSnapshot snap = BattleState::consume_exit_snapshot();
    if (snap.valid && pending_battle_ == PendingBattle::GuardianTile &&
        snap.type == BattleType::Guardian) {
        resolve_guardian_after_battle(snap.result);
    } else if (snap.valid && pending_battle_ == PendingBattle::ArenaPvP &&
               snap.type == BattleType::PvP) {
        resolve_arena_after_battle(snap.result);
    } else if (snap.valid && snap.type == BattleType::Wild) {
        Player& pl = players_[current_player_];
        if (snap.result == BattleResult::Win) {
            pl.add_gold(snap.gold_reward);
        } else if (snap.result == BattleResult::Lose) {
            const int pen = 100 + Random::get().range(0, 200);
            const int pay = std::min(pen, pl.gold());
            pl.spend_gold(pay);
        }
    }
    pending_battle_ = PendingBattle::None;
    pending_guardian_tile_pos_ = -1;
}

void PlayState::resolve_guardian_after_battle(BattleResult r) {
    int pos = pending_guardian_tile_pos_;
    if (pos < 0 || pos >= kBoardSize) {
        return;
    }
    Player& visitor = players_[current_player_];
    Tile& tile = board_.tile_at(pos);
    int oid = tile.owner_id();
    if (oid < 0 || oid > 1 || oid == visitor.id()) {
        return;
    }
    Player& owner = players_[oid];

    if (r == BattleResult::Win) {
        int gix = tile.guardian_idx();
        if (tile.has_guardian() && gix >= 0 && gix < static_cast<int>(owner.monsters().size())) {
            Monster stolen = owner.monsters()[gix];
            tile.clear_guardian();
            owner.remove_monster(gix);
            adjust_guardian_indices_after_remove(oid, gix);
            visitor.add_monster(stolen);
        }
        message_ = L"\u51fb\u8d25\u9a7b\u5b88\uff0c\u83b7\u5f97\u5bf9\u65b9\u7cbe\u7075\uff01";
    } else if (r == BattleResult::Lose) {
        int toll = tile.toll();
        if (visitor.gold() >= toll) {
            visitor.spend_gold(toll);
            owner.add_gold(toll);
            message_ = L"\u9a7b\u5b88\u6218\u5931\u8d25\uff0c\u652f\u4ed8\u8fc7\u8def\u8d39";
        } else {
            message_ = L"\u91d1\u5e01\u4e0d\u8db3\uff0c\u65e0\u6cd5\u4ea4\u8fc7\u8def\u8d39";
        }
    } else {
        message_ = L"\u6218\u6597\u7ed3\u675f";
    }
}

void PlayState::adjust_guardian_indices_after_remove(int owner_id, int removed_index) {
    for (int i = 0; i < kBoardSize; ++i) {
        Tile& t = board_.tile_at(i);
        if (t.owner_id() != owner_id || !t.has_guardian()) {
            continue;
        }
        int g = t.guardian_idx();
        if (g > removed_index) {
            t.set_guardian(g - 1);
        }
    }
}

bool PlayState::try_assign_guardian_at_current(int monster_index) {
    if (phase_ != TurnPhase::WaitRoll) {
        return false;
    }
    Player& p = players_[current_player_];
    Tile& t = board_.tile_at(p.position());
    if (t.type() != TileType::Empty || t.owner_id() != p.id()) {
        return false;
    }
    if (t.has_guardian()) {
        return false;
    }
    if (monster_index < 0 || monster_index >= static_cast<int>(p.monsters().size())) {
        return false;
    }
    if (!p.monsters()[monster_index].is_alive()) {
        return false;
    }
    for (int i = 0; i < kBoardSize; ++i) {
        const Tile& ot = board_.tile_at(i);
        if (ot.owner_id() == p.id() && ot.has_guardian() && ot.guardian_idx() == monster_index) {
            return false;
        }
    }
    t.set_guardian(monster_index);
    message_ = L"\u5df2\u6d3e\u9a7b\u5b88\u7cbe\u7075";
    return true;
}

void PlayState::ai_try_assign_guardian() {
    if (game_mode_ != GameMode::PvE || current_player_ != 1) {
        return;
    }
    Player& p = players_[1];
    Tile& t = board_.tile_at(p.position());
    if (t.type() != TileType::Empty || t.owner_id() != p.id() || t.has_guardian()) {
        return;
    }
    int slot = AISystem::get().choose_guardian_slot(p, board_);
    if (slot < 0) {
        return;
    }
    t.set_guardian(slot);
}

void PlayState::enter_turn_end_(const std::wstring& msg) {
    message_ = msg;
    phase_ = TurnPhase::TurnEnd;
    move_timer_ = 0.f;
}

void PlayState::try_wild_capture() {
    Player& p = players_[current_player_];
    float bonus = 0.f;
    if (!consume_one_ball(p, bonus)) {
        enter_turn_end_(L"\u6ca1\u6709\u7cbe\u7075\u7403\uff01");
        return;
    }
    if (MonsterSystem::get().try_capture(wild_encounter_, bonus)) {
        p.add_monster(wild_encounter_);
        enter_turn_end_(L"\u6355\u6349\u6210\u529f\uff01");
    } else {
        enter_turn_end_(L"\u6355\u6349\u5931\u8d25\uff01");
    }
}

void PlayState::start_wild_battle() {
    Player& p = players_[current_player_];
    if (p.monsters().empty()) {
        enter_turn_end_(L"\u6ca1\u6709\u7cbe\u7075\uff01");
        return;
    }
    Monster& player_mon = const_cast<Monster&>(p.monsters()[0]);
    const bool ai = (game_mode_ == GameMode::PvE && current_player_ == 1);
    push_state(Game::get().state_manager(),
               new BattleState(player_mon, wild_encounter_, BattleType::Wild, ai));
    phase_ = TurnPhase::TurnEnd;
    move_timer_ = 0.f;
}

void PlayState::finish_wild_flee() {
    enter_turn_end_(L"\u5df2\u9003\u79bb\u91ce\u751f\u7cbe\u7075\u3002");
}

void PlayState::camp_heal_monster(int index) {
    Player& p = players_[current_player_];
    if (index < 0 || index >= static_cast<int>(p.monsters().size())) {
        return;
    }
    Monster& m = p.monsters()[index];
    if (!m.is_alive()) {
        return;
    }
    m.set_current_hp(m.max_hp());
    enter_turn_end_(L"\u7cbe\u7075\u5df2\u56de\u6ee1\u8840\u91cf\u3002");
}

void PlayState::camp_release_last() {
    Player& p = players_[current_player_];
    if (p.monsters().size() <= 1) {
        enter_turn_end_(L"\u81f3\u5c11\u7559\u4e00\u53ea\u7cbe\u7075\uff0c\u65e0\u6cd5\u653e\u751f\u3002");
        return;
    }
    p.remove_monster(static_cast<int>(p.monsters().size()) - 1);
    p.add_gold(50);
    enter_turn_end_(L"\u653e\u751f\u7cbe\u7075\uff0c\u83b7\u5f97 50 \u91d1\u5e01\u3002");
}

void PlayState::grant_random_loot_item(Player& player) {
    const auto& arr = ConfigManager::get().items();
    if (!arr.is_array() || arr.empty()) {
        return;
    }
    const int n = static_cast<int>(arr.size());
    const int idx = Random::get().range(0, n - 1);
    player.add_item(item_from_json(arr[idx]));
}

void PlayState::resolve_arena_after_battle(BattleResult r) {
    Player& p = players_[current_player_];
    if (r == BattleResult::Win) {
        p.add_gold(500);
        grant_random_loot_item(p);
        message_ = L"\u5bf9\u6218\u80dc\u5229\uff01+500G \u968f\u673a\u9053\u5177";
    } else if (r == BattleResult::Lose) {
        message_ = L"\u5bf9\u6218\u5931\u8d25\u3002";
    } else {
        message_ = L"\u5bf9\u6218\u7ed3\u675f\u3002";
    }
}

void PlayState::ai_wild_choice() {
    Player& p = players_[current_player_];
    bool has_ball = false;
    for (const auto& it : p.inventory()) {
        if ((it.type == ItemType::Pokeball || it.type == ItemType::SuperBall) && it.count > 0) {
            has_ball = true;
            break;
        }
    }
    const float hp_ratio = static_cast<float>(wild_encounter_.current_hp()) /
                           static_cast<float>(std::max(1, wild_encounter_.max_hp()));
    if (has_ball && hp_ratio < 0.45f && Random::get().probability(0.45f)) {
        try_wild_capture();
    } else if (Random::get().probability(0.12f)) {
        finish_wild_flee();
    } else {
        start_wild_battle();
    }
}

void PlayState::ai_camp_choice() {
    Player& p = players_[current_player_];
    for (int i = 0; i < static_cast<int>(p.monsters().size()); ++i) {
        Monster& m = p.monsters()[i];
        if (m.is_alive() && m.current_hp() < m.max_hp()) {
            camp_heal_monster(i);
            return;
        }
    }
    if (p.monsters().size() > 1) {
        camp_release_last();
    } else {
        enter_turn_end_(L"\u8425\u5730\u65e0\u4e8b\u53ef\u505a\u3002");
    }
}

void PlayState::ai_arena_choice() {
    if (Random::get().probability(0.82f)) {
        Player& p = players_[current_player_];
        Player& opponent = players_[1 - current_player_];
        if (!p.monsters().empty() && !opponent.monsters().empty()) {
            Monster& player_mon = const_cast<Monster&>(p.monsters()[0]);
            Monster& opponent_mon = const_cast<Monster&>(opponent.monsters()[0]);
            const bool ai = (game_mode_ == GameMode::PvE && current_player_ == 1);
            pending_battle_ = PendingBattle::ArenaPvP;
            push_state(Game::get().state_manager(),
                       new BattleState(player_mon, opponent_mon, BattleType::PvP, ai));
            phase_ = TurnPhase::TurnEnd;
            move_timer_ = 0.f;
        }
    } else {
        enter_turn_end_(L"\u9009\u62e9\u8df3\u8fc7\u5bf9\u6218\u3002");
    }
}

void PlayState::handle_event(const sf::Event& event) {
    if (event.type != sf::Event::KeyPressed) {
        return;
    }

    const bool human_turn = (game_mode_ == GameMode::PvP || current_player_ == 0);

    if (phase_ == TurnPhase::WildChoice && human_turn) {
        if (event.key.code == sf::Keyboard::C) {
            try_wild_capture();
        }
        if (event.key.code == sf::Keyboard::B) {
            start_wild_battle();
        }
        if (event.key.code == sf::Keyboard::V) {
            finish_wild_flee();
        }
        return;
    }

    if (phase_ == TurnPhase::CampChoice && human_turn) {
        int idx = -1;
        if (event.key.code == sf::Keyboard::F1) {
            idx = 0;
        }
        if (event.key.code == sf::Keyboard::F2) {
            idx = 1;
        }
        if (event.key.code == sf::Keyboard::F3) {
            idx = 2;
        }
        if (event.key.code == sf::Keyboard::F4) {
            idx = 3;
        }
        if (event.key.code == sf::Keyboard::F5) {
            idx = 4;
        }
        if (event.key.code == sf::Keyboard::F6) {
            idx = 5;
        }
        if (idx >= 0) {
            camp_heal_monster(idx);
        }
        if (event.key.code == sf::Keyboard::Backspace) {
            camp_release_last();
        }
        return;
    }

    if (phase_ == TurnPhase::ArenaChoice && human_turn) {
        if (event.key.code == sf::Keyboard::F1) {
            Player& p = players_[current_player_];
            Player& opponent = players_[1 - current_player_];
            if (!p.monsters().empty() && !opponent.monsters().empty()) {
                Monster& player_mon = const_cast<Monster&>(p.monsters()[0]);
                Monster& opponent_mon = const_cast<Monster&>(opponent.monsters()[0]);
                const bool ai = (game_mode_ == GameMode::PvE && current_player_ == 1);
                pending_battle_ = PendingBattle::ArenaPvP;
                push_state(Game::get().state_manager(),
                           new BattleState(player_mon, opponent_mon, BattleType::PvP, ai));
                phase_ = TurnPhase::TurnEnd;
                move_timer_ = 0.f;
            }
        }
        if (event.key.code == sf::Keyboard::F2) {
            enter_turn_end_(L"\u8df3\u8fc7\u5bf9\u6218\u3002");
        }
        return;
    }

    if (event.key.code == sf::Keyboard::Space && phase_ == TurnPhase::WaitRoll) {
        if (game_mode_ == GameMode::PvP || current_player_ == 0) {
            roll_dice();
        }
    }
    if (phase_ == TurnPhase::WaitRoll) {
        int idx = -1;
        if (event.key.code == sf::Keyboard::F1) {
            idx = 0;
        }
        if (event.key.code == sf::Keyboard::F2) {
            idx = 1;
        }
        if (event.key.code == sf::Keyboard::F3) {
            idx = 2;
        }
        if (event.key.code == sf::Keyboard::F4) {
            idx = 3;
        }
        if (event.key.code == sf::Keyboard::F5) {
            idx = 4;
        }
        if (event.key.code == sf::Keyboard::F6) {
            idx = 5;
        }
        if (idx >= 0 && (game_mode_ == GameMode::PvP || current_player_ == 0)) {
            try_assign_guardian_at_current(idx);
        }
    }
}

void PlayState::roll_dice() {
    dice_result_ = Random::get().roll_dice();
    steps_remaining_ = dice_result_;
    move_timer_ = 0.f;
    phase_ = TurnPhase::Moving;

    message_ = (current_player_ == 0 ? L"\u73a9\u5bb61" : L"\u73a9\u5bb62");
    message_ += L" \u63b7\u51fa\u4e86 " + std::to_wstring(dice_result_) + L" \u70b9";
}

void PlayState::update(float dt) {
    sf::Vector2f target_pos = renderer_.tile_center(players_[current_player_].position());
    camera_pos_ += (target_pos - camera_pos_) * 5.0f * dt;
    board_view_.setCenter(camera_pos_);

    // AI自动掷骰（PvE 且玩家2回合）
    if (phase_ == TurnPhase::WaitRoll) {
        if (game_mode_ == GameMode::PvE && current_player_ == 1) {
            if (!ai_guardian_attempted_) {
                ai_try_assign_guardian();
                ai_guardian_attempted_ = true;
            }
            ai_think_timer_ += dt;
            if (ai_think_timer_ >= kAiThinkDelay) {
                ai_think_timer_ = 0.f;
                roll_dice();
            }
        } else {
            ai_think_timer_ = 0.f;  // 重置，避免累积
        }
    }

    if (phase_ == TurnPhase::WildChoice && game_mode_ == GameMode::PvE && current_player_ == 1) {
        sub_menu_timer_ += dt;
        if (sub_menu_timer_ >= kSubMenuAiDelay) {
            sub_menu_timer_ = 0.f;
            ai_wild_choice();
        }
    }

    if (phase_ == TurnPhase::CampChoice && game_mode_ == GameMode::PvE && current_player_ == 1) {
        sub_menu_timer_ += dt;
        if (sub_menu_timer_ >= kSubMenuAiDelay) {
            sub_menu_timer_ = 0.f;
            ai_camp_choice();
        }
    }

    if (phase_ == TurnPhase::ArenaChoice && game_mode_ == GameMode::PvE && current_player_ == 1) {
        sub_menu_timer_ += dt;
        if (sub_menu_timer_ >= kSubMenuAiDelay) {
            sub_menu_timer_ = 0.f;
            ai_arena_choice();
        }
    }

    if (phase_ == TurnPhase::Moving) {
        move_timer_ += dt;
        if (move_timer_ >= kMoveInterval && steps_remaining_ > 0) {
            move_timer_ = 0.f;
            Player& p = players_[current_player_];
            int old_pos = p.position();
            int new_pos = board_.forward(old_pos, 1);
            p.set_position(new_pos);

            if (new_pos == 0 && old_pos != 0) {
                p.add_gold(kLapReward);
                p.add_lap();
            }

            --steps_remaining_;
            if (steps_remaining_ == 0) {
                phase_ = TurnPhase::TileAction;
                on_tile_landed();
            }
        }
    }

    if (phase_ == TurnPhase::TurnEnd) {
        move_timer_ += dt;
        if (move_timer_ >= 0.8f) {
            next_turn();
        }
    }
}

void PlayState::on_tile_landed() {
    pending_battle_ = PendingBattle::None;
    pending_guardian_tile_pos_ = -1;

    Player& p = players_[current_player_];
    const Tile& tile = board_.tile_at(p.position());

    switch (tile.type()) {
        case TileType::Start:
            message_ = L"\u56de\u5230\u8d77\u70b9\uff0c\u83b7\u5f97 " + std::to_wstring(kLapReward) + L" \u91d1\u5e01";
            break;
        case TileType::Empty:
            if (!tile.has_owner()) {
                // AI决策是否购买
                bool should_buy = (current_player_ == 0) || AISystem::get().should_buy_property(p, tile);
                if (should_buy && EconomySystem::get().try_purchase(p, board_.tile_at(p.position()))) {
                    message_ = L"\u8d2d\u4e70\u4e86\u5730\u76ae\uff01\u82b1\u8d39 " + std::to_wstring(tile.price()) + L"G";
                } else if (!should_buy) {
                    message_ = L"\u7a7a\u5730\uff0c\u8df3\u8fc7\u8d2d\u4e70";
                } else {
                    message_ = L"\u91d1\u5e01\u4e0d\u8db3\uff0c\u65e0\u6cd5\u8d2d\u4e70";
                }
            } else if (tile.owner_id() != p.id()) {
                Player& owner = players_[tile.owner_id()];
                if (tile.has_guardian()) {
                    int gix = tile.guardian_idx();
                    if (gix >= 0 && gix < static_cast<int>(owner.monsters().size()) &&
                        owner.monsters()[gix].is_alive() && !p.monsters().empty()) {
                        Monster& player_mon = const_cast<Monster&>(p.monsters()[0]);
                        Monster& gmon = owner.monsters()[gix];
                        const bool ai = (game_mode_ == GameMode::PvE && current_player_ == 1);
                        pending_battle_ = PendingBattle::GuardianTile;
                        pending_guardian_tile_pos_ = p.position();
                        push_state(Game::get().state_manager(),
                                   new BattleState(player_mon, gmon, BattleType::Guardian, ai));
                        phase_ = TurnPhase::TurnEnd;
                        move_timer_ = 0.f;
                        return;
                    }
                }
                int toll = tile.toll();
                if (p.gold() >= toll) {
                    p.spend_gold(toll);
                    players_[tile.owner_id()].add_gold(toll);
                    message_ = L"\u7ecf\u8fc7\u5bf9\u65b9\u5730\u76ae\uff0c\u4ea4\u8fc7\u8def\u8d39 " + std::to_wstring(toll) + L"G";
                } else {
                    message_ = L"\u91d1\u5e01\u4e0d\u8db3\uff0c\u65e0\u6cd5\u4ea4\u8fc7\u8def\u8d39\uff01";
                }
            } else {
                if (tile.can_upgrade()) {
                    int cost = tile.upgrade_cost();
                    bool should_upgrade = (current_player_ == 0) || AISystem::get().should_upgrade_property(p, tile);
                    if (should_upgrade && EconomySystem::get().try_upgrade(p, board_.tile_at(p.position()))) {
                        message_ = L"\u5347\u7ea7\u5730\u76ae\uff01\u82b1\u8d39 " + std::to_wstring(cost) + L"G";
                    } else if (!should_upgrade) {
                        message_ = L"\u81ea\u5df1\u7684\u5730\u76ae\uff0c\u8df3\u8fc7\u5347\u7ea7";
                    } else {
                        message_ = L"\u81ea\u5df1\u7684\u5730\u76ae\uff0c\u91d1\u5e01\u4e0d\u8db3\u5347\u7ea7";
                    }
                } else {
                    message_ = L"\u81ea\u5df1\u7684\u5730\u76ae\uff0c\u5df2\u6ee1\u7ea7";
                }
            }
            break;
        case TileType::Wild: {
            if (p.monsters().empty()) {
                message_ = L"\u6ca1\u6709\u7cbe\u7075\uff0c\u65e0\u6cd5\u5e94\u5bf9\u91ce\u751f\u7cbe\u7075\uff01";
                break;
            }
            wild_encounter_ = MonsterSystem::get().generate_wild_monster();
            phase_ = TurnPhase::WildChoice;
            sub_menu_timer_ = 0.f;
            message_ = L"\u91ce\u751f\u7cbe\u7075\u51fa\u6ca1! C=\u6355\u6349 B=\u6218\u6597 V=\u9003\u8dd1";
            return;
        }
        case TileType::Shop:
            push_state(Game::get().state_manager(), new ShopState(players_[current_player_]));
            phase_ = TurnPhase::TurnEnd;
            move_timer_ = 0.f;
            return;
        case TileType::Event: {
            EventSystem::get().load_from_json("assets/data/events.json");
            RandomEvent ev = EventSystem::get().trigger_event(players_[current_player_]);
            message_ = EventSystem::get().get_event_description(ev);
            break;
        }
        case TileType::Camp: {
            phase_ = TurnPhase::CampChoice;
            sub_menu_timer_ = 0.f;
            message_ = L"\u8425\u5730: F1-F6 \u56de\u6ee1\u5355\u53ea\u7cbe\u7075\uff0cBackspace \u653e\u751f\u6700\u540e\u4e00\u53ea+50G";
            return;
        }
        case TileType::Battle: {
            Player& opponent = players_[1 - current_player_];
            if (!p.monsters().empty() && !opponent.monsters().empty()) {
                phase_ = TurnPhase::ArenaChoice;
                sub_menu_timer_ = 0.f;
                message_ = L"\u5bf9\u6218\u683c: F1 \u5f00\u6218 F2 \u9003\u8dd1 (\u80dc+500G+\u9053\u5177)";
                return;
            }
            message_ = L"\u53cc\u65b9\u9700\u6709\u7cbe\u7075\u624d\u80fd\u5bf9\u6218\uff01";
            break;
        }
    }

    move_timer_ = 0.f;
    phase_ = TurnPhase::TurnEnd;
}

void PlayState::next_turn() {
    // 检查游戏是否结束
    int winner = check_game_over();
    if (winner == 2) {
        push_state(Game::get().state_manager(), new ResultState(GameResult::Draw, -1));
        return;
    }
    if (winner >= 0) {
        GameResult result = (winner == 0) ? GameResult::Player1Win : GameResult::Player2Win;
        push_state(Game::get().state_manager(), new ResultState(result, winner));
        return;
    }

    ++turn_count_;
    current_player_ = 1 - current_player_;
    phase_ = TurnPhase::WaitRoll;
    move_timer_ = 0.f;
    ai_guardian_attempted_ = false;

    const int gm = (game_mode_ == GameMode::PvP) ? 1 : 0;
    SaveManager::get().save_play_state(players_[0], players_[1], board_, current_player_, turn_count_,
                                        gm, SaveManager::kAutosavePath);

    message_ = (current_player_ == 0 ? L"\u73a9\u5bb61" : L"\u73a9\u5bb62");
    message_ += L" \u7684\u56de\u5408\uff0c\u6309\u7a7a\u683c\u63b7\u9ab0";
}

int PlayState::check_game_over() const {
    // 检查玩家1
    bool p1_alive = players_[0].has_alive_monsters();
    bool p1_bankrupt = (players_[0].gold() == 0) && (players_[0].total_properties() == 0);

    // 检查玩家2
    bool p2_alive = players_[1].has_alive_monsters();
    bool p2_bankrupt = (players_[1].gold() == 0) && (players_[1].total_properties() == 0);

    // 精灵全灭或破产则失败
    if (!p1_alive || p1_bankrupt) return 1;  // 玩家2获胜
    if (!p2_alive || p2_bankrupt) return 0;  // 玩家1获胜

    if (turn_count_ >= kStalemateTurnCount) {
        return 2;  // 平局
    }

    return -1;  // 游戏继续
}

// ── 渲染辅助 ──────────────────────────────────────────

static void draw_panel(sf::RenderWindow& window, float x, float y, float w, float h) {
    RetroUI::draw_box(window, x, y, w, h, sf::Color::White, sf::Color(40, 40, 40), 4.f);
}

static void draw_label(sf::RenderWindow& window, sf::Font& font,
                       const std::wstring& text, float x, float y,
                       int size, sf::Color color) {
    // 强制黑色或者深灰色，复古风
    RetroUI::draw_text(window, text, x, y, size, sf::Color(40, 40, 40));
}

static void draw_bar(sf::RenderWindow& window, float x, float y, float w, float h,
                     float ratio, sf::Color fill, sf::Color bg = sf::Color(40, 40, 60)) {
    RetroUI::draw_hp_bar(window, x, y, w, h, static_cast<int>(ratio * 100), 100);
}

void PlayState::render(sf::RenderWindow& window) {
    // ── 纯色平铺背景 ──────────────────────────────────────────
    sf::RectangleShape bg({1440.f, 960.f}); // 覆盖整个大窗口
    bg.setFillColor(sf::Color(248, 248, 240));
    window.setView(ui_view_);
    window.draw(bg);

    // 棋盘区域 (带摄像机跟随)
    window.setView(board_view_);
    renderer_.render(window, board_, players_[0], players_[1]);

    // 恢复 UI 视图
    window.setView(ui_view_);

    // ── 紧凑型 UI 面板 ──

    // 玩家1面板 (左上角)
    {
        float px = 20.f;
        float py = 20.f;
        float pw = 280.f;
        draw_panel(window, px, py, pw, 100.f);

        if (current_player_ == 0) {
            RetroUI::draw_text(window, L"►", px + 8.f, py + 10.f, 18, sf::Color(40, 40, 40));
        }

        draw_label(window, font_, L"玩家1", px + 30.f, py + 10.f, 18, sf::Color(40, 40, 40));
        draw_label(window, font_, L"金币", px + 30.f, py + 40.f, 16, sf::Color(40, 40, 40));
        draw_label(window, font_, std::to_wstring(players_[0].gold()) + L" G",
                   px + 80.f, py + 40.f, 16, sf::Color(40, 40, 40));

        draw_label(window, font_, L"地皮", px + 30.f, py + 66.f, 16, sf::Color(40, 40, 40));
        draw_label(window, font_, std::to_wstring(players_[0].total_properties()) + L" 块",
                   px + 80.f, py + 66.f, 16, sf::Color(40, 40, 40));

        draw_label(window, font_, L"精灵", px + 160.f, py + 66.f, 16, sf::Color(40, 40, 40));
        draw_label(window, font_, std::to_wstring(players_[0].monsters().size()) + L" / " + std::to_wstring(kMaxMonsters),
                   px + 210.f, py + 66.f, 16, sf::Color(40, 40, 40));
    }

    // 玩家2面板 (右上角)
    {
        float px = 1440.f - 300.f;
        float py = 20.f;
        float pw = 280.f;
        draw_panel(window, px, py, pw, 100.f);

        if (current_player_ == 1) {
            RetroUI::draw_text(window, L"►", px + 8.f, py + 10.f, 18, sf::Color(40, 40, 40));
        }

        draw_label(window, font_, L"玩家2", px + 30.f, py + 10.f, 18, sf::Color(40, 40, 40));
        draw_label(window, font_, L"金币", px + 30.f, py + 40.f, 16, sf::Color(40, 40, 40));
        draw_label(window, font_, std::to_wstring(players_[1].gold()) + L" G",
                   px + 80.f, py + 40.f, 16, sf::Color(40, 40, 40));

        draw_label(window, font_, L"地皮", px + 30.f, py + 66.f, 16, sf::Color(40, 40, 40));
        draw_label(window, font_, std::to_wstring(players_[1].total_properties()) + L" 块",
                   px + 80.f, py + 66.f, 16, sf::Color(40, 40, 40));

        draw_label(window, font_, L"精灵", px + 160.f, py + 66.f, 16, sf::Color(40, 40, 40));
        draw_label(window, font_, std::to_wstring(players_[1].monsters().size()) + L" / " + std::to_wstring(kMaxMonsters),
                   px + 210.f, py + 66.f, 16, sf::Color(40, 40, 40));
    }

    // 回合指示 (中上方)
    {
        float px = 720.f - 100.f;
        float py = 20.f;
        float pw = 200.f;
        draw_panel(window, px, py, pw, 46.f);
        std::wstring turn_text = L"第 " + std::to_wstring(players_[0].laps_completed() + players_[1].laps_completed() + 1) + L" 回合";
        draw_label(window, font_, turn_text, px + 50.f, py + 12.f, 20, sf::Color(40, 40, 40));
    }

    // 骰子显示区 (右下角偏左一点点)
    {
        float dx = 1440.f - 300.f;
        float dy = 960.f - 160.f;
        float dw = 280.f;
        draw_panel(window, dx, dy, dw, 140.f);

        draw_label(window, font_, L"掷骰", dx + 14.f, dy + 10.f, 18, sf::Color(40, 40, 40));

        if (dice_result_ > 0) {
            RetroUI::draw_text(window, std::to_wstring(dice_result_),
                               dx + dw / 2.f - 16.f, dy + 50.f, 48, sf::Color(40, 40, 40));
        } else {
            draw_label(window, font_, L"按空格掷骰", dx + dw / 2.f - 55.f, dy + 55.f,
                       18, sf::Color(40, 40, 40));
        }
    }

    // 消息栏 (底部通栏)
    {
        float mx = 20.f;
        float my = 960.f - 160.f;
        float mw = 1440.f - 340.f; // 留出骰子区的空间
        draw_panel(window, mx, my, mw, 140.f);

        RetroUI::draw_text(window, message_, mx + 30.f, my + 30.f, 24, sf::Color(40, 40, 40));

        // 操作提示
        draw_label(window, font_, L"空格: 掷骰    ESC: 退出",
                   mx + 30.f, my + 90.f, 16, sf::Color(120, 120, 120));
    }
}
