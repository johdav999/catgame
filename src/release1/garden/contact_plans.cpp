#include "contact_plans.hpp"
namespace paws::release1::garden {
std::map<std::string,Contact,std::less<>> contactGuides(std::size_t cat){
 std::map<std::string,Contact,std::less<>> out;
 out["garden_cushion"]={{0,0,.85f},{0,.16f,0},engine::pi,"settle_cushion","curled_rest","settle_cushion",2.4,2.4,0};
 out["drinking_bowl"]={{0,0,-1.1f},{0,0,-.61327f},0,"approach_bowl","drink","return_idle",1.8,1.8,0};
 if(cat==0)out["scratching_post"]={{0,0,1.05f},{0,0,.65f},engine::pi,"scratch","scratch","return_idle",3,1.8,.5};
 if(cat==1)out["sunny_perch"]={{0,0,1.25f},{0,.46f,0},engine::pi,"perch_step","perch_settle","perch_step",2.4,2.4,2.4,{{0,0,1.0f},{0,.2f,.575f},{0,.46f,0}}};
 if(cat==2)out["hammock"]={{1.05f,0,0},{0,.33f,0},-engine::pi/2,"settle_cushion","curled_rest","settle_cushion",2.4,2.4,0,{{.85f,0,0},{0,.33f,0}}};
 if(cat==0||cat==3)out["playhouse"]={{0,0,1.6f},{0,.07f,0},engine::pi,"playhouse_enter","playhouse_lookout","playhouse_enter",1.8,1.8,0,{{0,0,1.6f},{0,.07f,.75f},{0,.07f,0}}};
 // One reservation per furnishing. Cats gather on four spacious perimeter
 // anchors in the finale, rather than squeezing four bodies onto the small mat.
 const engine::Vec3 table[]{ {-.85f,0,0},{0,0,-.95f},{.85f,0,0},{0,0,.95f} };
 const float yaw[]{engine::pi/2,0,-engine::pi/2,engine::pi};
 out["low_picnic_table"]={table[cat]+table[cat]*.5f,table[cat],yaw[cat],"picnic_sit","picnic_sit","return_idle",2.4,1.8,2.4};
 const engine::Vec3 mat[]{ {-1.4f,0,0},{0,0,-1.4f},{1.4f,0,0},{0,.04f,.1f} };
 out["picnic_mat"]={mat[cat]+engine::Vec3{0,0,cat==1?-.4f:.8f},mat[cat],yaw[cat],"picnic_sit","picnic_sit","return_idle",2.4,1.8,2.4};
 return out;
}
}
