#include "engine/fbx_importer.hpp"
#include "engine/skeletal_mesh_component.hpp"
#include "pip_asset.hpp"
#include <iostream>
#include <cmath>
#include <map>
#include <algorithm>
using namespace engine;
Vec3 point(const Mesh& m,const SkinPose& p,size_t v) {
 Vec3 out{};for(int k=0;k<4;++k)out=out+transformPoint(p.bones.at(m.skin[v].joints[k]).transform,m.vertices[v].position)*m.skin[v].weights[k];return out;
}
int main(int argc,char** argv) {try {
 if(argc<2)return 2;
 FbxImportOptions options;
 if(argc>2)options.resolveTexture=[](std::string_view){return std::vector<std::byte>{};};
 const auto m=FbxImporter::loadFile(argv[1],options);
 std::cout<<"vertices "<<m->vertices.size()<<" triangles "<<m->indices.size()/3<<" skin "<<m->skin.size()<<" materials "<<m->materials.size()<<"\n";
 for(auto& w:m->warnings)std::cout<<"WARNING "<<w<<"\n";
 if(!m->skeleton){std::cout<<"NO SKELETON\n";return 1;}
 const auto& s=*m->skeleton;s.validate();
 std::cout<<"joints "<<s.joints.size()<<" bindings "<<s.bindings.size()<<" clips "<<s.clips.size()<<"\n";
 auto b=m->localBounds();std::cout<<"bounds "<<b.minimum.x<<","<<b.minimum.y<<","<<b.minimum.z<<" to "<<b.maximum.x<<","<<b.maximum.y<<","<<b.maximum.z<<"\n";
 std::map<unsigned,size_t> counts;size_t unweighted=0;float maxWeightError=0;
 for(auto& v:m->skin){float sum=0,weighted=0;for(int k=0;k<4;++k){sum+=v.weights[k];if(v.weights[k]>0){counts[v.joints[k]]++;if(v.joints[k])weighted+=v.weights[k];}}if(weighted<1e-6)unweighted++;maxWeightError=std::max(maxWeightError,std::abs(sum-1));}
 std::cout<<"identity_only_vertices "<<unweighted<<" weight_sum_max_error "<<maxWeightError<<"\n";
 for(size_t i=0;i<s.bindings.size();++i)std::cout<<"binding "<<i<<" joint "<<s.bindings[i].joint<<" "<<(i?s.joints[s.bindings[i].joint].name:"IDENTITY")<<" influenced_vertices "<<counts[unsigned(i)]<<"\n";
 SkeletalMeshComponent cat("FBX audit",m);auto skin=cat.skinPose();float maxBindError=0;
 for(size_t i=0;i<m->vertices.size();++i){auto d=point(*m,*skin,i)-m->vertices[i].position;maxBindError=std::max(maxBindError,std::sqrt(dot(d,d)));}
 std::cout<<"bind_position_max_error_m "<<maxBindError<<"\n";
 std::vector<JointPose> poses;for(auto& j:s.joints)poses.push_back(j.bindPose);
 // Move the cat armature root, leaving the mesh-node fallback stationary.
 for(size_t j=0;j<s.joints.size();++j)if(s.joints[j].name=="bone_0")poses[j].translation.x+=.1f;
 cat.skeleton().setPose(poses);skin=cat.skinPose();size_t stationary=0,moved=0;float maxFollowError=0;
 for(size_t i=0;i<m->vertices.size();++i){auto d=point(*m,*skin,i)-m->vertices[i].position;if(std::abs(d.x)<1e-5)stationary++;else moved++;maxFollowError=std::max(maxFollowError,std::abs(d.x-.1f));}
 std::cout<<"root_translation_test stationary "<<stationary<<" moved "<<moved<<" max_follow_error_m "<<maxFollowError<<"\n";
 try{paws::validatePip(*m);std::cout<<"PAWS_CONTRACT PASS\n";}catch(const std::exception& e){std::cout<<"PAWS_CONTRACT FAIL "<<e.what()<<"\n";}
 for(auto& j:s.joints)std::cout<<"joint "<<j.name<<" parent "<<j.parent<<"\n";
 }catch(const std::exception& e){std::cerr<<"IMPORT FAILURE "<<e.what()<<"\n";return 1;}
}

