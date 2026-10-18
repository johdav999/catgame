#include "cat_collision.hpp"
#include "engine/glb_importer.hpp"
#include "engine/mesh_component.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace {
int checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
float distance(engine::Vec3 a,engine::Vec3 b){auto d=a-b;return std::sqrt(engine::dot(d,d));}
}
int main(){try{
    paws::GardenMeshes meshes;
    auto root=std::filesystem::path(PAWS_SOURCE_DIR)/"assets/runtime/models/garden";
    for(auto name:{"cushion","ball","bowl","bench"})meshes[name]=engine::GlbImporter::loadFile(root/(std::string(name)+".glb"));
    engine::Scene scene;auto& cushion=scene.spawn("Cushion");cushion.addComponent<engine::MeshComponent>("Cushion",meshes.at("cushion"));
    paws::CatCollision collision;collision.build(scene,meshes);
    const engine::Vec3 start{-2,0,0},finish{2,0,0};
    check(!collision.clearSegment(scene,start,finish),"A flat walk cannot cross the cushion");
    auto stopped=collision.constrain(scene,start,finish);
    check(stopped.x<-.45f&&collision.clear(scene,stopped),"Even one large displacement stops outside the cushion");
    auto route=collision.route(scene,start,finish);check(route.size()>2,"Walking plans a detour, rather than stepping over arbitrary cushions");
    auto& cat=scene.spawn("Cat");cat.transform.position=start;std::size_t segment=0;
    for(int frame=0;frame<1000&&segment+1<route.size();++frame){auto before=cat.transform.position;
        collision.follow(scene,cat,route,segment,.015f);
        check(collision.clearSegment(scene,before,cat.transform.position)&&collision.clear(scene,cat.transform.position),"Each actual detour step stays outside solids");}
    check(distance(cat.transform.position,finish)<.0001f,"A blocked straight path still reaches its destination");
    // Large frame steps must traverse corners, not take their obstructed chord.
    cat.transform.position=start;segment=0;check(collision.follow(scene,cat,route,segment,100),"Large frame consumes the checked detour corners");
    auto entry=collision.supportedRoute(scene,{start,{0,.16f,0}});
    check(!entry.empty(),"Cushion remains usable through a checked lift and landing");
    cat.transform.position=start;segment=0;check(collision.follow(scene,cat,entry,segment,100),"Cat reaches the measured supported cushion root");
    check(distance(cat.transform.position,{0,.16f,0})<.0001f&&collision.clear(scene,cat.transform.position),"Supported use does not enter cushion volume");
    std::reverse(entry.begin(),entry.end());segment=0;check(collision.follow(scene,cat,entry,segment,100),"Cat can leave the furnishing over the same safe support path");
    cushion.enabled=false;collision.synchronize(scene);check(collision.clearSegment(scene,start,finish),"Disabled furnishing stops blocking immediately");
    cushion.enabled=true;cushion.transform.position={0,0,2};collision.synchronize(scene);
    check(collision.clearSegment(scene,start,finish),"Moved cushion releases its old position");
    check(!collision.clearSegment(scene,{-2,0,2},{2,0,2}),"Moved cushion blocks at its new position");
    cushion.destroy();collision.synchronize(scene);check(collision.clearSegment(scene,{-2,0,2},{2,0,2}),"Destroyed source cannot leave a ghost solid");
    engine::Scene ballScene;auto& ball=ballScene.spawn("Ball");ball.addComponent<engine::MeshComponent>("Ball",meshes.at("ball"));
    paws::CatCollision balls;balls.build(ballScene,meshes);
    check(!balls.clearSegment(ballScene,start,finish),"A small ball blocks walking even during a large frame step");
    auto around=balls.route(ballScene,start,finish);check(around.size()>2,"Cat routes around the ball without moving puzzle state");
    ball.transform.position={1,0,1};ball.transform.rotation.x=2;balls.synchronize(ballScene);
    check(balls.clearSegment(ballScene,start,finish),"Rolling or scripted ball synchronizes its collision position");
    ball.transform.position={0,0,0};ball.transform.rotation={};balls.synchronize(ballScene);
    ball.transform.scale={2,2,2};balls.synchronize(ballScene);
    check(!balls.clear(ballScene,{.3f,0,0}),"Scaled ball collider matches its visible radius");
    ball.transform.scale={1,1,1};balls.synchronize(ballScene);
    auto& walker=ballScene.spawn("Walker");walker.transform.position=start;segment=0;
    std::vector<engine::Vec3> stale{start,finish};check(!balls.follow(ballScene,walker,stale,segment,100),"A newly blocked route does not report arrival");
    check(walker.transform.position.x<0&&segment==0,"Collision stops movement before advancement to a use phase");
    // A purely visual placement preview must not block a walking cat.
    engine::Scene previews;auto& preview=previews.spawn("Preview");preview.setSavedId("preview.garden_cushion");preview.addComponent<engine::MeshComponent>("Preview",meshes.at("cushion"));
    paws::CatCollision ignored;ignored.build(previews,meshes);check(ignored.solids()==0&&ignored.clearSegment(previews,start,finish),"Placement previews have no physical authority");
    std::cout<<checks<<" cat collision checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
