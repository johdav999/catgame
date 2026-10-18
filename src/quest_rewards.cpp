#include "quest_rewards.hpp"
#include "engine/mesh_component.hpp"
#include <algorithm>
#include <cmath>

namespace paws {
namespace {
#include "reward_manifest.inc"
engine::Vec3 head(const engine::Actor& cat) {
    const auto* skin=cat.getComponent<engine::SkeletalMeshComponent>();
    const engine::Vec3 crown{.12f,.92f,.50f};
    if(skin&&skin->mesh->skeleton)if(const auto pose=skin->skinPose()){
        const auto& rig=*skin->mesh->skeleton;
        for(std::size_t n=1;n<rig.bindings.size();++n)
            if(rig.joints[rig.bindings[n].joint].name=="head")
                return engine::transformPoint(skin->worldMatrix(),engine::transformPoint(pose->bones[n].transform,crown));
    }
    return engine::transformPoint(cat.worldMatrix(),crown);
}
}
std::span<const GardenAsset> questRewardAssets(){return rewardInputs;}
std::vector<engine::Vec3> QuestRewards::origins(const engine::Scene& scene) const {
    std::vector<engine::Vec3> out;
    for(const auto& target:targets_)if(const auto* cat=scene.findActor(target.cat))out.push_back(head(*cat));
    return out;
}
void QuestRewards::install(engine::Scene& scene,const GardenMeshes& meshes,const std::vector<engine::ActorId>& cats) {
    targets_.clear();phase_=Phase::Idle;elapsed_=0;emitted_=0;
    if(cats.empty())return;
    // Require both real accepted assets before installing any presentation.
    const auto coin=meshes.at("reward_coin"),heart=meshes.at("reward_heart");
    for(const auto id:cats){
        if(!scene.findActor(id))continue;
        Target target;target.cat=id;
        for(bool friendship:{false,true}){
            auto& actor=scene.spawn(friendship?"Friendship heart reward":"Coin reward");
            auto& mesh=actor.addComponent<engine::MeshComponent>(actor.name,friendship?heart:coin);
            mesh.color={1,1,1};mesh.castsShadow=mesh.receivesShadow=false;
            // Semantic world-space feedback must remain visible above cats
            // standing inside the playhouse or behind a foreground prop.
            mesh.layer=engine::RenderLayer::Overlay;
            mesh.blend=engine::BlendMode::Alpha;actor.enabled=false;
            (friendship?target.heart:target.coin)=actor.id();
        }
        targets_.push_back(target);
    }
}
void QuestRewards::start(engine::Scene& scene,bool reducedMotion) {
    cancel(scene);elapsed_=0;emitted_=0;
    phase_=targets_.empty()?Phase::Finished:Phase::Coin;
    pose(scene,{0,2,6},reducedMotion);
}
unsigned QuestRewards::advance(engine::Scene& scene,double delta,engine::Vec3 eye,bool reducedMotion) {
    if(!active()||!std::isfinite(delta)||delta<0)return None;
    if(std::none_of(targets_.begin(),targets_.end(),[&](const auto& t){return scene.findActor(t.cat)!=nullptr;})){
        cancel(scene);return None;
    }
    elapsed_=std::min(coinSeconds+heartSeconds,elapsed_+delta);
    const unsigned wanted=elapsed_>=coinSeconds?CoinSound|PurrSound:CoinSound;
    const auto cues=wanted&~emitted_;emitted_|=wanted;
    phase_=elapsed_>=coinSeconds+heartSeconds?Phase::Finished:elapsed_>=coinSeconds?Phase::Heart:Phase::Coin;
    pose(scene,eye,reducedMotion);return cues;
}
void QuestRewards::cancel(engine::Scene& scene) {
    for(const auto& t:targets_)for(auto id:{t.coin,t.heart})if(auto* a=scene.findActor(id))a->enabled=false;
    phase_=Phase::Idle;elapsed_=0;emitted_=0;
}
void QuestRewards::pose(engine::Scene& scene,engine::Vec3 eye,bool reducedMotion) {
    const bool friendship=phase_==Phase::Heart;
    const float t=float(std::clamp(friendship?(elapsed_-coinSeconds)/heartSeconds:elapsed_/coinSeconds,0.,1.));
    const float eased=1-(1-t)*(1-t);
    for(const auto& target:targets_){
        auto* cat=scene.findActor(target.cat);
        for(bool heart:{false,true})if(auto* a=scene.findActor(heart?target.heart:target.coin)){
            a->enabled=cat&&cat->enabled&&active()&&heart==friendship;
            if(!a->enabled)continue;
            auto position=head(*cat);position.y+=reducedMotion?.28f:.06f+.75f*eased;
            a->transform.position=position;
            const auto toward=eye-position;
            a->transform.rotation={0,std::atan2(toward.x,toward.z)+(reducedMotion?0:(friendship?.12f:.45f)*std::sin(t*engine::pi)),0};
            const float scale=.5f*(reducedMotion?1.f:.88f+.12f*std::min(1.f,t*5));
            a->transform.scale={scale,scale,scale};
            a->getComponent<engine::MeshComponent>()->opacity=reducedMotion?1.f:std::min({1.f,t*10,(1-t)*6});
        }
    }
}
} // namespace paws
