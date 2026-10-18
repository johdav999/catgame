#pragma once
#include "starter_scene.hpp"
#include "engine/skeletal_mesh_component.hpp"
#include <array>
#include <string_view>
namespace paws {
// Keeps original idle and walking clips available for A/B review and rollback.
// The historical name and idlePose URL switch control both forward variants.
void setForwardIdleCorrection(bool enabled);
inline constexpr std::string_view pipAssetId="cats/pip@1";
inline constexpr std::string_view pipAssetUrl="assets/models/pip/pip.glb";
bool isToonCat(const engine::Mesh& mesh);
engine::Vec3 catMuzzleBind(const engine::Mesh& mesh);
// Authored contact travel for garden_walk; actor movement owns translation.
inline constexpr float pipWalkDuration=.8f;
inline constexpr float pipWalkStride=.36f;
inline constexpr float pipWalkSpeed=pipWalkStride/pipWalkDuration;
// The replacement rig's measured front-paw stance sweeps ~1.035 m/cycle.
// Keep world speed unchanged and let every playback path use the baked timing.
inline constexpr float toonWalkDuration=2.3f;
inline constexpr float toonWalkStride=toonWalkDuration*pipWalkSpeed;
// Measured front-paw stance displacement per complete crouching cycle.
inline constexpr float toonCrawlStride=.99f;
struct PipAction { std::string_view name; bool loop; double stillSeconds; };
inline constexpr std::array<PipAction,11> pipActions{{
    {"relaxed_idle",true,0},{"attentive_request",false,1.2},{"happy_pounce",false,.6},
    {"garden_walk",true,0},{"settle_cushion",false,2.4},{"curled_rest",true,0},
    {"approach_bowl",false,1.8},{"drink",true,0},{"return_idle",false,1.8},
    {"turn_left",true,0},{"turn_right",true,0}
}};
void validatePip(const engine::Mesh& mesh);
std::size_t pipClip(const engine::Mesh& mesh,std::string_view name,bool useForwardCorrection=true);
void presentPip(engine::SkeletalMeshComponent& cat,std::size_t action,bool reducedMotion);
// Scene-owned presentation state; never participates in saves or puzzle rules.
class CatTurn : public engine::Component {
public:
    CatTurn():Component("Cat turning"){}
    float weight=0; // Negative is anatomical left, positive is right (+Y yaw).
};
bool turnCatTowards(engine::Actor& actor,float yaw,double delta);
void sampleCatWalk(engine::Actor& actor,engine::SkeletalMeshComponent& skin,double seconds,bool paused);
bool faceCat(engine::Actor& actor,engine::SkeletalMeshComponent& skin,float yaw,double delta);
struct PipPreview { StarterScene scene; engine::ComponentId cat,contact; };
PipPreview makePipPreview(std::shared_ptr<const engine::Mesh> mesh);
}
