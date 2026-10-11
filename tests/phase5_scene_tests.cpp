#include "garden_scene.hpp"
#include "afternoon.hpp"
#include "engine/application.hpp"
#include "engine/glb_importer.hpp"
#include <iostream>
#include <stdexcept>
using namespace paws;
void check(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
int main() {try {
    GardenMeshes meshes;
    for(const auto& spec:gardenAssets()) {
        auto file=std::string(PAWS_SOURCE_DIR)+"/"+std::string(spec.file).replace(0,7,"assets/runtime/");
        meshes.emplace(spec.name,engine::GlbImporter::loadFile(file));
    }
    meshes.emplace("pip",engine::GlbImporter::loadFile(std::string(PAWS_SOURCE_DIR)+"/assets/runtime/models/pip/pip.glb"));
    auto garden=makeHomeGarden(meshes,false,true);
    check(garden.bindings.garden&&garden.bindings.production,"dedicated production garden");
    check(furnishingSpots().size()==3,"three authored spots");
    for(const auto& spot:furnishingSpots()) {
        check(garden.scene->findSavedNode(spot.id),"stable furnishing identity");
        check(spot.footprint.x>=.94f&&spot.footprint.z>=1.18f,"cushion footprint fits");
    }
    Afternoon afternoon;
    const auto send=[&](const SolutionStep& step) {
        const auto s=afternoon.session().state();return afternoon.dispatch({s.attempt,s.revision,step.action,step.object,step.direction,step.target});
    };
    check(send({Action::Move,"pot_a",Direction::North}).accepted,"first move");
    const auto attempt=afternoon.session().state().attempt;
    const auto oldRevision=afternoon.session().state().revision;
    check(afternoon.selectQuest(0)&&afternoon.session().state().attempt==attempt,"scene change preserves attempt identity");
    check(!afternoon.dispatch({attempt,oldRevision,Action::Roll,{}}).accepted,"scene change rejects queued old revision");
    check(send({Action::Garden,{}}).accepted,"suspend attempt");
    auto saved=Afternoon::decode(afternoon.encode());
    check(saved.session().state().inGarden&&saved.session().state().puzzle==afternoon.session().state().puzzle,"interrupted attempt restores");
    afternoon=std::move(saved);check(send({Action::Resume,{}}).accepted,"resume");
    check(send({Action::Restart,{}}).accepted,"restart");
    check(afternoon.session().state().attempt!=attempt,"restart creates a fresh attempt");
    for(const auto& step:afternoon.session().content().solution) check(send(step).accepted,"known solution");
    check(afternoon.coins()==20&&afternoon.friendship()==1&&afternoon.rewarded(0),"first reward committed with completion");
    check(send({Action::Undo,{}}).accepted,"undo completion");check(send({Action::Roll,{}}).accepted,"complete again");
    check(afternoon.coins()==20&&!afternoon.lastRewardGranted(),"reward cannot repeat across undo");
    auto restored=Afternoon::decode(afternoon.encode());
    check(restored.coins()==20&&restored.friendship()==1&&restored.session().state().puzzle.completed,"coherent completed restore");
    check(restored.session().state().attempt==afternoon.session().state().attempt&&restored.session().state().revision==afternoon.session().state().revision,"portable command identities persist");
    auto legacy=Afternoon::decode("PAWS_AFTERNOON 1 0 0 0 0 70 60 80\n\"missing-ball-v1\" 1\n1 \"pot_a\" 0 \"\"\n\"tidy-toys-v1\" 0\n\"stuck-gate-v1\" 0\n");
    check(legacy.session().state().puzzle.objects[0].cell.y==1&&legacy.coins()==0,"version 1 journal migrates without rewards");
    auto forged=afternoon.encode();auto ledger=forged.find(" 20 1 1 0 0\n");check(ledger!=std::string::npos,"reward fixture");
    forged.replace(ledger,std::string(" 20 1 1 0 0\n").size()," 40 2 1 0 0\n");bool invalidLedger=false;
    try {Afternoon::decode(forged);}catch(...){invalidLedger=true;}
    check(invalidLedger,"wallet and reward ledger must agree with committed history");
    auto home=makeHomeGarden(meshes,true,false);
    check(home.scene->findSavedNode("pip.returned_ball")&&!garden.scene->findSavedNode("pip.returned_ball"),"returned ball appears in garden");
    engine::Application app;
    auto first=app.loadScene(std::move(garden.scene));app.start(first);app.advance(0);
    const auto oldCat=garden.bindings.cat;
    auto next=app.loadScene(std::move(home.scene));app.requestTransition(next,true);check(app.commitTransition(),"prepared garden publication");
    check(!app.activeScene()->findComponent(oldCat),"old generation handle rejected");
    auto incomplete=meshes;incomplete.erase("bench");bool rejected=false;
    try {makeHomeGarden(incomplete,false,false);}catch(...){rejected=true;}
    check(rejected&&app.activeSceneId()==next,"failed candidate retains active garden");
    app.shutdown();std::cout<<"Phase 5 garden lifecycle, footprints, interrupted restore and idempotent coherent rewards passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
