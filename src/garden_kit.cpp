#include "garden_kit.hpp"
#include "engine/light_component.hpp"
#include <cmath>
#include <stdexcept>
namespace paws {
namespace {
#include "garden_manifest.inc"
engine::Vec3 reviewPosition(std::string_view name){
    for(const auto& instance:reviewInstances)if(name==instance.asset)return instance.position;
    throw std::invalid_argument("Missing garden review instance");
}
}
std::span<const GardenAsset> gardenAssets(){return assetSpecs;}
std::span<const GardenInstance> gardenReviewInstances(){return reviewInstances;}
engine::Vec3 gardenAnchor(std::string_view asset,std::string_view anchor){
    for(const auto& a:anchorSpecs)if(asset==a.asset&&anchor==a.name)return a.position;
    throw std::invalid_argument("Missing garden anchor");
}
void validateGardenAsset(const GardenAsset& spec,const engine::Mesh& mesh){
    if(mesh.vertices.empty()||mesh.indices.empty()||!mesh.warnings.empty())throw std::invalid_argument(std::string(spec.name)+": empty or unsupported GLB");
    const auto& b=mesh.localBounds();
    const auto deltaMin=b.minimum-spec.minimum,deltaMax=b.maximum-spec.maximum;
    if(!b.valid||engine::dot(deltaMin,deltaMin)>.000064f||engine::dot(deltaMax,deltaMax)>.000064f)
        throw std::invalid_argument(std::string(spec.name)+": export bounds differ from contract");
    if(mesh.indices.size()/3!=spec.triangles)throw std::invalid_argument(std::string(spec.name)+": triangle budget changed");
}
GardenPreview makeGardenPreview(const GardenMeshes& meshes,std::shared_ptr<const engine::Mesh> pip){
    for(const auto& spec:gardenAssets())validateGardenAsset(spec,*meshes.at(spec.name));
    validatePip(*pip);
    GardenPreview result;result.scene.scene=std::make_unique<engine::Scene>("Garden kit workshop");
    auto& scene=*result.scene.scene;scene.ambient={.33f,.37f,.40f};scene.environmentIntensity=.65f;
    for(const auto& instance:gardenReviewInstances()){
        auto& a=scene.spawn(instance.asset);a.transform.position=instance.position;
        a.addComponent<engine::MeshComponent>(instance.asset,meshes.at(instance.asset)).color={1,1,1};
        const std::string_view name=instance.asset;
        if(name=="gate_leaf")result.gate=a.id();if(name=="switch_plate")result.plate=a.id();
        if(name=="ball")result.ball=a.id();if(name=="mouse")result.mouse=a.id();if(name=="fish")result.fish=a.id();
    }
    // Shared immutable paving mesh, rather than enlarged stone texture coverage.
    for(int x=-3;x<3;++x)for(int z=-2;z<3;++z){
        auto& a=scene.spawn("Courtyard paving");a.transform.position={x+.5f,-.005f,z-.5f};
        a.addComponent<engine::MeshComponent>("Sandstone",meshes.at("paving")).color={1,1,1};
    }
    auto& catActor=scene.spawn("Pip scale reference");catActor.transform.position=reviewPosition("cushion")+gardenAnchor("cushion","rest_root");result.pip=catActor.id();
    auto& cat=catActor.addComponent<engine::SkeletalMeshComponent>("Pip",std::move(pip));cat.color={1,1,1};result.cat=cat.id();presentPip(cat,5,true);
    auto& light=scene.spawn("Garden sun").addComponent<engine::LightComponent>("Sun",engine::LightType::Directional);
    light.direction={-.5f,-1,-.6f};light.color={1,.95f,.87f};light.intensity=1.7f;light.shadow.enabled=true;light.shadow.halfExtent=5;light.shadow.halfDepth=8;light.shadow.normalBias=.02f;
    auto& camera=scene.spawn("Workshop camera").addComponent<engine::CameraComponent>("Camera");
    camera.setPerspective(engine::pi*35/180,.05f,60);camera.transform.position={6,7,9};camera.lookAt({0,.15f,0});result.scene.camera=&camera;
    return result;
}
void poseGardenPreview(engine::Scene& scene,const GardenPreview& b,int mode){
    if(mode<0||mode>3)throw std::out_of_range("Unknown workshop pose");
    // Reset every moving part to its authored transform before projecting a pose.
    for(auto [name,id]:{std::pair{"gate_leaf",b.gate},{"switch_plate",b.plate},{"ball",b.ball},{"mouse",b.mouse},{"fish",b.fish}}){
        if(auto* a=scene.findActor(id))for(const auto& i:gardenReviewInstances())if(std::string_view(i.asset)==name){a->transform.position=i.position;a->transform.rotation={};}
    }
    auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(b.cat));auto* actor=scene.findActor(b.pip);
    if(cat&&actor){actor->transform.position=reviewPosition("cushion")+gardenAnchor("cushion","rest_root");presentPip(*cat,5,true);}
    if(mode==1){
        if(auto* a=scene.findActor(b.gate))a->transform.rotation.y=gateOpenDegrees*engine::pi/180;
        if(auto* a=scene.findActor(b.plate))a->transform.position=reviewPosition("switch_base")+gardenAnchor("switch_base","plate_pressed");
        if(auto* a=scene.findActor(b.ball))a->transform.position=reviewPosition("switch_base")+gardenAnchor("switch_base","ball_root_pressed");
    }else if(mode==2){
        const auto basket=reviewPosition("basket");
        for(auto [name,id]:{std::pair{"ball_socket",b.ball},{"mouse_socket",b.mouse},{"fish_socket",b.fish}})
            if(auto* a=scene.findActor(id))a->transform.position=basket+gardenAnchor("basket",name);
    }else if(mode==3&&cat&&actor){
        actor->transform.position=reviewPosition("bowl")+gardenAnchor("bowl","drink_root");presentPip(*cat,7,true);
    }
}
}
