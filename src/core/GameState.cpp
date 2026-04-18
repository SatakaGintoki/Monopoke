#include "GameState.h"
#include "StateManager.h"

// push_state：暂停当前状态，压入新状态
// 例如：游戏中打开商店 → push_state(ShopState)
void GameState::push_state(StateManager& mgr, GameState* new_state) {
    mgr.push_state(new_state);
}

// pop_state：弹出当前状态，恢复上一个状态
// 例如：关闭商店 → pop_state() → 返回PlayState
void GameState::pop_state(StateManager& mgr) {
    mgr.pop_state();
}

// switch_state：直接替换当前状态（不保留旧状态）
// 例如：游戏结束 → switch_state(ResultState)
void GameState::switch_state(StateManager& mgr, GameState* new_state) {
    mgr.switch_state(new_state);
}
