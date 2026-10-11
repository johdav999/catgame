#pragma once
#include "starter_scene.hpp"
#include "engine/skeletal_mesh_component.hpp"
#include <array>
#include <string_view>
namespace paws {
inline constexpr std::string_view pipAssetId="cats/pip@1";
inline constexpr std::string_view pipAssetUrl="assets/models/pip/pip.glb";
// Authored contact travel for garden_walk; actor movement owns translation.
inline constexpr float pipWalkDuration=1.2f;
inline constexpr float pipWalkStride=.36f;
inline constexpr float pipWalkSpeed=pipWalkStride/pipWalkDuration;
struct PipAction { std::string_view name; bool loop; double stillSeconds; };
inline constexpr std::array<PipAction,9> pipActions{{
    {"relaxed_idle",true,0},{"attentive_request",false,1.2},{"happy_pounce",false,.6},
    {"garden_walk",true,0},{"settle_cushion",false,2.4},{"curled_rest",true,0},
    {"approach_bowl",false,1.8},{"drink",true,0},{"return_idle",false,1.8}
}};
void validatePip(const engine::Mesh& mesh);
std::size_t pipClip(const engine::Mesh& mesh,std::string_view name);
void presentPip(engine::SkeletalMeshComponent& cat,std::size_t action,bool reducedMotion);
struct PipPreview { StarterScene scene; engine::ComponentId cat,contact; };
PipPreview makePipPreview(std::shared_ptr<const engine::Mesh> mesh);
}
