#include "garden_kit.hpp"
#include "engine/glb_importer.hpp"
#if PAWS_TERRAIN
#include "engine/terrain.hpp"
#endif
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <stdexcept>
int main(int argc,char** argv){try{
    if(argc!=2)throw std::runtime_error("Expected model directory");
    paws::GardenMeshes meshes;
    for(const auto& a:paws::gardenAssets()){
        auto mesh=engine::GlbImporter::loadFile(std::filesystem::path(argv[1])/(std::string(a.name)+".glb"));
        paws::validateGardenAsset(a,*mesh);meshes.emplace(a.name,mesh);
    }
    if(meshes.size()!=48||meshes.contains("back_enclosure"))throw std::runtime_error("Garden kit, paving/foliage LODs, sequence blackboard, fruit trees and perimeter details are required");
    for(auto size:{engine::Vec3{7,0,5},engine::Vec3{8,0,6},engine::Vec3{12,0,10}}){
        engine::Scene scene("Enclosure clearance");
        paws::addGardenEnclosure(scene,meshes,-size.x/2,size.x/2,-size.z/2,size.z/2);
        unsigned count=0;
        while(auto* actor=dynamic_cast<engine::Actor*>(scene.findSavedNode("garden.enclosure."+std::to_string(count)))){
            for(const auto& v:actor->getComponent<engine::MeshComponent>()->mesh->vertices){
                auto p=engine::transformPoint(actor->transform.matrix(),v.position);
                if(std::abs(p.x)<size.x/2-.02f&&std::abs(p.z)<size.z/2-.02f)throw std::runtime_error("Enclosure enters playable floor");
                if(p.y<-.01f||p.y>.87f)throw std::runtime_error("Enclosure ground contact or height changed");
            }
            ++count;
        }
        if(count<4)throw std::runtime_error("Missing front or side enclosure");
        auto* house=dynamic_cast<engine::Actor*>(scene.findSavedNode("garden.house"));
        if(!house)throw std::runtime_error("Missing background house");
        for(const auto& v:house->getComponent<engine::MeshComponent>()->mesh->vertices){
            auto p=engine::transformPoint(house->transform.matrix(),v.position);
            if(p.z>-size.z/2-.19f||p.y<-.01f)throw std::runtime_error("House enters courtyard or loses ground contact");
        }
#if PAWS_TERRAIN
        auto* terrain=dynamic_cast<engine::Actor*>(scene.findSavedNode("garden.terrain"));
        if(!terrain)throw std::runtime_error("Missing WebEngine terrain");
        const auto chunks=terrain->getComponents<engine::TerrainComponent>();
        if(chunks.size()!=64)throw std::runtime_error("Terrain chunk budget changed");
        const auto& asset=chunks.front()->asset();const auto origin=terrain->transform.position;
        const auto ground=[&](float x,float z){return asset->ground(x-origin.x,z-origin.z);};
        // Query the actual interpolated terrain, including slab seams and the
        // complete house base, rather than only comparing source samples.
        const auto flat=[&](float x,float z){
            const auto p=ground(x,z);
            if(!p||std::abs(p->position.y+.08f)>.0001f||p->normal.y<.9999f)
                throw std::runtime_error("House/garden apron is not flat");
        };
        for(float x=-size.x/2-6;x<=size.x/2+6;x+=.7f)
            for(float z=-size.z/2-6;z<=size.z/2+6;z+=.7f)flat(x,z);
        for(const auto& v:house->getComponent<engine::MeshComponent>()->mesh->vertices){
            const auto p=engine::transformPoint(house->transform.matrix(),v.position);flat(p.x,p.z);
        }
        const auto [low,high]=std::minmax_element(asset->data().heights.begin(),asset->data().heights.end());
        if(*high-*low<8)throw std::runtime_error("Surrounding hills missing");
        for(const auto* chunk:chunks){
            if(chunk->asset()!=asset||chunk->mesh->lods.empty()||chunk->castsShadow)
                throw std::runtime_error("Terrain sharing/LOD/shadow policy changed");
        }
        if(scene.physicsIfCreated())throw std::runtime_error("Decorative terrain created physics");
        for(unsigned i=0;i<20;++i){
            auto* tree=dynamic_cast<engine::Actor*>(scene.findSavedNode("garden.hill-tree."+std::to_string(i)));
            if(!tree)throw std::runtime_error("Missing hillside grove tree");
            const auto p=tree->transform.position;
            const auto contact=ground(p.x,p.z);
            const auto* mesh=tree->getComponent<engine::MeshComponent>();
            const float base=p.y+mesh->mesh->localBounds().minimum.y*tree->transform.scale.y;
            if(!contact||std::abs(base-contact->position.y+.04f)>.001f)
                throw std::runtime_error("Hillside trunk loses terrain contact");
            if(contact->position.y<.2f||p.z>size.z/2)
                throw std::runtime_error("Grove must occupy a side or rear hill");
        }
        // Entire near/far frustum footprint must fit inside terrain, even at
        // low orbit angles, both sides, and pulled-back chapter portrait views.
        auto& view=scene.spawn("Terrain coverage camera").addComponent<engine::CameraComponent>();
        view.setPerspective(engine::pi*35/180,.05f,180);
        for(float aspect:{390.f/844,1440.f/900,844.f/390,3.6f})for(float yaw:{0.f,1.5f,3.f,4.5f})for(float elevation:{.05f,.478f,1.45f}){
            view.transform.position={std::sin(yaw)*std::cos(elevation)*64,std::sin(elevation)*64,std::cos(yaw)*std::cos(elevation)*64};
            view.transform.rotation={-elevation,yaw,0};
            const engine::PixelViewport viewport{0,0,900*aspect,900};
            for(float x:{0.f,viewport.width})for(float y:{0.f,900.f})for(float d:{0.f,1.f}){
                const auto p=view.screenToWorld({x,y},d,viewport);
                if(!ground(p.x,p.z))throw std::runtime_error("Terrain fails camera frustum coverage");
            }
        }
#endif
        unsigned greenery=0,shrubs=0;
        const engine::Mesh* sharedFoliage=nullptr;
        while(auto* actor=dynamic_cast<engine::Actor*>(scene.findSavedNode("garden.greenery."+std::to_string(greenery)))){
            const auto& mesh=actor->getComponent<engine::MeshComponent>()->mesh;
            if(actor->name=="Dense exterior foliage"){
                ++shrubs;
                if(sharedFoliage&&sharedFoliage!=mesh.get())throw std::runtime_error("Foliage geometry is not shared");
                sharedFoliage=mesh.get();
                if(mesh->lods.size()!=2)throw std::runtime_error("Foliage LOD chain missing");
                for(const auto& level:mesh->lods){
                    if(level.mesh->indices.size()>=mesh->indices.size()||level.mesh->materials!=mesh->materials)
                        throw std::runtime_error("LOD geometry is not reduced or materials are duplicated");
                    for(const auto& v:level.mesh->vertices){
                        const auto p=engine::transformPoint(actor->transform.matrix(),v.position);
                        if(std::abs(p.x)<size.x/2-.02f&&std::abs(p.z)<size.z/2-.02f)throw std::runtime_error("Foliage LOD enters playable floor");
                    }
                }
                if(mesh->materials.size()!=2||mesh->materials[0]->doubleSided||!mesh->materials[1]->doubleSided)
                    throw std::runtime_error("Solid foliage culling or thin-leaf coverage changed");
            }
            for(const auto& v:mesh->vertices){
                auto p=engine::transformPoint(actor->transform.matrix(),v.position);
                if(std::abs(p.x)<size.x/2-.02f&&std::abs(p.z)<size.z/2-.02f)throw std::runtime_error("Exterior greenery enters playable floor");
                if(std::abs(p.x)<size.x/2&&p.z>size.z/2&&p.y>.47f)throw std::runtime_error("Foreground greenery hides the low fence");
            }
            ++greenery;
        }
        if(shrubs<100||shrubs>270)throw std::runtime_error("Exterior border density budget changed");
        std::cout<<"Courtyard "<<size.x<<'x'<<size.z<<": "<<shrubs<<" shared foliage instances\n";
        for(unsigned i=0;i<6;++i){
            auto* actor=dynamic_cast<engine::Actor*>(scene.findSavedNode("garden.tree."+std::to_string(i)));
            if(!actor)throw std::runtime_error("Missing fruit tree");
            for(const auto& v:actor->getComponent<engine::MeshComponent>()->mesh->vertices){
                const auto p=engine::transformPoint(actor->transform.matrix(),v.position);
                if(std::abs(p.x)<size.x/2&&std::abs(p.z)<size.z/2)throw std::runtime_error("Fruit crown enters playable floor");
                if(p.y<-.01f)throw std::runtime_error("Fruit tree loses ground contact");
            }
        }
    }
    std::cout<<"House, three borders and planting validated; puzzle and chapter clearance passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
