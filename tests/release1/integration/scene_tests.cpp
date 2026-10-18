#include "release1/integration/puzzle_scene.hpp"
#include "release1/props/prop_assets.hpp"
#include "release1/cats/cat_assets.hpp"
#include "afternoon.hpp"
#include "engine/application.hpp"
#include "engine/glb_importer.hpp"
#include "engine/selection.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace paws;using namespace paws::release1;
namespace view=paws::release1::presentation;
int checks=0;void check(bool v,std::string why){++checks;if(!v)throw std::runtime_error(why);}
void waterState(engine::Scene& scene,const SceneBindings& bindings,bool filled,bool picnic=false){
 unsigned count=0;
 for(const auto& binding:bindings.objects)for(const auto& child:scene.findActor(binding.actor)->children()){
  auto* mesh=dynamic_cast<engine::MeshComponent*>(child.get());
  if(!mesh||(mesh->name!="Validated water surface"&&mesh->name!="Filled bowl"&&mesh->name!="Bowl inlet ribbon"))continue;
  ++count;check(!mesh->castsShadow,"Water must not cast a solid shadow");
  if(mesh->name=="Filled bowl"){
   check(std::abs(mesh->transform.position.y-gardenAnchor("bowl","water").y)<.0001f,"Bowl water reaches the authored fill line");
   check(mesh->enabled==filled,"Bowl fill follows committed state");
  }else if(mesh->name=="Validated water surface")check(mesh->enabled==filled,"Gutter fill follows committed state");
#if PAWS_WATER_SHADER
  const auto material=mesh->resolvedMaterial(0);
  check(material.shader==engine::MaterialShader::Water,"Every water surface uses the engine shader");
  check(!material.water.usesSceneInputs(),"Small quest surfaces avoid full-canvas captures");
  check(mesh->mesh->materials.empty()||mesh->mesh->materials[0]->shader!=engine::MaterialShader::Water,"Water overrides preserve imported kit materials");
#endif
 }
 check(count==(picnic?1u:7u),"Expected gutter, inlet and bowl water surfaces");
}
int main(int argc,char** argv){try{
 const auto root=std::filesystem::path(PAWS_SOURCE_DIR);GardenMeshes meshes;
 auto load=[&](const GardenAsset& a){auto m=engine::GlbImporter::loadFile(root/"assets/runtime"/std::string(a.file).substr(7));validateGardenAsset(a,*m);meshes[a.name]=m;};
 for(auto a:gardenAssets())load(a);for(auto a:props::assets())load(a);
 for(const auto& a:cats::assets)meshes[std::string(a.cat)]=engine::GlbImporter::loadFile(root/"assets/runtime"/std::string(a.url).substr(7));
 for(auto name:{"pip","mabel","button","pepper"}){
  const auto& m=*meshes.at(name);const auto& sk=*m.skeleton;
  cats::validate(m);cats::validateCanonical(m,*meshes.at("pip"));
  if(isToonCat(m)){
   check(sk.clips[pipClip(m,"garden_walk")].name=="garden_walk","Replacement gait uses the authored forward clip");
   continue;
  }
  for(auto action:{"relaxed_idle","garden_walk"}){
   const auto original=pipClip(m,action,false),corrected=pipClip(m,action);
   check(sk.clips[corrected].name==std::string(action)+"_forward","Idle and walking select the corrected clip");
   setForwardIdleCorrection(false);check(pipClip(m,action)==original,"Rollback selects the preserved original clip");setForwardIdleCorrection(true);
   const auto& a=sk.clips[original];const auto& b=sk.clips[corrected];check(a.frameCount==b.frameCount&&a.duration==b.duration,"Forward correction preserves duration and sampling");
   bool headChanged=false;
   for(std::size_t i=0;i<a.frames.size();++i){
    const auto x=a.frames[i].matrix(),y=b.frames[i].matrix();
    const auto& joint=sk.joints[i%sk.joints.size()].name;
    for(int k=0;k<16;++k)if(joint=="head"||joint=="neck")headChanged|=std::abs(x.m[k]-y.m[k])>.01f;
    else check(std::abs(x.m[k]-y.m[k])<.00001f,"Correction preserves all other local joint poses, including gait and eyelids");
   }
   check(headChanged,"Corrected head pose must differ from the original");
   if(std::string_view(action)=="garden_walk")for(std::size_t j=0;j<sk.joints.size();++j){
    const auto first=b.frames[j].matrix(),last=b.frames[(b.frameCount-1)*sk.joints.size()+j].matrix();
    for(int k=0;k<16;++k)check(std::abs(first.m[k]-last.m[k])<.001f,"Corrected walk has no loop seam");
   }
  }
 }
 Afternoon owner;owner.setBeachDevelopmentAccess(true);engine::Application app;engine::SceneId previous;
 for(std::size_t q=0;q<progression::quests.size();++q){
  const auto quest=std::string(progression::quests[q].id);check(owner.dispatchIntent(owner.intent(Verb::SelectQuest,{},quest)).accepted,"Owner selection");
  if(argc==2&&(q==6||q==9)){
   std::filesystem::create_directories(argv[1]);
   std::ofstream(std::filesystem::path(argv[1])/(q==6?"water.save":"picnic.save"))<<owner.encode();
  }
  auto built=q<3?makePuzzleScene(owner.session(),&meshes):view::build(owner.puzzleView(),meshes);
  check(built.bindings.production,"Accepted production meshes");const auto definition=rules::quest(quest);auto bindings=std::move(built.bindings);
  auto id=app.loadScene(std::move(built.scene));if(!previous)app.start(id);else{app.requestTransition(id,true);check(app.commitTransition(),"Production scene transition");}previous=id;auto& scene=*app.activeScene();
  if(q==6||q==9)waterState(scene,bindings,q==9,q==9);
  if(quest=="button_cushion_bed"){
   const auto* fence=dynamic_cast<engine::Actor*>(scene.findSavedNode("garden.enclosure.0"));
   check(fence&&fence->transform.position.z>3.5f,"Button's garden keeps its metre-scale courtyard around the cushion puzzle");
   const auto* gate=dynamic_cast<engine::Actor*>(scene.findSavedNode("garden.gate.frame"));
   check(gate&&gate->transform.position.x>5.f,"Button's garden gate stays outside the full courtyard");
   const auto a=view::world(quest,{0,0}),b=view::world(quest,{1,0});
   check(std::abs(b.x-a.x-.25f)<.0001f,"Cushion cells retain their quarter-metre spacing");
  }
  if(progression::isBeachQuest(quest)){
   for(unsigned stage=0;stage<6;++stage){auto* marker=scene.findActor(bindings.beachMilestones[stage]);check(marker&&marker->enabled==bool(owner.puzzleView().beachProgress&(1u<<stage)),"Cove milestones rebuild from saved progress");}
  }

  if(quest==progression::beachQuestId){
   check(scene.name=="A Seaside Afternoon - beach greybox","Beach owns a distinct scene");
   check(bindings.requesters.size()==1,"Pip accompanies the beach quest");
   for(const auto& binding:bindings.objects)if(binding.object=="basket_a"||binding.object=="basket_b"){
    const auto* a=scene.findActor(binding.actor);const auto* mesh=a->getComponent<engine::MeshComponent>();
    check(mesh&&mesh->name=="Beach basket greybox","Greybox baskets use solid grab surfaces");
    const auto hit=scene.selection().resolveRay({a->transform.position+engine::Vec3{0,3,0},{0,-1,0}});
    check(hit&&hit->actor==binding.actor,"Center grab selects basket rather than its covered channel");
   }
  }
  if(q==7){
   unsigned frames=0;scene.visit([&](const engine::Node& n,const engine::Mat4&){if(auto* a=dynamic_cast<const engine::Actor*>(&n);a&&a->name=="Garden bell frame"){++frames;check(!a->getComponent<engine::InteractionComponent>(),"Bell frame must not steal response clicks");}});
   check(frames==1,"Three bells share one imported frame");
   for(const auto& binding:bindings.objects)if(binding.object=="leaf"||binding.object=="star"||binding.object=="paw"){
    const auto* actor=scene.findActor(binding.actor);const auto p=actor->transform.position+engine::Vec3{0,-.20f,.24f};
    const auto hit=scene.selection().resolveRay({p+engine::Vec3{0,0,2},{0,0,-1}});
    check(hit&&hit->actor==binding.actor,"Visible Tripo symbol plaque selects its own bell");
   }
   check(bindings.sequencePictures.size()==3&&bindings.sequencePlayback.picture()=="leaf","Memory board starts with exactly the first clue");
   view::animate(scene,bindings,1);check(bindings.sequencePlayback.picture()=="star","Scene clock advances the blackboard picture");
   unsigned visible=0;for(const auto& [symbol,id]:bindings.sequencePictures){auto* a=scene.findActor(id);check(a!=nullptr,"Picture retains a checked scene handle");visible+=a->enabled;check(a->enabled==(symbol=="star"),"Scene displays only the current picture");}
   check(visible==1,"Multiple sequence pictures overlap");
   view::animate(scene,bindings,0);check(bindings.sequencePlayback.picture()=="star","Paused scene changes clue");
   check(owner.dispatchIntent(owner.intent(Verb::ReplaySequence)).accepted,"Replay is an authoritative journal command");
   view::project(scene,bindings,owner.puzzleView(),true);check(bindings.sequencePlayback.picture()=="leaf","Replay restarts at first picture even with reduced motion");
  }
  if(q==3){
   auto* path=scene.findActor(bindings.benchPath);check(path&&!path->children().empty(),"Bench scene exposes Button's route independently of reach sockets");
   auto* mark=dynamic_cast<engine::MeshComponent*>(path->children().front().get());check(mark&&mark->color.x>.6f,"Blocked route has warm inlays");
  }
  if(q==4){
   auto crossing=[&](std::string_view name)->engine::Actor&{for(const auto& b:bindings.objects)if(b.object==name)return *scene.findActor(b.actor);throw std::runtime_error("Missing crossing");};
   check(crossing("unsupported_socket").children().empty(),"Unsupported gap is not advertised as a supported crossing");
   auto& guide=*dynamic_cast<engine::MeshComponent*>(crossing("long_socket").children().front().get());
   const auto before=owner.encode();view::guidePlank(scene,bindings,owner.puzzleView(),"long_plank");
   check(guide.color.x>.8f,"Matching bridge receives drag guidance");
   check(owner.encode()==before,"Drag guidance cannot change progression or moves");
   view::project(scene,bindings,owner.puzzleView(),true);check(guide.color.x<.3f,"Cancellation restores quiet guidance");
  }
  if(q==8){
   auto actor=[&](std::string_view id)->engine::Actor&{for(const auto& b:bindings.objects)if(b.object==id)return *scene.findActor(b.actor);throw std::runtime_error("Missing playhouse actor");};
   check(actor("crate").getComponent<engine::InteractionComponent>()!=nullptr,"Cooperation crate is draggable");
   check(actor("pip_switch").transform.scale.x>2&&actor("inside_latch").getComponent<engine::MeshComponent>(),"Crate-sized switch and inside latch use real kit meshes");
   check(std::abs(actor("side_gate").transform.rotation.y)<.001f&&std::abs(actor("playhouse_door").transform.rotation.y-engine::pi)<.001f,"Both gates start closed");
   const auto unchanged=owner.encode();view::animate(scene,bindings,20);check(owner.encode()==unchanged,"Scene time cannot open gates or grant progress");
  }
  for(auto step:definition.solution){
   if(step.verb==Verb::Slide||step.verb==Verb::Fit||step.verb==Verb::AssignCat){
    const auto saved=owner.encode();const auto before=owner.puzzleView();const auto options=owner.movementOptions(step.object);
    const auto moving=std::find_if(before.objects.begin(),before.objects.end(),[&](const auto& o){return o.id==step.object;});
    check(std::any_of(options.begin(),options.end(),[&](const auto& i){return i.verb==step.verb&&(step.verb==Verb::Slide?i.destination==adjacent(moving->cell,step.direction):i.target==step.target&&(q!=5||i.destination==step.destination));}),"Every solution movement has a legal destination hint: "+step.object);
    check(owner.encode()==saved&&owner.puzzleView().revision==before.revision,"Guidance preserves attempt, journal and rewards");
    view::guideMovement(scene,bindings,before,options);
    if(q==8&&step.verb==Verb::Slide)for(const auto& option:options)if(option.destination==Cell{1,3}){
     const auto target=view::world(quest,{1,3},before.content);
     check(std::any_of(bindings.movementGuides.begin(),bindings.movementGuides.end(),[&](auto id){const auto* guide=scene.findActor(id);return guide->enabled&&std::abs(guide->transform.position.x-target.x)<.001f&&std::abs(guide->transform.position.z-target.z)<.001f&&guide->transform.position.y>.18f;}),"Crate destination border clears raised paw switch");
    }
    check(std::any_of(bindings.movementGuides.begin(),bindings.movementGuides.end(),[&](auto id){return scene.findActor(id)->enabled;}),"Legal destination has visible outline geometry");
    if(moving->role=="lookout_cushion")for(auto id:bindings.movementGuides){
     const auto* guide=scene.findActor(id);if(!guide->enabled)continue;
     check(guide->transform.position.y>.38f,"Cushion destination outline sits above the raised lookout seat");
    }
    view::guideMovement(scene,bindings,before,{});
    for(auto id:bindings.movementGuides)check(!scene.findActor(id)->enabled,"Hidden guidance clears every rectangle");
   }
   auto intent=owner.intent(step.verb,step.object,step.target);intent.direction=step.direction;intent.destination=step.destination;check(owner.dispatchIntent(intent).accepted,"Authoritative solution step");check(q<3?projectPuzzle(scene,bindings,owner.session(),true):view::project(scene,bindings,owner.puzzleView(),false),"Actual committed scene projection");
   if(q<3)for(int tick=0;tick<150;++tick)try{animatePuzzle(scene,bindings,1.f/60,false);}catch(const std::exception& ex){throw std::runtime_error(std::string(ex.what())+" in "+quest+" time "+std::to_string(bindings.ending.elapsed)+" walk "+std::to_string(bindings.ending.walkSeconds));}
  }
  if(q<3){for(int tick=0;tick<6000&&bindings.ending.active;++tick)try{animatePuzzle(scene,bindings,1.f/60,false);}catch(const std::exception& ex){throw std::runtime_error(std::string(ex.what())+" in "+quest+" time "+std::to_string(bindings.ending.elapsed)+" walk "+std::to_string(bindings.ending.walkSeconds));}
   check(bindings.ending.finished&&!bindings.ending.active,"Animated production ending reaches physical arrival");}
  check(owner.puzzleView().completed,"All ten known traces");check(app.loadedSceneCount()==1,"No retained outgoing scene");
  if(q==8){
   auto find=[&](engine::Scene& s,const SceneBindings& b,std::string_view id)->engine::Actor&{for(const auto& o:b.objects)if(o.object==id)return *s.findActor(o.actor);throw std::runtime_error("Missing playhouse projection");};
   check(find(scene,bindings,"playhouse_door").transform.rotation.y>4&&find(scene,bindings,"side_gate").transform.rotation.y<-1,"Successful playhouse projects both open gates");
   {const auto& door=find(scene,bindings,"playhouse_door");const auto& crate=find(scene,bindings,"crate");const auto* mesh=door.getComponent<engine::MeshComponent>();
    for(const auto& vertex:mesh->mesh->vertices){const auto p=engine::transformPoint(door.transform.matrix(),vertex.position);check(std::abs(p.x-crate.transform.position.x)>.48f||std::abs(p.z-crate.transform.position.z)>.48f,"Open doorway clears the weighted crate");}}
   for(auto id:{"pip","pepper"})check(bindings.collision.clear(scene,find(scene,bindings,id).transform.position),"Inside cat has physical clearance");
   auto undo=owner;auto restored=view::build(undo.puzzleView(),meshes,true);
   for(int n=0;n<2;++n){check(undo.dispatchIntent(undo.intent(Verb::Undo)).accepted,"Undo entry/latch");check(view::project(*restored.scene,restored.bindings,undo.puzzleView(),true),"Paused Undo projects playhouse immediately");}
   check(std::abs(find(*restored.scene,restored.bindings,"inside_latch").transform.position.y-.09f)<.001f,"Undo raises inside latch plate");
   check(undo.dispatchIntent(undo.intent(Verb::Restart)).accepted,"Restart playhouse");view::project(*restored.scene,restored.bindings,undo.puzzleView(),true);
   check(std::abs(find(*restored.scene,restored.bindings,"side_gate").transform.rotation.y)<.001f&&std::abs(find(*restored.scene,restored.bindings,"playhouse_door").transform.rotation.y-engine::pi)<.001f,"Restart closes both gates");
   const auto crateOffset=find(*restored.scene,restored.bindings,"crate").transform.position-view::world(quest,{5,1},undo.puzzleView().content);
   check(engine::dot(crateOffset,crateOffset)<.0001f,"Restart returns crate to storage");
  }
  if(q==3){auto* mark=dynamic_cast<engine::MeshComponent*>(scene.findActor(bindings.benchPath)->children().front().get());check(mark->color.x<.3f,"Cleared route receives quiet green inlays");}
  if(q==17){
   for(int tick=0;tick<3600;++tick)view::animate(scene,bindings,1./30);
   for(std::size_t n=0;n<bindings.requesters.size();++n){const auto& r=bindings.requesters[n];check(r.route.empty(),"Both friends finish their sunny cushion approach");auto seat=std::find_if(bindings.objects.begin(),bindings.objects.end(),[&](const auto& o){return o.object==(n==0?"bowl":"bed");});check(seat!=bindings.objects.end(),"Sunny seat exists");auto a=scene.findActor(r.actor)->transform.position,b=scene.findActor(seat->actor)->transform.position;check(std::abs(a.x-b.x)<.2f&&std::abs(a.z-b.z)<.2f,"Friend actually reaches its cushion");}
  }
  if((q>=3&&q<=7)||q==13||q==15||q==16){
   check(bindings.ending.active&&!bindings.ending.finished,"New completion starts a visible ending");
   auto* timingCat=scene.findActor(bindings.ending.actor);auto* timingSkin=timingCat->getComponent<engine::SkeletalMeshComponent>();
   static constexpr const char* endings[]={"quest_mouse_collect","quest_sun_stretch","quest_bed_knead","quest_drink_finish","quest_bell_touch"};
   check(std::abs(bindings.ending.clipSeconds-timingSkin->mesh->skeleton->clips[pipClip(*timingSkin->mesh,endings[q==13||q==15?1:q==16?3:q-3])].duration)<.001,"Ending uses full duration of selected clip");
   if((q==3||q==7)&&isToonCat(*timingSkin->mesh)){
    const auto saved=bindings.ending;const auto catPose=timingCat->transform;
    auto* toy=scene.findActor(saved.toy);const auto toyPose=toy->transform;
    auto sampleCue=[&](float t){bindings.ending=saved;bindings.ending.segment=saved.route.size()-1;timingCat->transform.position=saved.target;bindings.ending.elapsed=saved.leadSeconds+saved.walkSeconds+t;bindings.ending.facingComplete=true;view::project(scene,bindings,owner.puzzleView(),false);};
    if(q==3){sampleCue(1.9f);auto d=toy->transform.position-saved.toyStart;check(engine::dot(d,d)<.000001f,"Mouse stays still until the mouth reaches pickup pose");
     sampleCue(2.1f);d=toy->transform.position-saved.toyStart;check(engine::dot(d,d)>.0001f,"Mouse attaches during pickup, before the head rises");}
    else{sampleCue(1.25f);check(std::abs(toy->transform.rotation.z)<.00001f,"Bell stays still before paw extension");
     sampleCue(1.4f);check(std::abs(toy->transform.rotation.z)>.01f,"Bell reacts when the new paw reaches forward");}
    bindings.ending=saved;timingCat->transform=catPose;toy->transform=toyPose;
   }
   if(q==5&&isToonCat(*timingSkin->mesh)){
    const auto savedTransform=timingCat->transform;auto saved=bindings.ending;bindings.ending.elapsed=saved.leadSeconds+saved.walkSeconds+4.5f;bindings.ending.facingComplete=true;
    view::project(scene,bindings,owner.puzzleView(),false);
    engine::SkeletalMeshComponent expected("Expected uninterrupted lie-down",timingSkin->mesh);
    expected.animation().play(pipClip(*timingSkin->mesh,"quest_bed_knead"),false);expected.animation().seek(4.5);expected.animation().pause();
    auto actual=timingSkin->skinPose(),reference=expected.skinPose();
    for(std::size_t j=0;j<actual->bones.size();++j)for(int k=0;k<16;++k)check(std::abs(actual->bones[j].transform.m[k]-reference->bones[j].transform.m[k])<.001,"Bed ending must not replace lie-down with legacy spin");
    bindings.ending=saved;timingCat->transform=savedTransform;
   }
   const auto unchanged=owner.encode();view::animate(scene,bindings,0);check(bindings.ending.elapsed==0,"Pause holds ending clock");
   for(int tick=0;tick<3000&&bindings.ending.active;++tick)view::animate(scene,bindings,1./30);
   check(bindings.ending.finished&&!bindings.ending.active&&owner.encode()==unchanged,"Ending finishes within bounds without committing gameplay");
   view::project(scene,bindings,owner.puzzleView(),true);
   check(bindings.ending.finished&&!bindings.ending.active,"Reduced motion projects the final ending");
   if(q==6)waterState(scene,bindings,true);
   auto restoredEnding=view::build(owner.puzzleView(),meshes,false);check(restoredEnding.bindings.ending.finished,"Completed save restores without replaying the ending");
   if(q==6)waterState(*restoredEnding.scene,restoredEnding.bindings,true);
   auto restarted=owner;check(restarted.dispatchIntent(restarted.intent(Verb::Restart)).accepted,"Restart ending quest");
   check(view::project(*restoredEnding.scene,restoredEnding.bindings,restarted.puzzleView(),false),"Restart projects into completed scene");
   check(!restoredEnding.bindings.ending.active&&!restoredEnding.bindings.ending.finished,"Restart clears ending state");
   if(q==6)waterState(*restoredEnding.scene,restoredEnding.bindings,false);
   auto* resetCat=restoredEnding.scene->findActor(restoredEnding.bindings.ending.actor);
   const auto resetOffset=resetCat->transform.position-restoredEnding.bindings.ending.home;
   check(engine::dot(resetOffset,resetOffset)<.0001f,"Restart returns cat to waiting place");
   auto finalPosition=scene.findActor(bindings.ending.actor)->transform.position;
   view::project(scene,bindings,owner.puzzleView(),true);
   check(engine::dot(scene.findActor(bindings.ending.actor)->transform.position-finalPosition,scene.findActor(bindings.ending.actor)->transform.position-finalPosition)<.0001f,"Reprojection preserves final cat position");
  }
  if(q==4)for(const auto& b:bindings.objects)if(b.object=="long_socket"||b.object=="step_socket"||b.object=="finish_socket")for(const auto& child:scene.findActor(b.actor)->children())check(!child->enabled,"Fitted bridges hide empty-crossing guidance");
  if(q==5){ // Final cushion origins/rotations must match normalized occupied cells.
   for(auto& b:bindings.objects)if(b.object=="square"||b.object=="bar"||b.object=="elbow"){
    const auto v=owner.puzzleView();const auto& object=*std::find_if(v.objects.begin(),v.objects.end(),[&](auto& o){return o.id==b.object;});auto* a=scene.findActor(b.actor);auto* m=a->getComponent<engine::MeshComponent>();check(bool(m),"Final cushion mesh");
    for(auto vertex:m->mesh->vertices){auto p=engine::transformPoint(a->transform.matrix(),vertex.position);bool inside=false;for(auto c:object.footprint){auto origin=view::world(quest,c);if(p.x>=origin.x-.001f&&p.x<=origin.x+.251f&&p.z>=origin.z-.001f&&p.z<=origin.z+.251f)inside=true;}check(inside,"Final rotated cushion stays in authored occupancy");}
   }
  }
  if(q>=3){auto stale=owner.puzzleView();--stale.attempt;check(!view::project(scene,bindings,stale,true),"Stale attempt projection rejects");auto before=owner.encode();auto failed=meshes;failed.erase("crate");if(q==3){try{view::build(owner.puzzleView(),failed);check(false,"Missing required mesh rejected");}catch(const std::out_of_range&){}check(app.activeSceneId()==id&&owner.encode()==before,"Failed candidate retains scene and save");}}
  if(quest=="beach_picnic"){
   check(bindings.requesters.size()==4,"All four cats attend the picnic");
   for(const auto& requester:bindings.requesters){auto* skin=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(requester.skin));check(skin&&skin->animation().state()==engine::AnimationState::Paused&&skin->animation().time()>1&&!skin->animation().blending(),"Closing card projects a settled sitting pose without waiting for paused simulation");}
   const char* mats[]={"pip_mat","mabel_mat","button_mat","pepper_mat"};
   for(unsigned n=0;n<4;++n){const auto v=owner.puzzleView();const auto mat=std::find_if(v.objects.begin(),v.objects.end(),[&](const auto& o){return o.id==mats[n];});const auto* cat=scene.findActor(bindings.requesters[n].actor);const auto target=view::world(quest,mat->cell,v.content);check(std::abs(cat->transform.position.x-target.x)<.01f&&std::abs(cat->transform.position.z-target.z)<.01f,"Each cat settles at its assigned mat");}
  }
  check(owner.returnToGarden(owner.intent(Verb::Select)).accepted,"Return owner to garden");
 }
 check(owner.coins()==360,"Production traces share once-only ledger");auto restored=Afternoon::decode(owner.encode());check(restored.coins()==360&&!restored.picnicSeen(),"Finale remains ahead of recorded presentation");
 app.shutdown();std::cout<<checks<<" integration scene checks passed\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

