#include "prop_assets.hpp"
#include "engine/light_component.hpp"
namespace paws::release1::props {
namespace {
#include "manifest.inc"
}
std::span<const GardenAsset> assets(){return assetSpecs;}
std::span<const PropAnchor> anchors(){return anchorSpecs;}
std::span<const PropInstance> instances(){return instanceSpecs;}
std::span<const PropView> views(){return viewSpecs;}
Preview makePreview(const GardenMeshes& meshes){
    for(const auto& a:assets())validateGardenAsset(a,*meshes.at(a.name));
    Preview result;result.scene.scene=std::make_unique<engine::Scene>("Release one prop workshop");
    auto& scene=*result.scene.scene;scene.ambient={.33f,.37f,.40f};scene.environmentIntensity=.65f;
    for(const auto& i:instances()){
        auto& actor=scene.spawn(i.asset);actor.transform.position=i.position;actor.transform.rotation=i.rotationDegrees*(engine::pi/180);
        actor.addComponent<engine::MeshComponent>(i.asset,meshes.at(i.asset)).color={1,1,1};result.bindings.push_back({actor.id(),i});
    }
    // Neutral, contact-plane inspection floor. One draw keeps the workshop's
    // background from dominating the asset budget or obscuring silhouettes.
    auto& floor=scene.spawn("Neutral review floor");
    floor.addComponent<engine::MeshComponent>("Floor",engine::Mesh::plane(100)).color={.58f,.60f,.52f};
    auto& sun=scene.spawn("Neutral garden sun").addComponent<engine::LightComponent>("Sun",engine::LightType::Directional);
    sun.direction={-.5f,-1,-.6f};sun.color={1,.95f,.87f};sun.intensity=1.7f;sun.shadow.enabled=true;sun.shadow.halfExtent=20;sun.shadow.halfDepth=30;sun.shadow.normalBias=.02f;
    auto& camera=scene.spawn("Inspection camera").addComponent<engine::CameraComponent>("Camera");
    camera.setPerspective(engine::pi*35/180,.05f,150);camera.transform.position={12,18,22};camera.lookAt({0,.2f,0});result.scene.camera=&camera;
    return result;
}
void pose(engine::Scene& scene,const Preview& preview,bool active){
    for(const auto& b:preview.bindings)if(auto* a=scene.findActor(b.actor)){
        a->transform.position=b.instance.position;a->transform.rotation=b.instance.rotationDegrees*(engine::pi/180);
        if(!active||b.instance.group==0)continue;
        const std::string_view name=b.instance.asset;
        if(name=="playhouse_door")a->transform.rotation.y=100*engine::pi/180;
        if(name=="switch_plate")a->transform.position.y-=.035f;
        if(name=="bell_leaf"||name=="bell_star"||name=="bell_paw")a->transform.rotation.x=.25f;
        if(name.starts_with("cushion_"))a->transform.position.y+=.3f;
    }
}
}
