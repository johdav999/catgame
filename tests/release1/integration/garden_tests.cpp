#include "release1/garden/presentation.hpp"
#include "release1/props/prop_assets.hpp"
#include "release1/cats/cat_assets.hpp"
#include "release1/progression/policy.hpp"
#include "afternoon.hpp"
#include "engine/application.hpp"
#include "engine/glb_importer.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
namespace {
struct CatInput {const char* cat;const char* id;const char* url;};
struct PortraitInput {const char* cat;const char* url;};
#include "release1/integration/accepted.inc"
int checks=0;void check(bool b,std::string why){++checks;if(!b)throw std::runtime_error(why);}
}
int main(){try{
 using namespace paws;using namespace paws::release1;namespace g=paws::release1::garden;
 const auto root=std::filesystem::path(PAWS_SOURCE_DIR);g::Assets assets;
 auto load=[&](GardenAsset a){assets.props[a.name]=engine::GlbImporter::loadFile(root/"assets/runtime"/std::string(a.file).substr(7));};
 for(auto a:gardenAssets())load(a);for(auto a:props::assets())load(a);
 for(std::size_t i=0;i<4;++i){check(*catInputs[i].url,"Four accepted final cat URLs");assets.cats[i].mesh=engine::GlbImporter::loadFile(root/"assets/runtime"/std::string(catInputs[i].url).substr(7));cats::validate(*assets.cats[i].mesh);}
 acceptedContacts(assets);
 { Afternoon story;
  for(auto q:{0,12,13,14}){check(story.dispatchIntent(story.intent(Verb::SelectQuest,{},std::string(progression::quests[q].id))).accepted,"Select story");for(auto step:rules::quest(progression::quests[q].id).solution){auto i=story.intent(step.verb,step.object,step.target);i.direction=step.direction;i.destination=step.destination;check(story.dispatchIntent(i).accepted,"Story solution");}}
  story.returnToGarden(story.intent(Verb::Select));story=Afternoon::decode(story.encode());
  auto built=g::build(assets,story.progressView(),false);auto& scene=*built.scene.scene;auto& b=built.presentation;
  check(b.lookoutReady()&&scene.findActor(b.lookoutSeat()),"Saved story creates permanent seat");
  std::string why;auto ticket=b.use(scene,"pip_lookout",0,b.stamp(),why);check(bool(ticket),"Lookout use: "+why);
  for(int n=0;n<3600&&b.cats()[0].phase!=g::Phase::Use;++n)b.update(scene,1./60);
  check(b.cats()[0].phase==g::Phase::Use,"Pip reaches lookout on supported path");
  for(int n=0;n<600;++n)b.update(scene,1./60);check(b.cats()[0].phase==g::Phase::Use,"Quiet moment is untimed");
  check(b.cancel(scene,*ticket)&&b.cats()[0].item.empty(),"Quiet moment can end");
 }
 Afternoon owner;
 for(std::size_t q=0;q<10;++q){check(owner.dispatchIntent(owner.intent(Verb::SelectQuest,{},std::string(progression::quests[q].id))).accepted,"Select actual quest");for(auto step:rules::quest(progression::quests[q].id).solution){auto i=owner.intent(step.verb,step.object,step.target);i.direction=step.direction;i.destination=step.destination;check(owner.dispatchIntent(i).accepted,"Earn actual reward");}check(owner.returnToGarden(owner.intent(Verb::Select)).accepted,"Garden return");}
 for(auto c:progression::catIds)if(c!="pip")check(owner.introduce(c,owner.intent(Verb::Select)).accepted,"Actual introductions");
 {
  const auto saved=owner.encode();auto social=g::build(assets,owner.progressView(),false);auto& world=*social.scene.scene;auto& behavior=social.presentation;
  bool greeted=false,strolled=false,finished=false;std::array<bool,4> paired{};
  for(int frame=0;frame<18000&&!finished;++frame){behavior.update(world,1./30);
   std::vector<std::size_t> walking;
   for(std::size_t i=0;i<4;++i){const auto phase=behavior.cats()[i].phase;
    greeted|=phase==g::Phase::WelcomeHello;
    if(phase==g::Phase::WelcomeHome){paired[i]=true;walking.push_back(i);strolled=true;}
   }
   if(walking.size()==2){auto d=world.findActor(behavior.cats()[walking[0]].actor)->transform.position-world.findActor(behavior.cats()[walking[1]].actor)->transform.position;check(engine::dot(d,d)>.4f,"Resident pair keeps space through the garden loop");}
   if(strolled){finished=true;for(std::size_t i=0;i<4;++i)if(paired[i])finished&=behavior.cats()[i].phase==g::Phase::Idle;}
  }
  check(greeted&&strolled&&finished,"Existing residents greet, stroll together and finish naturally");
  check(owner.encode()==saved,"Resident social behavior never changes introductions or progress");
  bool nextGreeting=false;
  for(int frame=0;frame<18000&&!nextGreeting;++frame){behavior.update(world,1./30);for(const auto& cat:behavior.cats())nextGreeting|=cat.phase==g::Phase::WelcomeHello;}
  check(nextGreeting,"Social strolls recur after a quiet interval");
  behavior.cancelAll(world);
  for(const auto& cat:behavior.cats())check(cat.phase!=g::Phase::WelcomeWalk&&cat.phase!=g::Phase::WelcomeHello&&cat.phase!=g::Phase::WelcomeHome,"Player activity cancels the resident pair");
 }
 // Run real animation ticks: presentation-only loops cannot detect a clip
 // that is restarted on every frame while one member of the pair is waiting.
 for(std::size_t newcomer=1;newcomer<4;++newcomer){
  auto arrivals=owner.progressView();arrivals.cats[newcomer].introduced=false;
  auto welcome=g::build(assets,arrivals,false);auto& w=welcome.presentation;
  auto& world=*welcome.scene.scene;
  std::array<int,4> waiting{};bool sawStableWait=false;
  for(int frame=0;frame<7200&&!w.cats()[newcomer].arrivalComplete;++frame){
   w.update(world,1./60);
   for(const auto& c:w.cats())world.findActor(c.actor)->getComponent<engine::SkeletalMeshComponent>()->animation().update(1./60);
   for(std::size_t i=0;i<4;++i){const auto& cat=w.cats()[i];
    const bool waitingForPartner=(cat.phase==g::Phase::Arrival||cat.phase==g::Phase::WelcomeWalk)&&!cat.path.empty()&&cat.segment+1>=cat.path.size()&&
     std::any_of(w.cats().begin(),w.cats().end(),[&](const auto& other){return &cat!=&other&&(other.phase==g::Phase::Arrival||other.phase==g::Phase::WelcomeWalk)&&other.segment+1<other.path.size();});
    waiting[i]=waitingForPartner?waiting[i]+1:0;
    if(waiting[i]==5){
     auto* skin=world.findActor(cat.actor)->getComponent<engine::SkeletalMeshComponent>();
     check(skin->animation().time()>.04,"Waiting cat's idle animation must advance instead of restarting every frame");sawStableWait=true;
    }
   }
  }
  check(sawStableWait,"Welcome exercises an early arrival waiting for its partner: "+std::to_string(newcomer));
  check(w.cats()[newcomer].arrivalComplete,"Welcome with ticking animation finishes");
 }
 const char* placement[]{"bench_corner","sunny_corner","scratch_corner","perch_corner","hammock_nook","playhouse_corner","picnic_table_spot","picnic_mat_spot"};
 for(std::size_t i=0;i<8;++i){owner.setItemAvailable(progression::items[i].id,true);check(owner.dispatchIntent(owner.intent(Verb::Buy,std::string(progression::items[i].id))).accepted,"Actual purchase");check(owner.dispatchIntent(owner.intent(Verb::PlaceFurnishing,std::string(progression::items[i].id),placement[i])).accepted,"Actual placement");}
 {
  auto arrivals=owner.progressView();arrivals.cats[3].introduced=false;
  auto welcome=g::build(assets,arrivals,false);auto& w=welcome.presentation;auto& world=*welcome.scene.scene;
  bool outbound=false,hello=false,escorted=false;double greetingSeconds=0;
  for(int frame=0;frame<12000&&!w.cats()[3].arrivalComplete;++frame){
   w.update(world,1./60);
   outbound|=w.cats()[2].phase==g::Phase::WelcomeWalk;
   hello|=w.cats()[2].phase==g::Phase::WelcomeHello;
   greetingSeconds+=w.cats()[2].phase==g::Phase::WelcomeHello?1./60:0;
   if(w.cats()[3].phase==g::Phase::WelcomeHome){escorted=true;
    auto d=world.findActor(w.cats()[3].actor)->transform.position-world.findActor(w.cats()[2].actor)->transform.position;
    check(engine::dot(d,d)>.45f,"Welcoming pair must not overlap on the shared return path");
   }
  }
  check(outbound&&hello&&escorted&&greetingSeconds>=2.4,"Resident walks out, greets and escorts the newcomer");
  check(w.cats()[3].arrivalComplete,"Welcome finishes and releases the newcomer introduction");
  for(auto i:{2,3}){auto d=world.findActor(w.cats()[i].actor)->transform.position-g::home(i);check(engine::dot(d,d)<.0001f,"Both cats return to their own start positions");}
 }
 auto built=g::build(assets,owner.progressView(),false);engine::Application app;auto id=app.loadScene(std::move(built.scene.scene));app.start(id);auto& b=built.presentation;auto& scene=*app.activeScene();double time=0;app.advance(time);
 const auto catVolume=engine::CollisionShape::capsule(.06f,.22f);engine::PhysicsQueryFilter solids;solids.layers=1u<<14;solids.includeSensors=false;
 auto advance=[&](double seconds){for(int n=0;n<int(seconds*60);++n){
  std::array<float,4> previousYaw{};std::array<g::Phase,4> previousPhase{};
  for(std::size_t i=0;i<4;++i){previousYaw[i]=scene.findActor(b.cats()[i].actor)->transform.rotation.y;previousPhase[i]=b.cats()[i].phase;}
  time+=1./60;engine::ApplicationFrameCallbacks cb;cb.simulate=[&](engine::Scene& s,const engine::ApplicationFrame& f){b.update(s,f.simulation.delta);};app.advance(time,cb);
  for(std::size_t i=0;i<4;++i)if(previousPhase[i]==g::Phase::Approach&&b.cats()[i].phase==g::Phase::Approach){
   const float delta=std::remainder(scene.findActor(b.cats()[i].actor)->transform.rotation.y-previousYaw[i],2*engine::pi);
   check(std::abs(delta)<=.0501f,"Route arrival must not spend the yaw budget twice in one frame");
  }
  for(const auto& binding:b.cats())if(auto* cat=scene.findActor(binding.actor);cat&&cat->enabled){engine::Transform pose;pose.position=cat->transform.position+engine::Vec3{0,.305f,0};
   for(const auto& hit:scene.physics().overlap(*catVolume,pose,solids))check(hit.penetrationDepth<=.003f,"Actual cat body stays outside solid geometry throughout walking, entry, use and leave");}
 }};
 auto* nap=dynamic_cast<engine::Actor*>(scene.findSavedNode("garden.nap.cat"));check(nap,"Accepted sleeping guest exists");
 const auto sleepingRoot=assets.cats[2].contacts.at("garden_cushion").root;
 auto near=[](engine::Vec3 a,engine::Vec3 b){auto d=a-b;return engine::dot(d,d)<.000001f;};
 advance(29);check(near(nap->transform.position,sleepingRoot),"Long first nap stays supported on cushion");
 app.pause();advance(4);check(near(nap->transform.position,sleepingRoot),"Application pause freezes nap cycle");app.resume();
 advance(4);check(near(nap->transform.position,{0,sleepingRoot.y,0}),"Sleeping guest rises into standing idle on cushion");
 advance(5);check(near(nap->transform.position,{0,sleepingRoot.y,0}),"Guest lingers awake rather than immediately sleeping");
 advance(8);check(near(nap->transform.position,sleepingRoot),"Guest settles into measured sleeping contact again");
 advance(44);check(near(nap->transform.position,{0,sleepingRoot.y,0}),"Second wake proves nap cycle repeats");
 b.project(scene,owner.progressView(),true);advance(60);check(near(nap->transform.position,sleepingRoot),"Reduced motion keeps guest resting");
 b.project(scene,owner.progressView(),false);advance(45);check(near(nap->transform.position,{0,sleepingRoot.y,0}),"Nap cycle resumes after reduced motion");
 // Full reactions must survive beyond the obsolete 2.4-second cutoff.
 for(std::size_t i=0;i<4;++i){b.cancelAll(scene);b.react(scene,i,b.stamp());advance(2.5);
  check(b.cats()[i].phase==g::Phase::Reaction,"Reaction is not cut off before its final poses");
  advance(.6);check(b.cats()[i].phase==g::Phase::Idle,"Reaction releases after its full clip");}
 b.cancelAll(scene);const auto ambientSave=owner.encode();
 bool wandered=false,rested=false,returned=false;std::array<bool,4> away{};
 std::array<double,4> restingTime{};double longestRest=0;
 for(int sample=0;sample<720;++sample){advance(.5);
  for(std::size_t i=0;i<4;++i){const auto& cat=b.cats()[i];auto* actor=scene.findActor(cat.actor);
   wandered|=cat.phase==g::Phase::Wander;rested|=cat.phase==g::Phase::Resting;
   restingTime[i]=cat.phase==g::Phase::Resting?restingTime[i]+.5:0;longestRest=std::max(longestRest,restingTime[i]);
   if(cat.phase==g::Phase::WanderHome)away[i]=true;
   if(away[i]&&cat.phase==g::Phase::Idle&&near(actor->transform.position,g::home(i)))returned=true;
  }
 }
 check(wandered&&rested&&returned,"Ambient cats walk, rest and physically return home");
 check(longestRest>=19.5,"Ambient rest is a prolonged pause rather than a brief pose");
 check(owner.encode()==ambientSave,"Ambient behavior leaves progression and saves unchanged");
 auto frozen=b.cats();std::array<engine::Vec3,4> poses{};for(std::size_t i=0;i<4;++i)poses[i]=scene.findActor(frozen[i].actor)->transform.position;
 app.pause();advance(3);app.resume();for(std::size_t i=0;i<4;++i)check(near(poses[i],scene.findActor(frozen[i].actor)->transform.position),"Pause freezes ambient travel and rest");
 b.project(scene,owner.progressView(),true);advance(40);for(const auto& cat:b.cats())check(!g::ambient(cat.phase),"Reduced motion disables ambient movement");
 b.project(scene,owner.progressView(),false);
 check(b.attention(scene,0,b.stamp()),"Explicit request attention preempts ambient movement");b.cancelAll(scene);
 const std::size_t useCats[]{2,1,0,1,2,0,0,0};const auto unchanged=owner.encode();
 for(std::size_t i=0;i<8;++i){b.cancelAll(scene);std::string why;auto ticket=b.use(scene,progression::items[i].id,useCats[i],b.stamp(),why);check(bool(ticket),"Final furnishing request: "+std::string(progression::items[i].id)+" "+why);bool used=false;
  for(int sec=0;sec<90&&!used;++sec){advance(.5);used=b.cats()[useCats[i]].phase==g::Phase::Use;}check(used,"Final furnishing reaches actual use: "+std::string(progression::items[i].id));
  auto* cat=scene.findActor(b.cats()[useCats[i]].actor);auto before=cat->transform.position;app.pause();advance(.5);check(cat->transform.position.x==before.x&&cat->transform.position.y==before.y&&cat->transform.position.z==before.z,"Pause freezes contact sequence");app.resume();
  if(i==4){auto* skin=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(b.cats()[2].mesh));const auto mesh=assets.cats[2].mesh;float needed=0;
   for(int frame=0;frame<=240;++frame){skin->animation().seek(frame/60.);skin->animation().pause();const auto pose=skin->skinPose();
    for(std::size_t v=0;v<mesh->vertices.size();++v){engine::Vec3 p{};for(int k=0;k<4;++k)p=p+engine::transformPoint(pose->bones[mesh->skin[v].joints[k]].transform,mesh->vertices[v].position)*mesh->skin[v].weights[k];
     auto local=p+assets.cats[2].contacts.at("hammock").root;local.y=p.y;
     if(std::abs(local.x)<=.4f&&std::abs(local.z)<=.59f){float support=.33f+.30f*(local.z/.59f)*(local.z/.59f)+.035f*(local.x/.4f)*(local.x/.4f);needed=std::max(needed,support-p.y);}
    }
   }std::cout<<"Measured Button hammock root Y: "<<needed<<'\n';
   const float height=assets.cats[2].contacts.at("hammock").root.y;check(height>=needed&&height<=needed+.007f,"Actual full-loop curl meets cloth without floating or penetration");
  }
  // Exercise actual leaving and return routes, rather than cancelling the pose.
  for(int sec=0;sec<100&&!b.cats()[useCats[i]].item.empty();++sec)advance(1);
  check(b.cats()[useCats[i]].item.empty(),"Actual furnishing releases after a collision-safe return: "+std::string(progression::items[i].id));
 }
 b.cancelAll(scene);check(b.celebrationProps(scene,b.stamp(),true),"Supply picnic props");std::string why;check(b.gather(scene,"picnic_mat",b.stamp(),why,true).size()==4,"Atomic actual four-cat gathering: "+why);
 bool together=false;for(int sec=0;sec<360&&!together;++sec){advance(.5);together=true;for(auto& c:b.cats())together&=c.phase==g::Phase::Use;}
 if(!together)for(auto& c:b.cats()){auto p=scene.findActor(c.actor)->transform.position;std::cerr<<"Gathering phase "<<int(c.phase)<<" "<<c.reason<<" "<<p.x<<","<<p.z<<'\n';}
 check(together,"Four actual cats reach their reserved seats");
 advance(8);for(auto& c:b.cats())check(c.phase==g::Phase::Use,"Closing tableau remains while its dialogue is read");
 check(owner.encode()==unchanged,"Presentation never grants rewards or changes save");check(app.loadedSceneCount()==1,"One Application scene");app.shutdown();
 { Afternoon story;
  for(auto q:{0,1,2,12,13,14,15,16,17}){check(story.dispatchIntent(story.intent(Verb::SelectQuest,{},std::string(progression::quests[q].id))).accepted,"Select Mabel story");for(auto step:rules::quest(progression::quests[q].id).solution){auto i=story.intent(step.verb,step.object,step.target);i.direction=step.direction;i.destination=step.destination;check(story.dispatchIntent(i).accepted,"Mabel solution");}}
  for(auto c:{"mabel","button"})check(story.introduce(c,story.intent(Verb::Select)).accepted,"Story introductions");
  story.returnToGarden(story.intent(Verb::Select));story=Afternoon::decode(story.encode());
  auto built=g::build(assets,story.progressView(),false);auto& scene=*built.scene.scene;auto& b=built.presentation;
  check(b.nookReady()&&scene.findActor(b.nookSeat()),"Permanent sunny nook");std::string why;
  const bool visit=b.sunnyVisit(scene,b.stamp(),why);check(visit,"Sunny visit: "+why);
  for(int n=0;n<7200&&(b.cats()[0].phase!=g::Phase::Use||b.cats()[1].phase!=g::Phase::Use);++n)b.update(scene,1./60);
  check(b.cats()[0].phase==g::Phase::Use&&b.cats()[1].phase==g::Phase::Use,"Both cats reach sunny cushions");
  for(int n=0;n<1500;++n)b.update(scene,1./60);check(b.cats()[0].phase==g::Phase::Use&&b.cats()[1].phase==g::Phase::Use,"Shared rest is untimed");
  b.cancelAll(scene);check(b.cats()[0].item.empty()&&b.cats()[1].item.empty(),"Leave nook releases both cats");
 }
std::cout<<checks<<" final-cat garden checks passed\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

