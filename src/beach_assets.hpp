#pragma once
#include "garden_kit.hpp"

namespace paws {
// Immutable coastal kit; source masters and review evidence live in artifacts/.
std::span<const GardenAsset> beachAssets();
void addBeachDressing(engine::Scene&,const GardenMeshes&,int boardWidth,int boardDepth,float frontExtension=0.f);
}
