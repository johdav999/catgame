#include "pip_asset.hpp"
#include "engine/light_component.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace paws {
std::size_t pipClip(const engine::Mesh& mesh,std::string_view name) {
    if(!mesh.skeleton) throw std::invalid_argument("Pip has no skeleton");
    const auto& clips=mesh.skeleton->clips;
    auto it=std::find_if(clips.begin(),clips.end(),[&](const auto& clip){return clip.name==name;});
    if(it==clips.end()) throw std::invalid_argument("Pip is missing action: "+std::string(name));
    return static_cast<std::size_t>(it-clips.begin());
}
void validatePip(const engine::Mesh& mesh) {
    if(mesh.vertices.empty() || mesh.skin.size()!=mesh.vertices.size() || !mesh.skeleton)
        throw std::invalid_argument("Pip needs a complete skinned mesh");
    mesh.skeleton->validate();
    for(const auto& action:pipActions) {
        const auto& clip=mesh.skeleton->clips[pipClip(mesh,action.name)];
        if(clip.duration<=0 || clip.frameCount<2) throw std::invalid_argument("Pip has an empty action");
    }
    for(auto required:{"root","pelvis","spine","head","ear.L","ear.R","tail.00","tail.06","blink.L","blink.R"})
        if(std::none_of(mesh.skeleton->joints.begin(),mesh.skeleton->joints.end(),[&](const auto& j){return j.name==required;}))
            throw std::invalid_argument("Pip is missing a canonical bone");
    const auto& b=mesh.localBounds();
    if(!b.valid || b.maximum.y<.8f || b.maximum.y>1.2f) throw std::invalid_argument("Pip is outside metre-scale bounds");
    if(!mesh.warnings.empty()) throw std::invalid_argument("Pip import warning: "+mesh.warnings.front());
}
void presentPip(engine::SkeletalMeshComponent& cat,std::size_t action,bool reducedMotion) {
    if(action>=pipActions.size()) throw std::out_of_range("Unknown Pip action");
    const auto& spec=pipActions[action];auto& animation=cat.animation();const auto clip=pipClip(*cat.mesh,spec.name);
    if(reducedMotion) { animation.play(clip,spec.loop);animation.seek(spec.stillSeconds);animation.pause(); }
    else animation.crossFade(clip,.18,spec.loop);
}
PipPreview makePipPreview(std::shared_ptr<const engine::Mesh> mesh) {
    validatePip(*mesh);
    PipPreview result{{std::make_unique<engine::Scene>("Pip asset preview"),nullptr},{},{}};
    auto& scene=*result.scene.scene;scene.ambient={.30f,.34f,.38f};scene.environmentIntensity=.65f;
    auto& floor=scene.spawn("Review ground");floor.transform.position={0,-.027f,0};floor.transform.scale={4,.05f,4};
    floor.addComponent<engine::MeshComponent>("Sandstone review floor",engine::Mesh::cube()).color={.63f,.66f,.53f};
    auto& cat=scene.spawn("Pip").addComponent<engine::SkeletalMeshComponent>("Pip",std::move(mesh));
    cat.color={1,1,1};result.cat=cat.id();cat.animation().play(pipClip(*cat.mesh,"relaxed_idle"));
    // A quiet code-authored contact cue when shadow maps are disabled.
    auto texture=std::make_shared<engine::Texture>();texture->name="Pip contact falloff";texture->width=texture->height=32;
    texture->wrapU=texture->wrapV=engine::TextureWrap::Clamp;texture->pixels.resize(32*32*4);
    for(int y=0;y<32;++y)for(int x=0;x<32;++x) {
        const float dx=(x-15.5f)/15.5f,dy=(y-15.5f)/15.5f;
        const float falloff=std::max(0.f,1-dx*dx-dy*dy);
        const auto i=(y*32+x)*4;texture->pixels[i]=texture->pixels[i+1]=texture->pixels[i+2]=255;
        texture->pixels[i+3]=static_cast<std::uint8_t>(falloff*falloff*85);
    }
    auto& marker=scene.spawn("Shadow-free contact");marker.transform.position={0,.0005f,0};marker.transform.scale={.7f,1,1.1f};
    auto& contact=marker.addComponent<engine::MeshComponent>("Ground contact",engine::Mesh::plane());
    auto material=std::make_shared<engine::Material>();material->name="Contact cue";material->shader=engine::MaterialShader::Unlit;
    material->baseColor={.24f,.30f,.22f};material->baseColorTexture=texture;material->alphaMode=engine::AlphaMode::Blend;
    contact.setMaterial(0,engine::MaterialInstance{material});contact.color={1,1,1};contact.castsShadow=contact.receivesShadow=false;
    contact.enabled=false;result.contact=contact.id();
    auto& light=scene.spawn("Review light").addComponent<engine::LightComponent>("Garden sun",engine::LightType::Directional);
    light.direction={-.5f,-1,-.6f};light.color={1,.94f,.83f};light.intensity=2;light.shadow.enabled=true;light.shadow.halfExtent=2;light.shadow.halfDepth=3;light.shadow.normalBias=.003f;
    auto& camera=scene.spawn("Review camera").addComponent<engine::CameraComponent>("Pip camera");
    camera.setPerspective(engine::pi*35/180,.05f,50);
    camera.transform.position={1.5f,1.3f,2.5f};camera.lookAt({0,.48f,0});result.scene.camera=&camera;
    return result;
}
}
