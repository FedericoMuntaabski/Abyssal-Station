#ifndef ABYSSAL_STATION_SRC_CORE_GAMESTATE_H
#define ABYSSAL_STATION_SRC_CORE_GAMESTATE_H

#include <cstdint>
#include <vector>
#include <nlohmann/json.hpp>

namespace core {

struct GameState {
    int version = 1;

    struct PlayerState {
        std::uint32_t id = 0;
        float x = 0.f;
        float y = 0.f;
        int health = 100;
    };

    std::vector<PlayerState> players;
    std::vector<std::uint32_t> itemsCollected;
    std::vector<std::uint32_t> puzzlesCompleted;
};

// nlohmann serialization
inline void to_json(nlohmann::json& j, const GameState::PlayerState& p) {
    j = nlohmann::json{{"id", p.id}, {"x", p.x}, {"y", p.y}, {"health", p.health}};
}
inline void from_json(const nlohmann::json& j, GameState::PlayerState& p) {
    if (j.contains("id")) p.id = j.at("id").get<std::uint32_t>();
    if (j.contains("x")) p.x = j.at("x").get<float>();
    if (j.contains("y")) p.y = j.at("y").get<float>();
    if (j.contains("health")) p.health = j.at("health").get<int>();
}

inline void to_json(nlohmann::json& j, const GameState& s) {
    j = nlohmann::json{
        {"version", s.version},
        {"players", s.players},
        {"itemsCollected", s.itemsCollected},
        {"puzzlesCompleted", s.puzzlesCompleted}
    };
}
inline void from_json(const nlohmann::json& j, GameState& s) {
    if (j.contains("version")) s.version = j.at("version").get<int>();
    if (j.contains("players")) s.players = j.at("players").get<std::vector<GameState::PlayerState>>();
    if (j.contains("itemsCollected")) s.itemsCollected = j.at("itemsCollected").get<std::vector<std::uint32_t>>();
    if (j.contains("puzzlesCompleted")) s.puzzlesCompleted = j.at("puzzlesCompleted").get<std::vector<std::uint32_t>>();
}

} // namespace core

#endif // ABYSSAL_STATION_SRC_CORE_GAMESTATE_H
