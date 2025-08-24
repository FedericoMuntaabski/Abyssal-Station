#include "AssetManager.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

AssetManager& AssetManager::instance()
{
	static AssetManager s_instance;
	return s_instance;
}

void AssetManager::loadTexturesFrom(const std::string& folderPath)
{
	try {
		for (auto& p : fs::directory_iterator(folderPath)) {
			if (!p.is_regular_file()) continue;
			auto path = p.path();
			auto ext = path.extension().string();
			if (ext == ".png" || ext == ".jpg" || ext == ".bmp") {
				auto tex = std::make_shared<sf::Texture>();
				if (tex->loadFromFile(path.string())) {
					m_textures[path.stem().string()] = tex;
				} else {
					std::cerr << "Failed to load texture: " << path.string() << std::endl;
				}
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Error enumerating textures: " << e.what() << std::endl;
	}
}

bool AssetManager::hasTexture(const std::string& name) const
{
	return m_textures.find(name) != m_textures.end();
}

void AssetManager::removeTexture(const std::string& name)
{
	auto it = m_textures.find(name);
	if (it != m_textures.end()) m_textures.erase(it);
}

void AssetManager::reloadTexturesFrom(const std::string& folderPath)
{
	// Clear and reload to reflect disk changes
	m_textures.clear();
	loadTexturesFrom(folderPath);
}

void AssetManager::loadSoundsFrom(const std::string& folderPath)
{
	try {
		for (auto& p : fs::directory_iterator(folderPath)) {
			if (!p.is_regular_file()) continue;
			auto path = p.path();
			auto ext = path.extension().string();
			if (ext == ".wav" || ext == ".ogg" || ext == ".flac") {
				auto buf = std::make_shared<sf::SoundBuffer>();
				if (buf->loadFromFile(path.string())) {
					m_sounds[path.stem().string()] = buf;
				} else {
					std::cerr << "Failed to load sound: " << path.string() << std::endl;
				}
			}
		}
	} catch (const std::exception& e) {
		std::cerr << "Error enumerating sounds: " << e.what() << std::endl;
	}
}

std::shared_ptr<sf::Texture> AssetManager::getTexture(const std::string& name) const
{
	auto it = m_textures.find(name);
	if (it != m_textures.end()) return it->second;
	return nullptr;
}

std::shared_ptr<sf::SoundBuffer> AssetManager::getSound(const std::string& name) const
{
	auto it = m_sounds.find(name);
	if (it != m_sounds.end()) return it->second;
	return nullptr;
}

