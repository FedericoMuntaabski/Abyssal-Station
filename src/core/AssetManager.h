#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <unordered_map>
#include <memory>
#include <string>
#include <mutex>

class AssetManager {
public:
    // Get the singleton instance
    static AssetManager& instance();

    // Load single assets
    bool loadTexture(const std::string& name, const std::string& filepath);
    bool loadSound(const std::string& name, const std::string& filepath);

    // Load all assets from a directory (textures: .png/.jpg/.bmp/.tga, sounds: .wav/.ogg/.flac)
    void loadTexturesFromFolder(const std::string& folderPath);
    void loadSoundsFromFolder(const std::string& folderPath);

    // Accessors: return shared_ptr (nullptr if not found)
    std::shared_ptr<sf::Texture> getTexture(const std::string& name) const;
    std::shared_ptr<sf::SoundBuffer> getSoundBuffer(const std::string& name) const;

    bool hasTexture(const std::string& name) const;
    bool hasSound(const std::string& name) const;

private:
    AssetManager() = default;
    ~AssetManager() = default;

    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<sf::Texture>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<sf::SoundBuffer>> m_sounds;
};
