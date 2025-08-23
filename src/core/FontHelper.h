#ifndef ABYSSAL_STATION_SRC_CORE_FONTHELPER_H
#define ABYSSAL_STATION_SRC_CORE_FONTHELPER_H

#include <string>
#include <SFML/Graphics/Font.hpp>

namespace core {

// Attempts to find a usable font file on disk. Returns an empty string if none found.
std::string findFontFile();

// Convenience: try to open a font into the provided sf::Font and return whether successful.
bool loadBestFont(sf::Font& font);

} // namespace core

#endif // ABYSSAL_STATION_SRC_CORE_FONTHELPER_H
