#include "pip_asset.hpp"
#include "engine/glb_importer.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
void require(bool ok,const char* message) {if(!ok)throw std::runtime_error(message);}
int main(int argc,char** argv) {
 try {
  require(argc==2,"Expected Pip GLB");
  const auto mesh=engine::GlbImporter::loadFile(argv[1]);paws::validatePip(*mesh);
  const auto& skeleton=*mesh->skeleton;auto preview=paws::makePipPreview(mesh);
  auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(preview.scene.scene->findComponent(preview.cat));
  for(const auto name:{"settle_cushion","curled_rest"}) {
   const auto index=paws::pipClip(*mesh,name);const auto& clip=skeleton.clips[index];
   require(std::abs(clip.duration-(std::string_view(name)=="settle_cushion"?2.4:4.))<.002,"Clip timing changed");
   cat->animation().play(index,false);float lowest=10,highest=-10,maxStep=0;
   std::vector<engine::Vec3> previous;
   for(int sample=0;sample<=int(std::round(clip.duration*60));++sample) {
    const double time=std::min(clip.duration,sample/60.);
    for(std::size_t j=0;j<skeleton.joints.size();++j) {
     const auto& joint=skeleton.joints[j];const auto pose=clip.sampleJoint(time,j,skeleton.joints.size());
     if(joint.name=="pelvis"||joint.name.starts_with("blink."))continue;
     const auto d=pose.translation-joint.bindPose.translation,s=pose.scale-engine::Vec3{1,1,1};
     require(engine::dot(d,d)<4e-8f,"Disconnected settle/rest joint");
     require(engine::dot(s,s)<4e-8f,"Scaled settle/rest joint");
    }
    cat->animation().seek(time);const auto skin=cat->skinPose();std::vector<engine::Vec3> current;
    for(std::size_t v=0;v<mesh->vertices.size();++v) {
     engine::Vec3 p{};
     for(int k=0;k<4;++k)p=p+engine::transformPoint(skin->bones[mesh->skin[v].joints[k]].transform,mesh->vertices[v].position)*mesh->skin[v].weights[k];
     require(std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z),"Nonfinite skin");
     lowest=std::min(lowest,p.y);highest=std::max(highest,p.y);
     if(!previous.empty()){const auto d=p-previous[v];maxStep=std::max(maxStep,std::sqrt(engine::dot(d,d)));}
     current.push_back(p);
    }
    previous=std::move(current);
   }
   require(lowest>=-.005f,"Settle/rest penetrates contact plane");
   require(maxStep<.06f,"Settle/rest has a frame pop");
   std::cout<<name<<": floor="<<lowest<<", height="<<highest<<", max 60Hz vertex step="<<maxStep<<'\n';
  }
  const auto& a=skeleton.clips[paws::pipClip(*mesh,"settle_cushion")];const auto& b=skeleton.clips[paws::pipClip(*mesh,"curled_rest")];
  for(std::size_t j=0;j<skeleton.joints.size();++j) {
   const auto x=a.sampleJoint(a.duration,j,skeleton.joints.size()).matrix(),y=b.sampleJoint(0,j,skeleton.joints.size()).matrix(),z=b.sampleJoint(b.duration,j,skeleton.joints.size()).matrix();
   for(int k=0;k<16;++k)require(std::abs(x.m[k]-y.m[k])<.001&&std::abs(y.m[k]-z.m[k])<.001,"Settle/rest seam");
  }
  std::cout<<"Connected joints, interpolation, ground, temporal continuity and rest seam passed\n";
 } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
