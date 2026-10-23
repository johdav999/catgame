#pragma once
#include "engine/scene.hpp"
#include "engine/texture.hpp"
#include "engine/mesh.hpp"

namespace paws {
// Decorative ground only: puzzle contacts, routes and saved cells remain at Y=0.
inline constexpr float beachGroundHeight=-.02f;
inline constexpr float beachWaterHeight=-.18f;
// World Y for an upright plant: seat its lower stems in the sand at the
// supplied world X/Z, scale and yaw. The supplied Y is ignored.
float beachPlantHeight(engine::Scene&,const engine::Mesh&,engine::Transform);
struct BeachSurfState { double age=0; unsigned starts=0; float travel=0,opacity=0; };
// Scene-owned decorative wash, triggered only once an audio voice really starts.
void startBeachSurf(engine::Scene&);
BeachSurfState updateBeachSurf(engine::Scene&,double delta,bool reducedMotion,bool followAudio);
void addBeachEnvironment(engine::Scene& scene,bool runnel,int boardWidth=5,int boardDepth=4,
    std::shared_ptr<const engine::Texture> sand={});
}
