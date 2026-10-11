#include "garden_scene.hpp"
#include "engine/light_component.hpp"
#include <cmath>
#include <algorithm>
namespace paws {
namespace {
constexpr FurnishingSpot spots[]{
    {gardenSpots[0].id.data(),{gardenSpots[0].position.x,0,gardenSpots[0].position.z},{gardenSpots[0].width,0,gardenSpots[0].depth}},
    {gardenSpots[1].id.data(),{gardenSpots[1].position.x,0,gardenSpots[1].position.z},{gardenSpots[1].width,0,gardenSpots[1].depth}},
    {gardenSpots[2].id.data(),{gardenSpots[2].position.x,0,gardenSpots[2].position.z},{gardenSpots[2].width,0,gardenSpots[2].depth}}
};
}
std::span<const FurnishingSpot> furnishingSpots() { return spots; }
PuzzleScene makeHomeGarden(const GardenMeshes& meshes,bool returned,bool reduced,const Afternoon* afternoon) {
    for(const auto& spec:gardenAssets()) validateGardenAsset(spec,*meshes.at(spec.name));
    validatePip(*meshes.at("pip"));
    PuzzleScene out;out.scene=std::make_unique<engine::Scene>("Pip's courtyard");
    out.bindings.production=true;out.bindings.garden=true;out.bindings.reducedMotion=reduced;
    auto& scene=*out.scene;scene.ambient={.33f,.37f,.40f};scene.environmentIntensity=.65f;
    const auto prop=[&](const char* asset,const std::string& name,engine::Vec3 p,float yaw=0.f)->engine::Actor& {
        auto& a=scene.spawn(name);a.transform.position=p;a.transform.rotation.y=yaw;
        a.addComponent<engine::MeshComponent>(name,meshes.at(asset)).color={1,1,1};return a;
    };
    for(int x=-3;x<3;++x) for(int z=-2;z<2;++z)
        prop("paving","Courtyard sandstone",{x+.5f,0,z+.5f});
    for(int x=-3;x<3;++x) {
        prop("fence","Low garden boundary",{x+.5f,0,-2});
        prop("edge","Courtyard front edge",{x+.5f,0,2});
    }
    for(int z=-2;z<2;++z) for(float x:{-3.f,3.f})
        prop("edge","Courtyard side edge",{x,0,z+.5f},engine::pi/2);
    prop("bench","Pip's bench",{-.6f,0,-1.5f});
    prop("planting","Left flowers",{-2.6f,0,-1.45f});
    prop("planting","Right flowers",{2.6f,0,-1.45f});
    prop("pot_round","Garden terracotta",{-2.65f,0,1.45f});
    prop("pot_fluted","Garden fluted pot",{2.65f,0,1.45f});
    // Code-native ground cues reserve compatible placement footprints for phase 7.
    for(const auto& spot:spots) {
        auto& a=scene.spawn(spot.id);a.setSavedId(spot.id);a.transform.position=spot.position+engine::Vec3{0,.006f,0};
        auto outline=std::make_shared<engine::Material>();outline->shader=engine::MaterialShader::Unlit;
        outline->baseColor={.43f,.55f,.44f};outline->name="Quiet furnishing outline";
        for(int side=0;side<4;++side) {
            auto& line=a.addComponent<engine::MeshComponent>("Empty furnishing outline",engine::Mesh::cube());
            const bool alongX=side<2;
            line.transform.position=alongX?engine::Vec3{0,0,(side?1.f:-1.f)*spot.footprint.z/2}:
                engine::Vec3{(side==2?-1.f:1.f)*spot.footprint.x/2,0,0};
            line.transform.scale=alongX?engine::Vec3{spot.footprint.x,.008f,.018f}:engine::Vec3{.018f,.008f,spot.footprint.z};
            line.setMaterial(0,engine::MaterialInstance{outline});line.color={1,1,1};line.castsShadow=false;
        }
    }
    auto& catActor=scene.spawn("Pip");catActor.setSavedId("pip");catActor.transform.position={0,0,.25f};
    auto& cat=catActor.addComponent<engine::SkeletalMeshComponent>("Pip",meshes.at("pip"));
    cat.color={1,1,1};out.bindings.cat=cat.id();out.bindings.gardenCat=catActor.id();presentPip(cat,0,reduced);
    for(std::size_t i=0;i<shopItems.size();++i) {
        const auto& item=shopItems[i];
        for(bool preview:{false,true}) {
            auto& a=prop(item.asset.data(),preview?"Furnishing preview":std::string(item.name),{});
            a.setSavedId(std::string(preview?"preview.":"furnishing.")+std::string(item.id));
            if(item.id=="drinking_bowl") {
                auto& water=a.addComponent<engine::MeshComponent>("Cool water",meshes.at("bowl_water"));
                water.color={1,1,1};water.transform.position=gardenAnchor("bowl","water");
            }
            a.enabled=false;
            (preview?out.bindings.furnishingPreviews:out.bindings.furnishings)[i]=a.id();
        }
    }
    if(afternoon)projectHomeGarden(scene,out.bindings,*afternoon);
    if(returned) prop("ball","Pip's returned ball",{-.65f,0,.45f}).setSavedId("pip.returned_ball");
    auto& sun=scene.spawn("Warm garden sun").addComponent<engine::LightComponent>("Sun",engine::LightType::Directional);
    sun.direction={-.5f,-1,-.6f};sun.color={1,.95f,.87f};sun.intensity=1.7f;
    auto& camera=scene.spawn("Garden view").addComponent<engine::CameraComponent>("Camera");
    camera.setPerspective(engine::pi*40/180,.05f,60);out.camera=&camera;
    return out;
}
void projectHomeGarden(engine::Scene& scene,SceneBindings& b,const Afternoon& afternoon,std::string_view previewItem,std::string_view previewSpot) {
    if(!b.garden)return;
    for(const auto& spot:gardenSpots)if(auto* node=scene.findSavedNode(spot.id)) {
        if(auto* actor=dynamic_cast<engine::Actor*>(node))for(auto* line:actor->getComponents<engine::MeshComponent>()) {
            auto material=std::make_shared<engine::Material>();material->shader=engine::MaterialShader::Unlit;material->name="Furnishing footprint";
            const bool selected=spot.id==previewSpot;
            material->baseColor=selected?(afternoon.placementBlockage(previewItem,spot.id).empty()?engine::Vec3{.08f,.4f,.34f}:engine::Vec3{.66f,.18f,.12f}):engine::Vec3{.43f,.55f,.44f};
            line->setMaterial(0,engine::MaterialInstance{material});
        }
    }
    for(std::size_t i=0;i<shopItems.size();++i) {
        if(auto* a=scene.findActor(b.furnishings[i])) {
            const auto* spot=gardenSpot(afternoon.placement(shopItems[i].id));a->enabled=spot;
            if(spot)a->transform.position={spot->position.x,0,spot->position.z};
        }
        if(auto* a=scene.findActor(b.furnishingPreviews[i])) {
            const auto* spot=gardenSpot(previewSpot);a->enabled=spot&&previewItem==shopItems[i].id;
            if(a->enabled) {
                a->transform.position={spot->position.x,.025f,spot->position.z};
                const bool valid=afternoon.placementBlockage(previewItem,previewSpot).empty();
                for(auto* mesh:a->getComponents<engine::MeshComponent>())mesh->color=valid?engine::Vec3{.8f,1,.9f}:engine::Vec3{1,.6f,.5f};
            }
        }
    }
}
GardenUse beginGardenUse(engine::Scene& scene,const SceneBindings& b,std::string_view id,std::string_view spotId,bool reduced) {
    GardenUse use;const auto* spot=gardenSpot(spotId);const auto* item=shopItem(id);
    auto* actor=scene.findActor(b.gardenCat);auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(b.cat));
    if(!b.garden||!spot||!item||!actor||!cat)return use;
    use.actor=actor->id();use.cat=cat->id();use.item=id;use.spot=spotId;use.cushion=id=="garden_cushion";use.reduced=reduced;
    use.root=engine::Vec3{spot->position.x,0,spot->position.z}+gardenAnchor(item->asset,use.cushion?"rest_root":"drink_root");
    // Walk the clear central lane, then approach the authored use anchor. Never
    // traverse the bench, boundary, plants or another furnishing footprint.
    actor->transform.position={0,0,.15f};actor->transform.rotation={};
    use.yaw=use.cushion?engine::pi:0;
    const float approachZ=use.cushion?spot->position.z+.72f:use.root.z;
    if(spotId=="quiet_corner"&&!use.cushion) {
        // Approach from the courtyard side so Pip's body stays inside the fence.
        use.yaw=engine::pi/2;auto anchor=gardenAnchor("bowl","drink_root");
        use.root={spot->position.x+anchor.z,anchor.y,spot->position.z-anchor.x};
        use.path={{0,0,.15f},{.75f,0,.15f},{.75f,0,use.root.z},use.root};
    } else if(spotId=="quiet_corner") {
        use.path={{0,0,.15f},{.75f,0,.15f},{.75f,0,approachZ},{use.root.x,0,approachZ},use.root};
    } else if(use.cushion)use.path={{0,0,.15f},{0,0,approachZ},{use.root.x,0,approachZ},use.root};
    else use.path={{0,0,.15f},{use.root.x,0,.15f},use.root};
    use.phase=reduced?3:1;
    if(reduced) {actor->transform.position=use.root;actor->transform.rotation.y=use.yaw;presentPip(*cat,use.cushion?5:7,true);}
    else presentPip(*cat,3,false);
    return use;
}
void cancelGardenUse(engine::Scene& scene,GardenUse& use,bool reduced) {
    if(auto* actor=scene.findActor(use.actor)){actor->transform.position={0,0,.25f};actor->transform.rotation={};}
    if(auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(use.cat)))presentPip(*cat,0,reduced);
    use={};
}
void updateGardenUse(engine::Scene& scene,GardenUse& use,float delta) {
    auto* actor=scene.findActor(use.actor);auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(use.cat));
    if(!actor||!cat||!use.phase||delta<=0)return;
    use.elapsed+=delta;
    if(use.phase==1) {
        const auto from=use.path[use.segment],to=use.path[use.segment+1],d=to-from;
        const float duration=std::max(.0001f,std::sqrt(engine::dot(d,d))/pipWalkSpeed),t=std::min(1.f,use.elapsed/duration);
        actor->transform.position=from+d*t;actor->transform.rotation.y=std::atan2(d.x,d.z);
        if(t>=1) {use.elapsed=0;if(++use.segment+1==use.path.size()){use.phase=2;actor->transform.position=use.root;actor->transform.rotation.y=use.yaw;presentPip(*cat,use.cushion?4:6,false);}}
    } else if(use.phase==2&&use.elapsed>=(use.cushion?2.4f:1.8f)) {
        use.phase=3;use.elapsed=0;presentPip(*cat,use.cushion?5:7,false);
    } else if(use.phase==3&&use.elapsed>=5.f) {
        use.phase=4;use.elapsed=0;presentPip(*cat,use.cushion?4:8,use.cushion||use.reduced);
    } else if(use.phase==4&&use.elapsed>=(use.cushion?2.4f:1.8f)) {
        // Bounded use returns along the same clear path in full motion.
        if(use.reduced){cancelGardenUse(scene,use,true);return;}
        std::reverse(use.path.begin(),use.path.end());use.path.back()={0,0,.25f};
        use.phase=5;use.segment=0;use.elapsed=0;presentPip(*cat,3,false);
    } else if(use.phase==4&&use.cushion&&!use.reduced) {
        cat->animation().seek(std::max(0.f,2.4f-use.elapsed));
    } else if(use.phase==5) {
        const auto from=use.path[use.segment],to=use.path[use.segment+1],d=to-from;
        const float duration=std::max(.0001f,std::sqrt(engine::dot(d,d))/pipWalkSpeed),t=std::min(1.f,use.elapsed/duration);
        actor->transform.position=from+d*t;actor->transform.rotation.y=std::atan2(d.x,d.z);
        if(t>=1){use.elapsed=0;if(++use.segment+1==use.path.size())cancelGardenUse(scene,use,false);}
    }
}
}
