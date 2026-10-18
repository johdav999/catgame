#include "release1/props/prop_assets.hpp"
#include "engine/glb_importer.hpp"
#include "engine/application.hpp"
#include <filesystem>
#include <iostream>
#include <cmath>
#include <stdexcept>
namespace prop=paws::release1::props;
void check(bool v,const std::string& message){if(!v)throw std::runtime_error(message);}
float surface(const engine::Mesh& m,float x,float z){
    float top=-100;
    for(std::size_t i=0;i<m.indices.size();i+=3){
        auto a=m.vertices[m.indices[i]].position,b=m.vertices[m.indices[i+1]].position,c=m.vertices[m.indices[i+2]].position;
        float d=(b.z-c.z)*(a.x-c.x)+(c.x-b.x)*(a.z-c.z);if(std::abs(d)<1e-9f)continue;
        float u=((b.z-c.z)*(x-c.x)+(c.x-b.x)*(z-c.z))/d,v=((c.z-a.z)*(x-c.x)+(a.x-c.x)*(z-c.z))/d;
        if(u>=-.00001f&&v>=-.00001f&&u+v<=1.00001f)top=std::max(top,u*a.y+v*b.y+(1-u-v)*c.y);
    }return top;
}
int main(){try{
    auto root=std::filesystem::path(__FILE__).parent_path().parent_path().parent_path().parent_path();paws::GardenMeshes meshes;unsigned total=0;
    auto load=[&](const paws::GardenAsset& a){
        auto relative=std::string(a.file).substr(7);auto m=engine::GlbImporter::loadFile(root/"assets/runtime"/relative);
        paws::validateGardenAsset(a,*m);total+=m->indices.size()/3;
        for(const auto& v:m->vertices)check(std::isfinite(v.position.x)&&std::isfinite(v.position.y)&&std::isfinite(v.position.z),std::string(a.name)+" nonfinite geometry");
        // The older garden imports include accepted 1024px maps. This prop kit
        // and the new sequence blackboard retain their tighter 512px delivery.
        const unsigned mapLimit=std::string_view(a.file).starts_with("assets/models/release1/props/")||std::string_view(a.name)=="sequence_blackboard"?512:1024;
        for(const auto& mat:m->materials){mat->validate();if(mat->baseColorTexture)check(mat->baseColorTexture->width<=mapLimit&&mat->baseColorTexture->height<=mapLimit,std::string(a.name)+" exceeds its map budget");}
        meshes.emplace(a.name,std::move(m));
    };
    for(const auto& a:prop::assets())load(a);for(const auto& a:paws::gardenAssets())load(a);
    check(prop::assets().size()>=30,"Missing release role");
    for(auto name:{"plank_1m","plank_2m"})for(float x:{-.40f,0.f,.40f})check(std::abs(surface(*meshes.at(name),x,.12f)-.16f)<.003f,"Plank support height drift");
    for(auto kind:{"square","bar","elbow"}){
        auto& cushion=*meshes.at(std::string("cushion_")+kind);auto& socket=*meshes.at(std::string("socket_")+kind);unsigned collisions=0;
        for(const auto& v:cushion.vertices){auto p=v.position;p.y+=.036f;if(surface(socket,p.x,p.z)>p.y+.002f)++collisions;}
        check(collisions==0,std::string(kind)+" cushion collides with socket: "+std::to_string(collisions));
    }
    const auto& house=*meshes.at("playhouse_frame");auto hb=house.localBounds();
    check(hb.maximum.x-hb.minimum.x<=1.5f&&hb.maximum.z-hb.minimum.z<=1.6f,"Playhouse exceeds frozen footprint");
    // Vertices in the central entrance prism may only belong to the supported floor.
    for(const auto& v:house.vertices){auto p=v.position;check(!(std::abs(p.x)<.3f&&p.z>.63f&&p.y>.071f&&p.y<1.17f),"Playhouse entrance below 1.1 m clear height");}
    for(const auto& v:meshes.at("bench")->vertices){auto p=v.position;check(!(std::abs(p.x)<.3f&&std::abs(p.z)<.25f&&p.y>.001f&&p.y<.25f),"Bench retrieval clearance obstructed");}
    for(auto [name,w,d]:{std::tuple{"scratching_post",.65f,.65f},{"sunny_perch",.8f,1.4f},{"hammock",1.1f,1.6f},{"low_picnic_table",1.2f,1.4f},{"picnic_mat",1.5f,1.5f}}){auto b=meshes.at(name)->localBounds();check(b.maximum.x-b.minimum.x<=w+.001f&&b.maximum.z-b.minimum.z<=d+.001f,std::string(name)+" footprint exceeded");}
    auto preview=prop::makePreview(meshes);engine::Application app;auto id=app.loadScene(std::move(preview.scene.scene));app.start(id);app.advance(0);
    for(int i=0;i<12;++i){prop::pose(*app.activeScene(),preview,i%2);app.advance(.02*(i+1));}
    prop::pose(*app.activeScene(),preview,true);
    for(const auto& b:preview.bindings)if(b.instance.group&&std::string_view(b.instance.asset)=="switch_plate")check(std::abs(app.activeScene()->findActor(b.actor)->transform.position.y-.055f)<.001f,"Pressed plate drift");
    prop::pose(*app.activeScene(),preview,false);
    for(const auto& b:preview.bindings){auto delta=app.activeScene()->findActor(b.actor)->transform.position-b.instance.position;check(engine::dot(delta,delta)<.00000001f,"Rest pose retains old transform");}
    app.shutdown();check(!app.findScene(id),"Preview scene survived shutdown");
    // Stale bindings are harmless in a new generation.
    engine::Scene empty("Replacement");prop::pose(empty,preview,true);
    std::cout<<prop::assets().size()<<" new / "<<meshes.size()<<" total meshes, "<<total<<" triangles: import, fits, footprints, clearance, assembly reset and lifecycle passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
