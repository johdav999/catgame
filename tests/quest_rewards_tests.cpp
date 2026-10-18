#include "quest_rewards.hpp"
#include "afternoon.hpp"
#include "release1/rules/content.hpp"
#include "release1/progression/policy.hpp"
#include "engine/glb_importer.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <stdexcept>
using namespace paws;
void check(bool value,const char* why){if(!value)throw std::runtime_error(why);}
int main(int argc,char** argv){try{
    GardenMeshes meshes;const auto root=std::filesystem::path(PAWS_SOURCE_DIR)/"assets/runtime";
    for(const auto& spec:questRewardAssets()){
        auto mesh=engine::GlbImporter::loadFile(root/std::string(spec.file).substr(7));validateGardenAsset(spec,*mesh);
        check(!mesh->materials.empty()&&mesh->materials[0]->baseColorTexture,"Provider base color survives export/import");
        check(mesh->materials[0]->baseColorTexture->width<=512,"Runtime maps use the bounded derived resolution");
        meshes[spec.name]=std::move(mesh);
    }
    Afternoon owner;const auto save=owner.encode();engine::Scene scene("Reward fixture");
    auto& cat=scene.spawn("Cat");cat.transform.position={2,0,-1};
    QuestRewards rewards;rewards.install(scene,meshes,{cat.id()});scene.synchronize();
    auto tokens=[&](const char* name){std::vector<const engine::Actor*> out;scene.visit([&](const engine::Node& n,const engine::Mat4&){if(auto* a=dynamic_cast<const engine::Actor*>(&n);a&&a->name==name)out.push_back(a);});return out;};
    check(!rewards.active()&&tokens("Coin reward").empty(),"Installing assets cannot start or award a reward");
    rewards.start(scene,false);check(rewards.advance(scene,0,{0,2,6},false)==QuestRewards::CoinSound,"Coin cue starts once");
    check(rewards.advance(scene,0,{0,2,6},false)==0,"Paused presentation cannot repeat the coin cue");
    check(tokens("Coin reward").size()==1&&tokens("Friendship heart reward").empty(),"Coin appears before heart");
    auto start=tokens("Coin reward")[0]->transform.position;
    check(tokens("Coin reward")[0]->getComponent<engine::MeshComponent>()->layer==engine::RenderLayer::Overlay,"Playhouse roofs cannot hide the reward feedback");
    rewards.advance(scene,.6,{0,2,6},false);auto risen=tokens("Coin reward")[0]->transform.position;
    check(risen.y>start.y+.2f,"Coin rises above its cat");
    check(rewards.advance(scene,.61,{0,2,6},false)==QuestRewards::PurrSound,"Heart phase emits exactly one purr");
    check(tokens("Coin reward").empty()&&tokens("Friendship heart reward").size()==1,"Heart follows coin without overlap");
    check(rewards.advance(scene,.5,{0,2,6},false)==0,"Purr cannot repeat each frame");
    rewards.advance(scene,3,{0,2,6},false);check(!rewards.active()&&tokens("Friendship heart reward").empty(),"Finished reward releases visible tokens");
    rewards.start(scene,true);rewards.advance(scene,.2,{0,2,6},true);start=tokens("Coin reward")[0]->transform.position;
    rewards.advance(scene,.6,{0,2,6},true);risen=tokens("Coin reward")[0]->transform.position;
    check(engine::dot(risen-start,risen-start)<.00001f,"Reduced motion shows a still coin");
    rewards.cancel(scene);check(!rewards.active()&&tokens("Coin reward").empty(),"Navigation/restart cancels presentation");
    rewards.start(scene,false);cat.destroy();scene.synchronize();check(rewards.advance(scene,.4,{0,2,6},false)==0&&!rewards.active(),"Removed cat invalidates delayed presentation and cues");
    check(owner.encode()==save,"Presentation cannot change coins, friendship or saves");
    engine::Scene group("Two requesting cats");auto a=group.spawn("Pip").id(),b=group.spawn("Pepper").id();
    QuestRewards shared;shared.install(group,meshes,{a,b});shared.start(group,false);
    check(shared.recipients()==2&&shared.advance(group,.2,{0,2,6},false)==QuestRewards::CoinSound,"Each cat gets shared assets with one sequence sound");
    if(argc==2){
        namespace r=paws::release1;Afternoon fixture;
        std::filesystem::create_directories(argv[1]);
        for(std::size_t q=0;q<10;++q){
            check(fixture.dispatchIntent(fixture.intent(r::Verb::SelectQuest,{},std::string(r::progression::quests[q].id))).accepted,"Select fixture quest");
            if(q==0||q==8||q==9)std::ofstream(std::filesystem::path(argv[1])/("quest-"+std::to_string(q)+".save"))<<fixture.encode();
            for(const auto& step:r::rules::quest(r::progression::quests[q].id).solution){auto i=fixture.intent(step.verb,step.object,step.target);i.direction=step.direction;i.destination=step.destination;check(fixture.dispatchIntent(i).accepted,"Solve fixture prefix");}
            check(fixture.returnToGarden(fixture.intent(r::Verb::Select)).accepted,"Return fixture home");
        }
    }
    std::cout<<"Reward asset, ordering, timing, pause, reduced-motion, cancellation, stale-handle and multi-cat checks passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
