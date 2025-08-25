#ifndef ABYSSAL_STATION_SRC_AUDIO_AUDIO_MANAGER_H
#define ABYSSAL_STATION_SRC_AUDIO_AUDIO_MANAGER_H

#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>

namespace audio {

enum class AudioContext {
    Menu,
    Gameplay,
    Combat,
    Exploration,
    Danger
};

class AudioManager {
public:
    AudioManager();
    ~AudioManager() = default;

    // Audio loading
    bool loadMusic(const std::string& id, const std::string& filepath);
    bool loadSound(const std::string& id, const std::string& filepath);
    
    // Music control with smooth transitions
    void playMusic(const std::string& id, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    
    // Sound effects
    void playSound(const std::string& id, float volume = 100.0f);
    
    // Context-based audio switching
    void setAudioContext(AudioContext context);
    AudioContext getCurrentContext() const { return m_currentContext; }
    
    // Volume control
    void setMasterVolume(float volume); // 0-100
    void setMusicVolume(float volume);  // 0-100
    void setSoundVolume(float volume);  // 0-100
    
    // Fade effects
    void fadeOutMusic(float duration = 2.0f);
    void fadeInMusic(const std::string& id, float duration = 2.0f, bool loop = true);
    void crossfadeMusic(const std::string& newId, float duration = 3.0f, bool loop = true);
    
    // Update for fade effects
    void update(float deltaTime);
    
    // Get current playing music ID
    std::string getCurrentMusicId() const { return m_currentMusicId; }
    
private:
    struct MusicTrack {
        std::unique_ptr<sf::Music> music;
        std::string filepath;
        float baseVolume{100.0f};
    };
    
    struct SoundEffect {
        sf::SoundBuffer buffer;
        float baseVolume{100.0f};
    };
    
    struct FadeOperation {
        enum Type { FadeOut, FadeIn, CrossFade };
        Type type;
        float duration;
        float elapsed;
        std::string targetMusicId;
        bool targetLoop;
        float startVolume;
        float targetVolume;
    };
    
    std::unordered_map<std::string, MusicTrack> m_musicTracks;
    std::unordered_map<std::string, SoundEffect> m_soundEffects;
    std::vector<std::unique_ptr<sf::Sound>> m_activeSounds;
    
    AudioContext m_currentContext{AudioContext::Menu};
    std::string m_currentMusicId;
    
    float m_masterVolume{100.0f};
    float m_musicVolume{100.0f};
    float m_soundVolume{100.0f};
    
    std::unique_ptr<FadeOperation> m_currentFade;
    
    // Context-specific music mapping
    std::unordered_map<AudioContext, std::string> m_contextMusic;
    
    void setupContextMapping();
    void cleanupFinishedSounds();
    void updateFade(float deltaTime);
    float calculateEffectiveVolume(float baseVolume, bool isMusic) const;
};

} // namespace audio

#endif // ABYSSAL_STATION_SRC_AUDIO_AUDIO_MANAGER_H
