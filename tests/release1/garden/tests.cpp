#include "release1/garden/presentation.hpp"
#include "release1/garden/contact_plans.hpp"
#include "release1/props/prop_assets.hpp"
#include "release1/progression/codec.hpp"
#include "engine/application.hpp"
#include "engine/glb_importer.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
namespace g=paws::release1::garden;namespace p=paws::release1::progression;using paws::release1::Verb;
bool near(engine::Vec3 a,engine::Vec3 b){auto d=a-b;return engine::dot(d,d)<1e-10f;}
int assertions=0;void check(bool b,std::string why){++assertions;if(!b)throw std::runtime_error(why);}
void solve(paws::Afternoon& owner,std::size_t q){check(owner.dispatchIntent(owner.intent(Verb::SelectQuest,{},std::string(p::quests[q].id))).accepted,"select known trace");for(auto& step:paws::release1::rules::quest(p::quests[q].id).solution){auto intent=owner.intent(step.verb,step.object,step.target);intent.destination=step.destination;intent.direction=step.direction;check(owner.dispatchIntent(intent).accepted,"trace step");}check(owner.returnToGarden(owner.intent(Verb::Select)).accepted,"garden return");}
int main(){try{
 const auto root=std::filesystem::path(__FILE__).parent_path().parent_path().parent_path().parent_path();
 g::Assets assets;auto load=[&](const paws::GardenAsset& a){assets.props[a.name]=engine::GlbImporter::loadFile((root/"assets/runtime"/std::string(a.file).substr(7)).string());};for(auto& a:paws::gardenAssets())load(a);for(auto& a:paws::release1::props::assets())load(a);
 auto pip=engine::GlbImporter::loadFile((root/"assets/runtime/models/pip/pip.glb").string());assets.cats[0]=g::legacyPip(pip);
 assets.props["quest_marker"]=engine::GlbImporter::loadFile((root/"assets/runtime/models/garden/quest_marker.glb").string());
 paws::Afternoon owner;owner.returnToGarden(owner.intent(Verb::Select));auto fresh=g::build(assets,owner.progressView(),false);check(fresh.presentation.requests(0).size()==1,"fresh Pip entry point");check(!fresh.scene.scene->findActor(fresh.presentation.cats()[1].actor),"missing cats are absent");
 auto* marker=fresh.scene.scene->findActor(fresh.presentation.cats()[0].questMarker);check(marker&&marker->enabled,"available request shows marker");
 fresh.presentation.update(*fresh.scene.scene,.5);check(marker->transform.rotation.y>0,"marker spins around vertical axis");
 fresh.presentation.project(*fresh.scene.scene,owner.progressView(),true);fresh.presentation.update(*fresh.scene.scene,.5);check(marker->transform.rotation.y==0,"reduced motion stops spin");
 solve(owner,0);fresh.presentation.project(*fresh.scene.scene,owner.progressView(),false);check(marker->enabled&&fresh.presentation.requests(0).front().id=="pip_tidy_toys","next request replaces completed request");
 for(std::size_t q=1;q<3;++q)solve(owner,q);fresh.presentation.project(*fresh.scene.scene,owner.progressView(),false);check(marker->enabled&&fresh.presentation.requests(0).front().id=="pip_friend_ball","Pip offers his optional friendship story");
 {auto finished=owner;for(auto q:{12,13,14})solve(finished,q);auto quiet=g::build(assets,finished.progressView(),true);check(!quiet.scene.scene->findActor(quiet.presentation.cats()[0].questMarker)->enabled,"marker hidden without an available request");}
 {
  auto all=assets;for(std::size_t i=1;i<4;++i)all.cats[i]=g::legacyPip(pip);
  auto pending=paws::Afternoon::decode(owner.encode());const auto before=pending.encode();
  auto arrival=g::build(all,pending.progressView(),false);auto& scene=*arrival.scene.scene;auto& b=arrival.presentation;
  check(b.cats()[1].phase==g::Phase::ArrivalQueued&&b.cats()[2].phase==g::Phase::ArrivalQueued,"Chapter two queues both newcomers");
  check(!scene.findActor(b.cats()[3].actor)->enabled,"Pepper stays absent before chapter three");
  b.react(scene,0,b.stamp());b.update(scene,.1);
  check(b.cats()[1].phase==g::Phase::ArrivalQueued,"Newcomer waits until its resident can greet it");
  for(int frame=0;frame<240&&b.cats()[1].phase==g::Phase::ArrivalQueued;++frame)b.update(scene,1./60);
  b.update(scene,1./60);auto* mabel=scene.findActor(b.cats()[1].actor);
  check(mabel->enabled&&mabel->transform.position.x>6.24f&&b.cats()[1].phase==g::Phase::Arrival,"First cat begins outside the gate");
  check(!scene.findActor(b.cats()[2].actor)->enabled,"Second newcomer waits offstage");
  check(scene.findActor(b.cats()[1].questMarker)->enabled,"Arriving requester carries an exclamation mark");
  const auto pose=mabel->transform.position;pending.setUserPaused(true,pending.intent(Verb::Select));b.project(scene,pending.progressView(),false);
  check(near(pose,mabel->transform.position)&&b.cats()[1].phase==g::Phase::Arrival,"Pause generation preserves arrival route");
  pending.setUserPaused(false,pending.intent(Verb::Select));b.project(scene,pending.progressView(),false);
  bool crossed=false;
  for(int n=0;n<12000&&!b.cats()[2].arrivalComplete;++n){b.update(scene,1./60);if(std::abs(mabel->transform.position.x-6.24f)<.03f){crossed=true;check(std::abs(mabel->transform.position.z+2.5f)<.001f,"Crossing stays centred in gate opening");}}
  check(crossed&&b.cats()[1].arrivalComplete&&b.cats()[2].arrivalComplete,"Both cats physically finish arrival");
  check(near(mabel->transform.position,g::home(1)),"Mabel reaches her home");
  check(!pending.progressView().cats[1].introduced&&pending.coins()==owner.coins(),"Walking never mutates progression or rewards");
  check(pending.introduce("mabel",pending.intent(Verb::Select)).accepted&&pending.introduce("button",pending.intent(Verb::Select)).accepted,"Host records arrivals through owner");
  auto restored=paws::Afternoon::decode(pending.encode());auto reload=g::build(all,restored.progressView(),false);
  check(reload.presentation.cats()[1].phase==g::Phase::Idle&&reload.presentation.cats()[2].phase==g::Phase::Idle,"Saved residents never repeat arrival");
  auto reduced=g::build(all,paws::Afternoon::decode(before).progressView(),true);
  check(reduced.presentation.cats()[1].arrivalComplete&&reduced.scene.scene->findActor(reduced.presentation.cats()[1].actor)->enabled,"Reduced motion places new resident directly");
 }
 owner=paws::Afternoon{};
 for(std::size_t q=0;q<10;++q)solve(owner,q);for(auto cat:{"mabel","button","pepper"})check(owner.introduce(cat,owner.intent(Verb::Select)).accepted,"introduction committed by owner");
 const int placements[]{0,1,3,4,5,6,7,8};for(std::size_t i=0;i<8;++i){owner.setItemAvailable(p::items[i].id,true);check(owner.dispatchIntent(owner.intent(Verb::Buy,std::string(p::items[i].id))).accepted,"buy through owner");check(owner.dispatchIntent(owner.intent(Verb::PlaceFurnishing,std::string(p::items[i].id),std::string(p::spots[placements[i]].id))).accepted,"place through owner");}
 check(owner.coins()==10,"one authoritative economy");auto built=g::build(assets,owner.progressView(),false);auto& b=built.presentation;
 engine::Application app;auto id=app.loadScene(std::move(built.scene.scene));app.start(id);double time=0;app.advance(time);
 auto advance=[&](double seconds){for(int i=0;i<int(seconds*60);++i){time+=1./60;engine::ApplicationFrameCallbacks cb;cb.simulate=[&](engine::Scene& s,const engine::ApplicationFrame& f){b.update(s,f.simulation.delta);};app.advance(time,cb);
  if(auto* s=app.activeScene())for(std::size_t a=0;a<4;++a)for(std::size_t c=a+1;c<4;++c){auto* first=s->findActor(b.cats()[a].actor);auto* second=s->findActor(b.cats()[c].actor);if(first&&second&&first->enabled&&second->enabled){auto d=first->transform.position-second->transform.position;check(engine::dot(d,d)>=.72f*.72f,"walking lane preserves other cat contact space: "+std::to_string(a)+"/"+std::to_string(c)+" phases "+std::to_string(int(b.cats()[a].phase))+"/"+std::to_string(int(b.cats()[c].phase))+" distance "+std::to_string(std::sqrt(engine::dot(d,d))));}}
 }};
 auto& scene=*app.activeScene();std::string reason;auto stateBefore=owner.encode();auto t=b.use(scene,"garden_cushion",0,b.stamp(),reason);check(bool(t),"accepted legacy contact: "+reason);check(!b.use(scene,"drinking_bowl",0,b.stamp(),reason),"one use per cat");
 advance(1);auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(b.cats()[0].mesh));auto pos=scene.findActor(b.cats()[0].actor)->transform.position;auto anim=cat->animation().time();app.pause();advance(2);check(cat->animation().time()==anim&&near(scene.findActor(b.cats()[0].actor)->transform.position,pos),"pause freezes schedule and skin");app.setSuspended(true);app.resume();advance(1);check(cat->animation().time()==anim,"hidden freezes both clocks");app.setSuspended(false);
 check(owner.encode()==stateBefore,"animation never mutates owner");b.previewPlacement(scene,"garden_cushion","quiet_corner",true);check(b.cats()[0].phase!=g::Phase::Idle,"placement preview keeps use");b.previewPlacement(scene,{},{},false);check(b.cats()[0].phase!=g::Phase::Idle,"cancel preview keeps use");
 check(owner.dispatchIntent(owner.intent(Verb::PlaceFurnishing,"garden_cushion","quiet_corner")).accepted,"commit move");b.project(scene,owner.progressView(),false);check(b.cats()[0].phase==g::Phase::Idle,"commit moving used item cancels contact");check(!b.cancel(scene,*t),"old ticket cannot cancel new use");
 auto stale=b.stamp();--stale.revision;check(!b.use(scene,"garden_cushion",0,stale,reason),"stale view rejected");auto snapshot=owner.progressView();--snapshot.revision;b.project(scene,snapshot,false);check(b.stamp().revision==owner.ownerRevision(),"out of order projection ignored");
 check(owner.dispatchIntent(owner.intent(Verb::PlaceFurnishing,"garden_cushion","bench_corner")).accepted,"restore spot");b.project(scene,owner.progressView(),true);t=b.use(scene,"garden_cushion",0,b.stamp(),reason);check(bool(t),"reduced use begins");advance(.2);check(b.cats()[0].phase==g::Phase::Use,"reduced uses static supported root");anim=cat->animation().time();pos=scene.findActor(b.cats()[0].actor)->transform.position;advance(2);check(cat->animation().time()==anim&&near(scene.findActor(b.cats()[0].actor)->transform.position,pos),"reduced static does not drift");b.cancelAll(scene);check(!b.cancel(scene,*t),"cancel token invalidated");
 auto invalid=assets;invalid.props.erase("hammock");try{g::build(invalid,owner.progressView(),false);check(false,"missing model rejected");}catch(const std::invalid_argument&){}check(app.activeSceneId()==id,"candidate failure preserves outgoing scene");
 // Synthetic clip aliases exercise scheduling contracts ONLY. Never staged or
 // described as four accepted identities or artwork/animation acceptance.
 auto synthetic=std::make_shared<engine::Mesh>(*pip);auto skeleton=std::make_shared<engine::Skeleton>(*pip->skeleton);for(auto name:{"scratch","perch_step","perch_settle","playhouse_enter","playhouse_lookout","picnic_sit"}){auto clip=skeleton->clips[paws::pipClip(*pip,"relaxed_idle")];clip.name=name;skeleton->clips.push_back(std::move(clip));}synthetic->skeleton=skeleton;
 for(std::size_t c=0;c<4;++c){assets.cats[c].mesh=synthetic;assets.cats[c].contacts=g::contactGuides(c);for(auto& [_,contact]:assets.cats[c].contacts)contact.verified=true;}
 auto four=g::build(assets,owner.progressView(),true);auto oldActor=b.cats()[0].actor;auto next=app.loadScene(std::move(four.scene.scene));app.requestTransition(next,true);check(app.commitTransition(),"replace garden");b=std::move(four.presentation);check(!app.activeScene()->findActor(oldActor),"scene handles reject old generation");auto& replacement=*app.activeScene();
 for(std::size_t i=0;i<4;++i)check(bool(replacement.findActor(b.cats()[i].actor)),"independent cat actor");
 const int users[]{2,1,0,1,2,0,0,3};for(std::size_t item=0;item<8;++item){std::string why;auto ticket=b.use(replacement,p::items[item].id,users[item],b.stamp(),why);check(bool(ticket),"schedule item "+std::string(p::items[item].id)+": "+why);advance(.2);check(b.cats()[users[item]].phase==g::Phase::Use,"static item contact");check(owner.encode()==stateBefore||owner.coins()==10,"presentation never awards coins");check(b.cancel(replacement,*ticket),"cancel valid ticket");}
 b.project(replacement,owner.progressView(),false);auto a=b.use(replacement,"drinking_bowl",1,b.stamp(),reason);auto d=b.use(replacement,"garden_cushion",2,b.stamp(),reason);check(a&&d,"different furnishings queue");advance(.5);check(b.cats()[1].phase==g::Phase::Approach&&b.cats()[2].phase==g::Phase::Queued,"bounded shared approach reservation");check(!b.use(replacement,"garden_cushion",0,b.stamp(),reason),"same furnishing reservation");
 for(int wait=0;wait<80&&!(b.cats()[1].phase==g::Phase::Use&&b.cats()[2].phase==g::Phase::Approach);++wait)advance(1);
 check(b.cats()[1].phase==g::Phase::Use&&b.cats()[2].phase==g::Phase::Approach,"reservation hands lane to second cat");auto transaction=owner.encode();check(b.cancel(replacement,*a),"cancel resting cat during another approach");check(b.cats()[1].phase==g::Phase::Idle&&b.cats()[2].phase==g::Phase::Idle,"cancel safely invalidates in-flight path through old home");check(owner.encode()==transaction,"cancel is presentation only");
 a=b.use(replacement,"drinking_bowl",1,b.stamp(),reason);d=b.use(replacement,"garden_cushion",2,b.stamp(),reason);check(a&&d,"resume bounded uses after cancellation");advance(180);check(b.cats()[1].item.empty()&&b.cats()[2].item.empty(),"both bounded uses finish and release lane");
 b.project(replacement,owner.progressView(),true);auto group=b.gather(replacement,"low_picnic_table",b.stamp(),reason);check(group.size()==4,"four distinct picnic table seats: "+reason);advance(.2);for(auto& c:b.cats())check(c.phase==g::Phase::Use,"group static supported seats");b.cancelAll(replacement);
 group=b.gather(replacement,"picnic_mat",b.stamp(),reason);check(group.size()==4,"mat plus grounded companion seats: "+reason);advance(.2);for(auto& c:b.cats())check(c.phase==g::Phase::Use,"mat companions settle without overlapping centre");b.cancelAll(replacement);
 paws::Afternoon minimal;for(std::size_t q=0;q<10;++q)solve(minimal,q);for(auto name:{"mabel","button","pepper"})minimal.introduce(name,minimal.intent(Verb::Select));check(minimal.dispatchIntent(minimal.intent(Verb::Buy,"drinking_bowl")).accepted,"minimal ending purchase");check(minimal.dispatchIntent(minimal.intent(Verb::PlaceFurnishing,"drinking_bowl","quiet_corner")).accepted,"minimal ending placement");auto supplied=g::build(assets,minimal.progressView(),true);auto bytes=minimal.encode();check(supplied.presentation.celebrationProps(*supplied.scene.scene,supplied.presentation.stamp(),true),"supplied props require eligible owner");check(supplied.scene.scene->findSavedNode("supplied.celebration.0")->enabled&&supplied.scene.scene->findSavedNode("supplied.celebration.1")->enabled,"picnic without table or mat purchase");check(minimal.encode()==bytes&&!minimal.owned("picnic_mat"),"celebration neither purchases nor pays rewards");check(!fresh.presentation.celebrationProps(*fresh.scene.scene,fresh.presentation.stamp(),true),"ineligible owner cannot start finale");
 auto restored=paws::Afternoon::decode(owner.encode());auto rebuilt=g::build(assets,restored.progressView(),false);check(restored.pendingIntroductions().empty()&&restored.coins()==10,"reload neither introduction nor reward repeated");for(auto& cat:rebuilt.presentation.cats())check(cat.phase==g::Phase::Idle,"reload restores garden without stale use callbacks");
 app.shutdown();check(!app.findScene(next),"shutdown releases scenes");std::cout<<assertions<<" assertions passed; synthetic four-cat scheduling is not asset acceptance.\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
