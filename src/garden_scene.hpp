#pragma once
#include "puzzle_scene.hpp"
#include "afternoon.hpp"
namespace paws {
struct FurnishingSpot { const char* id; engine::Vec3 position; engine::Vec3 footprint; };
std::span<const FurnishingSpot> furnishingSpots();
PuzzleScene makeHomeGarden(const GardenMeshes&, bool ballReturned, bool reducedMotion,const Afternoon* afternoon=nullptr);
void projectHomeGarden(engine::Scene&,SceneBindings&,const Afternoon&,std::string_view previewItem={},std::string_view previewSpot={});
struct GardenUse {
    CatCollision collision;
    engine::ActorId actor;
    engine::ComponentId cat;
    std::vector<engine::Vec3> path;
    std::size_t segment=0;
    float elapsed=0;
    int phase=0;
    bool cushion=false,reduced=false;
    engine::Vec3 root;
    float yaw=0;
    std::string item,spot;
};
GardenUse beginGardenUse(engine::Scene&,const SceneBindings&,std::string_view item,std::string_view spot,bool reducedMotion);
void updateGardenUse(engine::Scene&,GardenUse&,float delta);
void cancelGardenUse(engine::Scene&,GardenUse&,bool reducedMotion);
}
