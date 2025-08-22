(#ifndef ASSET_MANAGER_H)
#define ASSET_MANAGER_H

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <memory>
#include <string>
#include <unordered_map>

// Simple AssetManager singleton. Loads textures and sound buffers from disk
// and provides shared_ptr accessors to ensure safe lifetime management.
class AssetManager {
public:
	// Get the global instance
	static AssetManager& instance();

	// Load all assets from predefined folders (relative to repo root or working dir)
	void loadTexturesFrom(const std::string& folderPath);
	void loadSoundsFrom(const std::string& folderPath);

	// Getters return shared_ptr; nullptr if not found
	std::shared_ptr<sf::Texture> getTexture(const std::string& name) const;
	std::shared_ptr<sf::SoundBuffer> getSound(const std::string& name) const;

	// Non-copyable
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;

private:
	AssetManager() = default;

	std::unordered_map<std::string, std::shared_ptr<sf::Texture>> m_textures;
	std::unordered_map<std::string, std::shared_ptr<sf::SoundBuffer>> m_sounds;
};

#endif // ASSET_MANAGER_H

