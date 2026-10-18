#include "release1/cats/cat_assets.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace paws::release1::cats {
namespace {
void require(bool value,const char* message) { if(!value)throw std::invalid_argument(message); }
bool near(const engine::Mat4& a,const engine::Mat4& b,float tolerance=1e-5f) {
    for(int i=0;i<16;++i)if(!std::isfinite(a.m[i])||std::abs(a.m[i]-b.m[i])>tolerance)return false;
    return true;
}
}
void validate(const engine::Mesh& mesh) {
    paws::validatePip(mesh);
    require(mesh.indices.size()/3<=28000,"Cat exceeds 28k triangle allocation including eyelid topology");
    require(mesh.materials.size()<=3,"Cat exceeds material allocation");
    const auto& skeleton=*mesh.skeleton;
    const auto rootName=paws::isToonCat(mesh)?"root_bone":"root";
    auto root=std::find_if(skeleton.joints.begin(),skeleton.joints.end(),[&](auto& j){return j.name==rootName;});
    require(root!=skeleton.joints.end(),"Missing canonical root");
    const auto ri=std::size_t(root-skeleton.joints.begin());
    for(const auto& skin:mesh.skin) {
        float sum=0;
        for(int k=0;k<4;++k) {
            require(std::isfinite(skin.weights[k])&&skin.weights[k]>=0,"Invalid skin weight");
            if(skin.weights[k]>0)require(skin.joints[k]>0&&skin.joints[k]<skeleton.bindings.size(),"Unbound skin vertex");
            sum+=skin.weights[k];
        }
        require(std::abs(sum-1)<.0001f,"Unnormalized skin vertex");
    }
    for(const auto& spec:actions) {
        const auto& clip=skeleton.clips[paws::pipClip(mesh,spec.name)];
        const bool gait=spec.name=="garden_walk"||spec.name=="turn_left"||spec.name=="turn_right";
        const double duration=gait?(paws::isToonCat(mesh)?paws::toonWalkDuration:paws::pipWalkDuration):spec.duration;
        require(std::abs(clip.duration-duration)<.002,"Unexpected action duration");
        for(unsigned f=0;f<clip.frameCount;++f) {
            require(near(clip.frames[f*skeleton.joints.size()+ri].matrix(),root->bindPose.matrix(),.0001f),"Action moves the root");
            for(std::size_t j=0;j<skeleton.joints.size();++j) {
                const auto& pose=clip.frames[f*skeleton.joints.size()+j];
                require(std::abs(pose.scale.x-1)<.0002f&&std::abs(pose.scale.y-1)<.0002f&&std::abs(pose.scale.z-1)<.0002f,"Non-unit animated scale");
            }
        }
        if(spec.loop)for(std::size_t j=0;j<skeleton.joints.size();++j)
            require(near(clip.frames[j].matrix(),clip.frames[(clip.frameCount-1)*skeleton.joints.size()+j].matrix(),.001f),"Loop endpoint mismatch");
    }
    for(auto pair:{std::pair{"settle_cushion","curled_rest"},std::pair{"approach_bowl","drink"},std::pair{"drink","return_idle"},std::pair{"return_idle","relaxed_idle"}}) {
        // Exact joins remain a contract of the original authored clips. The
        // forward idle is an optional presentation variant entered by crossfade.
        const auto& a=skeleton.clips[paws::pipClip(mesh,pair.first,false)];
        const auto& b=skeleton.clips[paws::pipClip(mesh,pair.second,false)];
        for(std::size_t j=0;j<skeleton.joints.size();++j)
            require(near(a.frames[(a.frameCount-1)*skeleton.joints.size()+j].matrix(),b.frames[j].matrix(),.001f),"Action transition mismatch");
    }
}
void validateCanonical(const engine::Mesh& mesh,const engine::Mesh& authority) {
    require(mesh.skeleton&&authority.skeleton,"Missing skeleton authority");
    const auto& s=*mesh.skeleton;const auto& a=*authority.skeleton;
    const bool toon=paws::isToonCat(authority);
    require(paws::isToonCat(mesh)==toon,"Mixed cat rig generations");
    // Imported wrapper nodes are not portable bone identities. Compare every
    // canonical deform joint by name, hierarchy, local bind and inverse bind.
    constexpr std::array<std::string_view,31> canonicalNames{"root","pelvis","spine","chest","neck","head","ear.L","blink.L","ear.R","blink.R","blink.lower.L","blink.lower.R","front_upper.L","front_lower.L","front_paw.L","front_upper.R","front_lower.R","front_paw.R","hind_upper.L","hind_lower.L","hind_paw.L","hind_upper.R","hind_lower.R","hind_paw.R","tail.00","tail.01","tail.02","tail.03","tail.04","tail.05","tail.06"};
    std::size_t compared=0;
    for(std::size_t bindingIndex=1;bindingIndex<a.bindings.size();++bindingIndex) {
        const auto& binding=a.bindings[bindingIndex];
        if(binding.joint>=a.joints.size()||a.joints[binding.joint].name=="PipRig")continue;
        const auto& expected=a.joints[binding.joint];
        if(toon&&expected.parent<0)continue; // Importer's rigid mesh wrapper binding.
        if(!toon&&std::find(canonicalNames.begin(),canonicalNames.end(),expected.name)==canonicalNames.end())continue;
        ++compared;
        auto actual=std::find_if(s.joints.begin(),s.joints.end(),[&](auto& j){return j.name==expected.name;});
        require(actual!=s.joints.end(),"Missing canonical joint");
        require(near(actual->bindPose.matrix(),expected.bindPose.matrix()),"Canonical local bind changed");
        require((actual->parent<0)==(expected.parent<0),"Canonical parent changed");
        if(actual->parent>=0)require(s.joints[actual->parent].name==a.joints[expected.parent].name,"Canonical hierarchy changed");
        auto found=std::find_if(s.bindings.begin(),s.bindings.end(),[&](auto& b){return b.joint==std::size_t(actual-s.joints.begin());});
        require(found!=s.bindings.end()&&near(found->inverseBind,binding.inverseBind),"Canonical inverse bind changed");
    }
    require(compared==(toon?51:canonicalNames.size()),"Authority does not expose its complete canonical rig");
}
void present(engine::SkeletalMeshComponent& cat,std::size_t index,bool reducedMotion) {
    if(index>=actions.size())throw std::out_of_range("Unknown cat action");
    const auto& spec=actions[index];auto& player=cat.animation();
    auto clip=paws::pipClip(*cat.mesh,spec.name);
    if(reducedMotion){player.play(clip,spec.loop);player.seek(spec.still);player.pause();}
    else player.crossFade(clip,.18,spec.loop);
}
}
