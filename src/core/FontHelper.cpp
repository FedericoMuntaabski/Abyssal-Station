#include "FontHelper.h"
#include "Logger.h"

#include <filesystem>

namespace core {

using namespace std::string_literals;

static std::string findUpFromCwd(const std::string& rel) {
    namespace fs = std::filesystem;
    fs::path p = fs::current_path();
    for (int i = 0; i < 8; ++i) {
        fs::path cand = p / rel;
        if (fs::exists(cand) && fs::is_regular_file(cand)) return cand.string();
        if (p.has_parent_path()) p = p.parent_path(); else break;
    }
    return std::string();
}

std::string findFontFile() {
    namespace fs = std::filesystem;
    // Common locations
    std::string fontPath = findUpFromCwd("assets/fonts/arial.ttf");
    if (fontPath.empty()) fontPath = findUpFromCwd("assets/textures/arial.ttf");

    // Try any ttf/otf in assets/fonts
    if (fontPath.empty()) {
        fs::path p = fs::current_path();
        for (int i = 0; i < 8 && fontPath.empty(); ++i) {
            fs::path candDir = p / "assets" / "fonts";
            if (fs::exists(candDir) && fs::is_directory(candDir)) {
                for (auto &entry : fs::directory_iterator(candDir)) {
                    if (!entry.is_regular_file()) continue;
                    auto ext = entry.path().extension().string();
                    if (ext == ".ttf" || ext == ".otf") { fontPath = entry.path().string(); break; }
                }
            }
            if (p.has_parent_path()) p = p.parent_path(); else break;
        }
    }

#ifdef _WIN32
    if (fontPath.empty()) {
        char* windir = nullptr;
        size_t len = 0;
        if (_dupenv_s(&windir, &len, "WINDIR") == 0 && windir) {
            fs::path winFont = fs::path(windir) / "Fonts" / "arial.ttf";
            if (fs::exists(winFont)) fontPath = winFont.string();
        }
        if (windir) free(windir);
    }
#endif

    if (fontPath.empty()) Logger::instance().warning("FontHelper: no font file found");
    else Logger::instance().info(std::string("FontHelper: found font: ") + fontPath);
    return fontPath;
}

bool loadBestFont(sf::Font& font) {
    auto path = findFontFile();
    if (path.empty()) return false;
    return font.openFromFile(path);
}

} // namespace core
