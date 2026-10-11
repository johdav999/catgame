#pragma once
#include <array>
#include <string_view>

namespace paws {
// Metres, shared by portable placement rules and garden composition.
struct GardenPoint { float x=0,y=0,z=0; };
struct ShopItem { std::string_view id,name,asset; int price; float width,depth; };
struct GardenSpot { std::string_view id,name; GardenPoint position; float width,depth; };
inline constexpr std::array<ShopItem,2> shopItems{{
    {"garden_cushion","Garden cushion","cushion",30,.94f,1.18f},
    {"drinking_bowl","Drinking bowl","bowl",25,.56f,.56f}
}};
inline constexpr std::array<GardenSpot,3> gardenSpots{{
    {"bench_corner","Beside the bench",{-1.65f,0,.85f},1.1f,1.35f},
    {"sunny_corner","Sunny corner",{1.65f,0,.85f},1.1f,1.35f},
    {"quiet_corner","Quiet nook",{1.65f,0,-.85f},1.1f,1.35f}
}};
inline const ShopItem* shopItem(std::string_view id) {
    for(const auto& item:shopItems) if(item.id==id)return &item;
    return nullptr;
}
inline const GardenSpot* gardenSpot(std::string_view id) {
    for(const auto& spot:gardenSpots) if(spot.id==id)return &spot;
    return nullptr;
}
inline bool fits(const ShopItem& item,const GardenSpot& spot) {
    return item.width<=spot.width&&item.depth<=spot.depth;
}
}
