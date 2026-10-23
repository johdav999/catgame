#pragma once
#include <array>
#include <string_view>

namespace paws {
// Golden Afternoon Garden 1, then 2, repeating in the authored order.
inline constexpr std::array<std::string_view,2> gardenMusicPlaylist{
    "music_garden_afternoon_01", "music_garden_puzzles_01"
};
inline constexpr std::array<std::string_view,1> beachMusicPlaylist{
    "music_beachside_puzzle_01"
};
}
