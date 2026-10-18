#pragma once
#include "pip_asset.hpp"
#include <array>
#include <string_view>

namespace paws::release1::cats {
struct Asset { std::string_view cat,id,url; };
inline constexpr std::array<Asset,4> assets{{
    {"pip","release1/cats/pip@3","assets/models/release1/cats/toon-v2/pip/pip.glb"},
    {"mabel","release1/cats/mabel@3","assets/models/release1/cats/toon-v2/mabel/mabel.glb"},
    {"button","release1/cats/button@3","assets/models/release1/cats/toon-v2/button/button.glb"},
    {"pepper","release1/cats/pepper@3","assets/models/release1/cats/toon-v2/pepper/pepper.glb"}
}};
struct Action { std::string_view name; double duration; bool loop; double still; };
inline constexpr std::array<Action,18> actions{{
    {"relaxed_idle",4,true,0},{"attentive_request",2.4,false,1.2},
    {"happy_pounce",1.2,false,.6},{"garden_walk",paws::toonWalkDuration,true,0},
    {"settle_cushion",2.4,false,2.4},{"curled_rest",4,true,0},
    {"approach_bowl",1.8,false,1.8},{"drink",2,true,0},
    {"return_idle",1.8,false,1.8},{"scratch",3,true,.5},
    {"perch_step",2.4,false,.6},{"perch_settle",2.4,false,2.4},
    {"playhouse_enter",1.8,false,.9},{"playhouse_lookout",3,true,0},
    {"picnic_sit",2.4,false,2.4},{"personality_reaction",3,false,1.5},
    {"turn_left",paws::toonWalkDuration,true,0},{"turn_right",paws::toonWalkDuration,true,0}
}};
// Presentation only. Clip completion never commits game state or moves an actor.
void validate(const engine::Mesh& mesh);
void present(engine::SkeletalMeshComponent& cat,std::size_t action,bool reducedMotion);
void validateCanonical(const engine::Mesh& mesh,const engine::Mesh& authority);
}
