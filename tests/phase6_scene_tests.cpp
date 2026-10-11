#include "puzzle_scene.hpp"
#include "route_traversal.hpp"
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
    const auto near=[](engine::Vec3 a,engine::Vec3 b){auto d=a-b;return engine::dot(d,d)<.000001f;};
    const std::array<engine::Vec3,4> samples{{{0,0,0},{0,0,0},{2,0,0},{2,0,2}}};
    check(near(sampleRoute(samples,3).position,{2,0,1}),"distance sampling spans duplicate points and corners");
    check(sampleRoute(samples,4).finished&&!sampleRoute(samples,3).finished,"arrival is exact and bounded");
    check(sampleRoute({},0).finished&&near(sampleRoute(samples,-1).position,{}),"empty and negative route samples");
    for(auto definition:{missingBall(),stuckGate()}) {
        PawsSession session(definition);auto view=makePuzzleScene(session,&meshes);
        for(const auto& step:definition.solution) {
            if(step.action==Action::Roll)break;
            check(session.dispatch({session.state().attempt,session.state().revision,step.action,step.object,step.direction,step.target}).accepted,"prepare rolling route");
        }
        projectPuzzle(*view.scene,view.bindings,session);
        auto* ball=view.scene->findActor(view.bindings.ball);const auto start=ball->transform.position;
        check(session.dispatch({session.state().attempt,session.state().revision,Action::Roll,{}}).accepted,"roll accepted");
        projectPuzzle(*view.scene,view.bindings,session,true);
        const auto route=view.bindings.ballRoute;
        check(route.size()>10&&near(ball->transform.position,start),"roll starts at source with curved channel samples");
        animatePuzzle(*view.scene,view.bindings,.1f,false);
        check(!view.bindings.ballRoute.empty()&&!near(ball->transform.position,start),"ball visibly travels before arrival");
        check(std::abs(ball->transform.rotation.z)>0,"ball rotates while travelling");
        const auto midway=ball->transform.position;
        projectPuzzle(*view.scene,view.bindings,session,true);
        check(near(ball->transform.position,midway)&&view.bindings.ballRoute.size()==route.size(),"reprojection preserves active traversal");
        // Different frame partitions sample the same position along the route.
        animatePuzzle(*view.scene,view.bindings,.3f,false);
        check(near(ball->transform.position,sampleRoute(route,.4f*1.6f).position),"frame-independent travel speed");
        check(session.dispatch({session.state().attempt,session.state().revision,Action::Undo,{}}).accepted,"undo mid-roll");
        projectPuzzle(*view.scene,view.bindings,session,false);
        check(view.bindings.ballRoute.empty()&&near(ball->transform.position,start),"undo cancels roll and restores source");
        check(session.dispatch({session.state().attempt,session.state().revision,Action::Roll,{}}).accepted,"roll again");
        projectPuzzle(*view.scene,view.bindings,session,true);animatePuzzle(*view.scene,view.bindings,0,true);
        check(view.bindings.ballRoute.empty()&&near(ball->transform.position,route.back()),"reduced motion arrives immediately");
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
        check(view.bindings.motion.empty()&&view.bindings.ballRoute.empty(),"presentation catches latest commit");
        for(std::size_t i=0;i<definition.objects.size();++i) {
            const auto& object=definition.objects[i];const auto& state=session.state().puzzle.objects[i];
            auto* actor=view.scene->findActor(view.bindings.objects[i].actor);
            auto* immutable=second.scene->findActor(second.bindings.objects[i].actor);
            if(object.kind!=Kind::Socket) check(actor->getComponent<engine::MeshComponent>()->mesh==immutable->getComponent<engine::MeshComponent>()->mesh,"immutable meshes shared between attempts");
            if(state.location==Location::Socket) {
                auto anchor=gardenAnchor("basket",object.shape==Shape::Ball?"ball_socket":object.shape==Shape::Mouse?"mouse_socket":"fish_socket");
                check(std::abs(actor->transform.position.x-1)<.001f&&std::abs(actor->transform.position.z-anchor.x)<.001f&&
                      std::abs(actor->transform.rotation.y+engine::pi/2)<.001f,"toy matches rotated basket anchor and silhouette");
            }
            if(object.kind==Kind::Gate) check(std::abs(actor->transform.rotation.y-(engine::pi/2+100*engine::pi/180))<.001f,"gate opens around authored hinge");
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
        check(view.bindings.motion.empty(),"reduced motion has no pending transform animation");
    }
    std::cout<<"Phase 6 production scenes: shared immutable meshes, basket alignment, route ports, switch/gate anchors, animation and undo passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
