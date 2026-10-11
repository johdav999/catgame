#include "engine/fbx_importer.hpp"
#include "engine/glb_importer.hpp"
#include "engine/skeletal_mesh_component.hpp"
#include "pip_asset.hpp"
#include <map>
#include <set>
#include <iostream>
#include <cmath>
#include <stdexcept>
using namespace engine;
void check(bool c,const char* e){if(!c)throw std::runtime_error(e);}
Vec3 vertex(const Mesh& m,const SkinPose& s,size_t v){Vec3 p{};for(int k=0;k<4;k++)p=p+transformPoint(s.bones.at(m.skin[v].joints[k]).transform,m.vertices[v].position)*m.skin[v].weights[k];return p;}
int main(int argc,char** argv){try{
 check(argc==3,"Expected repaired asset and canonical GLB");
 auto m=std::filesystem::path(argv[1]).extension()==".fbx"?FbxImporter::loadFile(argv[1]):GlbImporter::loadFile(argv[1]);
 auto canonical=GlbImporter::loadFile(argv[2]);paws::validatePip(*m);
 std::map<std::string,size_t> reference,actual;std::set<std::string> bones;
 for(size_t i=0;i<canonical->skeleton->joints.size();++i)reference[canonical->skeleton->joints[i].name]=i;
 for(size_t i=0;i<m->skeleton->joints.size();++i){actual[m->skeleton->joints[i].name]=i;auto& n=m->skeleton->joints[i].name;if(reference.contains(n)&&n!="PipRig"&&n!="Pip continuous coat"&&n!="")bones.insert(n);}
 const auto& s=*m->skeleton;
 float maxRestError=0;size_t compared=0;
 check(bones.size()==29,"Missing canonical bone");
 for(auto& [name,ri]:reference){if(!bones.contains(name))continue;
 check(actual.contains(name),"Missing canonical joint");const auto& a=s.joints[actual.at(name)];const auto& b=canonical->skeleton->joints[ri];
 if(name=="root")continue;
 check(a.parent>=0&&b.parent>=0&&s.joints[a.parent].name==canonical->skeleton->joints[b.parent].name,"Canonical hierarchy differs");
 const auto am=a.bindPose.matrix(),bm=b.bindPose.matrix();float diff=0;for(int k=0;k<16;k++){diff=std::max(diff,std::abs(am.m[k]-bm.m[k]));maxRestError=std::max(maxRestError,std::abs(am.m[k]-bm.m[k]));}if(diff>.0001f)std::cout<<"REST DIFFERENCE "<<name<<" "<<diff<<"\n";compared++;}
 check(maxRestError<.0001f,"Canonical local bind pose differs");
 std::map<std::string,size_t> influences;
 for(auto& v:m->skin){float sum=0;for(int k=0;k<4;k++){auto w=v.weights[k];sum+=w;if(w>0){auto& n=s.joints.at(s.bindings.at(v.joints[k]).joint).name;check(bones.contains(n)||n=="blink.lower.L"||n=="blink.lower.R","Vertex falls back to a mesh node or identity");influences[n]++;}}check(std::abs(sum-1)<.0001f,"Weights not normalized");}
 SkeletalMeshComponent cat("Rig check",m);auto skin=cat.skinPose();float bindError=0;
 for(size_t i=0;i<m->vertices.size();i++){auto d=vertex(*m,*skin,i)-m->vertices[i].position;bindError=std::max(bindError,std::sqrt(dot(d,d)));}
 check(bindError<.00001f,"Bind geometry differs");
 std::vector<JointPose> pose;for(auto& j:s.joints)pose.push_back(j.bindPose);pose.at(actual.at("root")).translation.x+=.1f;cat.skeleton().setPose(pose);skin=cat.skinPose();float followError=0;
 for(size_t i=0;i<m->vertices.size();i++){auto d=vertex(*m,*skin,i)-m->vertices[i].position;followError=std::max(followError,std::abs(std::sqrt(dot(d,d))-.1f));}
 check(followError<.00001f,"A vertex fails to follow root");
 for(auto n:{"front_upper.L","front_lower.L","front_paw.L","front_upper.R","front_lower.R","front_paw.R","hind_upper.L","hind_lower.L","hind_paw.L","hind_upper.R","hind_lower.R","hind_paw.R","head","ear.L","ear.R","blink.L","blink.R","tail.00","tail.01","tail.02","tail.03","tail.04","tail.05","tail.06"})check(influences[n]>0,"An anatomical joint has no weights");
 std::cout<<"PASS canonical hierarchy and "<<compared<<" local bind poses; max rest error "<<maxRestError<<"; all "<<m->vertices.size()<<" vertices follow root; max follow error "<<followError<<"; bind error "<<bindError<<"; all limb/face/tail bones weighted\n";
 for(auto& [n,count]:influences)std::cout<<n<<" "<<count<<" influenced vertices\n";
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}

