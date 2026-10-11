#include "pip_asset.hpp"
#include "engine/application.hpp"
#include "engine/glb_importer.hpp"
#include "engine/fbx_importer.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <algorithm>

void check(bool value,const char* message) {if(!value)throw std::runtime_error(message);}
float poseDifference(const engine::JointPose& a,const engine::JointPose& b) {
    float difference=0;
    const auto am=a.matrix(),bm=b.matrix();
    for(int i=0;i<16;++i)difference=std::max(difference,std::abs(am.m[i]-bm.m[i]));
    return difference;
}
int main(int argc,char** argv) {
    try {
        check(argc==2,"Expected Pip GLB path");
        const auto mesh=std::filesystem::path(argv[1]).extension()==".fbx" ? engine::FbxImporter::loadFile(argv[1]) : engine::GlbImporter::loadFile(argv[1]);paws::validatePip(*mesh);
        check(mesh->indices.size()/3<=18000,"Pip exceeds the 18000 triangle allocation");
        check(mesh->materials.size()<=2,"Too many Pip materials");
        auto& skeleton=*mesh->skeleton;
        auto root=std::find_if(skeleton.joints.begin(),skeleton.joints.end(),[](const auto& j){return j.name=="root";});
        const auto rootIndex=std::size_t(root-skeleton.joints.begin());
        check(rootIndex<skeleton.joints.size(),"Missing root");
        for(auto& skin:mesh->skin) {
            float sum=0;for(auto w:skin.weights){check(std::isfinite(w)&&w>=0,"Bad weight");sum+=w;}
            check(std::abs(sum-1)<.0001f,"Weights not normalized");
        }
        const double expectedDurations[]={4,2.4,1.2,1.2,2.4,4,1.8,2,1.8};
        for(auto& action:paws::pipActions) {
            const auto& clip=skeleton.clips[paws::pipClip(*mesh,action.name)];
            check(std::abs(clip.duration-expectedDurations[&action-paws::pipActions.data()])<.001,"Canonical action duration differs");
            for(unsigned frame=0;frame<clip.frameCount;++frame)
                check(poseDifference(clip.frames[frame*skeleton.joints.size()+rootIndex],skeleton.joints[rootIndex].bindPose)<.0001f,"Root moves in an in-place clip");
            if(action.loop)for(std::size_t j=0;j<skeleton.joints.size();++j)
                check(poseDifference(clip.frames[j],clip.frames[(clip.frameCount-1)*skeleton.joints.size()+j])<.001f,"Loop pose seam");
        }
        for(auto pair:{std::pair{4,5},std::pair{6,7},std::pair{7,8},std::pair{8,0}}) {
            const auto& a=skeleton.clips[paws::pipClip(*mesh,paws::pipActions[pair.first].name)];
            const auto& b=skeleton.clips[paws::pipClip(*mesh,paws::pipActions[pair.second].name)];
            for(std::size_t j=0;j<skeleton.joints.size();++j)
                check(poseDifference(a.frames[(a.frameCount-1)*skeleton.joints.size()+j],b.frames[j])<.001f,"Authored transition pose mismatch");
        }
        auto preview=paws::makePipPreview(mesh);const auto catId=preview.cat;
        engine::Application app;const auto sceneId=app.loadScene(std::move(preview.scene.scene));app.start(sceneId);
        auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(app.activeScene()->findComponent(catId));check(cat,"Missing Pip binding");
        double timestamp=0;app.advance(timestamp);
        for(std::size_t i=0;i<paws::pipActions.size();++i) {
            paws::presentPip(*cat,i,false);
            for(int f=0;f<150;++f) {
                timestamp+=1./30;app.advance(timestamp);const auto skin=cat->skinPose();
                for(const auto& bone:skin->bones)for(float v:bone.transform.m)check(std::isfinite(v),"Nonfinite animated skin");
                if(f%10==0 && f>10) for(std::size_t v=0;v<mesh->vertices.size();++v) {
                    engine::Vec3 p{};
                    for(int k=0;k<4;++k)p=p+engine::transformPoint(skin->bones[mesh->skin[v].joints[k]].transform,mesh->vertices[v].position)*mesh->skin[v].weights[k];
                    if(!(p.y>=-.005f && p.y<1.5f)){std::cerr << "Envelope: " << paws::pipActions[i].name << " frame " << f << " vertex " << v << " y " << p.y << "\\n"; throw std::runtime_error("Animated skin violates ground or height envelope");}
                }
            }
            paws::presentPip(*cat,i,true);const auto still=cat->animation().time();timestamp+=.03;app.advance(timestamp);
            check(cat->animation().time()==still,"Reduced motion advanced");
        }
        paws::presentPip(*cat,0,false);app.pause();const auto paused=cat->animation().time();timestamp+=.03;app.advance(timestamp);check(cat->animation().time()==paused,"Pause advanced animation");
        app.resume();app.setSuspended(true);timestamp+=.03;app.advance(timestamp);check(cat->animation().time()==paused,"Hidden page advanced animation");
        app.shutdown();check(!app.findScene(sceneId),"Scene survives shutdown");
        std::cout<<"Pip: "<<mesh->indices.size()/3<<" triangles, "<<skeleton.joints.size()<<" joints, "<<skeleton.clips.size()<<" clips; root, loops, skin, static poses and lifecycle passed\n";
    } catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
}


