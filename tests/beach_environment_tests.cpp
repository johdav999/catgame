#include "beach_environment.hpp"
#include "release1/rules/content.hpp"
#include "engine/mesh_component.hpp"
#include "engine/camera_component.hpp"
#if PAWS_TERRAIN
#include "engine/terrain.hpp"
#endif
#include <iostream>
#include <stdexcept>

void check(bool condition,const char* message){if(!condition)throw std::runtime_error(message);}
int main(){try{
 for(const auto& definition:paws::release1::rules::catalogue()){
    if(!definition.quest.starts_with("beach_"))continue;
    const int width=definition.width,depth=definition.height;
    const bool runnel=definition.family==paws::release1::rules::Family::Planks;
    engine::Scene scene; paws::addBeachEnvironment(scene,runnel,width,depth);scene.synchronize();
    check(!scene.physicsIfCreated(),"Beach decoration must not create a physics owner");
    paws::startBeachSurf(scene);
    auto surf=paws::updateBeachSurf(scene,.2,false,true);
    check(surf.starts==1&&surf.opacity>0,"A started wave exposes the incoming foam");
    const auto early=surf;
    for(int i=0;i<5;++i)surf=paws::updateBeachSurf(scene,.2,false,true);
    check(surf.travel>early.travel&&surf.opacity>0,"Sound-linked wash moves toward the beach");
    const auto paused=paws::updateBeachSurf(scene,0,false,true);
    check(paused.age==surf.age&&paused.travel==surf.travel,"Pause preserves surf phase");
    for(int i=0;i<9;++i)surf=paws::updateBeachSurf(scene,.2,false,true);
    check(surf.opacity==0&&surf.starts==1,"Foam recedes by recording end without inventing another sound");
    surf=paws::updateBeachSurf(scene,.25,false,false);
    check(surf.age>3&&surf.opacity==0,"Muted surf also waits for the halved wave cadence");
    for(int i=0;i<18;++i)surf=paws::updateBeachSurf(scene,.2,false,false);
    check(surf.age<.5&&surf.opacity>0,"Muted beach retains gentle incoming surf");
    check(paws::updateBeachSurf(scene,.2,true,false).opacity==0,"Reduced motion suppresses moving foam");
    paws::startBeachSurf(scene);surf=paws::updateBeachSurf(scene,0,false,true);
    check(surf.starts==2&&surf.age==0,"Next actual audio start resynchronizes the wash");
#if PAWS_TERRAIN
    auto* ground=dynamic_cast<engine::Actor*>(scene.findSavedNode("beach.terrain"));
    check(ground,"Beach terrain missing");
    const auto chunks=ground->getComponents<engine::TerrainComponent>();check(chunks.size()==64,"Bounded beach chunk count");
    const auto asset=chunks.front()->asset();
    for(float z=-depth*.5f-.45f;z<=depth*.5f+.45f;z+=.23f)for(float x=-width*.5f-.45f;x<=width*.5f+1.45f;x+=.23f){
        const auto p=asset->ground(x+128,z+128);
        check(p&&std::abs(p->position.y-paws::beachGroundHeight)<.00001f&&p->normal.y>.9999f,"Playable apron must remain exactly flat, including interpolated triangles");
    }
    check(asset->ground(128,118)->position.y<-1,"Seabed must slope below the surface");
    check(asset->ground(138,128)->position.y>.5f,"Side dunes must rise beyond the puzzle");
    // Every rotated boardwalk corner, including interpolation between source
    // cells, needs the same sand height; sloped dunes must not swallow a post.
    for(float x=-.9f;x<=.9f;x+=.15f)for(float z=-1.3f;z<=1.3f;z+=.15f){
        const auto p=asset->ground(128-width*.5f-1.55f+x,128-depth*.5f-.15f+z);
        if(!p||std::abs(p->position.y-paws::beachGroundHeight)>=.00001f)
            throw std::runtime_error("Boardwalk bed "+definition.quest+" offset "+std::to_string(x)+","+std::to_string(z)+" height "+std::to_string(p?p->position.y:999.f));
    }
    for(const auto* chunk:chunks)check(chunk->asset()==asset&&!chunk->castsShadow&&!chunk->mesh->lods.empty(),"Shared terrain LOD and shadow policy");
    auto& camera=scene.spawn("Coverage").addComponent<engine::CameraComponent>();camera.setPerspective(engine::pi*35/180,.05f,80);
    for(float aspect:{390.f/844,1600.f/1000,844.f/390})for(float elevation:{.50f,.82f,.88f}){
        camera.transform.position={0,35*std::sin(elevation),35*std::cos(elevation)};camera.transform.rotation={-elevation,0,0};
        engine::PixelViewport vp{0,0,900*aspect,900};
        for(float x:{0.f,vp.width})for(float y:{0.f,vp.height})for(float d:{0.f,1.f}){
            const auto p=camera.screenToWorld({x,y},d,vp);check(asset->ground(p.x+128,p.z+128).has_value(),"Terrain must cover the full camera frustum");
        }
    }
#endif
    unsigned surfaces=0;
    scene.visit([&](const engine::Node& n,const engine::Mat4&){
        const auto* m=dynamic_cast<const engine::MeshComponent*>(&n);
        if(!m||(n.name!="Beach sea"&&n.name!="Shallow beach runnel"))return;
        ++surfaces;check(!m->castsShadow,"Water has no solid shadow");
        for(std::size_t i=0;i<m->mesh->indices.size();i+=3){
            const auto& vs=m->mesh->vertices;const auto a=vs[m->mesh->indices[i]].position,b=vs[m->mesh->indices[i+1]].position,c=vs[m->mesh->indices[i+2]].position;
            check(engine::cross(b-a,c-a).y>0,"Water grid must face upward");
        }
#if PAWS_WATER_SHADER
        const auto material=m->resolvedMaterial(0);check(material.shader==engine::MaterialShader::Water,"Sea and runnel use engine water");
        if(n.name=="Beach sea"){
            check(material.water.foamStrength>0&&material.water.refractionStrength>0,"Sea includes shallow-water effects");
            check(m->mesh->vertices.size()>1000,"Geometric waves require subdivided sea geometry");
            check(material.water.planarReflectionStrength==0&&material.water.underwaterStrength==0,"No unnecessary reflection or submerged-view capture");
        }
#endif
    });check(surfaces==(runnel?2u:1u),"One sea, plus the bridge's optional runnel");
 }
 std::cout<<"Beach ground, water and frustum checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
