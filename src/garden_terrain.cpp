#include "garden_terrain.hpp"
#if PAWS_TERRAIN
#include "engine/terrain.hpp"
#include <algorithm>
#include <cmath>

namespace paws {
engine::Actor* addGardenTerrain(engine::Scene& scene,const GardenMeshes& meshes,
    float left,float right,float back,float front,engine::Actor* parent) {
    constexpr unsigned samples=257;
    constexpr float spacing=3.f,extent=(samples-1)*spacing,repeat=3.f;
    const float originX=(left+right-extent)*.5f,originZ=(back+front-extent)*.5f;
    const auto height=[&](float x,float z){
        // One source-cell guard keeps interpolated triangles flat throughout
        // the footprint, even where its boundary falls between sample rows.
        const float dx=std::max({left-3.f-x,0.f,x-right-3.f});
        const float dz=std::max({back-3.f-z,0.f,z-front-3.f});
        const float t=std::clamp(std::hypot(dx,dz)/16.f,0.f,1.f);
        const float blend=t*t*t*(t*(t*6-15)+10);
        const float hills=5.5f+3.f*std::sin(x*.077f+z*.031f)
            +2.f*std::cos(z*.093f-x*.026f)+1.2f*std::sin(x*.043f-z*.067f);
        return -.08f+blend*hills;
    };
    engine::TerrainData data;
    data.samplesX=data.samplesZ=samples;data.spacing={spacing,spacing};
    data.chunkCells=32;data.textureMeters=repeat;
    data.heights.reserve(samples*samples);
    auto weights=std::make_shared<engine::Texture>();weights->name="Meadow and limestone terrain weights";
    weights->width=weights->height=samples;weights->wrapU=weights->wrapV=engine::TextureWrap::Clamp;
    weights->pixels.reserve(samples*samples*4);
    for(unsigned z=0;z<samples;++z)for(unsigned x=0;x<samples;++x){
        const float wx=originX+x*spacing,wz=originZ+z*spacing,h=height(wx,wz);
        data.heights.push_back(h);
        const float slope=std::hypot(height(wx+1,wz)-height(wx-1,wz),height(wx,wz+1)-height(wx,wz-1))*.5f;
        const auto earth=static_cast<std::uint8_t>(255.f*std::clamp((slope-.16f)*.65f+std::max(0.f,h-7.f)*.035f,0.f,.42f));
        weights->pixels.insert(weights->pixels.end(),{static_cast<std::uint8_t>(255-earth),earth,0,0});
    }
    auto material=std::make_shared<engine::Material>();material->name="ImageGen meadow terrain";material->roughness=1;
    material->layers.enabled=true;material->layers.weights=weights;
    material->layers.weightScale={repeat/extent,repeat/extent};
    // Native geometry fixtures may omit HTTP-loaded textures; runtime preparation
    // requires both before publishing a scene.
    for(unsigned i=0;i<2;++i){
        auto found=meshes.find(i?"terrain_earth":"grass_ground");
        if(found!=meshes.end())material->layers.layers[i].texture=found->second->materials.at(0)->baseColorTexture;
    }
    material->layers.layers[0].color={.78f,.9f,.72f};
    material->layers.layers[1].uvScale={.75f,.75f};
    data.material=material;
    auto actor=engine::makeTerrainActor("Rolling garden terrain",engine::TerrainAsset::create(std::move(data)));
    actor->transform.position={originX,0,originZ};
    if(!parent)actor->setSavedId("garden.terrain");
    for(auto* chunk:actor->getComponents<engine::TerrainComponent>()){
        chunk->color={1,1,1};chunk->castsShadow=false;
    }
    auto* result=actor.get();
    (parent?static_cast<engine::Node*>(parent):static_cast<engine::Node*>(&scene))->adoptChild(std::move(actor));
    return result;
}
}
#endif
