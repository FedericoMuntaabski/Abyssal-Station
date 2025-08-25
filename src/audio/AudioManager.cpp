#include "AudioManager.h"
#include "../core/Logger.h"
#include <algorithm>
#include <filesystem>

namespace audio {

AudioManager::AudioManager() {
    setupContextMapping();
    
    // Load default audio files if they exist
    if (std::filesystem::exists("assets/sounds/background_music.wav")) {
        loadMusic("menu_music", "assets/sounds/background_music.wav");
        loadMusic("gameplay_music", "assets/sounds/background_music.wav"); // Same for now
    }
    
    if (std::filesystem::exists("assets/sounds/hover_select.wav")) {
        loadSound("hover", "assets/sounds/hover_select.wav");
    }
    
    if (std::filesystem::exists("assets/sounds/confirm.wav")) {
        loadSound("confirm", "assets/sounds/confirm.wav");
    }
    
    core::Logger::instance().info("AudioManager: Initialized with dynamic audio system");
}

void AudioManager::setupContextMapping() {
    // Map audio contexts to music tracks
    m_contextMusic[AudioContext::Menu] = "menu_music";
    m_contextMusic[AudioContext::Gameplay] = "gameplay_music";
    m_contextMusic[AudioContext::Combat] = "combat_music";
    m_contextMusic[AudioContext::Exploration] = "exploration_music";
    m_contextMusic[AudioContext::Danger] = "danger_music";
}

bool AudioManager::loadMusic(const std::string& id, const std::string& filepath) {
    if (m_musicTracks.find(id) != m_musicTracks.end()) {
        core::Logger::instance().warning("AudioManager: Music track '" + id + "' already loaded");
        return true;
    }
    
    MusicTrack track;
    track.music = std::make_unique<sf::Music>();
    track.filepath = filepath;
    
    if (!track.music->openFromFile(filepath)) {
        core::Logger::instance().error("AudioManager: Failed to load music '" + id + "' from " + filepath);
        return false;
    }
    
    m_musicTracks[id] = std::move(track);
    core::Logger::instance().info("AudioManager: Loaded music '" + id + "' from " + filepath);
    return true;
}

bool AudioManager::loadSound(const std::string& id, const std::string& filepath) {
    if (m_soundEffects.find(id) != m_soundEffects.end()) {
        core::Logger::instance().warning("AudioManager: Sound effect '" + id + "' already loaded");
        return true;
    }
    
    SoundEffect effect;
    if (!effect.buffer.loadFromFile(filepath)) {
        core::Logger::instance().error("AudioManager: Failed to load sound '" + id + "' from " + filepath);
        return false;
    }
    
    m_soundEffects[id] = std::move(effect);
    core::Logger::instance().info("AudioManager: Loaded sound '" + id + "' from " + filepath);
    return true;
}

void AudioManager::playMusic(const std::string& id, bool loop) {
    auto it = m_musicTracks.find(id);
    if (it == m_musicTracks.end()) {
        core::Logger::instance().warning("AudioManager: Music track '" + id + "' not found");
        return;
    }
    
    // Stop current music if different
    if (m_currentMusicId != id) {
        stopMusic();
    }
    
    auto& track = it->second;
    track.music->setLooping(loop);
    track.music->setVolume(calculateEffectiveVolume(track.baseVolume, true));
    track.music->play();
    
    m_currentMusicId = id;
    core::Logger::instance().info("AudioManager: Playing music '" + id + "'");
}

void AudioManager::stopMusic() {
    if (!m_currentMusicId.empty()) {
        auto it = m_musicTracks.find(m_currentMusicId);
        if (it != m_musicTracks.end()) {
            it->second.music->stop();
        }
        m_currentMusicId.clear();
    }
}

void AudioManager::pauseMusic() {
    if (!m_currentMusicId.empty()) {
        auto it = m_musicTracks.find(m_currentMusicId);
        if (it != m_musicTracks.end()) {
            it->second.music->pause();
        }
    }
}

void AudioManager::resumeMusic() {
    if (!m_currentMusicId.empty()) {
        auto it = m_musicTracks.find(m_currentMusicId);
        if (it != m_musicTracks.end()) {
            it->second.music->play();
        }
    }
}

void AudioManager::playSound(const std::string& id, float volume) {
    auto it = m_soundEffects.find(id);
    if (it == m_soundEffects.end()) {
        core::Logger::instance().warning("AudioManager: Sound effect '" + id + "' not found");
        return;
    }
    
    // Create a new sound instance
    auto sound = std::make_unique<sf::Sound>(it->second.buffer);
    sound->setVolume(calculateEffectiveVolume(volume, false));
    sound->play();
    
    m_activeSounds.push_back(std::move(sound));
}

void AudioManager::setAudioContext(AudioContext context) {
    if (m_currentContext == context) return;
    
    AudioContext prevContext = m_currentContext;
    m_currentContext = context;
    
    // Find music for new context
    auto it = m_contextMusic.find(context);
    if (it != m_contextMusic.end() && !it->second.empty()) {
        // Check if we have the music loaded
        if (m_musicTracks.find(it->second) != m_musicTracks.end()) {
            // Smooth transition if currently playing music
            if (!m_currentMusicId.empty()) {
                crossfadeMusic(it->second, 2.0f);
            } else {
                fadeInMusic(it->second, 1.5f);
            }
        }
    }
    
    core::Logger::instance().info("AudioManager: Context changed from " + std::to_string(static_cast<int>(prevContext)) + 
                                 " to " + std::to_string(static_cast<int>(context)));
}

void AudioManager::setMasterVolume(float volume) {
    m_masterVolume = std::clamp(volume, 0.0f, 100.0f);
    
    // Update current music volume
    if (!m_currentMusicId.empty()) {
        auto it = m_musicTracks.find(m_currentMusicId);
        if (it != m_musicTracks.end()) {
            it->second.music->setVolume(calculateEffectiveVolume(it->second.baseVolume, true));
        }
    }
}

void AudioManager::setMusicVolume(float volume) {
    m_musicVolume = std::clamp(volume, 0.0f, 100.0f);
    
    // Update current music volume
    if (!m_currentMusicId.empty()) {
        auto it = m_musicTracks.find(m_currentMusicId);
        if (it != m_musicTracks.end()) {
            it->second.music->setVolume(calculateEffectiveVolume(it->second.baseVolume, true));
        }
    }
}

void AudioManager::setSoundVolume(float volume) {
    m_soundVolume = std::clamp(volume, 0.0f, 100.0f);
}

void AudioManager::fadeOutMusic(float duration) {
    if (m_currentMusicId.empty()) return;
    
    auto it = m_musicTracks.find(m_currentMusicId);
    if (it == m_musicTracks.end()) return;
    
    m_currentFade = std::make_unique<FadeOperation>();
    m_currentFade->type = FadeOperation::FadeOut;
    m_currentFade->duration = duration;
    m_currentFade->elapsed = 0.0f;
    m_currentFade->startVolume = it->second.music->getVolume();
    m_currentFade->targetVolume = 0.0f;
}

void AudioManager::fadeInMusic(const std::string& id, float duration, bool loop) {
    auto it = m_musicTracks.find(id);
    if (it == m_musicTracks.end()) return;
    
    // Start playing at volume 0
    it->second.music->setVolume(0.0f);
    it->second.music->setLooping(loop);
    it->second.music->play();
    m_currentMusicId = id;
    
    m_currentFade = std::make_unique<FadeOperation>();
    m_currentFade->type = FadeOperation::FadeIn;
    m_currentFade->duration = duration;
    m_currentFade->elapsed = 0.0f;
    m_currentFade->startVolume = 0.0f;
    m_currentFade->targetVolume = calculateEffectiveVolume(it->second.baseVolume, true);
}

void AudioManager::crossfadeMusic(const std::string& newId, float duration, bool loop) {
    if (m_currentMusicId.empty()) {
        fadeInMusic(newId, duration, loop);
        return;
    }
    
    auto it = m_musicTracks.find(newId);
    if (it == m_musicTracks.end()) return;
    
    m_currentFade = std::make_unique<FadeOperation>();
    m_currentFade->type = FadeOperation::CrossFade;
    m_currentFade->duration = duration;
    m_currentFade->elapsed = 0.0f;
    m_currentFade->targetMusicId = newId;
    m_currentFade->targetLoop = loop;
    
    auto currentIt = m_musicTracks.find(m_currentMusicId);
    if (currentIt != m_musicTracks.end()) {
        m_currentFade->startVolume = currentIt->second.music->getVolume();
    }
}

void AudioManager::update(float deltaTime) {
    cleanupFinishedSounds();
    updateFade(deltaTime);
}

void AudioManager::cleanupFinishedSounds() {
    m_activeSounds.erase(
        std::remove_if(m_activeSounds.begin(), m_activeSounds.end(),
            [](const std::unique_ptr<sf::Sound>& sound) {
                return sound->getStatus() != sf::SoundSource::Status::Playing;
            }),
        m_activeSounds.end()
    );
}

void AudioManager::updateFade(float deltaTime) {
    if (!m_currentFade) return;
    
    m_currentFade->elapsed += deltaTime;
    float progress = std::min(m_currentFade->elapsed / m_currentFade->duration, 1.0f);
    
    switch (m_currentFade->type) {
        case FadeOperation::FadeOut: {
            if (!m_currentMusicId.empty()) {
                auto it = m_musicTracks.find(m_currentMusicId);
                if (it != m_musicTracks.end()) {
                    float volume = m_currentFade->startVolume * (1.0f - progress);
                    it->second.music->setVolume(volume);
                    
                    if (progress >= 1.0f) {
                        it->second.music->stop();
                        m_currentMusicId.clear();
                    }
                }
            }
            break;
        }
        
        case FadeOperation::FadeIn: {
            if (!m_currentMusicId.empty()) {
                auto it = m_musicTracks.find(m_currentMusicId);
                if (it != m_musicTracks.end()) {
                    float volume = m_currentFade->startVolume + 
                                  (m_currentFade->targetVolume - m_currentFade->startVolume) * progress;
                    it->second.music->setVolume(volume);
                }
            }
            break;
        }
        
        case FadeOperation::CrossFade: {
            // Fade out current music
            if (!m_currentMusicId.empty()) {
                auto currentIt = m_musicTracks.find(m_currentMusicId);
                if (currentIt != m_musicTracks.end()) {
                    float volume = m_currentFade->startVolume * (1.0f - progress);
                    currentIt->second.music->setVolume(volume);
                    
                    // Start new music at halfway point
                    if (progress >= 0.5f && !m_currentFade->targetMusicId.empty()) {
                        auto newIt = m_musicTracks.find(m_currentFade->targetMusicId);
                        if (newIt != m_musicTracks.end()) {
                            if (newIt->second.music->getStatus() != sf::SoundSource::Status::Playing) {
                                newIt->second.music->setLooping(m_currentFade->targetLoop);
                                newIt->second.music->setVolume(0.0f);
                                newIt->second.music->play();
                            }
                            
                            float newVolume = calculateEffectiveVolume(newIt->second.baseVolume, true) * (progress - 0.5f) * 2.0f;
                            newIt->second.music->setVolume(newVolume);
                        }
                    }
                    
                    if (progress >= 1.0f) {
                        currentIt->second.music->stop();
                        m_currentMusicId = m_currentFade->targetMusicId;
                    }
                }
            }
            break;
        }
    }
    
    if (progress >= 1.0f) {
        m_currentFade.reset();
    }
}

float AudioManager::calculateEffectiveVolume(float baseVolume, bool isMusic) const {
    float typeVolume = isMusic ? m_musicVolume : m_soundVolume;
    return (baseVolume / 100.0f) * (typeVolume / 100.0f) * (m_masterVolume / 100.0f) * 100.0f;
}

} // namespace audio
