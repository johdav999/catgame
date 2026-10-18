#include "presentation.hpp"
#include "water_surface.hpp"
#include "release1/progression/policy.hpp"
#include "release1/cats/cat_assets.hpp"
#include "engine/light_component.hpp"
#include "engine/interaction_component.hpp"
#include <algorithm>
#include <cmath>
#include <queue>
#include <stdexcept>
#include <atomic>

namespace paws::release1::garden {
namespace p=paws::release1::progression;
namespace {
const std::array<Spot,9> authored{{
 {"bench_corner",{-1.65f,0,.85f},1.1f,1.35f,1},
 {"sunny_corner",{1.65f,0,.85f},1.1f,1.35f,1},
 {"quiet_corner",{1.65f,0,-.85f},1.1f,1.35f,1},
 {"scratch_corner",{-4,0,.4f},.8f,.8f,1},
 {"perch_corner",{-4,0,-2.1f},.9f,1.5f,2},
 {"hammock_nook",{4,0,-2.1f},1.2f,1.7f,2},
 {"playhouse_corner",{4,0,2},1.6f,1.7f,3},
 {"picnic_table_spot",{-4,0,3.6f},1.3f,1.5f,3},
 {"picnic_mat_spot",{0,0,4},1.6f,1.6f,3}
}};
constexpr std::array<std::string_view,8> modelNames{"cushion","bowl","scratching_post","sunny_perch","hammock","playhouse_frame","low_picnic_table","picnic_mat"};
const Spot nookSpot{"mabel_nook_spot",Presentation::nookPosition(),3.f,1.5f,2};
const Spot lookoutSpot{"pip_lookout_spot",Presentation::lookoutPosition(),2.5f,1.4f,1};
const Spot* spot(std::string_view id){if(id==lookoutSpot.id)return &lookoutSpot;if(id==nookSpot.id)return &nookSpot;for(auto& s:authored)if(s.id==id)return &s;return nullptr;}
const ItemView* item(const ProgressView& v,std::string_view id){for(auto& i:v.items)if(i.id==id)return &i;return nullptr;}
bool has(const engine::Mesh& mesh,std::string_view name){return mesh.skeleton&&std::any_of(mesh.skeleton->clips.begin(),mesh.skeleton->clips.end(),[&](auto& c){return c.name==name;});}
double randomSeconds(CatBinding& c,double low,double high){
 c.randomState=c.randomState*1664525u+1013904223u;
 return low+(high-low)*double(c.randomState>>8)/16777216.;
}
float distance(engine::Vec3 a,engine::Vec3 b){auto d=b-a;return std::sqrt(engine::dot(d,d));}
engine::Vec3 localToWorld(const Spot& s,engine::Vec3 local){return s.position+local;}
engine::Actor& prop(engine::Scene& s,const Assets& assets,std::string_view name,std::string id,engine::Vec3 at={}) {
 auto& a=s.spawn(std::string(name));a.setSavedId(std::move(id));a.transform.position=at;
 a.addComponent<engine::MeshComponent>(std::string(name),assets.props.at(std::string(name))).color={1,1,1};return a;
}

}
bool ambient(Phase p){return p>=Phase::Wander&&p<=Phase::WanderHome;}
std::span<const Spot> spots(){return authored;}
engine::Vec3 home(std::size_t c){return {-1.8f+1.2f*float(c),0,-3.6f};}
CatModel legacyPip(std::shared_ptr<const engine::Mesh> mesh){
 validatePip(*mesh);CatModel out;out.mesh=std::move(mesh);
 out.contacts["garden_cushion"]={{0,0,.85f},{0,.16f,0},engine::pi,"settle_cushion","curled_rest","settle_cushion",2.4,2.4,0,{},true};
 out.contacts["drinking_bowl"]={{0,0,-1.1f},gardenAnchor("bowl","drink_root"),0,"approach_bowl","drink","return_idle",1.8,1.8,0,{},true};
 return out;
}
struct Builder {
 static Built make(const Assets& assets,const ProgressView& view,bool reduced){
  for(auto name:{"paving","fence","edge","bench","planting","pot_round","cushion","bowl","bowl_water","scratching_post","sunny_perch","hammock","playhouse_frame","playhouse_door","low_picnic_table","picnic_mat"})
   if(!assets.props.contains(name)||!assets.props.at(name))throw std::invalid_argument(std::string("Missing garden model: ")+name);
  for(auto& c:assets.cats)if(c.mesh){validatePip(*c.mesh);
   if(assets.cats[0].mesh)cats::validateCanonical(*c.mesh,*assets.cats[0].mesh);
   for(auto& [_,contact]:c.contacts)if(contact.verified){
    for(auto at:{contact.approach,contact.root})if(!std::isfinite(at.x)||!std::isfinite(at.y)||!std::isfinite(at.z))throw std::invalid_argument("Nonfinite contact root");
    if(!std::isfinite(contact.yaw)||!std::isfinite(contact.enterSeconds)||!std::isfinite(contact.leaveSeconds)||!std::isfinite(contact.still)||contact.enterSeconds<0||contact.leaveSeconds<0||contact.still<0)throw std::invalid_argument("Invalid contact timing");
    if(contact.loop.empty())throw std::invalid_argument("Missing contact use action");
    for(auto& name:{contact.enter,contact.loop,contact.leave})if(!name.empty()&&!has(*c.mesh,name))throw std::invalid_argument("Missing verified contact action: "+name);
    for(auto at:contact.supportedRoots)if(!std::isfinite(at.x)||!std::isfinite(at.y)||!std::isfinite(at.z))throw std::invalid_argument("Invalid supported contact path");
    if(!contact.supportedRoots.empty()&&distance(contact.supportedRoots.back(),contact.root)>.001f)throw std::invalid_argument("Supported path does not reach its measured root");
   }
  }
  Built result;result.scene.scene=std::make_unique<engine::Scene>("The release-one garden");
  auto& s=*result.scene.scene;auto& b=result.presentation;b.assets_=assets;
  if(auto found=b.assets_.cats[0].contacts.find("garden_cushion");found!=b.assets_.cats[0].contacts.end()){
   auto contact=found->second;contact.root.y+=.48f;
   contact.approach.z=1.95f;contact.supportedRoots={{0,0,1.95f},{0,.16f,1.f},{0,.32f,.72f},contact.root};
   b.assets_.cats[0].contacts["pip_lookout"]=std::move(contact);
  }
  auto& nook=s.spawn("Mabel's sunny nook");nook.setSavedId("reward.mabel_nook");nook.enabled=false;b.nook_=nook.id();
  for(std::size_t c=0;c<2;++c){const float x=c==0?-.7f:.7f;
   auto& seat=nook.addChild<engine::Actor>(c?"Mabel's sunny cushion":"Pip's sunny cushion");seat.transform.position=nookSpot.position+engine::Vec3{x,0,0};seat.addComponent<engine::MeshComponent>("Cushion",assets.props.at("cushion")).color={1,1,1};seat.addComponent<engine::InteractionComponent>("Share the sunshine");if(c==0)b.nookSeat_=seat.id();
   if(auto found=b.assets_.cats[c].contacts.find("garden_cushion");found!=b.assets_.cats[c].contacts.end()){auto contact=found->second;contact.approach.x+=x;contact.root.x+=x;b.assets_.cats[c].contacts["mabel_nook"]=contact;}
  }
  auto& bowl=nook.addChild<engine::Actor>("Mabel's cool drink");bowl.transform.position=nookSpot.position+engine::Vec3{1.65f,0,0};bowl.addComponent<engine::MeshComponent>("Bowl",assets.props.at("bowl")).color={1,1,1};auto& water=bowl.addComponent<engine::MeshComponent>("Cool water",assets.props.at("bowl_water"));water.transform.position=gardenAnchor("bowl","water");dressWater(water);
  auto& lookout=s.spawn("Pip's shared lookout");lookout.setSavedId("reward.pip_lookout");lookout.enabled=false;b.lookout_=lookout.id();
  auto part=[&](const char* mesh,engine::Vec3 at,engine::Vec3 scale){auto& a=lookout.addChild<engine::Actor>(mesh);a.transform.position=at;a.transform.scale=scale;a.addComponent<engine::MeshComponent>(mesh,assets.props.at(mesh)).color={1,1,1};return a.id();};
  const auto centre=lookoutSpot.position+engine::Vec3{-.6f,0,0};
  part("platform",centre+engine::Vec3{0,.32f,0},{2.5f,1,1.65f});
  for(float x:{-1.f,1.f})part("platform",centre+engine::Vec3{x,0,0},{.22f,2,1.4f});
  part("platform",lookoutSpot.position+engine::Vec3{0,0,1.f},{.8f,1,.65f});
  part("platform",lookoutSpot.position+engine::Vec3{0,.16f,.72f},{.8f,1,.55f});
  part("cushion",lookoutSpot.position+engine::Vec3{0,.48f,0},{1,1,1});
  b.lookoutSeat_=part("cushion",lookoutSpot.position+engine::Vec3{-1.2f,.48f,0},{1,1,1});
  s.findActor(b.lookoutSeat_)->addComponent<engine::InteractionComponent>("Sit with Pip");
  part("ball",centre+engine::Vec3{0,0,.15f},{1,1,1});
  static std::atomic<std::uint64_t> next{1};b.scene_=next++;s.ambient={.33f,.37f,.40f};s.environmentIntensity=.65f;
  // Each chapter has a continuously paved, enclosed footprint. Unlocking a wing
  // replaces the boundary instead of leaving a fence across the new paths.
  for(int region=0;region<3;++region){auto& area=s.spawn("Garden area");area.setSavedId("garden.area."+std::to_string(region));b.areas_[region]=area.id();
   for(int x=-6;x<6;++x)for(int z=-4;z<6;++z){int r=(x>=-5&&x<3&&z<2)?0:z<2?1:2;if(r!=region)continue;
    auto& a=area.addChild<engine::Actor>("Warm sandstone");a.transform.position={x+.5f,0,z+.5f};
    auto& paving=a.addComponent<engine::MeshComponent>("Paving",assets.props.at("paving"));paving.color={1,1,1};
    // Ground-level slabs receive the cats' and props' shadows. Their buried
    // underside contributes no useful shadow to the above-ground garden.
    paving.castsShadow=false;}}
  for(int chapter=1;chapter<=3;++chapter){
   auto& boundary=s.spawn("Chapter garden enclosure");b.enclosures_[chapter-1]=boundary.id();
   auto gate=addGardenEnclosure(s,assets.props,chapter==1?-5.f:-6.f,chapter==1?3.f:6.f,-4,chapter<3?2.f:6.f,&boundary);
   // Home is welcoming; the puzzle's switch-controlled gate remains separate.
   s.findActor(gate)->transform.rotation.y+=100*engine::pi/180;
  }
  auto& bench=prop(s,assets,"bench","garden.bench",{1.35f,0,-4.35f});
  b.bench_=bench.id();
  auto& benchCushion=prop(s,assets,"bench_cushion","garden.bench.cushion",{1.35f,.49f,-4.32f});
  b.benchCushion_=benchCushion.id();
  // A decorative sleeping guest uses the accepted Button rig and measured
  // cushion contact without changing friendship or furnishing ownership.
  if(assets.cats[2].mesh && assets.cats[2].naturalCurl){
   const auto& contact=assets.cats[2].contacts.at("garden_cushion");
   const engine::Vec3 nook{-3.2f,0,-3.15f};
   auto& nap=prop(s,assets,"cushion","garden.nap.cushion",nook);
   nap.transform.rotation.y=-1.2f;
   auto& sleeper=nap.addChild<engine::Actor>("Sleeping garden cat");sleeper.setSavedId("garden.nap.cat");
   sleeper.transform.position=contact.root;sleeper.transform.rotation.y=contact.yaw;
   auto& cat=sleeper.addComponent<engine::SkeletalMeshComponent>("Contented curl",assets.cats[2].mesh);cat.color={1,1,1};
   b.napActor_=sleeper.id();b.napMesh_=cat.id();
   cat.animation().play(pipClip(*assets.cats[2].mesh,"curled_rest"),true);
   cat.animation().seek(contact.still);
  }
  for(std::size_t i=0;i<8;++i)for(bool preview:{false,true}){
   auto& a=prop(s,assets,modelNames[i],std::string(preview?"preview.":"owned.")+std::string(p::items[i].id));a.enabled=false;
   if(i==1){auto& water=a.addComponent<engine::MeshComponent>("Cool drinking water",assets.props.at("bowl_water"));water.transform.position=gardenAnchor("bowl","water");dressWater(water);}
   if(i==5){auto& door=a.addChild<engine::Actor>("Open playhouse door");door.transform.position={-.36f,.07f,.73f};door.transform.rotation.y=100*engine::pi/180;door.addComponent<engine::MeshComponent>("Door",assets.props.at("playhouse_door")).color={1,1,1};}
   (preview?b.items_[i].preview:b.items_[i].actor)=a.id();
  }
  for(std::size_t i=0;i<2;++i){auto& a=prop(s,assets,i?"picnic_mat":"low_picnic_table","supplied.celebration."+std::to_string(i),authored[i?8:7].position);a.enabled=false;b.supplied_[i]=a.id();}
  for(std::size_t i=0;i<4;++i){if(!assets.cats[i].mesh)continue;
   auto& a=s.spawn(std::string(p::catNames[i]));a.setSavedId("cat."+std::string(p::catIds[i]));a.transform.position=home(i);
   auto& c=a.addComponent<engine::SkeletalMeshComponent>("Independent cat playback",assets.cats[i].mesh);c.color={1,1,1};
   b.cats_[i].actor=a.id();b.cats_[i].mesh=c.id();b.cats_[i].randomState=std::uint32_t(b.scene_*7919+i*104729);b.cats_[i].quiet=randomSeconds(b.cats_[i],8,18)+i*4;
   a.addComponent<engine::InteractionComponent>("Tap cat for a request");
   if(assets.props.contains("quest_marker")){
    auto& marker=prop(s,assets,"quest_marker","quest.marker."+std::string(p::catIds[i]));
    marker.enabled=false;b.cats_[i].questMarker=marker.id();
   }
   b.play(s,i,"relaxed_idle");
  }
  auto& sun=s.spawn("Late afternoon sun").addComponent<engine::LightComponent>("Sun",engine::LightType::Directional);
  sun.direction={-.5f,-1,-.6f};sun.color={1,.95f,.87f};sun.intensity=1.7f;sun.shadow.enabled=true;sun.shadow.halfExtent=8;sun.shadow.halfDepth=14;sun.shadow.normalBias=.01f;
  auto& camera=s.spawn("Garden camera").addComponent<engine::CameraComponent>("Camera");camera.setPerspective(engine::pi*35/180,.05f,80);result.scene.camera=&camera;
  b.collision_.build(s,assets.props);b.project(s,view,reduced);b.collision_.synchronize(s);return result;
 }
};
Built build(const Assets& a,const ProgressView& v,bool reduced){return Builder::make(a,v,reduced);}
void Presentation::play(engine::Scene& s,std::size_t i,std::string_view name,bool still,double time){
 auto* c=dynamic_cast<engine::SkeletalMeshComponent*>(s.findComponent(cats_[i].mesh));if(!c)return;
 const auto index=pipClip(*c->mesh,name);auto& player=c->animation();
 const bool loop=name=="relaxed_idle"||name=="garden_walk"||name=="curled_rest"||name=="drink"||name=="scratch"||name=="playhouse_lookout";
 if(still||reduced_){player.play(index,loop);player.seek(time);player.pause();}else player.crossFade(index,.18,loop);
}
void Presentation::idle(engine::Scene& s,std::size_t i){auto& c=cats_[i];
 if(auto* marker=s.findActor(c.questMarker))marker->enabled=projected_&&!requests(i).empty();
 if(auto* a=s.findActor(c.actor)){a->transform.position=home(i);a->transform.rotation={};if(auto* t=a->getComponent<CatTurn>())t->weight=0;}c.phase=Phase::Idle;c.path.clear();c.item.clear();c.spot.clear();c.elapsed=0;c.ticket={};c.outings=0;c.quiet=randomSeconds(c,12,30);play(s,i,"relaxed_idle");}
void Presentation::cancelAll(engine::Scene& s){sunnyPinned_=false;sunnyRemaining_=0;sunnyNext_=45;if(social_)yieldAmbient(s);for(std::size_t i=0;i<4;++i)if(i!=newcomer_&&i!=greeter_&&cats_[i].phase!=Phase::Arrival&&cats_[i].phase!=Phase::ArrivalQueued)idle(s,i);groupItem_.clear();++serial_;}
bool Presentation::cancel(engine::Scene& s,const Ticket& t){if(t.scene!=scene_||t.cat>=4||t.serial!=cats_[t.cat].ticket.serial||!t.serial||!s.findActor(cats_[t.cat].actor))return false;
 // Returning a cancelled cat home can invalidate another cat's in-flight route.
 if(laneBusy(t.cat))cancelAll(s);else {idle(s,t.cat);groupItem_.clear();}return true;}
void Presentation::project(engine::Scene& s,const ProgressView& v,bool reduced){
 if(!s.findActor(items_[0].actor))return;
 if(projected_&&(v.generation<stamp_.generation||(v.generation==stamp_.generation&&v.revision<stamp_.revision)))return;
 if(projected_&&(v.generation!=stamp_.generation||reduced!=reduced_)){cancelAll(s);celebrating_=false;}
 if(auto* a=s.findActor(bench_))a->transform.position.x=v.chapter==1?1.35f:2.35f;
 if(auto* a=s.findActor(benchCushion_))a->transform.position.x=v.chapter==1?1.35f:2.35f;
 if(reduced&&newcomer_<4){const auto n=newcomer_,g=greeter_;newcomer_=greeter_=4;idle(s,g);idle(s,n);if(!social_)cats_[n].arrivalComplete=true;social_=false;}
 reduced_=reduced;stamp_={v.generation,v.revision};view_=v;projected_=true;
 if(auto* a=s.findActor(lookout_))a->enabled=lookoutReady();
 if(auto* a=s.findActor(nook_))a->enabled=nookReady();
 const int visibleChapter=nookReady()?3:v.chapter;
 if(reduced_){
  napPhase_=NapPhase::Sleeping;napElapsed_=0;
  if(auto* a=s.findActor(napActor_))a->transform.position=assets_.cats[2].contacts.at("garden_cushion").root;
  if(auto* c=dynamic_cast<engine::SkeletalMeshComponent*>(s.findComponent(napMesh_))){
   c->animation().play(pipClip(*c->mesh,"curled_rest"),true);c->animation().seek(assets_.cats[2].contacts.at("garden_cushion").still);c->animation().pause();
  }
 }else if(napPhase_==NapPhase::Sleeping||napPhase_==NapPhase::Awake){
  if(auto* c=dynamic_cast<engine::SkeletalMeshComponent*>(s.findComponent(napMesh_)))c->animation().resume();
 }
 for(std::size_t i=0;i<3;++i){
  if(auto* a=s.findActor(areas_[i]))a->enabled=int(i)+1<=visibleChapter;
  if(auto* a=s.findActor(enclosures_[i]))a->enabled=int(i)+1==std::clamp(visibleChapter,1,3);
 }
 for(std::size_t i=0;i<8;++i){auto& b=items_[i];const auto* row=item(v,p::items[i].id);const auto* at=row&&row->owned?spot(row->spot):nullptr;
  const std::string now=at?std::string(at->id):"";
  if(now!=b.spot&&std::any_of(cats_.begin(),cats_.end(),[](auto& cat){return cat.phase!=Phase::Idle;}))cancelAll(s);
  b.spot=now;if(auto* a=s.findActor(b.actor)){a->enabled=at;if(at)a->transform.position=at->position;}
  if(auto* a=s.findActor(b.preview))a->enabled=false;
 }
 for(std::size_t i=0;i<2;++i)if(auto* a=s.findActor(supplied_[i]))a->enabled=celebrating_&&items_[i?7:6].spot.empty();
 for(std::size_t i=0;i<4;++i){auto& c=cats_[i];bool introduced=false;for(auto& row:v.cats)if(row.id==p::catIds[i])introduced=row.introduced;
  const bool eligible=bool(assets_.cats[i].mesh)&&v.chapter>=(i==0?1:i==3?3:2);
  if(!eligible){idle(s,i);c.arrivalComplete=false;}
  else if(introduced){if(!c.eligible)idle(s,i);c.arrivalComplete=true;}
  else if(!c.eligible){c.phase=Phase::ArrivalQueued;c.arrivalComplete=false;}
  c.eligible=eligible;c.introduced=introduced;
  if(reduced_&&eligible&&!introduced){idle(s,i);c.arrivalComplete=true;}
  const bool visible=eligible&&c.phase!=Phase::ArrivalQueued;
  if(auto* a=s.findActor(c.actor))a->enabled=visible;
  if(auto* marker=s.findActor(c.questMarker)){
   marker->enabled=visible&&c.item!="pip_lookout"&&c.item!="mabel_nook"&&!requests(i).empty();
   if(auto* a=s.findActor(c.actor))marker->transform.position=a->transform.position+engine::Vec3{0,1.05f,0};
   if(reduced_)marker->transform.rotation.y=0;
  }
  if(c.phase==Phase::Idle)play(s,i,"relaxed_idle",reduced_);
 }
}
bool Presentation::nookReady()const{return std::any_of(view_.quests.begin(),view_.quests.end(),[](const auto& q){return q.id=="mabel_friend_share"&&q.rewarded;});}
bool Presentation::lookoutReady()const{return std::any_of(view_.quests.begin(),view_.quests.end(),[](const auto& q){return q.id=="pip_friend_room"&&q.rewarded;});}
std::string Presentation::placement(std::string_view id)const{
 if(id=="mabel_nook"&&nookReady())return std::string(nookSpot.id);
 if(id=="pip_lookout"&&lookoutReady())return std::string(lookoutSpot.id);
 const auto* row=item(view_,id);if(row&&row->owned&&spot(row->spot))return row->spot;
 if(celebrating_&&id=="low_picnic_table")return "picnic_table_spot";
 if(celebrating_&&id=="picnic_mat")return "picnic_mat_spot";return {};
}
bool Presentation::celebrationProps(engine::Scene& s,Stamp expected,bool enabled){
 if(!s.findActor(items_[0].actor)||expected!=stamp_||(enabled&&!view_.picnicEligible&&!view_.picnicEndingSeen))return false;
 if(!enabled)cancelAll(s);celebrating_=enabled;
 for(std::size_t i=0;i<2;++i)if(auto* a=s.findActor(supplied_[i]))a->enabled=enabled&&items_[i?7:6].spot.empty();return true;
}
std::vector<QuestView> Presentation::requests(std::size_t cat)const{
 std::vector<QuestView> out;if(cat>=4||!cats_[cat].eligible||cats_[cat].phase==Phase::ArrivalQueued)return out;
 for(auto& q:view_.quests){auto n=p::questIndex(q.id);if(n<p::quests.size()&&p::quests[n].pack.empty()&&(p::quests[n].requesters&(1u<<cat))&&q.unlocked&&!q.completed)out.push_back(q);}return out;
}
std::string Presentation::useBlockage(std::string_view id,std::size_t cat)const{
 if(cat>=4)return "Choose a known cat.";if(!cats_[cat].introduced)return "This cat is still on its way into the garden.";
 if(!assets_.cats[cat].mesh)return std::string(p::catNames[cat])+"'s accepted runtime model has not arrived.";
 const auto target=placement(id);if(target.empty())return "Place an owned furnishing first.";
 const auto it=assets_.cats[cat].contacts.find(id);if(it==assets_.cats[cat].contacts.end()||!it->second.verified)return "Measured cat/contact sequence is pending the cat handoff.";
 for(auto& name:{it->second.enter,it->second.loop,it->second.leave})if(!name.empty()&&!has(*assets_.cats[cat].mesh,name))return "Required animation is missing: "+name;
 if(cats_[cat].phase!=Phase::Idle&&!ambient(cats_[cat].phase))return "This cat is already using or approaching a spot.";
 for(std::size_t c=0;c<4;++c)if(c!=cat&&cats_[c].item==id){
  if(id!="low_picnic_table"&&id!="picnic_mat"&&id!="mabel_nook")return "Another cat has reserved this furnishing.";
  if(distance(cats_[c].contact.root,it->second.root)<1.2f)return "Another cat has reserved this contact space.";
 }
 return {};
}
bool Presentation::laneBusy(std::size_t cat)const{if(newcomer_<4)return true;for(std::size_t c=0;c<4;++c)if(c!=cat&&(cats_[c].phase==Phase::Arrival||cats_[c].phase==Phase::Approach||cats_[c].phase==Phase::Return||cats_[c].phase==Phase::Enter||cats_[c].phase==Phase::Leave||cats_[c].phase==Phase::Attention||cats_[c].phase==Phase::Wander||cats_[c].phase==Phase::WanderHome))return true;return false;}
std::vector<engine::Vec3> Presentation::pathTo(engine::Scene& scene,std::size_t cat,engine::Vec3 target,std::string_view own,bool fromCurrent)const{
 // Bounded authored quarter-metre courtyard lane grid. Occupied furniture,
 // sleeping cats, bench and door sweep are clearance obstacles; no physics or
 // puzzle legality is inferred. Search never enters the puzzle's central apron.
 constexpr int W=49,H=45,N=W*H;constexpr float step=.25f;
 auto position=[](int n){return engine::Vec3{-6.f+(n%W)*step,0,-4.f+(n/W)*step};};
 auto index=[](engine::Vec3 at){return std::clamp(int(std::lround((at.x+6)/step)),0,W-1)+W*std::clamp(int(std::lround((at.z+4)/step)),0,H-1);};
 const auto* actor=scene.findActor(cats_[cat].actor);
 const auto origin=fromCurrent&&actor?engine::Vec3{actor->transform.position.x,0,actor->transform.position.z}:home(cat);
 const int start=index(origin),goal=index(target);
 auto blocked=[&](engine::Vec3 at){
  if(!collision_.clear(scene,at))return true;
  if(at.x<(view_.chapter==1?-4.65f:-5.65f)||at.x>(view_.chapter==1?2.65f:5.65f)||at.z<-3.85f||at.z>(view_.chapter<3&&!nookReady()?1.65f:5.9f))return true;
  if(std::abs(at.x-(view_.chapter==1?1.35f:2.35f))<1.1f&&std::abs(at.z+4.35f)<.8f)return true;
  if(assets_.cats[2].mesh&&assets_.cats[2].naturalCurl&&std::abs(at.x+3.2f)<.85f&&std::abs(at.z+3.15f)<.95f)return true;
  if(std::abs(at.x)<.8f&&at.z>0&&at.z<1.95f)return true;
  for(std::size_t i=0;i<8;++i){if(p::items[i].id==own)continue;auto* a=spot(placement(p::items[i].id));if(!a)continue;
   float extra=p::items[i].id=="playhouse"?.5f:0;
   if(std::abs(at.x-a->position.x)<p::items[i].width/2+.58f+extra&&std::abs(at.z-a->position.z)<p::items[i].depth/2+.58f+extra)return true;}
  for(std::size_t c=0;c<4;++c)if(c!=cat&&cats_[c].introduced&&assets_.cats[c].mesh){
   const auto& b=cats_[c];engine::Vec3 other=home(c);
   if((b.phase==Phase::Use||b.phase==Phase::Enter||b.phase==Phase::Leave)&&spot(b.spot))other=localToWorld(*spot(b.spot),b.contact.root);
   if(ambient(b.phase))if(auto* a=scene.findActor(b.actor))other={a->transform.position.x,0,a->transform.position.z};
   // The shared nook uses the narrow lane beside the lookout; keep a
   // measured root gap while mesh collision still checks every segment.
   if(distance(at,other)<(own=="mabel_nook"?.75f:.95f)||distance(at,home(c))<(own=="mabel_nook"?.65f:.78f))return true;}
  return false;
 };
 std::array<int,N> previous;previous.fill(-1);std::queue<int> queue;queue.push(start);previous[start]=start;
 while(!queue.empty()&&previous[goal]<0){int at=queue.front();queue.pop();for(auto [dx,dz]:{std::pair{1,0},std::pair{-1,0},std::pair{0,1},std::pair{0,-1}}){int x=at%W+dx,z=at/W+dz;if(x<0||x>=W||z<0||z>=H)continue;int next=x+W*z;if(previous[next]>=0||blocked(position(next)))continue;previous[next]=at;queue.push(next);}}
 if(previous[goal]<0)return {};
 std::vector<engine::Vec3> path;for(int n=goal;n!=start;n=previous[n])path.push_back(position(n));path.push_back(origin);std::reverse(path.begin(),path.end());
 // Keep corner samples; each segment stays on an inspected clear grid lane.
 std::vector<engine::Vec3> corners{path.front()};for(std::size_t i=1;i+1<path.size();++i){auto a=path[i]-path[i-1],b=path[i+1]-path[i];if(a.x!=b.x||a.z!=b.z)corners.push_back(path[i]);}corners.push_back(path.back());corners.push_back(target);
 std::vector<engine::Vec3> checked{corners.front()};
 for(std::size_t i=1;i<corners.size();++i){auto leg=collision_.route(scene,checked.back(),corners[i]);if(leg.empty())return {};checked.insert(checked.end(),leg.begin()+1,leg.end());}return checked;
}
std::optional<Ticket> Presentation::use(engine::Scene& s,std::string_view id,std::size_t cat,Stamp expected,std::string& reason){
 if(expected!=stamp_){reason="That use belongs to an earlier garden view.";return {};}
 if(cat>=4||!s.findActor(cats_[cat].actor)||!s.findComponent(cats_[cat].mesh)){reason="That cat's scene is no longer active.";return {};}
 yieldAmbient(s);
 if(cat<4&&(cats_[cat].phase==Phase::Attention||cats_[cat].phase==Phase::Reaction))idle(s,cat);
 reason=useBlockage(id,cat);if(!reason.empty())return {};
 auto& c=cats_[cat];c.contact=assets_.cats[cat].contacts.at(std::string(id));c.item=id;c.spot=placement(id);
 collision_.synchronize(s);
 auto approach=localToWorld(*spot(c.spot),c.contact.approach);c.path=pathTo(s,cat,approach,id);
 if(c.path.empty()){idle(s,cat);reason="The supported approach lane is occupied.";return {};}
 std::vector<engine::Vec3> entry{approach};for(auto root:c.contact.supportedRoots)entry.push_back(localToWorld(*spot(c.spot),root));entry.push_back(localToWorld(*spot(c.spot),c.contact.root));
 c.entryPath=collision_.supportedRoute(s,entry);c.entrySegment=0;
 if(c.entryPath.empty()){idle(s,cat);reason="The furnishing entry is obstructed.";return {};}
 if(id=="pip_lookout"||id=="mabel_nook")if(auto* marker=s.findActor(c.questMarker))marker->enabled=false;
 c.ticket={stamp_,scene_,++serial_,cat};c.segment=0;c.elapsed=0;c.phase=Phase::Queued;return c.ticket;
}
bool Presentation::sunnyVisit(engine::Scene& s,Stamp expected,std::string& reason,bool pinned){
 if(expected!=stamp_||!nookReady()){reason="Mabel's sunny nook is not ready yet.";return false;}
 for(std::size_t c=0;c<2;++c)if(!(reason=useBlockage("mabel_nook",c)).empty())return false;
 auto first=use(s,"mabel_nook",0,expected,reason);if(!first){reason="Pip: "+reason;return false;}
 auto second=use(s,"mabel_nook",1,expected,reason);if(!second){cancel(s,*first);reason="Mabel: "+reason;return false;}
 sunnyPinned_=pinned;sunnyRemaining_=20;sunnyNext_=60;return true;
}
std::vector<Ticket> Presentation::gather(engine::Scene& s,std::string_view id,Stamp expected,std::string& reason,bool stagedArrival){
 if(id!="low_picnic_table"&&id!="picnic_mat"){reason="Choose a picnic furnishing.";return {};}
 if(expected!=stamp_){reason="That gathering belongs to an earlier view.";return {};}
 yieldAmbient(s);
 for(std::size_t c=0;c<4;++c)if(!(reason=useBlockage(id,c)).empty())return {};
 const auto previous=cats_;std::vector<Ticket> tickets;
 for(std::size_t c=0;c<4;++c){auto ticket=use(s,id,c,expected,reason);if(!ticket){cats_=previous;return {};}tickets.push_back(*ticket);}
 groupItem_=id;stagedArrival_=stagedArrival;
 // A newly installed finale scene opens with friends at the picnic apron.
 // Ordinary furnishing use still walks from home at the canonical stride.
 if(stagedArrival_)for(auto& c:cats_)if(auto* a=s.findActor(c.actor))a->transform.position=localToWorld(*spot(c.spot),c.contact.approach);
 return tickets;
}
bool Presentation::move(engine::Scene& s,std::size_t cat,double dt){auto& c=cats_[cat];auto* a=s.findActor(c.actor);if(!a||c.path.size()<2)return true;
 const bool travelling=c.segment+1<c.path.size();
 const bool reached=collision_.follow(s,*a,c.path,c.segment,float(dt)*pipWalkSpeed);
 if(auto* skin=a->getComponent<engine::SkeletalMeshComponent>())sampleCatWalk(*a,*skin,skin->animation().time(),false);
 // Arrival facing starts on the following frame: never spend a frame's yaw
 // budget once following the route and again turning toward the furnishing.
 return reached&&!travelling;
}
bool Presentation::startSocial(engine::Scene& s){
 if(newcomer_<4||laneBusy(4)||std::any_of(cats_.begin(),cats_.end(),[](const auto& c){return !c.item.empty()||(c.eligible&&!c.introduced);}))return false;
 auto available=[](const CatBinding& c){return c.eligible&&c.introduced&&c.phase==Phase::Idle;};
 for(std::size_t g=0;g<4;++g)for(std::size_t n=g+1;n<4;++n){
  if(!available(cats_[g])||!available(cats_[n]))continue;
  const auto origin=s.findActor(cats_[n].actor)->transform.position;
  const auto visitor=s.findActor(cats_[g].actor)->transform.position;
  const auto destination=origin+engine::Vec3{0,0,float(randomSeconds(cats_[g],1.8,3.0))};
  // A loop keeps the pair travelling in the same direction at the far end.
  // Approach home from the right so the leading resident can continue left.
  const engine::Vec3 side{1.6f,0,0};
  std::vector<engine::Vec3> route{origin};bool connected=true;
  for(auto point:{destination,destination+side,home(n)+side,home(n)}){
   auto leg=collision_.route(s,route.back(),point);if(leg.empty()){connected=false;break;}
   route.insert(route.end(),leg.begin()+1,leg.end());
  }
  if(!connected||route.size()<2)continue;
  float remaining=1.6f;std::size_t segment=0;engine::Vec3 meet;
  for(;segment+1<route.size();++segment){const float length=distance(route[segment],route[segment+1]);
   if(length>=remaining){meet=route[segment]+(route[segment+1]-route[segment])*(remaining/length);break;}remaining-=length;}
  if(segment+1>=route.size())continue;
  auto approach=collision_.route(s,visitor,meet),tail=collision_.route(s,home(n),home(g));
  if(approach.empty()||tail.empty())continue;
  auto clearCats=[&](const std::vector<engine::Vec3>& path){
   for(std::size_t leg=1;leg<path.size();++leg)for(int step=0;step<=30;++step){auto at=path[leg-1]+(path[leg]-path[leg-1])*(step/30.f);
    for(std::size_t other=0;other<4;++other)if(other!=n&&other!=g&&cats_[other].eligible){auto* a=s.findActor(cats_[other].actor);if(a&&distance(at,a->transform.position)<.95f)return false;}}
   return true;
  };
  if(!clearCats(route)||!clearCats(approach)||!clearCats(tail))continue;
  newcomer_=n;greeter_=g;social_=true;welcomeStage_=0;welcomeReady_={};welcomeTime_=0;welcomeRoute_=route;
  cats_[n].path={origin};cats_[n].segment=0;cats_[n].phase=Phase::WelcomeWalk;play(s,n,"relaxed_idle");
  cats_[g].path=std::move(approach);cats_[g].segment=0;cats_[g].phase=Phase::WelcomeWalk;play(s,g,"garden_walk");
  cats_[g].entryPath=route;cats_[g].entryPath.insert(cats_[g].entryPath.end(),tail.begin()+1,tail.end());cats_[g].entrySegment=segment;
  return true;
 }
 socialQuiet_=5;return false;
}
bool Presentation::startWelcome(engine::Scene& s,std::size_t n,engine::Vec3 outside,engine::Vec3 inside){
 if(reduced_||celebrating_||n==0)return false;
 std::size_t g=4;
 for(std::size_t i=0;i<n;++i)if(cats_[i].eligible&&(cats_[i].introduced||cats_[i].arrivalComplete)&&cats_[i].phase==Phase::Idle)g=i;
 if(g==4)return false;
 auto route=collision_.route(s,inside,home(n));if(route.size()<2)return false;
 // Match resident greetings: leave room for both heads and the greeting lean.
 float remaining=1.6f;std::size_t segment=0;auto meet=inside;
 for(;segment+1<route.size();++segment){const float length=distance(route[segment],route[segment+1]);
  if(length>=remaining){meet=route[segment]+(route[segment+1]-route[segment])*(remaining/length);break;}remaining-=length;}
 if(segment+1>=route.size())return false;
 auto* resident=s.findActor(cats_[g].actor);
 auto outbound=collision_.route(s,resident->transform.position,meet);
 auto tail=collision_.route(s,home(n),home(g));
 if(outbound.empty()||tail.empty())return false;
 social_=false;newcomer_=n;greeter_=g;welcomeStage_=0;welcomeReady_={};welcomeTime_=0;welcomeRoute_=route;
 auto& fresh=cats_[n];fresh.path={outside,inside};fresh.segment=0;fresh.phase=Phase::Arrival;
 auto* arrival=s.findActor(fresh.actor);arrival->enabled=true;arrival->transform.position=outside;arrival->transform.rotation.y=-engine::pi/2;
 fresh.arrivalComplete=false;play(s,n,"garden_walk");
 if(auto* marker=s.findActor(fresh.questMarker))marker->enabled=!requests(n).empty();
 auto& host=cats_[g];host.path=std::move(outbound);host.segment=0;host.phase=Phase::WelcomeWalk;play(s,g,"garden_walk");
 // Retain the resident's onward path separately from its outward trip.
 host.entryPath=route;host.entryPath.insert(host.entryPath.end(),tail.begin()+1,tail.end());host.entrySegment=segment;
 return true;
}
void Presentation::updateWelcome(engine::Scene& s,double dt){
 if(newcomer_>=4)return;
 const auto n=newcomer_,g=greeter_;auto* fresh=s.findActor(cats_[n].actor);auto* host=s.findActor(cats_[g].actor);
 if(!fresh||!host){newcomer_=greeter_=4;social_=false;return;}
 auto* ns=fresh->getComponent<engine::SkeletalMeshComponent>();auto* gs=host->getComponent<engine::SkeletalMeshComponent>();
 if(reduced_){newcomer_=greeter_=4;idle(s,g);idle(s,n);if(!social_)cats_[n].arrivalComplete=true;social_=false;return;}
 // Once a cat reaches its endpoint, leave its idle player alone while its
 // partner catches up. Resampling the walk and cross-fading back to idle on
 // every frame resets the clip and never lets the waiting pose settle.
 auto travel=[&](std::size_t cat,std::size_t slot){
  if(welcomeReady_[slot])return true;
  if(!move(s,cat,dt))return false;
  welcomeReady_[slot]=true;play(s,cat,"relaxed_idle");return true;
 };
 if(welcomeStage_==0){
  const bool a=travel(n,0),b=travel(g,1);
  if(!a||!b)return;
  welcomeStage_=1;welcomeReady_={};return;
 }
 if(welcomeStage_==1){
  const auto direction=host->transform.position-fresh->transform.position;
  auto face=[&](std::size_t cat,std::size_t slot,engine::Actor& actor,engine::SkeletalMeshComponent& skin,float yaw){
   if(welcomeReady_[slot])return true;
   if(!faceCat(actor,skin,yaw,dt))return false;
   welcomeReady_[slot]=true;play(s,cat,"relaxed_idle");return true;
  };
  const bool a=face(n,0,*fresh,*ns,std::atan2(direction.x,direction.z));
  const bool b=face(g,1,*host,*gs,std::atan2(-direction.x,-direction.z));
  if(!a||!b)return;
  welcomeStage_=2;welcomeTime_=0;cats_[g].phase=Phase::WelcomeHello;play(s,g,"attentive_request");play(s,n,"relaxed_idle");return;
 }
 if(welcomeStage_==2){
  welcomeTime_+=dt;
  const double greeting=assets_.cats[g].mesh->skeleton->clips[pipClip(*assets_.cats[g].mesh,"attentive_request")].duration;
  if(welcomeTime_>=greeting&&cats_[n].phase!=Phase::WelcomeHello){cats_[n].phase=Phase::WelcomeHello;play(s,n,"attentive_request");play(s,g,"relaxed_idle");}
  const double reply=assets_.cats[n].mesh->skeleton->clips[pipClip(*assets_.cats[n].mesh,"attentive_request")].duration;
  if(welcomeTime_<greeting+reply)return;
  welcomeStage_=3;welcomeReady_={};welcomeTime_=0;
  cats_[n].path=welcomeRoute_;cats_[n].segment=0;
  cats_[g].path=cats_[g].entryPath;cats_[g].segment=cats_[g].entrySegment;
  cats_[n].phase=cats_[g].phase=Phase::WelcomeHome;play(s,n,"garden_walk");play(s,g,"garden_walk");return;
 }
 const bool a=travel(n,0),b=travel(g,1);
 if(!a||!b)return;
 const bool nf=turnCatTowards(*fresh,0,dt),gf=turnCatTowards(*host,0,dt);
 if(!nf||!gf)return;
 newcomer_=greeter_=4;idle(s,g);idle(s,n);if(!social_)cats_[n].arrivalComplete=true;social_=false;socialQuiet_=randomSeconds(cats_[g],45,85);
}
void Presentation::yieldAmbient(engine::Scene& s){
 if(social_){const auto n=newcomer_,g=greeter_;newcomer_=greeter_=4;social_=false;socialQuiet_=45;idle(s,n);idle(s,g);}
 for(std::size_t i=0;i<4;++i)if(ambient(cats_[i].phase))idle(s,i);
}
bool Presentation::wander(engine::Scene& s,std::size_t i,bool returning){
 auto& c=cats_[i];auto* actor=s.findActor(c.actor);
 if(!actor||reduced_||celebrating_||laneBusy(i))return false;
 if(std::any_of(cats_.begin(),cats_.end(),[](const auto& cat){return !cat.item.empty();}))return false;
 for(int attempt=0;attempt<(returning?1:12);++attempt){
  auto target=home(i);
  if(!returning){
   target={float(std::round(randomSeconds(c,view_.chapter==1?-4.1:-5.1,view_.chapter==1?2.1:5.1)*4)/4),0,
           float(std::round(randomSeconds(c,-2.8,view_.chapter<3?1.1:5.1)*4)/4)};
   const float length=distance(actor->transform.position,target);
   if(length<1.1f||length>3.7f)continue;
   // Reserve enough room for the whole resting silhouette, not just the root.
   bool clear=true;for(int n=0;n<8;++n){const float angle=n*engine::pi/4;
    clear&=collision_.clear(s,target+engine::Vec3{.8f*std::sin(angle),0,.8f*std::cos(angle)});}
   if(!clear)continue;
  }
  auto route=pathTo(s,i,target,{},true);if(route.empty())continue;
  c.path=std::move(route);c.segment=0;c.elapsed=0;
  c.phase=returning?Phase::WanderHome:Phase::Wander;play(s,i,"garden_walk");return true;
 }
 return false;
}
void Presentation::updateAmbient(engine::Scene& s,std::size_t i,double dt){
 auto& c=cats_[i];auto* actor=s.findActor(c.actor);if(!actor)return;
 const auto& mesh=*assets_.cats[i].mesh;
 const float restHeight=isToonCat(mesh)?.041f:0.f;
 if(reduced_||celebrating_){idle(s,i);return;}
 if(c.phase==Phase::Wander||c.phase==Phase::WanderHome){
  if(!move(s,i,dt))return;
  if(c.phase==Phase::WanderHome){
   auto* skin=actor->getComponent<engine::SkeletalMeshComponent>();
   if(skin&&!faceCat(*actor,*skin,0,dt))return;
   idle(s,i);return;
  }
  ++c.outings;c.phase=Phase::WanderPause;c.elapsed=0;
  c.ambientHold=randomSeconds(c,5,12);play(s,i,"relaxed_idle");return;
 }
 c.elapsed+=dt;
 if(c.phase==Phase::WanderPause){
  if(c.elapsed<c.ambientHold)return;
  if(c.outings>=3||randomSeconds(c,0,1)<.25){
   if(!wander(s,i,true)){c.elapsed=0;c.ambientHold=4;}return;
  }
  if(has(mesh,"rest_leave")&&randomSeconds(c,0,1)<.6){
   c.phase=Phase::RestEnter;c.elapsed=0;play(s,i,"settle_cushion");return;
  }
  if(!wander(s,i,false)){c.elapsed=0;c.ambientHold=5;}return;
 }
 if(c.phase==Phase::Resting){
  if(c.elapsed>=c.ambientHold){c.phase=Phase::RestLeave;c.elapsed=0;play(s,i,"rest_leave");}
  return;
 }
 const bool rising=c.phase==Phase::RestLeave;
 const double duration=mesh.skeleton->clips[pipClip(mesh,rising?"rest_leave":"settle_cushion")].duration;
 const float phase=float(std::clamp(c.elapsed/duration,0.,1.));
 actor->transform.position.y=restHeight*(rising?1-phase:phase);
 if(c.elapsed<duration)return;
 c.elapsed=0;
 if(rising){actor->transform.position.y=0;c.phase=Phase::WanderPause;c.ambientHold=randomSeconds(c,3,7);play(s,i,"relaxed_idle");}
 else {c.phase=Phase::Resting;c.ambientHold=randomSeconds(c,20,45);play(s,i,"curled_rest");}
}
void Presentation::updateNap(engine::Scene& s,double dt){
 if(reduced_)return;
 auto* a=s.findActor(napActor_);
 auto* c=dynamic_cast<engine::SkeletalMeshComponent*>(s.findComponent(napMesh_));
 if(!a||!c)return;
 const auto& contact=assets_.cats[2].contacts.at("garden_cushion");
 const engine::Vec3 standing{0,contact.root.y,0};
 auto& player=c->animation();
 // Application simulation time owns the cycle; no wall-clock callbacks survive
 // a replaced scene. Vary the long holds so the nap feels unhurried.
 constexpr double sleeps[]{30,42,35},wakes[]{10,14,11};
 const bool authoredRise=contact.leave!=contact.enter&&!contact.leave.empty();
 const double transition=std::max(.01,napPhase_==NapPhase::Rising?contact.leaveSeconds:contact.enterSeconds);
 while(dt>0){
  const double duration=napPhase_==NapPhase::Sleeping?sleeps[napCycle_%3]:napPhase_==NapPhase::Awake?wakes[napCycle_%3]:transition;
  const double take=std::min(dt,std::max(0.,duration-napElapsed_));napElapsed_+=take;dt-=take;
  if(napPhase_==NapPhase::Rising||napPhase_==NapPhase::Settling){
   const double sample=napPhase_==NapPhase::Rising?transition-napElapsed_:napElapsed_;
   player.seek(napPhase_==NapPhase::Rising&&authoredRise?napElapsed_:sample);player.pause();
   const float t=float(sample/transition);
   a->transform.position=standing+(contact.root-standing)*t;
  }
  if(napElapsed_<duration)break;
  napElapsed_=0;
  switch(napPhase_){
   case NapPhase::Sleeping:
    napPhase_=NapPhase::Rising;player.play(pipClip(*c->mesh,authoredRise?contact.leave:contact.enter),false);player.seek(authoredRise?0:transition);player.pause();break;
   case NapPhase::Rising:
    napPhase_=NapPhase::Awake;a->transform.position=standing;player.crossFade(pipClip(*c->mesh,"relaxed_idle"),.2,true);break;
   case NapPhase::Awake:
    napPhase_=NapPhase::Settling;player.play(pipClip(*c->mesh,contact.enter),false);player.seek(0);player.pause();break;
   case NapPhase::Settling:
    napPhase_=NapPhase::Sleeping;++napCycle_;a->transform.position=contact.root;player.crossFade(pipClip(*c->mesh,"curled_rest"),.2,true);break;
  }
 }
}
void Presentation::update(engine::Scene& s,double dt){if(!std::isfinite(dt)||dt<=0)return;
 if(nookReady()&&!sunnyPinned_){
  if(cats_[0].item=="mabel_nook"&&cats_[1].item=="mabel_nook"){if(cats_[0].phase==Phase::Use&&cats_[1].phase==Phase::Use){sunnyRemaining_-=dt;if(sunnyRemaining_<=0)cancelAll(s);}}
  else {sunnyNext_-=dt;if(sunnyNext_<=0&&cats_[0].phase==Phase::Idle&&cats_[1].phase==Phase::Idle){std::string why;sunnyVisit(s,stamp_,why,false);sunnyNext_=60;}}
 }
 collision_.synchronize(s);
 updateNap(s,dt);
 if(social_&&std::any_of(cats_.begin(),cats_.end(),[](const auto& c){return c.eligible&&c.phase==Phase::ArrivalQueued;}))yieldAmbient(s);
 updateWelcome(s,dt);
 if(!reduced_&&!celebrating_&&newcomer_>=4){socialQuiet_-=dt;if(socialQuiet_<=0&&startSocial(s))socialQuiet_=60;}
 for(std::size_t i=0;i<4;++i)if(auto* marker=s.findActor(cats_[i].questMarker)){
  if(marker->enabled&&!reduced_)marker->transform.rotation.y=std::fmod(marker->transform.rotation.y+float(dt)*.9f,2*engine::pi);
 }
 for(std::size_t i=0;i<4;++i){auto& c=cats_[i];if(!c.eligible||!assets_.cats[i].mesh)continue;
  if(!s.findActor(c.actor)||!s.findComponent(c.mesh)){c.phase=Phase::Idle;c.item.clear();c.spot.clear();c.ticket={};continue;}
  auto* a=s.findActor(c.actor);
  if(i==newcomer_||i==greeter_)continue;
  if(c.phase==Phase::ArrivalQueued){
   yieldAmbient(s);
   if(laneBusy(i))continue;
   bool resident=false,available=false;
   for(std::size_t j=0;j<i;++j)if(cats_[j].eligible&&(cats_[j].introduced||cats_[j].arrivalComplete)){
    resident=true;available|=cats_[j].phase==Phase::Idle;
   }
   if(resident&&!available)continue; // Let the host finish its furnishing use first.
   const auto gate=gardenGatePosition(view_.chapter==1?3.f:6.f,-4,view_.chapter<3?2.f:6.f);
   const auto outside=gate+engine::Vec3{1.35f,0,0},inside=gate-engine::Vec3{1.1f,0,0};
   // Force the threshold crossing before planning the indoor leg. A detour
   // must never substitute a route around the fence for walking in the gate.
   if(!collision_.clearSegment(s,outside,inside))continue;
   if(startWelcome(s,i,outside,inside))continue;
   auto route=collision_.route(s,inside,home(i));if(route.empty())continue;
   c.path={outside,inside};c.path.insert(c.path.end(),route.begin()+1,route.end());
   c.segment=0;a->transform.position=outside;a->transform.rotation.y=-engine::pi/2;
   a->enabled=true;c.phase=Phase::Arrival;play(s,i,"garden_walk");
   if(auto* marker=s.findActor(c.questMarker))marker->enabled=!requests(i).empty();
   continue;
  }
  if(c.phase==Phase::Arrival){
   if(reduced_){idle(s,i);c.arrivalComplete=true;continue;}
   if(!move(s,i,dt))continue;
   if(auto* skin=a->getComponent<engine::SkeletalMeshComponent>();skin&&!faceCat(*a,*skin,0,dt))continue;
   idle(s,i);c.arrivalComplete=true;continue;
  }
  if(ambient(c.phase)){updateAmbient(s,i,dt);continue;}
  if(c.phase==Phase::Idle){
   if(!reduced_&&!celebrating_&&c.introduced&&(c.quiet-=dt)<=0&&!laneBusy(i)){
    c.quiet=randomSeconds(c,12,30);wander(s,i,false);
   }continue;}
  if(c.phase==Phase::Attention){
   if(reduced_){c.elapsed+=dt;if(c.elapsed>=2.4)idle(s,i);continue;}
   const double walkSeconds=.3f/pipWalkSpeed;
   if(c.segment==0){c.elapsed+=dt;a->transform.position=collision_.constrain(s,a->transform.position,home(i)+engine::Vec3{0,0,float(std::min(c.elapsed,walkSeconds))*pipWalkSpeed});
    if(c.elapsed>=walkSeconds){c.segment=1;c.elapsed=0;play(s,i,"attentive_request");}}
   else if(c.segment==1){c.elapsed+=dt;if(c.elapsed>=2.4){auto* skin=a->getComponent<engine::SkeletalMeshComponent>();if(skin&&faceCat(*a,*skin,engine::pi,dt)){c.segment=2;c.elapsed=0;}}}
   else if(c.segment==2){c.elapsed+=dt;turnCatTowards(*a,engine::pi,dt);if(auto* skin=a->getComponent<engine::SkeletalMeshComponent>())sampleCatWalk(*a,*skin,skin->animation().time(),false);a->transform.position=collision_.constrain(s,a->transform.position,home(i)+engine::Vec3{0,0,.3f-float(std::min(c.elapsed,walkSeconds))*pipWalkSpeed});if(c.elapsed>=walkSeconds)c.segment=3;}
   else if(auto* skin=a->getComponent<engine::SkeletalMeshComponent>();skin&&faceCat(*a,*skin,0,dt))idle(s,i);
   continue;
  }
  if(c.phase==Phase::Reaction){c.elapsed+=dt;
   const auto& mesh=*assets_.cats[i].mesh;
   const auto name=has(mesh,"personality_reaction")?"personality_reaction":"attentive_request";
   if(c.elapsed>=mesh.skeleton->clips[pipClip(mesh,name)].duration)idle(s,i);continue;}
  const auto* at=spot(c.spot);if(!at){idle(s,i);continue;}
  if(c.phase==Phase::Queued){if(laneBusy(i))continue;
   // Another cat may have settled while this one waited. Replan against the
   // current reservations before acquiring the shared walking lane.
   c.path=stagedArrival_&&c.item==groupItem_?std::vector<engine::Vec3>{localToWorld(*at,c.contact.approach)}:pathTo(s,i,localToWorld(*at,c.contact.approach),c.item);
   if(c.path.empty()){c.reason="The supported approach lane is occupied.";idle(s,i);continue;}
   c.segment=0;c.elapsed=0;c.entrySegment=0;
   if(reduced_){a->transform.position=localToWorld(*at,c.contact.root);a->transform.rotation.y=c.contact.yaw;c.phase=Phase::Use;play(s,i,c.contact.loop,true,c.contact.still);}else{c.phase=Phase::Approach;play(s,i,"garden_walk");}continue;}
  if(c.phase==Phase::Approach){if(!move(s,i,dt))continue;auto* skin=a->getComponent<engine::SkeletalMeshComponent>();if(skin&&!faceCat(*a,*skin,c.contact.yaw,dt))continue;c.phase=Phase::Enter;c.elapsed=0;c.entryTravel=0;a->transform.rotation.y=c.contact.yaw;play(s,i,c.contact.enter.empty()?c.contact.loop:c.contact.enter);}
  else if(c.phase==Phase::Enter||c.phase==Phase::Leave){c.elapsed+=dt;const bool leaving=c.phase==Phase::Leave;double duration=leaving?c.contact.leaveSeconds:c.contact.enterSeconds;
   float travel=0;for(std::size_t n=1;n<c.entryPath.size();++n)travel+=distance(c.entryPath[n-1],c.entryPath[n]);
   const auto previous=a->transform.position;
   const bool reached=collision_.follow(s,*a,c.entryPath,c.entrySegment,duration>0?float(dt/duration)*travel:travel);
   a->transform.rotation.y=c.contact.yaw;
   const bool crawl=c.item=="playhouse"&&isToonCat(*assets_.cats[i].mesh);
   if(crawl){
    c.entryTravel+=distance(previous,a->transform.position);
    auto* skin=a->getComponent<engine::SkeletalMeshComponent>();
    const auto clip=pipClip(*skin->mesh,"playhouse_enter");const double cycle=skin->mesh->skeleton->clips[clip].duration;
    const double phase=std::fmod(c.entryTravel/toonCrawlStride,1.);
    skin->animation().play(clip,true);skin->animation().seek((leaving?1-phase:phase)*cycle);skin->animation().pause();
   }
   if(!crawl&&leaving&&c.contact.leave==c.contact.enter)if(auto* mesh=dynamic_cast<engine::SkeletalMeshComponent*>(s.findComponent(c.mesh))){mesh->animation().seek(std::max(0.,duration-c.elapsed));mesh->animation().pause();}
   if(reached&&c.elapsed>=duration){c.elapsed=0;if(leaving){
    // Other cats can finish and return home during a bounded use. The return
    // route must be planned against their current seats/homes, not the outgoing
    // route's historical obstacles.
    auto route=pathTo(s,i,localToWorld(*at,c.contact.approach),c.item);
    if(route.empty()){c.phase=Phase::Use;play(s,i,c.contact.loop);a->transform.position=localToWorld(*at,c.contact.root);continue;}
    route.push_back(a->transform.position);std::reverse(route.begin(),route.end());c.path=std::move(route);c.segment=0;c.phase=Phase::Return;play(s,i,"garden_walk");
   }else {c.phase=Phase::Use;play(s,i,c.contact.loop);}}
  }else if(c.phase==Phase::Use){if(reduced_)continue;
   // Keep the closing tableau intact while the player reads the dialogue.
   // Leaving the ending replaces the scene and releases these reservations.
   if(c.item=="pip_lookout"||c.item=="mabel_nook"||(celebrating_&&c.item==groupItem_))continue;
   if(c.item==groupItem_&&std::any_of(cats_.begin(),cats_.end(),[&](auto& seat){return seat.item==groupItem_&&(seat.phase==Phase::Queued||seat.phase==Phase::Approach||seat.phase==Phase::Enter);}))continue;
   c.elapsed+=dt;if(c.elapsed>=5&&!laneBusy(i)){c.elapsed=0;c.entryTravel=0;c.phase=Phase::Leave;std::reverse(c.entryPath.begin(),c.entryPath.end());c.entrySegment=0;play(s,i,c.contact.leave.empty()?"return_idle":c.contact.leave);}}
  else if(c.phase==Phase::Return&&move(s,i,dt)){auto* skin=a->getComponent<engine::SkeletalMeshComponent>();if(!skin||faceCat(*a,*skin,0,dt))idle(s,i);}
 }
 for(std::size_t i=0;i<4;++i)if(auto* marker=s.findActor(cats_[i].questMarker))
  if(auto* cat=s.findActor(cats_[i].actor))marker->transform.position=cat->transform.position+engine::Vec3{0,1.05f,0};
}
bool Presentation::attention(engine::Scene& s,std::size_t i,Stamp expected){if(expected!=stamp_)return false;yieldAmbient(s);if(i>=4||expected!=stamp_||!cats_[i].introduced||!assets_.cats[i].mesh||!s.findActor(cats_[i].actor)||laneBusy(i))return false;idle(s,i);cats_[i].segment=0;cats_[i].phase=Phase::Attention;play(s,i,reduced_?"attentive_request":"garden_walk",reduced_,reduced_?1.2:0);return true;}
void Presentation::react(engine::Scene& s,std::size_t i,Stamp expected){if(i>=4||expected!=stamp_||!cats_[i].introduced||!assets_.cats[i].mesh||cats_[i].phase!=Phase::Idle)return;cats_[i].phase=Phase::Reaction;cats_[i].elapsed=0;auto name=has(*assets_.cats[i].mesh,"personality_reaction")?"personality_reaction":"attentive_request";play(s,i,name,reduced_,1.2);}
void Presentation::previewPlacement(engine::Scene& s,std::string_view id,std::string_view at,bool legal){
 for(std::size_t i=0;i<8;++i)if(auto* a=s.findActor(items_[i].preview)){const auto* target=spot(at);a->enabled=target&&p::items[i].id==id;if(a->enabled){a->transform.position=target->position+engine::Vec3{0,.025f,0};for(auto* mesh:a->getComponents<engine::MeshComponent>())mesh->color=legal?engine::Vec3{.8f,1,.9f}:engine::Vec3{1,.6f,.5f};}}
}
}

