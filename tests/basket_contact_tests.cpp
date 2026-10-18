#include "puzzle_scene.hpp"
#include "pip_asset.hpp"
#include "engine/glb_importer.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
using namespace paws;
namespace {
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
const engine::Actor* basket(const engine::Scene& scene){const engine::Actor* found=nullptr;scene.visit([&](const engine::Node& n,const engine::Mat4&){if(auto* a=dynamic_cast<const engine::Actor*>(&n);a&&a->name=="basket")found=a;});return found;}
engine::MeshBounds bounds(const engine::Actor& a){
    const auto* mesh=a.getComponent<engine::MeshComponent>();engine::MeshBounds b;bool first=true;
    for(const auto& v:mesh->mesh->vertices){const auto p=engine::transformPoint(a.transform.matrix(),v.position);if(first){b.minimum=b.maximum=p;first=false;}
        b.minimum={std::min(b.minimum.x,p.x),std::min(b.minimum.y,p.y),std::min(b.minimum.z,p.z)};b.maximum={std::max(b.maximum.x,p.x),std::max(b.maximum.y,p.y),std::max(b.maximum.z,p.z)};}
    b.valid=!first;return b;
}
float intrusion(engine::SkeletalMeshComponent& skin,const engine::Actor& cat,const engine::MeshBounds& rim,bool left){
    const auto pose=skin.skinPose();float worst=-100;
    for(std::size_t v=0;v<skin.mesh->vertices.size();++v){engine::Vec3 local{};
        for(int k=0;k<4;++k)local=local+engine::transformPoint(pose->bones[skin.mesh->skin[v].joints[k]].transform,skin.mesh->vertices[v].position)*skin.mesh->skin[v].weights[k];
        const auto p=engine::transformPoint(cat.transform.matrix(),local);
        // The full low silhouette includes toes and lower legs, even where the
        // animated limb extends beyond the root-centred navigation capsule.
        if(p.y<=rim.maximum.y&&p.z>=rim.minimum.z&&p.z<=rim.maximum.z)worst=std::max(worst,left?p.x-rim.minimum.x:rim.maximum.x-p.x);
    }return worst;
}
bool feetInside(engine::SkeletalMeshComponent& skin,const engine::Actor& cat,const engine::MeshBounds& rim){
    const auto pose=skin.skinPose();
    for(std::size_t v=0;v<skin.mesh->vertices.size();++v){engine::Vec3 local{};
        for(int k=0;k<4;++k)local=local+engine::transformPoint(pose->bones[skin.mesh->skin[v].joints[k]].transform,skin.mesh->vertices[v].position)*skin.mesh->skin[v].weights[k];
        const auto p=engine::transformPoint(cat.transform.matrix(),local);
        if(p.y<=rim.maximum.y&&p.x>rim.minimum.x&&p.x<rim.maximum.x&&p.z>rim.minimum.z&&p.z<rim.maximum.z)return true;
    }return false;
}
}
int main(){try{
    GardenMeshes meshes;
    for(const auto& spec:gardenAssets())meshes[spec.name]=engine::GlbImporter::loadFile(std::string(PAWS_SOURCE_DIR)+"/"+std::string(spec.file).replace(0,7,"assets/runtime/"));
    // First establish that the actual scaled/rotated basket already is a solid.
    engine::Scene collisionScene;auto& solid=collisionScene.spawn("basket");solid.transform.position={2,0,0};solid.transform.rotation.y=-engine::pi/2;solid.transform.scale.x=1.9f;
    solid.addComponent<engine::MeshComponent>("Basket",meshes.at("basket"));CatCollision collision;collision.build(collisionScene,meshes);
    check(collision.solids()==1&&!collision.clearSegment(collisionScene,{0,0,0},{2,0,0}),"Basket collision must stop a large walking step");
    for(const auto asset:{"pip/pip.glb","release1/cats/pip/pip.glb"})for(const int version:{1,2}){
        meshes["pip"]=engine::GlbImporter::loadFile(std::string(PAWS_SOURCE_DIR)+"/assets/runtime/models/"+asset);
        PawsSession session(tidyToys(version));for(const auto& s:session.content().solution)check(session.dispatch({session.state().attempt,session.state().revision,s.action,s.object,s.direction,s.target}).accepted,"Solve basket fixture");
        auto view=makePuzzleScene(session,&meshes);auto* cat=view.scene->findActor(view.bindings.ending.actor);auto* skin=dynamic_cast<engine::SkeletalMeshComponent*>(view.scene->findComponent(view.bindings.cat));
        const auto rim=bounds(*basket(*view.scene));check(view.bindings.ending.finished,"Restored completion is settled");
        const bool left=cat->transform.position.x<(rim.minimum.x+rim.maximum.x)*.5f;float peak=-100;
        for(int frame=0;frame<=162;++frame){skin->animation().play(pipClip(*skin->mesh,"quest_basket_finish"),false);skin->animation().seek(frame/30.);skin->animation().pause();peak=std::max(peak,intrusion(*skin,*cat,rim,left));}
        std::cout<<asset<<" v"<<version<<" root="<<cat->transform.position.x<<" maximum low-silhouette intrusion="<<peak<<std::endl;
        check(peak<=-.015f,"Animated paws/lower legs must stay outside the basket rim, including the final reward pose");
        PawsSession fresh(tidyToys(version));auto moving=makePuzzleScene(fresh,&meshes);
        check(projectPuzzle(*moving.scene,moving.bindings,session,true),"Start actual animated completion");
        auto* walker=moving.scene->findActor(moving.bindings.ending.actor);auto* walkingSkin=dynamic_cast<engine::SkeletalMeshComponent*>(moving.scene->findComponent(moving.bindings.cat));
        for(int tick=0;tick<2400&&moving.bindings.ending.active;++tick){animatePuzzle(*moving.scene,moving.bindings,1.f/30,false);if(tick%3==0)check(!feetInside(*walkingSkin,*walker,rim),"Walking or turning puts a foot inside the basket");}
        check(moving.bindings.ending.finished,"Physical arrival still reaches the bounded completion pose");
        check(session.dispatch({session.state().attempt,session.state().revision,Action::Undo,{}}).accepted,"Undo completion");
        projectPuzzle(*moving.scene,moving.bindings,session,false);check(!moving.bindings.ending.finished&&!moving.bindings.ending.active,"Undo cancels the old basket contact");
        const auto& last=session.content().solution.back();check(session.dispatch({session.state().attempt,session.state().revision,last.action,last.object,last.direction,last.target}).accepted,"Complete after undo");
        projectPuzzle(*moving.scene,moving.bindings,session,true);animatePuzzle(*moving.scene,moving.bindings,0,true);
        check(moving.bindings.ending.finished&&!feetInside(*walkingSkin,*walker,rim),"Reduced motion reaches the same clear basket contact");
    }
    std::cout<<"Actual basket collision and animated/restored paw clearance passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
