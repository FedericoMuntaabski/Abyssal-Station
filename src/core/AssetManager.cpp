#include "AssetManager.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

AssetManager& AssetManager::instance()
{
    static AssetManager inst;
    return inst;
}

bool AssetManager::loadTexture(const std::string& name, const std::string& filepath)
{
    try {
        auto tex = std::make_shared<sf::Texture>();
        if (!tex->loadFromFile(filepath)) {
            return false;
        }

        std::lock_guard lock(m_mutex);
        m_textures[name] = std::move(tex);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "AssetManager: failed to load texture '" << filepath << "': " << e.what() << std::endl;
        return false;
    }
}

bool AssetManager::loadSound(const std::string& name, const std::string& filepath)
{
    try {
        auto sb = std::make_shared<sf::SoundBuffer>();
        if (!sb->loadFromFile(filepath)) {
            return false;
        }

        std::lock_guard lock(m_mutex);
        m_sounds[name] = std::move(sb);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "AssetManager: failed to load sound '" << filepath << "': " << e.what() << std::endl;
        return false;
    }
}

static bool hasTextureExtension(const fs::path& p)
{
    auto ext = p.extension().string();
    for (auto &c : ext) c = static_cast<char>(std::tolower(c));
    return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga";
}

static bool hasSoundExtension(const fs::path& p)
{
    auto ext = p.extension().string();
    for (auto &c : ext) c = static_cast<char>(std::tolower(c));
    return ext == ".wav" || ext == ".ogg" || ext == ".flac" || ext == ".aiff";
}

void AssetManager::loadTexturesFromFolder(const std::string& folderPath)
{
    try {
        if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) return;

        for (auto &entry : fs::directory_iterator(folderPath)) {
            if (!entry.is_regular_file()) continue;
            if (!hasTextureExtension(entry.path())) continue;

            std::string name = entry.path().stem().string();
            loadTexture(name, entry.path().string());
        }
    } catch (const std::exception& e) {
        std::cerr << "AssetManager: failed to load textures from folder '" << folderPath << "': " << e.what() << std::endl;
    }
}

void AssetManager::loadSoundsFromFolder(const std::string& folderPath)
{
    try {
        if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) return;

        for (auto &entry : fs::directory_iterator(folderPath)) {
            if (!entry.is_regular_file()) continue;
            if (!hasSoundExtension(entry.path())) continue;

            std::string name = entry.path().stem().string();
            loadSound(name, entry.path().string());
        }
    } catch (const std::exception& e) {
        std::cerr << "AssetManager: failed to load sounds from folder '" << folderPath << "': " << e.what() << std::endl;
    }
}

std::shared_ptr<sf::Texture> AssetManager::getTexture(const std::string& name) const
{
    std::lock_guard lock(m_mutex);
    auto it = m_textures.find(name);
    if (it == m_textures.end()) return nullptr;
    return it->second;
}

std::shared_ptr<sf::SoundBuffer> AssetManager::getSoundBuffer(const std::string& name) const
{
    std::lock_guard lock(m_mutex);
    auto it = m_sounds.find(name);
    if (it == m_sounds.end()) return nullptr;
    return it->second;
}

bool AssetManager::hasTexture(const std::string& name) const
{
    std::lock_guard lock(m_mutex);
    return m_textures.find(name) != m_textures.end();
}

bool AssetManager::hasSound(const std::string& name) const
{
    std::lock_guard lock(m_mutex);
    return m_sounds.find(name) != m_sounds.end();
}
