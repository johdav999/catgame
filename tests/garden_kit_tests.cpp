#include "garden_kit.hpp"
#include "engine/glb_importer.hpp"
#include "engine/application.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <set>
void check(bool v,const char* text){if(!v)throw std::runtime_error(text);}
// Highest triangle hit by a downward ray, independent of authored metadata.
float surface(const engine::Mesh& mesh,float x,float z){
    float top=-100;
    for(std::size_t i=0;i<mesh.indices.size();i+=3){
        const auto a=mesh.vertices[mesh.indices[i]].position,b=mesh.vertices[mesh.indices[i+1]].position,c=mesh.vertices[mesh.indices[i+2]].position;
        const float d=(b.z-c.z)*(a.x-c.x)+(c.x-b.x)*(a.z-c.z);if(std::abs(d)<1e-9f)continue;
        const float u=((b.z-c.z)*(x-c.x)+(c.x-b.x)*(z-c.z))/d,v=((c.z-a.z)*(x-c.x)+(a.x-c.x)*(z-c.z))/d;
        if(u>=-.00001f&&v>=-.00001f&&u+v<=1.00001f)top=std::max(top,u*a.y+v*b.y+(1-u-v)*c.y);
    }return top;
}
int main(int argc,char**argv){try{
    check(argc==2,"Expected runtime model directory");const std::filesystem::path root=argv[1];paws::GardenMeshes meshes;unsigned triangles=0;
    for(const auto& spec:paws::gardenAssets()){
        auto m=engine::GlbImporter::loadFile(root/"garden"/(std::string(spec.name)+".glb"));paws::validateGardenAsset(spec,*m);triangles+=m->indices.size()/3;
        for(const auto& v:m->vertices)check(std::isfinite(v.position.x)&&std::isfinite(v.normal.y),"Nonfinite geometry");
        for(const auto& material:m->materials){material->validate();if(material->baseColorTexture)check(material->baseColorTexture->width<=512&&material->baseColorTexture->height<=512,"Oversized prop texture");}
        meshes.emplace(spec.name,std::move(m));
    }
    check(meshes.size()==21&&triangles<60000,"Kit inventory or triangle budget");
    for(float x:{-.48f,-.25f,0.f,.25f,.48f})check(std::abs(surface(*meshes.at("channel_straight"),x,0)-.06f)<.004f,"Straight route blocked or floor discontinuity");
    for(int i=1;i<24;++i){float a=-engine::pi/2+i*engine::pi/48;float x=-.5f+.5f*std::cos(a),z=-(.5f+.5f*std::sin(a));check(std::abs(surface(*meshes.at("channel_curve"),x,z)-.06f)<.004f,"Curved route blocked or floor discontinuity");}
    const auto r=meshes.at("ball")->localBounds();check(r.maximum.x-r.minimum.x<.26f,"Ball does not clear channels");
    check(std::abs(surface(*meshes.at("bowl"),0,0)-.30f)<.004f,"Bowl must have a real open interior");
    bool socketsFit=true;
    for(auto [toy,socket]:{std::pair{"ball","ball_socket"},{"mouse","mouse_socket"},{"fish","fish_socket"}}){
        const auto anchor=paws::gardenAnchor("basket",socket);
        check(surface(*meshes.at("basket"),anchor.x,anchor.z)<.065f,"Basket socket capped at rim");
        unsigned colliding=0;
        for(const auto& v:meshes.at(toy)->vertices){auto p=v.position+anchor;
            if(p.y<.171f&&surface(*meshes.at("basket"),p.x,p.z)>.175f){++colliding;std::cout<<"Collision "<<toy<<" at "<<p.x<<","<<p.y<<","<<p.z<<'\n';}
        }
        std::cout<<toy<<" socket collisions: "<<colliding<<'\n';socketsFit&=colliding==0;
    }
    const auto pip=engine::GlbImporter::loadFile(root/"pip/pip.glb");auto preview=paws::makeGardenPreview(meshes,pip);
    engine::Application app;auto id=app.loadScene(std::move(preview.scene.scene));app.start(id);app.advance(0);
    auto* scene=app.activeScene();
    for(int repeat=0;repeat<4;++repeat)for(int mode=0;mode<4;++mode){paws::poseGardenPreview(*scene,preview,mode);app.advance((repeat*4+mode+1)*.02);}
    paws::poseGardenPreview(*scene,preview,1);
    check(std::abs(scene->findActor(preview.gate)->transform.rotation.y-100*engine::pi/180)<.001f,"Gate did not rotate about exported hinge");
    check(std::abs(scene->findActor(preview.plate)->transform.position.y-.055f)<.001f,"Pressure plate travel wrong");
    check(std::abs(scene->findActor(preview.ball)->transform.position.y-scene->findActor(preview.plate)->transform.position.y-.05f)<.001f,"Ball does not rest on the pressed plate");
    // Open gate geometry must vacate a Pip-wide passage centred at x=1.6.
    for(const auto& v:meshes.at("gate_leaf")->vertices){auto p=engine::transformPoint(scene->findActor(preview.gate)->transform.matrix(),v.position);check(p.x<1.18f,"Opened gate intrudes into clear passage");}
    paws::poseGardenPreview(*scene,preview,3);app.advance(.5);
    auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(scene->findComponent(preview.cat));
    const auto& joints=pip->skeleton->joints;auto head=std::find_if(joints.begin(),joints.end(),[](const auto& j){return j.name=="head";});
    auto muzzle=engine::transformPoint(cat->skinPose()->bones[std::size_t(head-joints.begin())].transform,{0,.643f,.647f});
    std::cout<<"Drink muzzle in cat space: "<<muzzle.x<<","<<muzzle.y<<","<<muzzle.z<<'\n';
    const auto drinkRoot=paws::gardenAnchor("bowl","drink_root"),water=paws::gardenAnchor("bowl","water");
    check(std::abs(muzzle.z+drinkRoot.z)<.025f&&std::abs(muzzle.y-water.y)<.025f,"Pip muzzle does not reach bowl water");
    paws::poseGardenPreview(*scene,preview,0);check(scene->findActor(preview.gate)->transform.rotation.y==0,"Reset left gate open");
    app.shutdown();check(!app.findScene(id),"Preview scene retained after shutdown");
    check(socketsFit,"Toy silhouette intersects basket insert/rim");
    std::cout<<"Garden: "<<meshes.size()<<" meshes, "<<triangles<<" triangles; import, route, sockets, assemblies and lifecycle passed\n";
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
