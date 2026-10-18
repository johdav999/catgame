#include "puzzle_scene.hpp"
#include "route_traversal.hpp"
#include "release1/props/prop_assets.hpp"
#include "engine/glb_importer.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
using namespace paws;
void check(bool value,const char* message) { if(!value) throw std::runtime_error(message); }
int main() {try {
    GardenMeshes meshes;
    for(const auto& spec:gardenAssets()) {
        auto mesh=engine::GlbImporter::loadFile(std::string(PAWS_SOURCE_DIR)+"/"+std::string(spec.file).replace(0,7,"assets/runtime/"));
        validateGardenAsset(spec,*mesh);meshes.emplace(spec.name,mesh);
    }
    meshes.emplace("pip",engine::GlbImporter::loadFile(std::string(PAWS_SOURCE_DIR)+"/assets/runtime/models/pip/pip.glb"));
    for(const auto& spec:release1::props::assets())if(std::string_view(spec.name)=="crate")
        meshes.emplace(spec.name,engine::GlbImporter::loadFile(std::string(PAWS_SOURCE_DIR)+"/"+std::string(spec.file).replace(0,7,"assets/runtime/")));
    const auto near=[](engine::Vec3 a,engine::Vec3 b){auto d=a-b;return engine::dot(d,d)<.000001f;};
    const auto ballCentre=gardenAnchor("ball","centre");
    const auto routePose=[&](const engine::Actor& ball){return engine::transformPoint(ball.transform.matrix(),ballCentre)-ballCentre;};
    {
        PawsSession session(missingBall());auto view=makePuzzleScene(session,&meshes);auto& scene=*view.scene;
        auto command=[&](Action action,const char* object,Direction direction=Direction::North){const auto& state=session.state();check(session.dispatch({state.attempt,state.revision,action,object,direction}).accepted,"Parking outline fixture command");check(projectPuzzle(scene,view.bindings,session),"Parking outline projection");};
        const auto visible=[&]{std::vector<Cell> cells;for(const auto& guide:view.bindings.slideGuides)if(scene.findActor(guide.actor)->enabled)cells.push_back(guide.cell);return cells;};
        check(visible().empty(),"Parking outlines stay hidden until an obstacle is selected");
        command(Action::Move,"pot_b",Direction::West);command(Action::Move,"pot_b");command(Action::Move,"pot_a");
        command(Action::Select,"pot_a");check(visible()==std::vector<Cell>{{1,2}},"Exact reported layout highlights only Left pot's backward escape");
        command(Action::Select,"pot_b");check(visible().empty(),"Blocked Middle pot never advertises an illegal drop");
        command(Action::Move,"pot_a",Direction::South);command(Action::Select,"pot_b");
        check(visible()==std::vector<Cell>{{2,1},{1,1}},"Freeing the lane updates reachable outlines, including the corner route");
        command(Action::Select,"channel_a");check(visible().empty(),"Selecting a channel clears obstacle hints");
        command(Action::Restart,"");check(visible().empty(),"Restart clears contextual parking outlines");
    }
    for(int version:{1,2}){
        PawsSession session(tidyToys(version));auto view=makePuzzleScene(session,&meshes);auto& scene=*view.scene;
        auto* pip=scene.findActor(view.bindings.ending.actor);
        check(pip->transform.position.x<-2.f&&std::abs(pip->transform.position.z)<.001f,"Pip waits on the logical access side");
        check(std::abs(pip->transform.rotation.y-engine::pi/2)<.001f,"Pip faces the toys through the entrance");
        const auto blockedRoute=view.bindings.collision.route(scene,pip->transform.position,{-.5f,0,1});
        check(blockedRoute.empty(),"Initial cushion physically blocks the only entrance to the toy area");
        auto state=session.state();check(session.dispatch({state.attempt,state.revision,Action::Move,"cushion",Direction::North}).accepted,"Original cushion north move remains legal");
        projectPuzzle(scene,view.bindings,session,false);view.bindings.collision.synchronize(scene);
        check(!view.bindings.collision.route(scene,pip->transform.position,{-1,0,1}).empty(),"Parking cushion north physically opens the lower pickup approach");
        if(version==2){state=session.state();check(session.dispatch({state.attempt,state.revision,Action::Move,"cushion",Direction::South}).accepted,"Restore centre parking");state=session.state();check(session.dispatch({state.attempt,state.revision,Action::Move,"cushion",Direction::South}).accepted,"Original lower parking remains legal");
            projectPuzzle(scene,view.bindings,session,false);view.bindings.collision.synchronize(scene);
            check(!view.bindings.collision.route(scene,pip->transform.position,{-1,0,-1}).empty(),"Parking cushion south physically opens the upper pickup approach");}
    }
    const std::array<engine::Vec3,4> samples{{{0,0,0},{0,0,0},{2,0,0},{2,0,2}}};
    check(near(sampleRoute(samples,3).position,{2,0,1}),"distance sampling spans duplicate points and corners");
    check(sampleRoute(samples,4).finished&&!sampleRoute(samples,3).finished,"arrival is exact and bounded");
    check(sampleRoute({},0).finished&&near(sampleRoute(samples,-1).position,{}),"empty and negative route samples");
    for(auto definition:{missingBall(),stuckGate()}) {
        PawsSession session(definition);auto view=makePuzzleScene(session,&meshes);
        if(definition.goal==Goal::Retrieve){
            auto* gate=dynamic_cast<engine::Actor*>(view.scene->findSavedNode("garden.gate"));
            auto gateBinding=std::find_if(view.bindings.objects.begin(),view.bindings.objects.end(),[](auto& b){return b.object=="gate";});
            check(gate&&gateBinding!=view.bindings.objects.end()&&gateBinding->actor==gate->id(),"quest controls the shared right boundary gate");
            for(const auto& v:gate->getComponent<engine::MeshComponent>()->mesh->vertices){
                const auto p=engine::transformPoint(gate->transform.matrix(),v.position);
                check(p.x>3.5f&&std::abs(p.z+1)<.85f,"closed gate fits right fence opening");
            }
            auto switchBinding=std::find_if(view.bindings.objects.begin(),view.bindings.objects.end(),[](auto& b){return b.object=="switch";});
            auto* plate=view.scene->findActor(switchBinding->actor);
            for(const auto& v:plate->getComponent<engine::MeshComponent>()->mesh->vertices){
                const auto p=engine::transformPoint(plate->transform.matrix(),v.position);
                check(p.x<3.2f,"pressure switch has clearance from the right fence");
            }
        }
        for(const auto& step:definition.solution) {
            if(step.action==Action::Roll)break;
            check(session.dispatch({session.state().attempt,session.state().revision,step.action,step.object,step.direction,step.target}).accepted,"prepare rolling route");
        }
        projectPuzzle(*view.scene,view.bindings,session);
        auto* ball=view.scene->findActor(view.bindings.ball);const auto start=ball->transform.position;
        engine::Actor* gate=nullptr;engine::Actor* trigger=nullptr;
        for(std::size_t i=0;i<definition.objects.size();++i) {
            if(definition.objects[i].kind==Kind::Gate)gate=view.scene->findActor(view.bindings.objects[i].actor);
            if(definition.objects[i].kind==Kind::Switch)trigger=view.scene->findActor(view.bindings.objects[i].actor);
        }
        const auto gateClosed=gate?gate->transform.rotation:engine::Vec3{};
        const auto triggerUp=trigger?trigger->transform.position:engine::Vec3{};
        check(session.dispatch({session.state().attempt,session.state().revision,Action::Roll,{}}).accepted,"roll accepted");
        projectPuzzle(*view.scene,view.bindings,session,true);
        const auto route=view.bindings.ballRoute;
        check(route.size()>10&&near(ball->transform.position,start),"roll starts at source with curved channel samples");
        if(gate)check(near(gate->transform.rotation,gateClosed)&&near(trigger->transform.position,triggerUp),"gate and trigger wait at roll start");
        animatePuzzle(*view.scene,view.bindings,.1f,false);
        check(!view.bindings.ballRoute.empty()&&!near(ball->transform.position,start),"ball visibly travels before arrival");
        check(std::abs(ball->transform.rotation.z)>0,"ball rotates while travelling");
        const auto midway=ball->transform.position;
        projectPuzzle(*view.scene,view.bindings,session,true);
        check(near(ball->transform.position,midway)&&view.bindings.ballRoute.size()==route.size(),"reprojection preserves active traversal");
        // Different frame partitions sample the same position along the route.
        animatePuzzle(*view.scene,view.bindings,.3f,false);
        check(near(routePose(*ball),sampleRoute(route,.4f*1.6f).position),"frame-independent travel speed around ball centre");
        // Sample multiple revolutions and both curved/straight sections. The
        // sphere's bottom must stay on the authored surface at every frame.
        while(!view.bindings.ballRoute.empty()) {
            animatePuzzle(*view.scene,view.bindings,.025f,false);
            if(gate)check(near(gate->transform.rotation,gateClosed)&&near(trigger->transform.position,triggerUp),"gate and trigger stay closed until ball arrival");
            const auto expected=sampleRoute(route,view.bindings.ballElapsed*1.6f).position;
            check(near(routePose(*ball),expected),"rolling centre stays inside channel route");
            const auto centre=engine::transformPoint(ball->transform.matrix(),ballCentre);
            check(std::abs(centre.y-.105f-expected.y)<.00001f,"rolling ball bottom never dips through channel floor");
        }
        if(gate) {
            animatePuzzle(*view.scene,view.bindings,.25f,false);
            check(!near(gate->transform.rotation,gateClosed)&&!near(trigger->transform.position,triggerUp),"arrival opens gate and depresses trigger");
        }
        projectPuzzle(*view.scene,view.bindings,session,true);
        check(near(routePose(*ball),route.back()),"arrival reprojection preserves centred rotation");
        check(session.dispatch({session.state().attempt,session.state().revision,Action::Undo,{}}).accepted,"undo mid-roll");
        projectPuzzle(*view.scene,view.bindings,session,false);
        check(view.bindings.ballRoute.empty()&&near(ball->transform.position,start),"undo cancels roll and restores source");
        if(gate)check(near(gate->transform.rotation,gateClosed)&&view.bindings.arrivalMotion.empty(),"undo closes gate and cancels pending arrival");
        check(session.dispatch({session.state().attempt,session.state().revision,Action::Roll,{}}).accepted,"roll again");
        if(gate) {
            projectPuzzle(*view.scene,view.bindings,session,true);animatePuzzle(*view.scene,view.bindings,.1f,false);
            check(session.dispatch({session.state().attempt,session.state().revision,Action::Undo,{}}).accepted,"undo during traversal");
            projectPuzzle(*view.scene,view.bindings,session,true);animatePuzzle(*view.scene,view.bindings,10,false);
            check(near(gate->transform.rotation,gateClosed)&&view.bindings.arrivalMotion.empty()&&view.bindings.ballRoute.empty(),"cancelled roll never opens gate later");
            check(session.dispatch({session.state().attempt,session.state().revision,Action::Roll,{}}).accepted,"roll after cancellation");
        }
        projectPuzzle(*view.scene,view.bindings,session,true);animatePuzzle(*view.scene,view.bindings,0,true);
        const auto reducedEnd=route.back()+(definition.goal==Goal::Route?engine::Vec3{-.20f,0,0}:engine::Vec3{});
        check(view.bindings.ballRoute.empty()&&near(routePose(*ball),reducedEnd),"reduced motion arrives at the completed reaction pose immediately");
        if(gate)check(!near(gate->transform.rotation,gateClosed)&&view.bindings.arrivalMotion.empty(),"reduced motion opens gate with arrival");
        check(session.dispatch({session.state().attempt,session.state().revision,Action::Restart,{}}).accepted,"restart after roll");
        projectPuzzle(*view.scene,view.bindings,session,false);
        check(view.bindings.ballRoute.empty()&&near(ball->transform.position,start),"restart restores source");
    }
    for(auto definition:{missingBall(),tidyToys(),stuckGate()}) {
        PawsSession session(definition);auto view=makePuzzleScene(session,&meshes);
        auto second=makePuzzleScene(session,&meshes);
        for(const auto& step:definition.solution) {
            check(session.dispatch({session.state().attempt,session.state().revision,step.action,step.object,step.direction,step.target}).accepted,"solution dispatch");
            check(projectPuzzle(*view.scene,view.bindings,session,true),"production projection");
            animatePuzzle(*view.scene,view.bindings,10,false);
        }
        for(int tick=0;tick<3000&&view.bindings.ending.active;++tick)animatePuzzle(*view.scene,view.bindings,1.f/30,false);
        check(view.bindings.motion.empty()&&view.bindings.ballRoute.empty(),"presentation catches latest commit");
        check(view.bindings.ending.finished&&!view.bindings.ending.active,"Each requested ending reaches a bounded final pose");
        auto restored=makePuzzleScene(session,&meshes);
        check(restored.bindings.ending.finished&&!restored.bindings.ending.active,"Completed save restores final pose without replaying celebration");
        for(std::size_t i=0;i<definition.objects.size();++i) {
            const auto& object=definition.objects[i];const auto& state=session.state().puzzle.objects[i];
            auto* actor=view.scene->findActor(view.bindings.objects[i].actor);
            auto* immutable=second.scene->findActor(second.bindings.objects[i].actor);
            if(object.kind!=Kind::Socket) check(actor->getComponent<engine::MeshComponent>()->mesh==immutable->getComponent<engine::MeshComponent>()->mesh,"immutable meshes shared between attempts");
            if(state.location==Location::Socket) {
                auto anchor=gardenAnchor("basket",object.shape==Shape::Ball?"ball_socket":object.shape==Shape::Mouse?"mouse_socket":"fish_socket");
                check(std::abs(actor->transform.position.x-(definition.packedToysBlockAccess?2:1))<.001f&&std::abs(actor->transform.position.z-anchor.x*(definition.packedToysBlockAccess?1.9f:1.f))<.001f&&
                      std::abs(actor->transform.rotation.y+engine::pi/2)<.001f,"toy matches rotated basket anchor and silhouette");
            }
            if(object.kind==Kind::Gate){
                check(std::abs(actor->transform.rotation.y-(engine::pi/2+100*engine::pi/180))<.001f,"gate opens around authored hinge");
                for(const auto& v:actor->getComponent<engine::MeshComponent>()->mesh->vertices){
                    const auto p=engine::transformPoint(actor->transform.matrix(),v.position);
                    check(std::abs(p.z+1)<.85f,"open gate stays clear of both right fence sections");
                }
            }
            if(object.kind==Kind::Switch) check(std::abs(actor->transform.position.y-.055f)<.001f,"switch depresses from occupancy");
            if(channel(object.kind)) {
                auto asset=object.kind==Kind::Straight?"channel_straight":"channel_curve";
                for(auto name:{"port_a","port_b"}) {
                    auto port=engine::transformPoint(actor->transform.matrix(),gardenAnchor(asset,name));bool matched=false;
                    for(auto direction:ports(object.kind,state.orientation)) {
                        auto neighbor=adjacent(state.cell,direction);
                        engine::Vec3 expected{float(state.cell.x-2)+(neighbor.x-state.cell.x)*.5f,.06f,float(state.cell.y-1)+(neighbor.y-state.cell.y)*.5f};
                        auto delta=port-expected;matched|=engine::dot(delta,delta)<.0001f;
                    }
                    check(matched,"visual channel ports agree with logical route");
                }
            }
        }
        auto state=session.state();check(session.dispatch({state.attempt,state.revision,Action::Undo,{}}).accepted,"undo");
        check(projectPuzzle(*view.scene,view.bindings,session,true),"undo projects");animatePuzzle(*view.scene,view.bindings,0,true);
        check(!view.bindings.ending.active&&!view.bindings.ending.finished,"Undo cancels the completed ending");
        check(view.bindings.motion.empty(),"reduced motion has no pending transform animation");
    }
    std::cout<<"Phase 6 production scenes: shared immutable meshes, basket alignment, route ports, switch/gate anchors, animation and undo passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
