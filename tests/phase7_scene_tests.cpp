#include "garden_scene.hpp"
#include "engine/application.hpp"
#include "engine/glb_importer.hpp"
#include <iostream>
#include <stdexcept>
using namespace paws;
void check(bool v,const char* message){if(!v)throw std::runtime_error(message);}
int main(){try {
    GardenMeshes meshes;
    for(const auto& spec:gardenAssets())meshes.emplace(spec.name,engine::GlbImporter::loadFile(
        std::string(PAWS_SOURCE_DIR)+"/"+std::string(spec.file).replace(0,7,"assets/runtime/")));
    meshes.emplace("pip",engine::GlbImporter::loadFile(std::string(PAWS_SOURCE_DIR)+"/assets/runtime/models/pip/pip.glb"));
    Afternoon afternoon;
    for(std::size_t q=0;q<3;++q){afternoon.selectQuest(q);for(const auto& step:afternoon.session().content().solution){auto s=afternoon.session().state();check(afternoon.dispatch({s.attempt,s.revision,step.action,step.object,step.direction,step.target}).accepted,"solve");}}
    auto s=afternoon.session().state();afternoon.dispatch({s.attempt,s.revision,Action::Garden,{}});
    const auto send=[&](Afternoon::GardenAction action,std::string item,std::string spot={}){auto st=afternoon.session().state();return afternoon.dispatchGarden({st.attempt,st.revision,action,item,spot});};
    send(Afternoon::GardenAction::Buy,"garden_cushion");send(Afternoon::GardenAction::Buy,"drinking_bowl");
    send(Afternoon::GardenAction::Place,"garden_cushion","bench_corner");send(Afternoon::GardenAction::Place,"drinking_bowl","sunny_corner");
    auto home=makeHomeGarden(meshes,true,false,&afternoon);auto& scene=*home.scene;
    const auto original=afternoon.encode();
    projectHomeGarden(scene,home.bindings,afternoon,"garden_cushion","quiet_corner");
    auto* cushion=scene.findActor(home.bindings.furnishings[0]);auto* preview=scene.findActor(home.bindings.furnishingPreviews[0]);
    check(cushion&&preview&&cushion->enabled&&preview->enabled,"committed and preview actors distinct");
    check(cushion->transform.position.x==-1.65f&&preview->transform.position.x==1.65f,"preview leaves original placement intact");
    projectHomeGarden(scene,home.bindings,afternoon);check(!preview->enabled&&cushion->enabled&&afternoon.encode()==original,"cancel discards preview without touching ownership or placement");
    auto use=beginGardenUse(scene,home.bindings,"garden_cushion","bench_corner",false);
    engine::Application app;auto id=app.loadScene(std::move(home.scene));app.start(id);
    engine::ApplicationFrameCallbacks callbacks;callbacks.simulate=[&](engine::Scene& scene,const auto& frame){updateGardenUse(scene,use,float(frame.simulation.delta));};
    double clock=0;app.advance(clock,callbacks);
    bool walking=false,settling=false,resting=false,returning=false;
    for(int n=0;n<2400&&use.phase;++n){
        const auto phase=use.phase;auto* actor=app.activeScene()->findActor(use.actor);
        const auto before=actor->transform.position;const auto segment=use.segment;
        clock+=.025;app.advance(clock,callbacks);
        if(phase==1&&use.phase==1&&segment==use.segment) {
            const auto d=actor->transform.position-before;
            check(std::abs(std::sqrt(engine::dot(d,d))-pipWalkSpeed*.025f)<.00005f,"walk translation matches authored stride speed");
        }
        walking|=use.phase==1;settling|=use.phase==2;resting|=use.phase==3;returning|=use.phase==5;
    }
    check(walking&&settling&&resting&&returning&&!use.phase,"complete bounded cushion use returns to idle");
    check(afternoon.encode()==original,"cat playback cannot change progress");
    use=beginGardenUse(*app.activeScene(),home.bindings,"drinking_bowl","sunny_corner",true);
    auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(app.activeScene()->findComponent(use.cat));
    check(use.phase==3&&cat&&cat->animation().state()==engine::AnimationState::Paused,"reduced motion shows readable static drinking pose");
    const auto position=app.activeScene()->findActor(use.actor)->transform.position;
    app.pause();clock+=1;app.advance(clock,callbacks);check(use.elapsed==0,"menu pause freezes use");
    app.resume();app.setSuspended(true);clock+=100;app.advance(clock,callbacks);check(use.elapsed==0,"host suspension drops hidden time");
    app.setSuspended(false);app.advance(clock,callbacks);
    for(int n=0;n<300&&use.phase;++n){clock+=.025;app.advance(clock,callbacks);}
    check(!use.phase,"static presentation has bounded duration");
    use=beginGardenUse(*app.activeScene(),home.bindings,"drinking_bowl","sunny_corner",false);
    auto next=makeHomeGarden(meshes,true,false,&afternoon);auto nextId=app.loadScene(std::move(next.scene));app.requestTransition(nextId,true);app.commitTransition();
    updateGardenUse(*app.activeScene(),use,1);check(!app.activeScene()->findActor(use.actor),"stale cat handle cannot alter replacement garden");
    check(position.y==gardenAnchor("bowl","drink_root").y,"authored bowl root alignment");
    app.shutdown();std::cout<<"Phase 7 preview cancellation, actual models, bounded cat use, pause/suspension and stale handles passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
