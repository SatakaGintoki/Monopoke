#include "AudioManager.h"

// 简化实现：实际使用需要 SFML Audio 库和音频文件
// 当前作为占位符，后续可扩展

AudioManager& AudioManager::get() {
    static AudioManager instance;
    return instance;
}

void AudioManager::play_music(MusicType type) {
    // 占位实现：需要 SFML SoundBuffer 和 Sound
    // 需要音频文件资源
    (void)type;
}

void AudioManager::stop_music() {
    // 占位实现
}

void AudioManager::set_music_volume(float volume) {
    music_volume_ = volume;
    if (music_volume_ < 0.0f) music_volume_ = 0.0f;
    if (music_volume_ > 1.0f) music_volume_ = 1.0f;
}

void AudioManager::play_sound(SoundType type) {
    // 占位实现
    (void)type;
}

void AudioManager::set_sound_volume(float volume) {
    sound_volume_ = volume;
    if (sound_volume_ < 0.0f) sound_volume_ = 0.0f;
    if (sound_volume_ > 1.0f) sound_volume_ = 1.0f;
}

void AudioManager::set_mute(bool mute) {
    muted_ = mute;
}
