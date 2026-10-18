#include "pip_asset.hpp"
#include "release1/cats/cat_assets.hpp"
#include "engine/glb_importer.hpp"
#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>

void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
int main(){try{
    for(const auto& asset:paws::release1::cats::assets){
        const std::string name(asset.cat);
        auto path=std::filesystem::path(PAWS_SOURCE_DIR)/"assets/runtime"/std::string(asset.url).substr(7);
        auto mesh=engine::GlbImporter::loadFile(path);paws::release1::cats::validate(*mesh);
        engine::Scene scene;auto& actor=scene.spawn(name);
        auto& skin=actor.addComponent<engine::SkeletalMeshComponent>(name,mesh);
        skin.animation().play(paws::pipClip(*mesh,"garden_walk"));
        // Measure the actual skinned stance at game travel speed. A clip-name
        // or duration assertion alone did not catch the replacement's longer stride.
        const auto cycle=mesh->skeleton->clips[paws::pipClip(*mesh,"garden_walk")].duration;
        if(paws::isToonCat(*mesh))for(bool left:{true,false}){
            std::vector<std::size_t> paw;
            const std::string bone=left?"foot_f_L":"foot_f_R";
            for(std::size_t v=0;v<mesh->skin.size();++v)for(int k=0;k<4;++k){
                const auto& influence=mesh->skin[v];
                if(influence.weights[k]>.3f&&mesh->skeleton->joints[mesh->skeleton->bindings[influence.joints[k]].joint].name==bone){paw.push_back(v);break;}
            }
            check(!paw.empty(),"No stance surface found");
            float low=100,high=-100,oldLow=100,oldHigh=-100;
            for(int frame=0;frame<=30;++frame){
                const double phase=(left?.08:.54)+frame/30.*(left?.32:.20);
                paws::sampleCatWalk(actor,skin,phase*cycle,true);
                const auto pose=skin.skinPose();float z=0;
                for(auto v:paw)for(int k=0;k<4;++k)
                    z+=engine::transformPoint(pose->bones[mesh->skin[v].joints[k]].transform,mesh->vertices[v].position).z*mesh->skin[v].weights[k];
                z/=paw.size();const float world=z+phase*cycle*paws::pipWalkSpeed;
                const float oldWorld=z+phase*.8*paws::pipWalkSpeed;
                low=std::min(low,world);high=std::max(high,world);oldLow=std::min(oldLow,oldWorld);oldHigh=std::max(oldHigh,oldWorld);
            }
            std::cout<<name<<" "<<bone<<": planted drift "<<high-low<<" m; old cadence "<<oldHigh-oldLow<<" m\n";
            check(high-low<.015f,"Walking feet slide faster than world movement");
            check(oldHigh-oldLow>.10f,"Regression fixture no longer exercises the old cadence mismatch");
        }
        if(paws::isToonCat(*mesh)){
            const auto clip=paws::pipClip(*mesh,"playhouse_enter");const auto duration=mesh->skeleton->clips[clip].duration;
            for(bool reverse:{false,true}){
                float low=100,high=-100;
                for(int frame=0;frame<=30;++frame){
                    const double phase=.05+frame/30.*.45;
                    skin.animation().play(clip,true);skin.animation().seek(phase*duration);skin.animation().pause();
                    auto pose=skin.skinPose();float z=0;int count=0;
                    for(std::size_t v=0;v<mesh->skin.size();++v){bool foot=false;
                        for(int k=0;k<4;++k)foot|=mesh->skin[v].weights[k]>.3f&&mesh->skeleton->joints[mesh->skeleton->bindings[mesh->skin[v].joints[k]].joint].name=="foot_f_L";
                        if(!foot)continue;++count;
                        for(int k=0;k<4;++k)z+=engine::transformPoint(pose->bones[mesh->skin[v].joints[k]].transform,mesh->vertices[v].position).z*mesh->skin[v].weights[k];
                    }
                    // Reverse traverses the same planted poses in reverse order.
                    const float world=z/count+(reverse?-(1-phase):phase)*paws::toonCrawlStride;
                    low=std::min(low,world);high=std::max(high,world);
                }
                check(high-low<.012f,"Crawling stance slides despite distance-driven playback");
            }
        }
        for(float sign:{-1.f,1.f}){
            actor.transform.rotation.y=0;
            for(int frame=0;frame<60;++frame){
                const float previous=actor.transform.rotation.y;
                paws::turnCatTowards(actor,sign*engine::pi/2,1./60);
                check(std::abs(actor.transform.rotation.y-previous)<=.05001f,"Heading snapped instead of turning");
                paws::sampleCatWalk(actor,skin,.37,true);
                check(skin.animation().state()==engine::AnimationState::Paused,"Sampled ending must remain paused");
                check(std::abs(skin.animation().time()-.37)<.00001,"Turn selection restarted the gait phase");
                if(frame==8){auto* turn=actor.getComponent<paws::CatTurn>();check(turn&&turn->weight*sign>.5f,"Wrong anatomical turn direction");check(skin.animation().blending(),"Turn gait not blended");}
            }
            check(std::abs(actor.transform.rotation.y-sign*engine::pi/2)<.001f,"Turn did not reach its target");
        }
        actor.transform.rotation.y=engine::pi-.01f;
        paws::turnCatTowards(actor,-engine::pi+.01f,1./60);
        check(std::abs(std::remainder(actor.transform.rotation.y-(-engine::pi+.01f),2*engine::pi))<.001f,"Wraparound took the long route");
        const auto before=actor.transform.rotation.y;
        paws::turnCatTowards(actor,0,0);check(actor.transform.rotation.y==before,"Paused frame advanced heading");
        auto& second=scene.spawn("Independent cat");second.addComponent<engine::SkeletalMeshComponent>("Other",mesh);
        paws::turnCatTowards(second,-1,1./60);check(actor.transform.rotation.y==before,"Instances share turning state");
        const auto& skeleton=*mesh->skeleton;
        for(auto clipName:{"turn_left","turn_right"}){
            const auto& clip=skeleton.clips[paws::pipClip(*mesh,clipName)];
            check(std::abs(clip.duration-skeleton.clips[paws::pipClip(*mesh,"garden_walk")].duration)<.001,"Turn cycle does not match walking");
            for(std::size_t j=0;j<skeleton.joints.size();++j){
                auto first=clip.frames[j].matrix(),last=clip.frames[(clip.frameCount-1)*skeleton.joints.size()+j].matrix();
                for(int k=0;k<16;++k)check(std::abs(first.m[k]-last.m[k])<.001f,"Turn loop has a pose seam");
            }
        }
        std::cout<<name<<": directional clips, smooth heading, wraparound, phase, pause and independent instances passed\n";
    }
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
