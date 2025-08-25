#include "AudioManager.h"
#include "../core/AssetManager.h"
#include "../core/Logger.h"
#include <algorithm>

namespace audio {

AudioManager::AudioManager() {
    core::Logger::instance().info("AudioManager: Initialized");
}

void AudioManager::playMusic(const std::string& musicName, bool loop) {
    if (m_currentMusic.getStatus() == sf::Music::Status::Playing) {
        m_currentMusic.stop();
    }
    
    // Try to load music from assets
    std::string musicPath = "assets/sounds/" + musicName + ".wav";
    if (m_currentMusic.openFromFile(musicPath)) {
        m_currentMusic.setLooping(loop);
        m_currentMusic.setVolume(getEffectiveVolume(m_musicVolume));
        m_currentMusic.play();
        core::Logger::instance().info("AudioManager: Playing music " + musicName);
    } else {
        core::Logger::instance().warning("AudioManager: Failed to load music " + musicName);
    }
}

void AudioManager::stopMusic() {
    if (m_currentMusic.getStatus() == sf::Music::Status::Playing) {
        m_currentMusic.stop();
        core::Logger::instance().info("AudioManager: Music stopped");
    }
}

void AudioManager::setMusicVolume(float volume) {
    m_musicVolume = std::clamp(volume, 0.0f, 100.0f);
    if (m_currentMusic.getStatus() != sf::Music::Status::Stopped) {
        m_currentMusic.setVolume(getEffectiveVolume(m_musicVolume));
    }
}

void AudioManager::playSound(const std::string& soundName) {
    // Try to get sound buffer from AssetManager
    auto buffer = AssetManager::instance().getSound(soundName);
    if (!buffer) {
        core::Logger::instance().warning("AudioManager: Sound not found: " + soundName);
        return;
    }
    
    // Create new sound instance
    auto sound = std::make_unique<sf::Sound>(*buffer);
    sound->setVolume(getEffectiveVolume(m_soundVolume));
    sound->play();
    
    m_activeSounds.push_back(std::move(sound));
    
    // Clean up finished sounds to prevent memory leak
    cleanupFinishedSounds();
}

void AudioManager::setSoundVolume(float volume) {
    m_soundVolume = std::clamp(volume, 0.0f, 100.0f);
    
    // Update volume for all active sounds
    for (auto& sound : m_activeSounds) {
        if (sound) {
            sound->setVolume(getEffectiveVolume(m_soundVolume));
        }
    }
}

void AudioManager::setMasterVolume(float volume) {
    m_masterVolume = std::clamp(volume, 0.0f, 100.0f);
    
    // Update music volume
    if (m_currentMusic.getStatus() != sf::Music::Status::Stopped) {
        m_currentMusic.setVolume(getEffectiveVolume(m_musicVolume));
    }
    
    // Update all active sounds
    for (auto& sound : m_activeSounds) {
        if (sound) {
            sound->setVolume(getEffectiveVolume(m_soundVolume));
        }
    }
}

void AudioManager::muteAll(bool muted) {
    m_muted = muted;
    
    // Apply mute to music
    if (m_currentMusic.getStatus() != sf::Music::Status::Stopped) {
        m_currentMusic.setVolume(muted ? 0.0f : getEffectiveVolume(m_musicVolume));
    }
    
    // Apply mute to all active sounds
    for (auto& sound : m_activeSounds) {
        if (sound) {
            sound->setVolume(muted ? 0.0f : getEffectiveVolume(m_soundVolume));
        }
    }
}

void AudioManager::update(float deltaTime) {
    // Clean up finished sounds
    cleanupFinishedSounds();
    
    // Future: Handle fade effects, ducking, etc.
}

void AudioManager::cleanupFinishedSounds() {
    m_activeSounds.erase(
        std::remove_if(m_activeSounds.begin(), m_activeSounds.end(),
            [](const std::unique_ptr<sf::Sound>& sound) {
                return !sound || sound->getStatus() == sf::Sound::Status::Stopped;
            }),
        m_activeSounds.end()
    );
}

float AudioManager::getEffectiveVolume(float baseVolume) const {
    if (m_muted) return 0.0f;
    return (baseVolume * m_masterVolume) / 100.0f;
}

} // namespace audio