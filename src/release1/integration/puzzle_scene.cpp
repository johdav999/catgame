#include "puzzle_scene.hpp"
#include "release1/rules/content.hpp"
#include "release1/progression/policy.hpp"
#include "release1/cats/cat_assets.hpp"
#include "engine/mesh_component.hpp"
#include "water_surface.hpp"
#include "beach_environment.hpp"
#include "beach_assets.hpp"
#include "engine/light_component.hpp"
#include "engine/interaction_component.hpp"
#include <algorithm>
#include <cmath>
namespace paws::release1::presentation {
namespace {
using rules::Family;
// Original UI artwork is embedded so tags require no additional scene loads.
#include "weight_tags.inc"
#include "cat_name_tags.inc"
std::shared_ptr<const engine::Material> catNameTag(unsigned index){
 static const auto tags=[] {
  std::array<std::shared_ptr<const engine::Material>,4> result;
  const std::span<const unsigned char> images[]={catNameTag0,catNameTag1,catNameTag2,catNameTag3};
  for(unsigned n=0;n<4;++n){
   auto m=std::make_shared<engine::Material>();m->shader=engine::MaterialShader::Unlit;
   m->baseColorTexture=engine::Texture::loadMemory(std::as_bytes(images[n]),"Cat name: "+std::string(progression::catNames[n]));
   m->doubleSided=true;m->alphaMode=engine::AlphaMode::Mask;m->alphaCutoff=.4f;result[n]=m;
  }return result;
 }();return tags.at(index);
}
void addCatName(engine::Actor& actor,unsigned index,engine::Vec3 position,float width){
 auto& label=actor.addComponent<engine::MeshComponent>("Name: "+std::string(progression::catNames[index]),engine::Mesh::plane());
 label.transform.position=position;label.transform.rotation.x=.88f;label.transform.scale={width,1,width*112.f/384};
 label.color={1,1,1};label.castsShadow=false;label.receivesShadow=false;
 label.setMaterial(0,engine::MaterialInstance{catNameTag(index)});
}
std::shared_ptr<const engine::Material> weightBadge(int weight) {
 static const auto badges=[] {
  std::array<std::shared_ptr<const engine::Material>,4> result;
  const std::span<const unsigned char> images[]={weightTag1,weightTag2,weightTag3,weightTag4};
  for(int i=0;i<4;++i){
   auto material=std::make_shared<engine::Material>();material->shader=engine::MaterialShader::Unlit;
   material->baseColorTexture=engine::Texture::loadMemory(std::as_bytes(images[i]),"Paws weight tag "+std::to_string(i+1));
   material->doubleSided=true;material->alphaMode=engine::AlphaMode::Mask;material->alphaCutoff=.4f;
   result[i]=material;
  }
  return result;
 }();
 return badges.at(weight-1);
}
engine::Actor& prop(engine::Scene& s,const GardenMeshes& m,std::string name,std::string asset,engine::Vec3 p={}) {
 auto& a=s.spawn(name);a.transform.position=p;
 a.addComponent<engine::MeshComponent>(name,m.at(asset)).color={1,1,1};return a;
}
void rim(engine::Scene& s,engine::Vec3 p,float size,engine::Vec3 color) {
 auto material=std::make_shared<engine::Material>();material->shader=engine::MaterialShader::Unlit;material->baseColor=color;
 for(int i=0;i<4;++i){auto& a=s.spawn("Logical destination outline");a.transform.position=p+engine::Vec3{i<2?0:(i==2?-1.f:1.f)*size/2,.004f,i<2?(i==0?-1.f:1.f)*size/2:0};
 auto& mesh=a.addComponent<engine::MeshComponent>("Outline",engine::Mesh::cube());mesh.transform.scale=i<2?engine::Vec3{size,.008f,.018f}:engine::Vec3{.018f,.008f,size};mesh.setMaterial(0,engine::MaterialInstance{material});mesh.castsShadow=false;}
}
void inlay(engine::Scene& s,engine::Vec3 from,engine::Vec3 to,engine::Vec3 color,float width=.035f) {
 auto& a=s.spawn("Playhouse connection inlay");auto d=to-from;
 auto& m=a.addComponent<engine::MeshComponent>("Connection",engine::Mesh::cube());
 a.transform.position=(from+to)*.5f+engine::Vec3{0,.025f,0};a.transform.rotation.y=std::atan2(d.x,d.z);
 a.transform.scale={width,.008f,std::sqrt(engine::dot(d,d))};
 auto material=std::make_shared<engine::Material>();material->shader=engine::MaterialShader::Unlit;material->baseColor=color;
 m.setMaterial(0,engine::MaterialInstance{material});m.castsShadow=false;
}
void playhouseCourtyard(engine::Scene& s,const GardenMeshes& m,const rules::Definition& d) {
 auto point=[&](float x,float y){return world(d.quest,{0,0},d.content)+engine::Vec3{x,0,y};};
 auto fence=[&](engine::Vec3 p,float length,float yaw){auto& a=prop(s,m,"Playhouse low fence","fence",p);a.transform.scale={length,.65f,1};a.transform.rotation.y=yaw;};
 // Recompose approved kit meshes as a cutaway: a rear roof canopy and low
 // walls leave every inside destination visible from the puzzle camera.
 auto& floor=prop(s,m,"Playhouse floor","platform",point(2, .8f));floor.transform.scale={3,.15f,3.1f};
 fence(point(2,-.55f),3,0);fence(point(.5f,.8f),2.7f,engine::pi/2);fence(point(3.5f,.8f),2.7f,engine::pi/2);
 auto& frame=prop(s,m,"Playhouse doorway frame","gate_frame",point(2,2));frame.transform.scale={1.5f,1.2f,1};
 for(float side:{-1.f,1.f}){
  auto& post=prop(s,m,"Playhouse rear canopy post","plank_1m",point(2+side*1.5f,-.45f)+engine::Vec3{0,.645f,0});
  post.transform.scale={1.25f,.85f,.3f};post.transform.rotation.z=engine::pi/2;
  auto& roof=prop(s,m,"Playhouse rear roof canopy","plank_2m",point(2+side*.70f,-.45f)+engine::Vec3{0,1.6f,0});
  roof.transform.scale={.85f,1,.85f};roof.transform.rotation.z=-side*.42f;
 }
 // Storage is enclosed on all four sides; its front opening is the side gate.
 fence(point(5.8f,-.35f),2.8f,0);fence(point(4.4f,.8f),2.3f,engine::pi/2);fence(point(7.2f,.8f),2.3f,engine::pi/2);
 fence(point(6.7f,1.5f),1,0);
 prop(s,m,"Side gate frame","gate_frame",point(5,1.5f));
 const auto paw=point(1,3),star=point(5,3);
 inlay(s,paw+engine::Vec3{-.48f,0,0},point(.15f,3),{.64f,.43f,.22f});
 inlay(s,point(.15f,3),point(.15f,2),{.64f,.43f,.22f});
 inlay(s,point(.15f,2),point(5,2)+engine::Vec3{0,0,-.35f},{.64f,.43f,.22f},.022f);
 inlay(s,star+engine::Vec3{0,0,-.45f},point(5,2.5f),{.15f,.4f,.36f});
 inlay(s,point(5,2.5f),point(2,2.5f),{.15f,.4f,.36f});
 inlay(s,point(2,2.5f),point(2,2),{.15f,.4f,.36f});
 // A broad track and matching footprint explain that the crate can replace Pip.
 const auto& crate=*std::find_if(d.objects.begin(),d.objects.end(),[](const auto& o){return o.id=="crate";});
 for(std::size_t n=1;n<crate.slots.size();++n){
  const auto from=world(d.quest,crate.slots[n-1],d.content),to=world(d.quest,crate.slots[n],d.content),axis=to-from;
  const auto side=engine::Vec3{-axis.z,0,axis.x}*.44f;
  for(float sign:{-1.f,1.f})inlay(s,from+side*sign,to+side*sign,{.42f,.34f,.23f});
 }
 for(auto c:crate.slots)rim(s,world(d.quest,c,d.content),.88f,{.42f,.34f,.23f});
}
engine::Vec3 shapeOrigin(const rules::Object& o,Cell cell,int rotation,std::string_view quest,std::string_view content) {
 auto p=world(quest,cell,content);int w=0,h=0;for(auto c:o.shape){w=std::max(w,c.x+1);h=std::max(h,c.y+1);}
 if(rotation==1)p.x+=h*.25f;else if(rotation==2){p.x+=w*.25f;p.z+=h*.25f;}else if(rotation==3)p.z+=w*.25f;
 return p;
}
float routeLength(const std::vector<engine::Vec3>& route){float length=0;for(std::size_t i=1;i<route.size();++i){auto d=route[i]-route[i-1];length+=std::sqrt(engine::dot(d,d));}return length;}
engine::Actor* object(engine::Scene& s,const SceneBindings& b,std::string_view name){for(auto& o:b.objects)if(o.object==name)return s.findActor(o.actor);return nullptr;}
void sample(engine::SkeletalMeshComponent& skin,std::string_view clip,float seconds,bool loop=false){
 const auto index=pipClip(*skin.mesh,clip);
 const auto duration=skin.mesh->skeleton->clips[index].duration;
 skin.animation().play(index,loop);skin.animation().seek(loop?std::fmod(std::max(0.f,seconds),duration):std::max(0.f,seconds));skin.animation().pause();
}
#include "pepper_trail.inc"
engine::Vec3 headPoint(engine::Actor& a,engine::SkeletalMeshComponent& skin,engine::Vec3 point){
 const auto& skeleton=*skin.mesh->skeleton;auto pose=skin.skinPose();
 for(std::size_t i=1;i<skeleton.bindings.size();++i)if(skeleton.joints[skeleton.bindings[i].joint].name=="head")return engine::transformPoint(a.transform.matrix(),engine::transformPoint(pose->bones[i].transform,point));
 return a.transform.position;
}
void endingPose(engine::Scene& s,SceneBindings& b,double delta=0){
 auto& e=b.ending;auto* a=s.findActor(e.actor);if(!a)return;auto* skin=a->getComponent<engine::SkeletalMeshComponent>();if(!skin)return;
 const float walkTime=e.elapsed-e.leadSeconds;
 const bool travelling=e.segment+1<e.route.size();
 if(walkTime>=0&&!e.route.empty()&&(!e.finished||e.segment+1<e.route.size())){
  if(!b.collision.sample(s,*a,e.route,e.segment,std::min(walkTime,e.walkSeconds)*pipWalkSpeed))
   e.elapsed=std::min(e.elapsed,e.leadSeconds+e.walkSeconds-.0001f);
 }
 const float t=e.elapsed-e.leadSeconds-e.walkSeconds;
 static constexpr const char* clips[]={"quest_mouse_collect","quest_sun_stretch","quest_bed_knead","quest_drink_finish","quest_bell_touch"};
 if(walkTime<0){a->transform.position=e.home;a->transform.rotation=e.homeRotation;}
 if(walkTime<0)sample(*skin,e.kind==8?"quest_bell_touch":"attentive_request",e.elapsed);
 else if(t<0){sampleCatWalk(*a,*skin,walkTime,true);}
 else{
  if(travelling&&delta>0){e.elapsed=e.leadSeconds+e.walkSeconds;sampleCatWalk(*a,*skin,walkTime,true);return;}
  if(!e.facingComplete&&delta>0&&!turnCatTowards(*a,e.finalYaw,delta)){
   e.elapsed=e.leadSeconds+e.walkSeconds;sampleCatWalk(*a,*skin,skin->animation().time()+delta,true);return;
  }
  e.facingComplete=true;
  if(e.kind!=4||t<=e.clipSeconds)a->transform.rotation.y=e.finalYaw;
  sample(*skin,clips[e.kind-4],std::min(t,e.clipSeconds));
  if(e.kind==6&&!isToonCat(*skin->mesh)&&t>=4&&t<5.8f){a->transform.rotation.y+=2*engine::pi*(t-4)/1.8f;sample(*skin,"turn_right",t-4,true);}
  if(e.kind==4&&t>e.clipSeconds){
   if(!b.collision.sample(s,*a,e.carryRoute,e.carrySegment,(t-e.clipSeconds)*pipWalkSpeed))e.elapsed=std::min(e.elapsed,e.leadSeconds+e.walkSeconds+e.clipSeconds+e.carrySeconds-.0001f);
   if(e.carrySegment+1>=e.carryRoute.size())sample(*skin,"relaxed_idle",t-e.clipSeconds,true);
   else sampleCatWalk(*a,*skin,t-e.clipSeconds,true);}
 }
 if(e.kind==4)if(auto* toy=s.findActor(e.toy)){
  toy->transform.position=e.toyStart;
  const float pickup=isToonCat(*skin->mesh)?e.clipSeconds*.5f:2.6f;
  if(!isToonCat(*skin->mesh)&&t>=.7f&&t<pickup)toy->transform.position.x-=.20f*std::clamp((t-.7f)/.55f,0.f,1.f);
  if(t>=pickup)toy->transform.position=headPoint(*a,*skin,isToonCat(*skin->mesh)?catMuzzleBind(*skin->mesh):engine::Vec3{.1570493f,.6611857f,.5438889f})-engine::Vec3{0,.025f,0};
 }
 if(e.kind==7){
  // Reveal water in route order before Mabel leaves her watching position.
  for(std::size_t i=0;i<e.flowRoute.size();++i)for(auto& o:b.objects)if(auto* actor=s.findActor(o.actor)){
   auto d=actor->transform.position-e.flowRoute[i];if(d.x*d.x+d.z*d.z>.01f)continue;
   for(auto& c:actor->children())if(c->name=="Validated water surface")c->enabled=e.elapsed>=1.5f*float(i)/std::max(std::size_t(1),e.flowRoute.size());
  }
  if(auto* bowl=s.findActor(e.toy))for(auto& c:bowl->children())if(c->name=="Filled bowl"||c->name=="Bowl inlet ribbon")c->enabled=c->name=="Filled bowl"?e.elapsed>=1.5f:(e.elapsed>=1.2f&&e.elapsed<1.8f);
  if(auto* tongue=s.findActor(e.tongue)){
   const bool lap=t>.85f&&t<3.15f;const bool lick=t>4.2f&&t<4.8f;
   const float wave=lap?std::max(0.f,std::sin(t*17)):lick?std::sin((t-4.2f)*engine::pi/.6f):0;
   tongue->enabled=wave>.12f&&!isToonCat(*skin->mesh); // New rig includes its own animated tongue.
   tongue->transform.position=headPoint(*a,*skin,{.1357133f+.0025278f*wave,.6707769f+(lick?.0186808f*wave:-.015f*wave),.4840926f+.012f*wave});tongue->transform.rotation.y=a->transform.rotation.y;
   tongue->transform.scale={1,wave,1};
  }
 }
 if(e.kind==8)if(auto* bell=s.findActor(e.toy)){
  const float since=t-(isToonCat(*skin->mesh)?1.3f:1.9f);bell->transform.rotation.z=since>0?.25f*std::exp(-since*.9f)*std::sin(since*9):walkTime<0?.10f*std::sin(e.elapsed*10):0;
 }
}
}
engine::Vec3 world(std::string_view quest,Cell c,std::string_view content) {
 const auto d=rules::quest(quest,content);const float step=d.family==Family::Shapes?.25f:1.f;
 return {(float(c.x)-(d.width-1)*.5f)*step,0,(float(c.y)-(d.height-1)*.5f)*step};
}
engine::Vec3 ballPosition(std::string_view quest,Cell c,std::string_view content) {
 auto p=world(quest,c,content);p.y=.42f;
 if(quest==progression::beachQuestId){
  const auto d=rules::quest(quest,content);
  p.y=c==d.source?0.f:c==d.destination?gardenAnchor("cushion","rest_root").y:gardenAnchor("channel_straight","port_a").y;
 }
 return p;
}
engine::Vec3 shapePosition(std::string_view quest,std::string_view object,Cell cell,int orientation,std::string_view content) {
 const auto d=rules::quest(quest,content);
 const auto o=std::find_if(d.objects.begin(),d.objects.end(),[&](const auto& o){return o.id==object;});
 if(o==d.objects.end())throw std::invalid_argument("Unknown cushion");
 return shapeOrigin(*o,cell,orientation,quest,content);
}
PuzzleScene build(const PuzzleView& v,const GardenMeshes& meshes,bool reducedMotion) {
 const auto d=rules::quest(v.quest,v.content);PuzzleScene out;out.scene=std::make_unique<engine::Scene>(v.title);
 out.bindings.questId=v.quest;out.bindings.production=true;auto& s=*out.scene;s.ambient={.38f,.38f,.36f};
 const float step=d.family==Family::Shapes?.25f:1.f;
 // Include the requesting cats' apron in the enclosure, keeping their approach clear.
 // The courtyard uses metre-sized paving, independently of puzzle cell size.
 // Cushion cells are only 25 cm: scaling the enclosure with them squeezed the
 // whole garden, moved the house forward and crowded Button against the fence.
 const float halfWidth=(d.width+2)*.5f,halfDepth=std::max((d.height+2)*.5f,2.f);
 const float right=halfWidth+(d.family==Family::Switches?0.f:1.5f);
 const bool beach=progression::isBeachQuest(v.quest);
 if(beach){
  s.name="A Seaside Afternoon - beach cove";
  // Small saved-progress landmarks occupy the strip behind the clear board.
  const char* landmarks[]={"Recovered ball","Button's bridge","Mabel's canopy","Recovered picnic basket","Shell-chime shelter","Picnic ready","Tidepool crossing","Fresh water","Shell workshop","Answering song","Lantern landing"};
  for(unsigned n=0;n<out.bindings.beachMilestones.size();++n){
   auto& group=s.spawn(std::string("Cove progress: ")+landmarks[n]);group.transform.position={float(n%6)*1.25f-3.1f,0,-halfDepth-.7f-float(n/6)*1.3f};
   out.bindings.beachMilestones[n]=group.id();
   auto part=[&](std::string name,const char* asset,engine::Vec3 p,engine::Vec3 scale){auto& m=group.addComponent<engine::MeshComponent>(name,meshes.at(asset));m.color={1,1,1};m.transform.position=p;m.transform.scale=scale;};
   if(n==0){auto& m=group.addComponent<engine::MeshComponent>("Recovered beach ball",meshes.at("ball"));m.color={1,1,1};m.transform.position.y=.15f;}
   if(n==1)for(int b=0;b<3;++b)part("Completed runnel bridge","beach_plank_short",{0,.04f,float(b)*.2f-.2f},{.9f,1,.30f});
   if(n==2){part("Supported shade canopy","beach_shade_panel",{0,.58f,0},{.5f,1,1});for(float x:{-.5f,.5f})part("Canopy post","beach_shade_post",{x,0,0},{1,1,1});}
   if(n==3){auto& m=group.addComponent<engine::MeshComponent>("Recovered basket",meshes.at("beach_basket"));m.color={1,1,1};m.transform.scale={.6f,.6f,.6f};}
   if(n==4){part("Chime frame","beach_chime_frame",{0,0,0},{.3f,.5f,.5f});for(int j=0;j<3;++j){auto& m=group.addComponent<engine::MeshComponent>("Hanging shell",meshes.at(std::array{"beach_shell_scallop","beach_shell_clam","beach_shell_spiral"}[j]));m.color={1,1,1};m.transform.position={float(j)*.25f-.25f,.60f,0};m.transform.rotation.x=engine::pi/2;m.transform.scale={.7f,.7f,.7f};}}
   if(n==5)part("Picnic blanket","beach_mat_mabel",{0,0,0},{1,1,1});
   if(n==6)part("Tidepool bridge","beach_plank_long",{0,.04f,0},{.45f,1,.6f});
   if(n==7)part("Fresh water bowl","bowl",{0,0,0},{.8f,.8f,.8f});
   if(n==8)part("Workshop canopy","beach_shade_panel",{0,.15f,0},{.45f,1,1});
   if(n==9)for(int j=0;j<3;++j)part("Encore shell","beach_shell_scallop",{float(j)*.22f-.22f,0,0},{1,1,1});
   if(n==10)part("Safely balanced lantern","beach_lantern",{0,0,0},{1,1,1});
  }
  auto sand=meshes.find("beach_sand");
  addBeachEnvironment(s,d.family==Family::Planks,d.width,d.height,
      sand==meshes.end()?nullptr:sand->second->materials.at(0)->baseColorTexture);
  addBeachDressing(s,meshes,d.width,d.height,v.quest=="beach_pepper_basket"?4.f:0.f);
 }else {
 for(float x=-halfWidth;x<right-.001f;x+=1)for(float z=-halfDepth;z<halfDepth-.001f;z+=1){
  const float w=std::min(1.f,right-x),h=std::min(1.f,halfDepth-z);
  auto& tile=prop(s,meshes,"Garden paving","paving",{x+w*.5f,-.015f,z+h*.5f});tile.transform.scale={w,1,h};
 }
 addGardenEnclosure(s,meshes,-halfWidth,right,-halfDepth,halfDepth);
 // Keep the house-side bench in quest scenes, aligned with the enclosure's
 // entrance, book stack and lantern as the courtyard expands.
 const float benchX=(-halfWidth+right)*.5f+2.35f;
 prop(s,meshes,"House-side garden bench","bench",{benchX,0,-halfDepth-.35f});
 prop(s,meshes,"House-side bench cushion","bench_cushion",{benchX,.49f,-halfDepth-.32f});
 }
 if(v.quest=="pip_friend_view")for(int n=0;n<5;++n){
  auto& butterfly=s.spawn("Lookout butterfly");butterfly.enabled=false;out.bindings.butterflies.push_back(butterfly.id());
  for(int side:{-1,1}){auto& wing=butterfly.addComponent<engine::MeshComponent>("Butterfly wing",meshes.at("sequence_picture_leaf"));wing.transform.position={side*.05f,0,0};wing.transform.scale={.14f,.21f,.14f};wing.transform.rotation.z=side*.65f;wing.castsShadow=false;auto color=std::make_shared<engine::Material>();color->shader=engine::MaterialShader::Unlit;color->baseColor=n%2?engine::Vec3{1,.73f,.30f}:engine::Vec3{.99f,.89f,.61f};wing.setMaterial(0,engine::MaterialInstance{color});}
 }
 const bool cooperation=rules::cooperativePlayhouse(d);
 if(v.quest=="beach_pepper_basket")out.bindings.pepper.stage=pepperStage(v,d);
 if(cooperation)playhouseCourtyard(s,meshes,d);
 for(auto& o:v.objects){
  std::string asset=o.id.starts_with("planter")?"pot_round":o.role;
  if(asset=="trail_clue")asset=o.id=="clue_0"?"paw_mat":o.id=="clue_1"?"mouse":"herb_pot";
  if(asset=="trail_choice")asset="pot_fluted";
  if(asset=="balance_weight")asset=d.sharedLookout?"pot_round":"cushion";
  if(asset=="lookout_cushion")asset="cushion";
  if(asset=="lookout_seat"||asset=="lookout_approach")asset="paw_mat";
  if(asset=="pot")asset="pot_round";
  if(asset=="catch_tray"||asset=="water_basin")asset="bowl";
  if(asset=="dry_cushion")asset="cushion";
  if(asset=="destination")asset="basket";
  if(beach&&o.role=="destination")asset="cushion";
  if(asset=="balance_test"||asset=="balance_socket"||asset=="balance_home")asset="paw_mat";
  if(asset=="balance_support")asset="crate";
  if(asset=="balance_beam")asset="plank_2m";
  if(asset=="balance_pip")asset="pip";
  if(asset=="balance_pad")asset="platform";
  if(v.quest=="beach_lantern_balance"&&o.role=="balance_weight")asset="beach_lantern";
  if(asset=="plank"){auto a=std::find_if(d.objects.begin(),d.objects.end(),[&](auto& x){return x.id==o.id;});asset=a!=d.objects.end()&&a->length==2?"plank_2m":"plank_1m";}
  if(asset=="picnic_bed")asset="cushion";if(asset=="picnic_bowl"||asset=="drinking_bowl")asset="bowl";
  if(beach&&o.role=="plank")asset=asset=="plank_2m"?"beach_plank_long":"beach_plank_short";
  if(beach&&o.role=="platform")asset="beach_support";
  if(d.beachPicnic&&o.role=="picnic_bed")asset="beach_mat_"+o.id.substr(0,o.id.find('_'));
  if(beach&&o.role.starts_with("bell_"))asset=o.role=="bell_leaf"?"beach_shell_scallop":o.role=="bell_star"?"beach_shell_clam":"beach_shell_spiral";
  if(asset=="gutter"||asset=="channel"){auto a=std::find_if(d.legacy->objects.begin(),d.legacy->objects.end(),[&](auto& x){return x.id==o.id;});asset=a!=d.legacy->objects.end()&&a->kind==Kind::Curve?"channel_curve":"channel_straight";}
  if(beach&&o.role=="channel")asset="beach_"+asset;
  if(asset=="switch_pip"||asset=="switch_pepper"||asset=="inside_latch"){
   auto& base=prop(s,meshes,o.name+" base","switch_base",world(v.quest,o.cell,v.content));
   if(cooperation)base.transform.scale={2.1f,1,2.1f};
   asset="switch_plate";
  }
  if(asset=="playhouse_door"&&!cooperation){prop(s,meshes,"Playhouse","playhouse_frame",world(v.quest,o.cell,v.content));}
  if(asset=="side_gate")asset="gate_leaf";
  if(asset=="cat_pip"||asset=="cat_pepper")asset=asset.substr(4);
  engine::Actor* a=nullptr;
  if(v.quest=="beach_pepper_basket")a=&pepperObject(s,out.bindings,meshes,o,d);
  else if(beach&&o.role=="shade_panel"){
   a=&prop(s,meshes,o.name,"beach_shade_panel");
  }
  else if(beach&&o.role=="crate"){
   a=&prop(s,meshes,o.name,"beach_basket");
  }
  // The garden's bowl/bed placard sits at the bed centre. Thin beach mats
  // expose that legacy interactive mesh, so it steals their drag input.
  // Beach clues use the per-cell markers below and the four identity symbols.
  else if(d.beachPicnic&&(o.role=="shade_clue"||o.role=="quiet_clue"||o.role=="adjacency_clue"))a=&s.spawn(o.name);
  else if(o.role=="plank_socket"||o.role=="shade_socket")a=&s.spawn(o.name);
  else if(meshes.contains(asset)) {
   if(o.role.starts_with("cat_")||o.role=="balance_pip"){a=&s.spawn(o.name);auto& skin=a->addComponent<engine::SkeletalMeshComponent>(o.name,meshes.at(asset));presentPip(skin,0,true);}
   else a=&prop(s,meshes,o.name,asset);
  }else if(o.role.starts_with("cat_")||o.role=="reach_socket"||o.role=="waiting"||o.role=="picnic_spot"||o.role=="cushion_basket"||o.role=="storage_spot"||o.role=="inside_entry"||o.role=="inside_spot")a=&s.spawn(o.name);
  else throw std::out_of_range("Required puzzle mesh missing: "+asset);
  if(o.role!="cushion_basket"&&(a->getComponent<engine::MeshComponent>()||o.role.starts_with("cat_")))a->addComponent<engine::InteractionComponent>();
  out.bindings.objects.push_back({o.id,a->id(),{}});
  if(beach&&o.role.starts_with("bell_")){
   auto* shell=a->getComponent<engine::MeshComponent>();
   a->getComponent<engine::InteractionComponent>()->proxyBounds=engine::MeshBounds{{-.30f,-.47f,-.25f},{.30f,.10f,.32f},true};
   shell->transform.rotation.x=engine::pi/2;shell->transform.scale={1.8f,1.8f,1.8f};shell->transform.position.y=-.20f;
   auto& cord=a->addComponent<engine::MeshComponent>("Shell hanging cord",engine::Mesh::cube());
   cord.transform.position.y=.075f;cord.transform.scale={.018f,.16f,.018f};cord.color={.51f,.39f,.23f};
   auto& symbol=a->addComponent<engine::MeshComponent>("Shell identity symbol",meshes.at("sequence_picture_"+o.role.substr(5)));
   symbol.transform.position={0,-.20f,shell->mesh->localBounds().maximum.y*1.8f+.025f};symbol.transform.scale={.25f,.25f,.25f};symbol.color={1,1,1};symbol.castsShadow=false;
  }
  if(d.beachPicnic&&o.role=="picnic_bed"){
   const auto catId=o.id.substr(0,o.id.find('_'));
   const auto index=std::find(progression::catIds.begin(),progression::catIds.end(),catId)-progression::catIds.begin();
   addCatName(*a,unsigned(index),{0,.15f,-.40f},.88f);
   const float top=a->getComponent<engine::MeshComponent>()->mesh->localBounds().maximum.y;
   const char* symbol=o.id=="button_mat"?"star":o.id=="mabel_mat"?"leaf":"paw";
   const int count=o.id=="pepper_mat"?2:1;
   for(int n=0;n<count;++n){
    auto& mark=a->addComponent<engine::MeshComponent>("Picnic identity symbol",meshes.at("sequence_picture_"+std::string(n?"star":symbol)));mark.color={1,1,1};
    const auto bounds=mark.mesh->localBounds();const auto extent=bounds.maximum-bounds.minimum;
    const float scale=(count==2?.38f:.65f)/std::max({extent.x,extent.y,extent.z});
    mark.transform.rotation.x=-engine::pi/2;mark.transform.scale={scale,scale,scale};
    const engine::Vec3 desired{count==2?(n?.24f:-.24f):0.f,top+.025f,0};
    mark.transform.position=desired-engine::transformPoint(mark.transform.matrix(),(bounds.minimum+bounds.maximum)*.5f);
    mark.castsShadow=false;
   }
  }
  if(o.role=="ball")out.bindings.ball=a->id();
  if(o.role=="balance_pip")out.bindings.requesters.push_back({a->id(),a->getComponent<engine::SkeletalMeshComponent>()->id(),{},0});
  if(o.role=="balance_weight"){
   const int count=std::stoi(o.id.substr(7));a->transform.scale={.9f,.9f,.9f};
   if(d.sharedLookout||v.quest=="beach_lantern_balance"){
    auto& badge=a->addComponent<engine::MeshComponent>("Weight "+std::to_string(count),engine::Mesh::plane());
    badge.transform.position={0,d.sharedLookout?1.22f:.82f,-.08f};badge.transform.scale={.66f,1,.825f};
    badge.transform.rotation.x=.85f;
    auto& stem=a->addComponent<engine::MeshComponent>("Weight tag stem",engine::Mesh::cube());
    stem.transform.position={0,d.sharedLookout?.73f:.50f,.18f};stem.transform.scale={.045f,.38f,.045f};
    stem.color={.40f,.25f,.12f};stem.castsShadow=false;
    badge.color={1,1,1};badge.castsShadow=false;badge.setMaterial(0,engine::MaterialInstance{weightBadge(count)});
   }else for(int n=0;n<count;++n){auto& pip=a->addComponent<engine::MeshComponent>("Weight mark",meshes.at("sequence_picture_paw"));pip.transform.position={-.24f+n*.16f,.19f,0};pip.transform.rotation.x=-engine::pi/2;pip.transform.scale={.23f,.23f,.23f};pip.castsShadow=false;auto ink=std::make_shared<engine::Material>();ink->shader=engine::MaterialShader::Unlit;ink->baseColor={.06f,.25f,.20f};pip.setMaterial(0,engine::MaterialInstance{ink});}
  }
  if(o.role=="lookout_seat"){auto& seat=prop(s,meshes,o.name+" platform","platform",world(v.quest,o.cell,v.content));seat.transform.scale={1.1f,2,1.5f};}
  if(o.role=="balance_socket"){
   for(int n=0;n<o.cell.x;++n){auto& tick=a->addComponent<engine::MeshComponent>("Beam position mark",meshes.at("sequence_picture_leaf"));tick.transform.position={-.25f+n*.075f,.035f,0};tick.transform.rotation.x=-engine::pi/2;tick.transform.scale={.10f,.10f,.10f};tick.castsShadow=false;}
  }
  if((o.role=="reach_socket"&&d.family!=Family::Bench)||o.role=="waiting"||(o.role=="picnic_spot"&&!d.sunnyNook))rim(s,world(v.quest,o.cell,v.content),.8f,{.15f,.4f,.36f});
  if(cooperation){
   if(o.role=="inside_entry"||o.role=="inside_spot"||o.role=="storage_spot"){
    rim(s,world(v.quest,o.cell,v.content)+engine::Vec3{0,.04f,0},.88f,{.15f,.4f,.36f});
    auto& mat=prop(s,meshes,o.name+" paw mat","paw_mat",world(v.quest,o.cell,v.content)+engine::Vec3{0,.03f,0});mat.transform.scale={1.2f,1,1.2f};
   }
   if(o.role=="switch_pip"||o.role=="switch_pepper"||o.role=="inside_latch"){
    a->transform.scale={2.1f,1,2.1f};
    auto& symbol=a->addComponent<engine::MeshComponent>("Switch picture",meshes.at(o.role=="switch_pepper"?"sequence_picture_star":"sequence_picture_paw"));
    symbol.transform.rotation.x=-engine::pi/2;symbol.transform.position={0,.075f,0};symbol.transform.scale={.55f,.55f,.55f};symbol.castsShadow=false;
    rim(s,world(v.quest,o.cell,v.content),.98f,{.15f,.4f,.36f});
   }
  }
  if(o.role=="water_basin"){auto& water=a->addComponent<engine::MeshComponent>("Basin water",meshes.at("bowl_water"));water.transform.position=gardenAnchor("bowl","water");dressWater(water);}
  if(o.role=="gutter")prop(s,meshes,o.name+" supports","water_support",world(v.quest,o.cell,v.content));
  if(o.role=="gutter"){
   const auto surface=asset=="channel_curve"?"water_curve":"water_straight";
   // Surface inherits the gutter's independently projected quarter turn.
   auto& water=a->addComponent<engine::MeshComponent>("Validated water surface",meshes.at(surface));water.transform.position={0,.08f,0};water.enabled=v.completed;dressWater(water,true);
  }
  if(o.role=="drinking_bowl"||o.role=="picnic_bowl"){
   auto& water=a->addComponent<engine::MeshComponent>("Filled bowl",meshes.at("bowl_water"));water.transform.position=gardenAnchor("bowl","water");water.enabled=o.role=="picnic_bowl"||v.completed;dressWater(water);
   if(o.role=="drinking_bowl"){
    auto& drop=a->addComponent<engine::MeshComponent>("Bowl inlet ribbon",meshes.at("water_drop"));drop.transform.rotation.y=engine::pi/2;drop.enabled=v.completed;dressWater(drop,true);
   }
  }
  if((o.role=="plank_socket"||o.role=="shade_socket")&&v.quest!="beach_windbreak") {
   auto socket=std::find_if(d.sockets.begin(),d.sockets.end(),[&](auto& x){return x.id==o.id;});
   if(socket!=d.sockets.end()&&socket->supported){
    // Guides belong to the crossing, not to a loose piece at its first endpoint.
    // They remain readable without motion, shadows or colour discrimination.
    const auto from=world(v.quest,socket->cell,v.content),to=world(v.quest,socket->end,v.content);
    const auto direction=(to-from)*(1.f/socket->length);
    for(float t:{.22f,float(socket->length)-.22f}){
     auto& guide=prop(s,meshes,o.name+" endpoint guide","plank_socket",from+direction*t);
     guide.transform.rotation.y=socket->orientation?engine::pi/2:0;
    }
    if(socket->supported){
     for(int n=1;n<socket->length*4;++n)for(float side:{-.34f,.34f}){
      auto& mark=a->addComponent<engine::MeshComponent>("Unfilled crossing inlay",engine::Mesh::cube());
      mark.transform.position=direction*(n*.25f)+engine::Vec3{-direction.z*side,.012f,direction.x*side};
      mark.transform.scale=socket->orientation?engine::Vec3{.055f,.008f,.11f}:engine::Vec3{.11f,.008f,.055f};
      mark.color={.16f,.40f,.36f};mark.castsShadow=false;
     }
    }
   }
  }
 }
 if(d.family==Family::PictureSequence){
  // The imported rack is a single frame. Bell meshes retain independent actors
  // for picking and Pepper's response animation.
  if(std::any_of(v.objects.begin(),v.objects.end(),[](const auto& o){return o.role.starts_with("bell_");}))prop(s,meshes,beach?"Driftwood shell chime frame":"Garden bell frame",beach?"beach_chime_frame":"bell_support",world(v.quest,{d.width/2,0},v.content));
  auto* board=object(s,out.bindings,"sequence_board");
  for(const auto& o:d.objects)if(o.kind==rules::Piece::Bell){
   auto& picture=board->addChild<engine::Actor>("Blackboard picture "+o.symbol);
   picture.transform.position={0,1.60f,.105f};
   auto& m=picture.addComponent<engine::MeshComponent>(o.symbol,meshes.at("sequence_picture_"+o.symbol));m.color={1,1,1};m.castsShadow=false;
   out.bindings.sequencePictures.emplace_back(o.symbol,picture.id());
  }
 }
 if(d.family==Family::Bench||d.family==Family::Planks){
  // Paving inlays describe the actual authored crate tracks, not reach sockets.
  auto line=[&](engine::Actor& a,engine::Vec3 from,engine::Vec3 to,float width,engine::Vec3 color){
   auto delta=to-from;auto& mark=a.addComponent<engine::MeshComponent>("Paving inlay",engine::Mesh::cube());
   mark.transform.position=(from+to)*.5f+engine::Vec3{0,.055f,0};
   mark.transform.rotation.y=std::atan2(delta.x,delta.z);mark.transform.scale={width,.008f,std::sqrt(engine::dot(delta,delta))};
   auto material=std::make_shared<engine::Material>();material->shader=engine::MaterialShader::Unlit;material->baseColor={1,1,1};
   mark.setMaterial(0,engine::MaterialInstance{material});mark.color=color;mark.castsShadow=false;
  };
  for(auto& o:d.objects)if(o.kind==rules::Piece::Crate){
   auto& track=s.spawn(o.name+" sliding track");
   for(std::size_t n=1;n<o.slots.size();++n){
    const auto from=world(v.quest,o.slots[n-1],v.content),to=world(v.quest,o.slots[n],v.content);
    const auto axis=to-from;const auto side=engine::Vec3{-axis.z,0,axis.x}*.46f;
    for(float sign:{-1.f,1.f})line(track,from+side*sign,to+side*sign,.035f,{.42f,.34f,.23f});
   }
   for(auto cell:o.slots)rim(s,world(v.quest,cell,v.content),.88f,{.42f,.34f,.23f});
  }
  if(d.family==Family::Bench){
  auto& path=s.spawn("Button's route to his mouse");out.bindings.benchPath=path.id();
  for(auto cell:d.walkable)for(auto dir:{Direction::East,Direction::South})if(std::find(d.walkable.begin(),d.walkable.end(),adjacent(cell,dir))!=d.walkable.end())
   line(path,world(v.quest,cell,v.content),world(v.quest,adjacent(cell,dir),v.content),.04f,{.64f,.43f,.22f});
  rim(s,world(v.quest,d.reach,v.content),.3f,{.64f,.43f,.22f});
  }
 }
 if(d.family==Family::Planks){
  // Shared junctions are built once; overlapping platforms hid the gaps.
  std::vector<Cell> junctions;
  for(const auto& socket:d.sockets)if(socket.supported)for(auto cell:{socket.cell,socket.end}){
   if(cell==d.source||cell==d.destination||std::find(junctions.begin(),junctions.end(),cell)!=junctions.end())continue;
   junctions.push_back(cell);auto& landing=prop(s,meshes,"Bridge landing",beach?"beach_support":"platform",world(v.quest,cell,v.content));if(!beach)landing.transform.scale={.58f,1,.72f};
  }
  auto& cushion=prop(s,meshes,"Sunny resting cushion","cushion",world(v.quest,d.destination,v.content)+engine::Vec3{0,.46f,0});cushion.transform.scale={.65f,.65f,.65f};
  auto& mat=prop(s,meshes,"Mabel's starting place","paw_mat",world(v.quest,d.source,v.content)+engine::Vec3{-.7f,.005f,0});mat.transform.scale={.8f,1,.8f};
 }
 if(d.family==Family::Shade){
  for(auto c:d.shade){
   if(v.quest!="beach_windbreak"){rim(s,world(v.quest,c,v.content),.85f,{.95f,.88f,.65f});continue;}
   auto& mark=s.spawn("Workshop cell needing shade");mark.transform.position=world(v.quest,c,v.content)+engine::Vec3{0,.018f,0};mark.transform.scale={.88f,.012f,.88f};
   auto& mesh=mark.addComponent<engine::MeshComponent>("Soft sand fill",engine::Mesh::cube());
   auto material=std::make_shared<engine::Material>();material->shader=engine::MaterialShader::Unlit;material->baseColor={.78f,.66f,.44f};material->alphaMode=engine::AlphaMode::Blend;material->opacity=.55f;
   mesh.setMaterial(0,engine::MaterialInstance{material});mesh.color={1,1,1};mesh.castsShadow=false;
   out.bindings.workshopCells.push_back({c,mark.id()});
  }
  std::vector<Cell> supports;
  for(const auto& socket:d.sockets)if(socket.supported)for(auto cell:{socket.cell,socket.end}){
   if(std::find(supports.begin(),supports.end(),cell)!=supports.end())continue;
   supports.push_back(cell);
   if(beach){prop(s,meshes,"Firm canopy support","beach_shade_post",world(v.quest,cell,v.content));continue;}
   auto& post=s.spawn("Firm canopy support");post.transform.position=world(v.quest,cell,v.content)+engine::Vec3{0,.27f,0};post.transform.scale={.05f,.54f,.05f};post.addComponent<engine::MeshComponent>("Canopy post",engine::Mesh::cube()).color={.6f,.4f,.24f};
  }
 }
 if(d.family==Family::Shapes){
  for(auto c:d.basket)rim(s,world(v.quest,c,v.content)+engine::Vec3{.125f,0,.125f},.25f,{.15f,.4f,.36f});
  for(auto tuple:{std::tuple{"square",Cell{0,0},0},std::tuple{"bar",Cell{2,0},1},std::tuple{"elbow",Cell{3,1},1}}){
   const auto [id,cell,rotation]=tuple;const auto& o=*std::find_if(d.objects.begin(),d.objects.end(),[&](auto& x){return x.id==id;});
   auto& a=prop(s,meshes,"Cushion recess",std::string("socket_")+id,shapeOrigin(o,cell,rotation,v.quest,v.content));a.transform.rotation.y=-rotation*engine::pi/2;
  }
 }
 if(d.sunnyNook){for(auto c:d.shade)rim(s,world(v.quest,c,v.content),.8f,{.9f,.7f,.25f});for(auto c:d.walkable)rim(s,world(v.quest,c,v.content),.55f,{.15f,.4f,.36f});}
 if(d.family==Family::Picnic&&!d.sunnyNook){
  for(auto c:d.shade){auto& mark=prop(s,meshes,"Shade mark","shade_clue",world(v.quest,c,v.content)+engine::Vec3{-.28f,.008f,-.28f});if(beach)mark.transform.scale={.4f,.4f,.4f};}
  for(auto c:d.quiet){auto& mark=prop(s,meshes,"Quiet mark","quiet_clue",world(v.quest,c,v.content)+engine::Vec3{.28f,.008f,.28f});if(beach)mark.transform.scale={.4f,.4f,.4f};}
 }
 if(d.family!=Family::Switches&&d.family!=Family::Balance){
  const auto q=progression::questIndex(v.quest);unsigned count=0;
  for(unsigned i=0;i<4;++i)if(q<paws::release1::progression::quests.size()&&(progression::quests[q].requesters&(1u<<i))&&meshes.contains(std::string(progression::catIds[i]))){
   auto& cat=s.spawn(std::string(progression::catNames[i]));auto& skin=cat.addComponent<engine::SkeletalMeshComponent>("Requesting cat",meshes.at(std::string(progression::catIds[i])));
   skin.color={1,1,1};cats::present(skin,1,reducedMotion);cat.transform.position={(d.width+1)*step/2+.6f,0,float(count++)*1.5f-1.f};
   if(d.family==Family::Planks){cat.transform.position=world(v.quest,d.source,v.content)+engine::Vec3{-.7f,0,0};cat.transform.rotation.y=engine::pi/2;}
   if(d.family==Family::Bench){cat.transform.position=world(v.quest,d.source,v.content)+engine::Vec3{-.4f,0,0};cat.transform.rotation.y=engine::pi/2;}
   if(beach&&d.family!=Family::Planks&&d.family!=Family::PictureSequence){cat.transform.position=world(v.quest,d.destination,v.content)+engine::Vec3{1.1f,0,0};cat.transform.position.z+=float(count-1)*1.05f;cat.transform.rotation.y=-engine::pi/2;}
   if(beach&&d.family==Family::Scent)cat.transform.position.x-=.8f;
   if((q>=3&&q<=7)||q==13||q==15||q==16){
    auto& e=out.bindings.ending;e.actor=cat.id();e.home=e.start=cat.transform.position;e.homeRotation=cat.transform.rotation;e.kind=q==13||q==15?5:q==16?7:int(q)+1;
    if(q==6||q==16){auto& tongue=prop(s,meshes,"Mabel's tongue","cat_tongue");tongue.enabled=false;e.tongue=tongue.id();}
   }
   // The continuous courtyard includes every requester's standing space.
   out.bindings.requesters.push_back({cat.id(),skin.id(),{},0});
   if(d.beachPicnic){
    auto& name=s.spawn("Picnic cat name: "+std::string(progression::catNames[i]));
    name.setSavedId("picnic.cat-name."+std::string(progression::catIds[i]));
    addCatName(name,i,{0,0,0},1.f);
   }
  }
 }
 auto& sun=s.spawn("Sun").addComponent<engine::LightComponent>("Garden sun",engine::LightType::Directional);sun.direction={-.5f,-1,-.3f};sun.intensity=1.5f;
 sun.color={1,.95f,.87f};sun.shadow.enabled=true;sun.shadow.halfExtent=8;sun.shadow.halfDepth=12;sun.shadow.normalBias=.01f;
 if(beach){
  // Keep warm directional light distinct from the sky fill so soft shadows
  // visibly ground the cats and props on pale sand on both renderers.
  s.ambient={.20f,.22f,.24f};sun.intensity=2.f;
  sun.shadow.center={0,0,-1};sun.shadow.halfExtent=12;sun.shadow.halfDepth=18;
 }
 auto& camera=s.spawn("View").addComponent<engine::CameraComponent>("Puzzle camera");camera.setPerspective(engine::pi*35/180,.05f,80);out.camera=&camera;
 out.bindings.collision.build(s,meshes);project(s,out.bindings,v,reducedMotion);return out;
}
void guideMovement(engine::Scene& s,SceneBindings& b,const PuzzleView& v,const std::vector<Intent>& options,std::string_view selected,const std::vector<Intent>& afterRotation) {
 struct Rect {engine::Vec3 p;float w,h;};std::vector<Rect> rectangles;
 const auto d=rules::quest(v.quest,v.content);
 auto add=[&](engine::Vec3 p,float w,float h){
  if(std::none_of(rectangles.begin(),rectangles.end(),[&](const auto& r){return r.p.x==p.x&&r.p.y==p.y&&r.p.z==p.z&&r.w==w&&r.h==h;}))rectangles.push_back({p,w,h});
 };
 for(const auto& option:options){
  if(v.quest=="beach_windbreak"&&selected.empty())continue;
  if(option.verb==Verb::Slide){
   auto p=world(v.quest,option.destination,v.content);
   // The paw switch has a raised plate: keep its legal crate destination
   // above that surface instead of burying the white border underneath it.
   if(rules::cooperativePlayhouse(d)&&option.destination==d.sockets[0].cell)p.y=.13f;
   add(p,.88f,.88f);continue;
  }
  if(d.family==Family::Shapes){
   const auto o=std::find_if(d.objects.begin(),d.objects.end(),[&](const auto& o){return o.id==option.object;});
   const auto current=std::find_if(v.objects.begin(),v.objects.end(),[&](const auto& o){return o.id==option.object;});
   if(o!=d.objects.end()&&current!=v.objects.end()){
    ObjectState state;state.cell=option.destination;state.orientation=current->orientation;
    for(auto cell:rules::footprint(*o,state))add(world(v.quest,cell,v.content)+engine::Vec3{.125f,0,.125f},.23f,.23f);
   }
  }else if(d.legacy){
   const auto socket=std::find_if(d.legacy->objects.begin(),d.legacy->objects.end(),[&](const auto& o){return o.id==(option.target=="home"?option.object:option.target);});
   if(socket!=d.legacy->objects.end())add(world(v.quest,socket->initial,v.content),.70f,.70f);
  }else{
   const auto socket=std::find_if(d.sockets.begin(),d.sockets.end(),[&](const auto& o){return o.id==option.target;});
   if(socket==d.sockets.end())continue;
   if(socket->role=="plank_socket"||socket->role=="shade_socket"){
    const auto from=world(v.quest,socket->cell,v.content),to=world(v.quest,socket->end,v.content);
    add((from+to)*.5f,std::abs(to.x-from.x)+.60f,std::abs(to.z-from.z)+.60f);
   }else {
    // Lookout seats are raised platforms; ground-level outlines are hidden underneath.
    const float height=socket->role=="lookout_seat"?.33f:0.f;
    add(world(v.quest,socket->cell,v.content)+engine::Vec3{0,height,0},.88f,.88f);
   }
  }
 }
 auto makeGuide=[&](const char* name,bool selection){
  auto& actor=s.spawn(name);actor.enabled=false;
  for(int edge=0;edge<4;++edge)for(bool border:{true,false}){
   auto& mesh=actor.addComponent<engine::MeshComponent>("Movement outline",engine::Mesh::cube());
   auto material=std::make_shared<engine::Material>();material->shader=engine::MaterialShader::Unlit;
   material->baseColor=border?engine::Vec3{.157f,.4f,.365f}:selection?engine::Vec3{1,.75f,.32f}:engine::Vec3{1,.973f,.914f};
   mesh.setMaterial(0,engine::MaterialInstance{material});mesh.color={1,1,1};mesh.castsShadow=false;
  }
  return actor.id();
 };
 auto placeGuide=[&](engine::Actor& actor,const Rect& r,bool broken=false){
  actor.enabled=true;actor.transform.position=r.p+engine::Vec3{0,.065f,0};
  unsigned index=0;for(auto& child:actor.children())if(auto* mesh=dynamic_cast<engine::MeshComponent*>(child.get())){
   const unsigned edge=index/2;const bool border=index++%2==0;const float width=border?.085f:.04f;
   mesh->transform.position={edge<2?0:(edge==2?-r.w:r.w)*.5f,border?0:.009f,edge<2?(edge==0?-r.h:r.h)*.5f:0};
   const float span=broken?.60f:1.f;
   mesh->transform.scale=edge<2?engine::Vec3{(r.w+width)*span,.008f,width}:engine::Vec3{width,.008f,(r.h+width)*span};
  }
 };
 while(b.movementGuides.size()<rectangles.size())b.movementGuides.push_back(makeGuide("Legal movement destination",false));
 for(std::size_t n=0;n<b.movementGuides.size();++n)if(auto* actor=s.findActor(b.movementGuides[n])){
  actor->enabled=n<rectangles.size();if(actor->enabled)placeGuide(*actor,rectangles[n]);
 }
 std::vector<Rect> turns;
 for(const auto& option:afterRotation){
  if(v.quest=="beach_windbreak")continue;
  if(std::any_of(options.begin(),options.end(),[&](const auto& i){return i.object==option.object&&i.target==option.target;}))continue;
  const auto socket=std::find_if(d.sockets.begin(),d.sockets.end(),[&](const auto& s){return s.id==option.target&&(s.role=="plank_socket"||s.role=="shade_socket");});
  if(socket==d.sockets.end())continue;
  const auto from=world(v.quest,socket->cell,v.content),to=world(v.quest,socket->end,v.content);
  turns.push_back({(from+to)*.5f,std::abs(to.x-from.x)+.60f,std::abs(to.z-from.z)+.60f});
 }
 while(b.rotationGuides.size()<turns.size())b.rotationGuides.push_back(makeGuide("Fits after a quarter-turn",true));
 for(std::size_t n=0;n<b.rotationGuides.size();++n)if(auto* actor=s.findActor(b.rotationGuides[n])){
  actor->enabled=n<turns.size();if(actor->enabled)placeGuide(*actor,turns[n],true);
 }
 const auto basket=std::find_if(v.objects.begin(),v.objects.end(),[&](const auto& o){return o.id==selected&&o.role=="crate";});
 if(basket!=v.objects.end()){
  if(!s.findActor(b.selectedBasketGuide))b.selectedBasketGuide=makeGuide("Selected basket",true);
  placeGuide(*s.findActor(b.selectedBasketGuide),{world(v.quest,basket->cell,v.content),1.12f,1.12f});
 }else if(auto* actor=s.findActor(b.selectedBasketGuide))actor->enabled=false;
}
void previewWorkshop(engine::Scene& s,SceneBindings& b,const PuzzleView& v,std::string_view panel,std::string_view target) {
 auto* ghost=s.findActor(b.workshopPreview);if(ghost)ghost->enabled=false;
 if(v.quest!="beach_windbreak"||panel.empty()||target.empty())return;
 const auto d=rules::quest(v.quest,v.content);
 const auto socket=std::find_if(d.sockets.begin(),d.sockets.end(),[&](const auto& t){return t.id==target;});
 const auto binding=std::find_if(b.objects.begin(),b.objects.end(),[&](const auto& o){return o.object==panel;});
 if(socket==d.sockets.end()||binding==b.objects.end())return;
 auto* source=s.findActor(binding->actor);if(!source)return;
 auto* mesh=source->getComponent<engine::MeshComponent>();if(!mesh)return;
 if(!ghost){
  ghost=&s.spawn("Workshop placement preview");b.workshopPreview=ghost->id();
  auto& preview=ghost->addComponent<engine::MeshComponent>("Translucent shade",mesh->mesh);
  auto material=std::make_shared<engine::Material>();material->baseColor={1,.97f,.88f};material->alphaMode=engine::AlphaMode::Blend;material->opacity=.42f;material->doubleSided=true;
  for(std::size_t n=0;n<std::max(std::size_t(1),mesh->mesh->materials.size());++n)preview.setMaterial(n,engine::MaterialInstance{material});
  preview.color={1,1,1};preview.castsShadow=false;
 }
 ghost->transform=source->transform;
 ghost->transform.position=(world(v.quest,socket->cell,v.content)+world(v.quest,socket->end,v.content))*.5f+engine::Vec3{0,.58f,0};
 ghost->transform.rotation.y=socket->cell.x==socket->end.x?engine::pi/2:0;ghost->enabled=true;
}
void guidePlank(engine::Scene& s,const SceneBindings& b,const PuzzleView& v,std::string_view object) {
 const auto d=rules::quest(v.quest,v.content);if(d.family!=Family::Planks)return;
 const auto plank=std::find_if(d.objects.begin(),d.objects.end(),[&](const auto& o){return o.id==object;});
 for(const auto& binding:b.objects){
  const auto socket=std::find_if(d.sockets.begin(),d.sockets.end(),[&](const auto& o){return o.id==binding.object;});
  if(socket==d.sockets.end())continue;
  const bool match=plank!=d.objects.end()&&plank->length==socket->length;
  if(auto* actor=s.findActor(binding.actor))for(auto& child:actor->children())if(auto* mesh=dynamic_cast<engine::MeshComponent*>(child.get())){
   mesh->color=match?engine::Vec3{.92f,.77f,.42f}:engine::Vec3{.16f,.40f,.36f};
   const float width=match?.11f:.055f;
   mesh->transform.scale=socket->orientation?engine::Vec3{width,.008f,.11f}:engine::Vec3{.11f,.008f,width};
  }
 }
}
bool project(engine::Scene& s,SceneBindings& b,const PuzzleView& v,bool reduced) {
 if(b.questId!=v.quest||v.attempt<b.attempt||(v.attempt==b.attempt&&v.revision<b.revision))return false;
 for(auto& binding:b.objects)if(!s.findActor(binding.actor))return false;
 const auto d=rules::quest(v.quest,v.content);
 for(unsigned n=0;n<b.beachMilestones.size();++n)if(auto* a=s.findActor(b.beachMilestones[n]))a->enabled=(v.beachProgress&(1u<<n))!=0;
 if(d.family==Family::PictureSequence){
  if(b.sequencePlayback.size()==0||v.attempt!=b.attempt||v.sequenceReplay!=b.sequenceReplay||(!v.completed&&b.completed))b.sequencePlayback.start(d.sequence,d.pictureSeconds);
  b.sequenceReplay=v.sequenceReplay;if(v.completed)b.sequencePlayback.finish();
  for(const auto& [symbol,id]:b.sequencePictures)if(auto* picture=s.findActor(id))picture->enabled=b.sequencePlayback.picture()==symbol;
 }
 if(v.quest=="beach_windbreak"){
  b.workshopCovered=rules::shadedCells(d,v);
  for(const auto& [cell,id]:b.workshopCells)if(auto* mark=s.findActor(id)){
   const bool covered=std::find(b.workshopCovered.begin(),b.workshopCovered.end(),cell)!=b.workshopCovered.end();
   auto* mesh=mark->getComponent<engine::MeshComponent>();
   if(reduced||b.attempt==0)mesh->opacity=covered?0.f:1.f;
   mark->enabled=!covered||mesh->opacity>0;
  }
 }
 for(auto& binding:b.objects){auto at=std::find_if(v.objects.begin(),v.objects.end(),[&](auto& o){return o.id==binding.object;});if(at==v.objects.end())return false;
  const auto& o=*at;auto& a=*s.findActor(binding.actor);auto p=world(v.quest,o.cell,v.content);float yaw=-o.orientation*engine::pi/2;
  if(v.quest==progression::beachQuestId&&o.role=="ball"){
   p=ballPosition(v.quest,v.completed?d.destination:d.source,v.content);
  }
  if(o.role.starts_with("cushion_")){auto obj=std::find_if(d.objects.begin(),d.objects.end(),[&](auto& x){return x.id==o.id;});if(obj!=d.objects.end())p=shapeOrigin(*obj,o.cell,o.orientation,v.quest,v.content);if(!o.socket.empty())p.y=.036f;}
  if((o.role=="plank"||o.role=="shade_panel")&&!o.socket.empty()){auto socket=std::find_if(d.sockets.begin(),d.sockets.end(),[&](auto& x){return x.id==o.socket;});if(socket!=d.sockets.end()){p=(world(v.quest,socket->cell,v.content)+world(v.quest,socket->end,v.content))*.5f;yaw=socket->cell.x==socket->end.x?engine::pi/2:0;}}
  if(o.role=="shade_panel")p.y=.58f;
  if(o.role=="plank_socket"||o.role=="shade_socket"){yaw=0;for(auto& child:a.children())child->enabled=!o.active;}
  if(d.family==Family::Planks&&o.id=="mabel_start"&&!progression::isBeachQuest(v.quest))a.transform.scale={.7f,1,.8f};
  if(o.role=="sunny_perch")yaw=-engine::pi/2;
  if(o.held)p.y+=.6f;if(o.collected)p.y=-3;
  // Increasing beam positions run along +X. A positive Z rotation lowers the
  // left end; a negative rotation lowers the right, matching the rules' pull.
  if(o.role=="balance_beam"){p.y=.20f;a.transform.scale={3.5f,1,1.3f};a.transform.rotation.z=o.orientation==1?.035f:o.orientation==2?-.035f:0;yaw=0;}
  if(o.role=="balance_support"){a.transform.scale={.30f,.20f,.55f};}
  if(o.role=="lookout_cushion")p.y=o.socket.empty()?0:.32f;
  if(o.role=="lookout_seat")p.y=.33f;
  if(d.legacy&&d.legacy->goal==Goal::Relay){
   const auto ball=std::find_if(v.objects.begin(),v.objects.end(),[](const auto& t){return t.role=="ball";});
   if(o.role=="channel"&&o.cell.x>d.legacy->checkpoint.x)a.enabled=ball!=v.objects.end()&&ball->cell!=d.source;
   if(o.role=="gutter"&&o.cell.x>d.legacy->checkpoint.x)a.enabled=std::any_of(v.objects.begin(),v.objects.end(),[](const auto& t){return t.role=="water_basin"&&t.active;});
   if(o.role=="ball")p.y=.42f;
  }
  if(o.role=="water_basin")for(auto& child:a.children())if(child->name=="Basin water")child->enabled=o.active;
  if(o.role=="balance_weight")p.y=o.socket.starts_with("beam_")?.30f:.035f;
  if(o.role=="balance_socket"){a.transform.scale={.75f,1,.75f};p.y=.30f;}
  if(o.role=="balance_pip"){if(o.active){p.z-=.65f;p.y=.35f;}else p.y=0;}
  if(o.role=="balance_pad"){p.z-=.65f;p.y=0;a.transform.scale={.8f,.7f,.9f};a.enabled=o.active;}
  if(o.role=="trail_clue"&&v.quest!="beach_pepper_basket"){if(auto* mesh=a.getComponent<engine::MeshComponent>())mesh->color=o.active?engine::Vec3{.65f,1,.75f}:engine::Vec3{1,1,1};}
  if(o.role.starts_with("bell_"))p.y=1.25f;
  if(o.role=="sequence_blackboard")p.z=-1.45f;
  if(o.role=="gutter"||o.role=="channel"){auto obj=std::find_if(d.legacy->objects.begin(),d.legacy->objects.end(),[&](auto& x){return x.id==o.id;});yaw=(obj->kind==Kind::Straight?1-o.orientation:3-o.orientation)*engine::pi/2;p.y=.4f;}
  // The beach kit has floor-contact pivots. Covered channels become available
  // as baskets slide away, without inheriting the legacy greybox elevation.
  if(v.quest==progression::beachQuestId&&o.role=="channel"){
   p.y=0;
   a.enabled=std::none_of(v.objects.begin(),v.objects.end(),[&](const auto& cover){return cover.role=="crate"&&cover.cell==o.cell;});
  }
  if(o.role=="playhouse_door"){
   if(rules::cooperativePlayhouse(d)){
    // Right hinge opens toward the clear approach, away from the crate plate.
    p=p+engine::Vec3{.85f,.04f,0};a.transform.scale={2.4f,1.2f,1};yaw=engine::pi+(o.active?100*engine::pi/180:0);
   }else {p=p+engine::Vec3{-.355f,.07f,.825f};yaw=o.active?-100*engine::pi/180:0;}
  }
  if(o.role=="side_gate"){p=p+engine::Vec3{-.54f,.08f,-.5f};yaw=o.active?-110*engine::pi/180:0;}
  if(o.role=="switch_pip"||o.role=="switch_pepper"||o.role=="inside_latch")p.y=o.active?.055f:.09f;
  if(rules::cooperativePlayhouse(d)&&o.role.starts_with("cat_")){
   p.y=o.socket=="pip_switch"||o.socket=="pepper_switch"?.10f:o.socket=="inside_latch"?.10f:o.socket=="inside_entry"||o.socket=="pepper_inside"?.04f:0;
   yaw=o.socket=="storage"?engine::pi/2:o.socket=="inside_latch"||o.socket=="inside_entry"?engine::pi:0;
  }
  if(rules::cooperativePlayhouse(d)&&o.id=="crate")p.y=o.cell==d.sockets[0].cell?.10f:0;
  a.transform.position=p;a.transform.rotation.y=yaw;
  if(o.role=="gutter"||o.role=="drinking_bowl")for(auto& child:a.children())if(child->name=="Validated water surface"||child->name=="Filled bowl"||child->name=="Bowl inlet ribbon")child->enabled=v.completed||(o.role=="gutter"&&d.legacy&&d.legacy->goal==Goal::Relay&&o.cell.x<d.legacy->checkpoint.x&&std::any_of(v.objects.begin(),v.objects.end(),[](const auto& t){return t.role=="water_basin"&&t.active;}));
  if(auto* cat=a.getComponent<engine::SkeletalMeshComponent>())presentPip(*cat,v.completed?2:0,reduced);
 }
 const bool hasEnding=b.ending.kind>=4&&b.ending.kind<=8;
 b.collision.synchronize(s);
 if(auto* path=s.findActor(b.benchPath)){
  const bool clear=v.completed||std::any_of(v.actions.begin(),v.actions.end(),[](const auto& a){return a.intent.verb==Verb::Retrieve&&a.enabled;});
  for(auto& child:path->children())if(auto* mark=dynamic_cast<engine::MeshComponent*>(child.get()))mark->color=clear?engine::Vec3{.18f,.43f,.38f}:engine::Vec3{.64f,.43f,.22f};
 }
 if(hasEnding){
  auto& e=b.ending;
  if(v.completed&&(!b.completed||v.attempt!=b.attempt)){
   e.active=true;e.finished=false;e.facingComplete=false;e.elapsed=0;e.route={e.home};e.carryRoute.clear();e.flowRoute.clear();e.leadSeconds=0;e.carrySeconds=0;e.segment=e.carrySegment=0;
   if(e.kind==4){
    auto* toy=object(s,b,"mouse");e.toy=toy->id();auto mouse=std::find_if(v.objects.begin(),v.objects.end(),[](const auto& o){return o.id=="mouse";});e.toyStart=world(v.quest,mouse->cell,v.content);
    e.target=e.toyStart+engine::Vec3{-.64f,0,-.137f};e.finalYaw=engine::pi/2;e.clipSeconds=4;
    e.route.push_back(world(v.quest,d.source,v.content));e.route.push_back(world(v.quest,d.reach,v.content));e.route.push_back(e.target);
    e.carryRoute={e.target,world(v.quest,d.reach,v.content),world(v.quest,d.source,v.content),world(v.quest,d.source,v.content)+engine::Vec3{0,0,1.05f}};e.carrySeconds=routeLength(e.carryRoute)/pipWalkSpeed;
   }else if(e.kind==5){
    for(auto cell:v.route)if(cell!=d.destination)e.route.push_back(world(v.quest,cell,v.content)+engine::Vec3{0,.16f,0});
    e.route.push_back(world(v.quest,d.destination,v.content)+engine::Vec3{-.48f,.46f,0});e.target=world(v.quest,d.destination,v.content)+engine::Vec3{0,.58f,0};e.route.push_back(e.target);e.finalYaw=engine::pi/2;e.clipSeconds=5;
   }else if(e.kind==6){e.target={.05f,.14f,-.22f};e.route.push_back({.9f,0,.35f});e.route.push_back(e.target);e.finalYaw=-engine::pi/2;e.clipSeconds=8;
   }else if(e.kind==7){auto* bowl=object(s,b,"drinking_bowl");e.toy=bowl->id();e.target=bowl->transform.position+engine::Vec3{.5223514f,0,-.0483272f};e.route.push_back(e.target);e.finalYaw=-engine::pi/2;e.leadSeconds=1.5f;e.clipSeconds=5;for(auto cell:v.route)e.flowRoute.push_back(world(v.quest,cell,v.content));
   }else{auto* bell=object(s,b,"star");e.toy=bell->id();e.target=bell->transform.position+engine::Vec3{-.137f,-1.25f,.58f};e.route.push_back({e.home.x,0,.75f});e.route.push_back(e.target);e.finalYaw=engine::pi;e.leadSeconds=1.0f;e.clipSeconds=4.5f;}
   static constexpr const char* endingClips[]={"quest_mouse_collect","quest_sun_stretch","quest_bed_knead","quest_drink_finish","quest_bell_touch"};
   const auto* skin=s.findActor(e.actor)->getComponent<engine::SkeletalMeshComponent>();
   e.clipSeconds=skin->mesh->skeleton->clips[pipClip(*skin->mesh,endingClips[e.kind-4])].duration;
   e.route=b.collision.supportedRoute(s,e.route);e.carryRoute=b.collision.supportedRoute(s,e.carryRoute);
   e.walkSeconds=routeLength(e.route)/pipWalkSpeed;e.carrySeconds=routeLength(e.carryRoute)/pipWalkSpeed;
   if((reduced||b.attempt==0)&&!e.route.empty()){e.elapsed=e.leadSeconds+e.walkSeconds+e.clipSeconds+e.carrySeconds;e.active=false;e.finished=true;}
  }
  if(!v.completed){e.active=e.finished=false;e.elapsed=0;if(auto* a=s.findActor(e.actor)){a->transform.position=e.home;a->transform.rotation=e.homeRotation;cats::present(*a->getComponent<engine::SkeletalMeshComponent>(),1,reduced);}if(auto* tongue=s.findActor(e.tongue))tongue->enabled=false;}
  else {if(reduced&&!e.route.empty()){e.elapsed=e.leadSeconds+e.walkSeconds+e.clipSeconds+e.carrySeconds;e.active=false;e.finished=true;}endingPose(s,b);}
 }
 for(auto& r:b.requesters)if(!hasEnding&&v.quest!="beach_pepper_basket")if(auto* skin=dynamic_cast<engine::SkeletalMeshComponent*>(s.findComponent(r.skin))){
  const bool newlyCompleted=v.completed&&!b.completed;
  if(d.beachPicnic){
   const auto n=std::size_t(&r-b.requesters.data());const char* ids[]={"pip_mat","mabel_mat","button_mat","pepper_mat"};
   if(auto* cat=s.findActor(r.actor)){
    auto mat=std::find_if(v.objects.begin(),v.objects.end(),[&](const auto& o){return o.id==ids[n];});
    if(v.completed&&mat!=v.objects.end()){
     const auto* bed=object(s,b,mat->id);const auto* mesh=bed?bed->getComponent<engine::MeshComponent>():nullptr;
     const float top=mesh?mesh->mesh->localBounds().maximum.y:.018f;
     cat->transform.position=world(v.quest,mat->cell,v.content)+engine::Vec3{0,top,0};cat->transform.rotation.y=engine::pi;
    }
    else {
     const auto initial=std::find_if(d.objects.begin(),d.objects.end(),[&](const auto& o){return o.id==ids[n];});
     cat->transform.position=world(v.quest,initial->initial,v.content)+engine::Vec3{0,0,1.25f};
     cat->transform.rotation.y=0;
    }
    if(auto* name=dynamic_cast<engine::Actor*>(s.findSavedNode("picnic.cat-name."+std::string(progression::catIds[n]))))
     name->transform.position=cat->transform.position+engine::Vec3{0,1.5f,.8f};
   }
   // Completion cards suspend simulation: project the final sitting pose now,
   // rather than freezing a crossfade on the standing request pose.
   cats::present(*skin,v.completed?14:1,v.completed||reduced);
  }else if(newlyCompleted&&d.sunnyNook){
   const auto n=std::size_t(&r-b.requesters.data());const auto seat=std::find_if(v.objects.begin(),v.objects.end(),[&](const auto& o){return o.id==(n==0?"bowl":"bed");});
   if(auto* a=s.findActor(r.actor);a&&seat!=v.objects.end()){
    const auto target=world(v.quest,seat->cell,v.content)+engine::Vec3{0,.21f,0};
    r.route=b.collision.route(s,a->transform.position,target+engine::Vec3{0,-.21f,.9f});r.route.push_back(target);r.route=b.collision.supportedRoute(s,r.route);r.elapsed=0;r.segment=0;
    if(reduced&&!r.route.empty())a->transform.position=r.route.back();
   }
   cats::present(*skin,11,reduced);
  }else if(newlyCompleted&&d.family==Family::Planks){
   if(auto* a=s.findActor(r.actor))r.route.push_back(a->transform.position);
   for(auto cell:v.route)if(cell!=d.destination)r.route.push_back(world(v.quest,cell,v.content)+engine::Vec3{0,.16f,0});
   r.route.push_back(world(v.quest,d.destination,v.content)+engine::Vec3{-.48f,.20f,0});
   r.route.push_back(world(v.quest,d.destination,v.content)+engine::Vec3{0,.54f,0});r.elapsed=0;r.segment=0;r.route=b.collision.supportedRoute(s,r.route);
   if(auto* a=s.findActor(r.actor);a&&!r.route.empty())a->transform.position=reduced?r.route.back():r.route.front();
   cats::present(*skin,reduced?11:3,reduced);
  }else if(r.route.empty())cats::present(*skin,v.completed?(d.family==Family::Balance?11:2):1,reduced);
  if(!v.completed){r.route.clear();r.elapsed=0;if(d.family==Family::Planks)if(auto* a=s.findActor(r.actor)){a->transform.position=world(v.quest,d.source,v.content)+engine::Vec3{-.7f,0,0};a->transform.rotation.y=engine::pi/2;}}
 }
 guidePlank(s,b,v,{});
 projectPepper(s,b,v,reduced);
 b.attempt=v.attempt;b.revision=v.revision;b.completed=v.completed;b.reducedMotion=reduced;return true;
}
void reactPepper(engine::Scene& s,SceneBindings& b,const PuzzleView& before,const PuzzleView& after,const Intent& i,bool accepted,bool reduced){
 startPepperReaction(s,b,before,after,i,accepted,reduced);
}
void presentPepperEvidence(PuzzleView& v,const SceneBindings& b){
 if(v.quest!="beach_pepper_basket")return;
 for(auto& o:v.objects)if(o.role=="trail_clue")
  o.active=o.active&&!(b.pepper.awaitingClues&(1u<<(o.id.back()-'0')));
}
void animate(engine::Scene& s,SceneBindings& b,double delta){
 for(const auto& [cell,id]:b.workshopCells)if(auto* mark=s.findActor(id)){
  const bool covered=std::find(b.workshopCovered.begin(),b.workshopCovered.end(),cell)!=b.workshopCovered.end();
  auto* mesh=mark->getComponent<engine::MeshComponent>();const float target=covered?0.f:1.f;
  const float step=b.reducedMotion?1.f:float(std::max(0.,delta))*5.f;
  mesh->opacity+=std::clamp(target-mesh->opacity,-step,step);mark->enabled=mesh->opacity>0;
 }
 b.collision.synchronize(s);
 if(b.questId=="beach_pepper_basket"){animatePepper(s,b,delta);return;}
 if(!b.ballRoute.empty()){
  b.ballElapsed+=b.reducedMotion?100.f:float(std::max(0.,delta))*2.f;
  const auto index=std::size_t(b.ballElapsed);if(auto* ball=s.findActor(b.ball)){
   if(index+1>=b.ballRoute.size()){ball->transform.position=b.ballRoute.back();b.ballRoute.clear();}
   else ball->transform.position=b.ballRoute[index]+(b.ballRoute[index+1]-b.ballRoute[index])*(b.ballElapsed-index);
  }else b.ballRoute.clear();
 }
 if(!b.butterflies.empty()){
  const float time=b.ending.elapsed-b.ending.walkSeconds;
  for(std::size_t n=0;n<b.butterflies.size();++n)if(auto* a=s.findActor(b.butterflies[n])){
   a->enabled=b.completed&&(b.reducedMotion||time>1);if(!a->enabled)continue;
   const float t=b.reducedMotion?2.f:std::max(0.f,time-1);
   a->transform.position=b.ending.target+engine::Vec3{-.9f+float(n)*.45f+.12f*std::sin(t*2+n),.7f+std::min(1.5f,t*.22f)+float(n%2)*.2f,-.8f};
   for(auto& c:a->children())c->transform.rotation.y=b.reducedMotion?.4f:std::sin(t*8+n)*.65f;
  }
 }
 if(b.sequencePlayback.size()){
  b.sequencePlayback.advance(delta);
  for(const auto& [symbol,id]:b.sequencePictures)if(auto* picture=s.findActor(id))picture->enabled=b.sequencePlayback.picture()==symbol;
 }
 if(b.ending.kind>=4&&b.ending.kind<=8){
  auto& e=b.ending;if(!e.active||e.route.empty()||!std::isfinite(delta)||delta<=0)return;
  const float total=e.leadSeconds+e.walkSeconds+e.clipSeconds+e.carrySeconds;
  e.elapsed=b.reducedMotion?total:std::min(total,e.elapsed+float(delta));endingPose(s,b,b.reducedMotion?0:delta);
  if(e.elapsed>=total){e.active=false;e.finished=true;}return;
 }
 if(b.reducedMotion||!std::isfinite(delta)||delta<=0)return;
 for(auto& r:b.requesters)if(r.route.size()>1){
  auto* a=s.findActor(r.actor);auto* skin=dynamic_cast<engine::SkeletalMeshComponent*>(s.findComponent(r.skin));if(!a||!skin)continue;
  r.elapsed+=delta;
  if(b.collision.sample(s,*a,r.route,r.segment,float(r.elapsed)*pipWalkSpeed)){cats::present(*skin,11,false);r.route.clear();}
  else sampleCatWalk(*a,*skin,r.elapsed,true);
 }
}
}




