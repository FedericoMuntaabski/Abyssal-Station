#ifndef ABYSSAL_STATION_SRC_AUDIO_AUDIOMANAGER_H
#define ABYSSAL_STATION_SRC_AUDIO_AUDIOMANAGER_H

#include <SFML/Audio.hpp>
#include <string>
#include <unordered_map>
#include <memory>

namespace audio {

class AudioManager {
public:
    AudioManager();
    ~AudioManager() = default;

    // Music management
    void playMusic(const std::string& musicName, bool loop = true);
    void stopMusic();
    void setMusicVolume(float volume); // 0-100
    
    // Sound effects management
    void playSound(const std::string& soundName);
    void setSoundVolume(float volume); // 0-100
    
    // Global audio control
    void setMasterVolume(float volume); // 0-100
    void muteAll(bool muted);
    
    // Update (for fade effects, etc.)
    void update(float deltaTime);

private:
    // Music system
    sf::Music m_currentMusic;
    float m_musicVolume{50.0f};
    
    // Sound system
    std::unordered_map<std::string, std::shared_ptr<sf::SoundBuffer>> m_soundBuffers;
    std::vector<std::unique_ptr<sf::Sound>> m_activeSounds;
    float m_soundVolume{50.0f};
    
    // Global settings
    float m_masterVolume{100.0f};
    bool m_muted{false};
    
    // Helper methods
    void cleanupFinishedSounds();
    float getEffectiveVolume(float baseVolume) const;
};

} // namespace audio

#endif // ABYSSAL_STATION_SRC_AUDIO_AUDIOMANAGER_H