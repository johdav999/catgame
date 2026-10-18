#include "pip_asset.hpp"
#include "engine/glb_importer.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
int main(int argc,char** argv){try {
    check(argc==2,"Expected Pip GLB path");
    const auto mesh=engine::GlbImporter::loadFile(argv[1]);paws::validatePip(*mesh);
    check(std::abs(mesh->skeleton->clips[paws::pipClip(*mesh,"garden_walk")].duration-paws::pipWalkDuration)<.0001,
          "Walking clip duration must match world travel cadence");
    auto preview=paws::makePipPreview(mesh);
    auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(preview.scene.scene->findComponent(preview.cat));
        // Measure exported skin in world space while translating at the authored
        // speed. A moving joint alone is not evidence of a planted paw.
        cat->animation().play(paws::pipClip(*mesh,"garden_walk"));
        for(int foot=0;foot<4;++foot) {
            const bool front=foot<2,left=foot%2==0;
            const double contact=front?(left?.25:.75):(left?0:.5);
            std::vector<std::size_t> sole;
            for(std::size_t v=0;v<mesh->vertices.size();++v) {
                const auto p=mesh->vertices[v].position;
                if(p.y<.028f&&(p.x<0)==left&&(p.z>0)==front)sole.push_back(v);
            }
            check(!sole.empty(),"Missing paw sole samples");
            float minZ=100,maxZ=-100,minX=100,maxX=-100,maxLift=0,maxFloorError=0;
            for(int sample=0;sample<=240;++sample) {
                const double phase=sample/120.;
                cat->animation().seek(phase*paws::pipWalkDuration);
                const auto skin=cat->skinPose();engine::Vec3 centroid{};float floor=100;
                for(auto v:sole) {
                    engine::Vec3 p{};
                    for(int k=0;k<4;++k)p=p+engine::transformPoint(skin->bones[mesh->skin[v].joints[k]].transform,mesh->vertices[v].position)*mesh->skin[v].weights[k];
                    centroid=centroid+p;floor=std::min(floor,p.y);
                }
                centroid=centroid*(1.f/sole.size());maxLift=std::max(maxLift,floor);
                // One contiguous contact interval; avoid wrap to the next footprint.
                if(phase>=contact+.025&&phase<=contact+2./3.-.025) {
                    const float worldZ=centroid.z+float(phase*paws::pipWalkDuration)*paws::pipWalkSpeed;
                    minZ=std::min(minZ,worldZ);maxZ=std::max(maxZ,worldZ);
                    minX=std::min(minX,centroid.x);maxX=std::max(maxX,centroid.x);
                    maxFloorError=std::max(maxFloorError,std::abs(floor-.003f));
                }
                check(floor>-.002f,"Walk sole penetrates floor between baked samples");
            }
            std::cout<<"Paw "<<foot<<": stance drift "<<maxZ-minZ<<" m, floor error "<<maxFloorError<<" m, lift "<<maxLift<<" m\n";
            check(maxZ-minZ<.003f,"Planted paw slides at authored travel speed");
            check(maxX-minX<.003f,"Planted paw slides sideways during weight transfer");
            check(maxFloorError<.003f,"Planted paw floats above contact plane");
            check(maxLift>.04f&&maxLift<.09f,"Swing paw has no controlled ground clearance");
        }

    std::cout<<"Exported walk contact, clearance and loop-boundary checks passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

