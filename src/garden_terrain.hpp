#pragma once
#include "garden_kit.hpp"

namespace paws {
// Bounds include the house, garden, gate exit and planted apron in parent space.
// The same height policy is used for every chapter and quest enclosure.
engine::Actor* addGardenTerrain(engine::Scene& scene,const GardenMeshes& meshes,
    float flatLeft,float flatRight,float flatBack,float flatFront,engine::Actor* parent);
}
