#pragma once
#include "garden_kit.hpp"
namespace paws::release1::props {
struct PropAnchor {const char* asset; const char* name; engine::Vec3 position;};
struct PropInstance {const char* asset; engine::Vec3 position,rotationDegrees; int group;};
struct PropView {const char* name; engine::Vec3 target; float width,height;};
std::span<const GardenAsset> assets();
std::span<const PropAnchor> anchors();
std::span<const PropInstance> instances();
std::span<const PropView> views();
struct Binding {engine::ActorId actor; PropInstance instance;};
struct Preview {StarterScene scene; std::vector<Binding> bindings;};
Preview makePreview(const GardenMeshes& meshes);
void pose(engine::Scene&,const Preview&,bool active);
}
