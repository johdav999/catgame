#pragma once
#include "pip_asset.hpp"
#include <span>
#include <string>
#include <unordered_map>

namespace paws {
struct GardenAsset { const char* name; const char* id; const char* file; engine::Vec3 minimum,maximum; unsigned triangles; };
struct GardenInstance { const char* asset; engine::Vec3 position; };
struct GardenAnchor { const char* asset; const char* name; engine::Vec3 position; };
std::span<const GardenAsset> gardenAssets();
std::span<const GardenInstance> gardenReviewInstances();
engine::Vec3 gardenAnchor(std::string_view asset,std::string_view anchor);
using GardenMeshes=std::unordered_map<std::string,std::shared_ptr<const engine::Mesh>>;
void validateGardenAsset(const GardenAsset& spec,const engine::Mesh& mesh);
// Derive shared presentation detail once after loading, before publishing scenes.
// Original geometry, material maps and gameplay footprints remain authoritative.
void prepareGardenPresentationMeshes(GardenMeshes& meshes);
// Bounds describe the courtyard; fence stays outside and small details occupy
// its outer decorative strip, leaving the central puzzle floor clear.
engine::Vec3 gardenGatePosition(float right,float back,float front);
// Returns the boundary gate leaf so gate quests animate this same instance.
engine::ActorId addGardenEnclosure(engine::Scene& scene,const GardenMeshes& meshes,
    float left,float right,float back,float front,engine::Actor* parent=nullptr);
struct GardenPreview {
    StarterScene scene;
    engine::ActorId gate,plate,ball,mouse,fish,pip;
    engine::ComponentId cat;
};
GardenPreview makeGardenPreview(const GardenMeshes& meshes,std::shared_ptr<const engine::Mesh> pip);
// Inspection poses only: no quest outcomes or gameplay rules are inferred from art.
void poseGardenPreview(engine::Scene& scene,const GardenPreview& bindings,int mode);
}
