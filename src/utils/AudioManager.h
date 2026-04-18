#pragma once

#include <string>

enum class MusicType { Menu, Play, Battle, Shop };
enum class SoundType { Dice, Purchase, Attack, Capture, Victory, Defeat };

class AudioManager {
public:
    static AudioManager& get();

    // 背景音乐
    void play_music(MusicType type);
    void stop_music();
    void set_music_volume(float volume);  // 0.0 ~ 1.0

    // 音效
    void play_sound(SoundType type);
    void set_sound_volume(float volume);  // 0.0 ~ 1.0

    // 静音
    void set_mute(bool mute);

private:
    AudioManager() = default;

    float music_volume_ = 0.5f;
    float sound_volume_ = 0.7f;
    bool muted_ = false;
};
