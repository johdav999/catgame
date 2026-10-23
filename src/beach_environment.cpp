#include "beach_environment.hpp"
#include "beach_assets.hpp"
#include "engine/mesh_component.hpp"
#if PAWS_TERRAIN
#include "engine/terrain.hpp"
#endif
#include <algorithm>
#include <cmath>

namespace paws {
namespace {
#include "beach_manifest.inc"
float smooth(float a,float b,float x){const float t=std::clamp((x-a)/(b-a),0.f,1.f);return t*t*(3-2*t);}
float shoreline(float x,float rear){return -rear-2.f+.3f*std::sin(x*.6f)+std::min(4.f,.025f*x*x);}
float groundHeight(engine::Scene& scene,float x,float z){
#if PAWS_TERRAIN
    auto* actor=dynamic_cast<engine::Actor*>(scene.findSavedNode("beach.terrain"));
    if(actor)if(auto* chunk=actor->getComponent<engine::TerrainComponent>())
        if(auto p=chunk->asset()->ground(x+128,z+128))return p->position.y;
#endif
    return beachGroundHeight;
}
class BeachSurf final:public engine::Component {
public:
    BeachSurf():Component("Incoming surf"){}
    BeachSurfState state{3.3,0,0,0};
    std::array<engine::ComponentId,2> bands{};
};
BeachSurf* surf(engine::Scene& scene){
    auto* a=dynamic_cast<engine::Actor*>(scene.findSavedNode("beach.surf"));
    return a?a->getComponent<BeachSurf>():nullptr;
}
void addSurf(engine::Scene& scene,float rear){
    // Two small transparent effect strips follow the cove, outside the puzzle.
    // Soft, broken alpha edges keep them from reading as solid white lines.
    auto texture=std::make_shared<engine::Texture>();texture->name="Soft broken sea foam";
    texture->width=128;texture->height=32;
    for(unsigned y=0;y<32;++y)for(unsigned x=0;x<128;++x){
        const float u=float(x)/128,v=(float(y)+.5f)/32;
        const float edge=std::pow(std::sin(engine::pi*v),1.5f);
        const float broken=.55f+.25f*std::sin(2*engine::pi*u*7+v*8)+.2f*std::cos(2*engine::pi*u*13-v*11);
        texture->pixels.insert(texture->pixels.end(),{240,247,232,std::uint8_t(255*edge*std::clamp(broken,0.f,1.f))});
    }
    auto material=std::make_shared<engine::Material>();material->name="Shorebreak foam";
    material->shader=engine::MaterialShader::Unlit;material->alphaMode=engine::AlphaMode::Blend;
    material->baseColorTexture=texture;
    auto mesh=std::make_shared<engine::Mesh>();mesh->materials={material};
    for(unsigned x=0;x<=128;++x){
        const float wx=-22+44.f*x/128;
        const float z=shoreline(wx,rear)-1.65f+.10f*std::sin(wx*2.3f);
        const float width=.20f+.06f*std::sin(wx*1.7f);
        for(unsigned side=0;side<2;++side)
            mesh->vertices.push_back({{wx,0,z+width*side},{0,1,0},{float(x)/128,float(side)}});
        if(x<128){const unsigned a=x*2;mesh->indices.insert(mesh->indices.end(),{a,a+1,a+2,a+2,a+1,a+3});}
    }
    mesh->generateTangents();
    auto& a=scene.spawn("Beach incoming foam");a.setSavedId("beach.surf");
    auto& state=a.addComponent<BeachSurf>();
    for(unsigned i=0;i<2;++i){
        auto& m=a.addComponent<engine::MeshComponent>("Incoming foam "+std::to_string(i),mesh);
        m.color={1,1,1};m.opacity=0;m.castsShadow=false;m.receivesShadow=false;
        m.transform.position.y=beachWaterHeight+.14f;
        state.bands[i]=m.id();
    }
}
float height(float x,float z,float left,float right,float rear){
    // A complete source-cell guard protects even interpolated apron triangles.
    const float shore=shoreline(x,rear);
    const float sea=std::max(0.f,shore+1.f-z);
    const float dry=smooth(shore+.5f,shore+2.5f,z);
    const float edge=x<0?smooth(left,left+2.5f,-x):smooth(right,right+2.5f,x);
    const float dunes=edge*dry*(1.4f+.7f*std::sin(x*.37f+z*.24f))
        *std::exp(-std::pow((z+1.f)/12.f,2));
    return beachGroundHeight-std::min(5.f,sea*.34f)+dunes;
}
// Original deterministic surface detail: no external image/model dependency.
std::shared_ptr<const engine::Texture> sandTexture(){
    static const auto texture=[] {
        auto t=std::make_shared<engine::Texture>();t->name="Beach sand fine mottling";t->width=t->height=128;
        for(unsigned z=0;z<128;++z)for(unsigned x=0;x<128;++x){
            const float u=2*engine::pi*x/128,v=2*engine::pi*z/128;
            const float n=std::sin(u*3+std::sin(v*2))*.025f+std::cos(v*7+u*2)*.012f;
            const auto c=std::uint8_t(std::clamp(.94f+n,0.f,1.f)*255);
            t->pixels.insert(t->pixels.end(),{c,c,c,255});
        }
        return std::shared_ptr<const engine::Texture>(t);
    }();return texture;
}
std::shared_ptr<const engine::Mesh> waterGrid(float width,float depth,unsigned nx,unsigned nz){
    auto mesh=std::make_shared<engine::Mesh>();
    for(unsigned z=0;z<=nz;++z)for(unsigned x=0;x<=nx;++x)
        mesh->vertices.push_back({{width*(float(x)/nx-.5f),0,depth*(float(z)/nz-.5f)},{0,1,0},{float(x)/nx,float(z)/nz}});
    for(unsigned z=0;z<nz;++z)for(unsigned x=0;x<nx;++x){
        const unsigned a=z*(nx+1)+x,b=a+nx+1;
        mesh->indices.insert(mesh->indices.end(),{a,b,a+1,a+1,b,b+1});
    }
    mesh->generateTangents();return mesh;
}
void water(engine::Scene& scene,bool runnel){
    auto material=std::make_shared<engine::Material>();material->name=runnel?"Beach runnel water":"Shallow turquoise cove";
    material->baseColor={.08f,.40f,.43f};material->roughness=.3f;
#if PAWS_WATER_SHADER
    material->shader=engine::MaterialShader::Water;
    auto& w=material->water;
    w.amplitude=runnel?.001f:.095f;w.wavelength=2.8f;w.speed=runnel?.45f:.62f;w.steepness=.22f;
    w.direction={.12f,1};w.rippleScale=3.5f;w.rippleStrength=.18f;w.reflectionStrength=.48f;
    w.horizon={.35f,.68f,.73f};w.zenith={.06f,.30f,.36f};
    w.refractionStrength=runnel?0:.65f;w.refractionDistortion=.004f;
    w.absorption={.35f,.10f,.065f};w.foamStrength=runnel?0:.85f;w.foamWidth=.30f;w.foamScale=3;
    w.foamColor={.94f,.95f,.88f};
#endif
    material->validate();
    auto& a=scene.spawn(runnel?"Shallow beach runnel":"Beach sea");
    a.transform.position=runnel?engine::Vec3{-1.5f,.008f,0}:engine::Vec3{0,beachWaterHeight,-57};
    auto& m=a.addComponent<engine::MeshComponent>(a.name,runnel?waterGrid(.75f,5,2,16):waterGrid(256,142,256,144));
    m.setMaterial(0,engine::MaterialInstance{material});m.color={1,1,1};m.castsShadow=false;
}
}
std::span<const GardenAsset> beachAssets(){return beachAssetSpecs;}
float beachPlantHeight(engine::Scene& scene,const engine::Mesh& mesh,engine::Transform pose){
    const auto bounds=mesh.localBounds();
    // Lowest five percent contains the stem bases, not the spreading leaf tips.
    // Sample those actual vertices after yaw/scale, so downhill roots cannot
    // float when the origin happens to sit on the uphill side of a dune.
    const float roots=bounds.minimum.y+(bounds.maximum.y-bounds.minimum.y)*.05f;
    pose.position.y=0;const auto world=pose.matrix();
    float y=groundHeight(scene,pose.position.x,pose.position.z)-bounds.minimum.y*pose.scale.y;
    for(const auto& vertex:mesh.vertices)if(vertex.position.y<=roots){
        const auto p=engine::transformPoint(world,vertex.position);
        y=std::min(y,groundHeight(scene,p.x,p.z)-p.y-.01f);
    }
    return y;
}
void startBeachSurf(engine::Scene& scene){
    if(auto* s=surf(scene)){s->state.age=0;++s->state.starts;}
}
BeachSurfState updateBeachSurf(engine::Scene& scene,double delta,bool reducedMotion,bool followAudio){
    auto* s=surf(scene);if(!s)return {};
    if(std::isfinite(delta)&&delta>0&&!reducedMotion){
        s->state.age+=std::min(delta,.25);
        // Silent play keeps a gentle wash, without queuing any delayed sound.
        if(!followAudio&&s->state.age>=6.5)s->state.age=0;
    }
    const float age=float(s->state.age);
    s->state.travel=1.75f*smooth(0,1.9f,age);
    s->state.opacity=reducedMotion?0:.72f*smooth(0,.25f,age)*(1-smooth(1.65f,2.55f,age));
    for(unsigned i=0;i<2;++i)if(auto* m=dynamic_cast<engine::MeshComponent*>(scene.findComponent(s->bands[i]))){
        m->transform.position.z=s->state.travel-i*.65f;
        m->opacity=s->state.opacity*(i?.45f:1.f);
    }
    return s->state;
}
void addBeachDressing(engine::Scene& scene,const GardenMeshes& meshes,int boardWidth,int boardDepth,float frontExtension){
    const float left=-boardWidth*.5f-.45f,right=boardWidth*.5f+1.45f;
    const float back=-boardDepth*.5f,front=boardDepth*.5f+frontExtension;
    unsigned index=0;
    const auto detail=[&](const char* asset,float x,float z,float scale,float yaw,float bury=0.f,bool levelDeck=false){
        auto& actor=scene.spawn(std::string("Coastal ")+asset);
        actor.setSavedId("beach.scenery."+std::to_string(index++));
        actor.transform.position={x,(levelDeck?beachGroundHeight:groundHeight(scene,x,z))-bury,z};actor.transform.scale={scale,scale,scale};actor.transform.rotation.y=yaw;
        if(std::string_view(asset).starts_with("beach_grass"))
            actor.transform.position.y=std::min(actor.transform.position.y,beachPlantHeight(scene,*meshes.at(asset),actor.transform));
        // Scenery never provides interaction, collision or route support.
        actor.addComponent<engine::MeshComponent>(asset,meshes.at(asset)).color={1,1,1};
    };
    // Four shared modules form a level pier. Continue along the deck's local Z
    // axis, rather than following the seabed down beneath the water surface.
    constexpr float pierYaw=-.15f,sectionLength=2.4f;
    for(int n=0;n<4;++n)
        detail("beach_boardwalk",left-1.1f-std::sin(pierYaw)*sectionLength*n,
            back-.15f-std::cos(pierYaw)*sectionLength*n,1.f,pierYaw,.025f,true);
    for(int side=0;side<2;++side){
        const float edge=side?right:left,sign=side?1.f:-1.f;
        for(int n=0;n<4;++n){
            const float z=back-1.6f+n*(front-back+2.6f)/3;
            // Leave a gap alongside the boardwalk's deck and its approach.
            if(!side&&n<=1)continue;
            const float x=edge+sign*(1.02f+.16f*(n%2));
            detail("beach_rock",x,z,.85f+.10f*(n%3),.65f*n+side,.22f);
            detail("beach_rock_flat",x+sign*.48f,z+.65f,.75f,.8f*n,.025f);
            detail("beach_grass_short",edge+sign*.68f,z+.55f,1.f,n*1.35f,.025f);
            detail(n%2?"beach_shell_clam":"beach_shell_scallop",x-sign*.12f,z+.9f,.8f,n*.6f,.005f);
        }
        for(int n=0;n<9;++n){
            const float z=back-1.6f+n*(front-back+3.1f)/8;
            const float x=edge+sign*(.67f+.22f*(n%3));
            if(!side&&z<back+1.f)continue;
            detail(n%2?"beach_grass_sparse":"beach_grass_short",x,z,.9f+.12f*(n%3),n*1.7f,.025f);
        }
        // A second, staggered band fills the dunes without narrowing the
        // existing clear apron or the entrance to the pier.
        for(int n=0;n<5;++n){
            const float x=edge+sign*(2.6f+.35f*(n%2));
            const float z=back+.8f+n*(front-back+1.4f)/4;
            detail("beach_rock_cluster",x,z,1.2f+.13f*(n%3),1.1f*n+side,.08f);
            detail("beach_grass_tall",x-sign*.6f,z+.55f,1.f,1.4f*n,.025f);
            detail("beach_grass",x+sign*.55f,z-.35f,1.05f,.9f*n+side,.09f);
        }
    }
    // Low clusters along the landward edge also read behind the puzzle when
    // the player orbits the camera. Their complete bounds remain off the board.
    for(int n=0;n<7;++n){
        const float x=left+.4f+n*(right-left-.8f)/6;
        const float z=front+1.7f+.3f*(n%2);
        if(n%2==0)detail("beach_rock_cluster",x,z,1.05f+.12f*(n%3),n*.85f,.08f);
        detail("beach_grass_sparse",x-.4f,z-.3f,1.15f,n*1.3f,.025f);
        detail("beach_grass_tall",x+.35f,z+.5f,1.f,n*.7f,.025f);
        if(n==2||n==4)detail("beach_shell_spiral",x,z-.4f,1.f,n*.7f,.005f);
    }
}
void addBeachEnvironment(engine::Scene& scene,bool runnel,int boardWidth,int boardDepth,std::shared_ptr<const engine::Texture> sand){
#if PAWS_TERRAIN
    const float left=std::ceil(boardWidth*.5f+.5f),right=std::ceil(boardWidth*.5f+1.5f),rear=std::ceil(boardDepth*.5f+1.f);
    engine::TerrainData data;data.samplesX=data.samplesZ=257;data.spacing={1,1};data.chunkCells=32;data.textureMeters=3;
    auto weights=std::make_shared<engine::Texture>();weights->name="Beach dry wet dune blend";weights->width=weights->height=257;
    weights->wrapU=weights->wrapV=engine::TextureWrap::Clamp;
    for(unsigned z=0;z<257;++z)for(unsigned x=0;x<257;++x){
        const float wx=float(x)-128,wz=float(z)-128;
        float h=height(wx,wz,left,right,rear);
        // A cleared sand bed supports the boardwalk's four posts. Include full
        // source-cell padding, so interpolated terrain stays level beneath the
        // rotated deck. The central apron already has this same ground height.
        const float padX=-boardWidth*.5f-1.55f,padZ=-boardDepth*.5f-.15f;
        const float bed=(1-smooth(1.65f,2.65f,std::abs(wx-padX)))
            *(1-smooth(2.3f,3.3f,std::abs(wz-padZ)));
        h+=(beachGroundHeight-h)*bed;
        data.heights.push_back(h);
        const float wet=1-smooth(shoreline(wx,rear)+.5f,shoreline(wx,rear)+2.f,wz);
        const float grass=smooth(.55f,1.6f,h)*.65f;
        const auto a=std::uint8_t(wet*255),b=std::uint8_t((1-wet)*grass*255);
        weights->pixels.insert(weights->pixels.end(),{std::uint8_t(255-a-b),a,b,0});
    }
    auto material=std::make_shared<engine::Material>();material->name="Warm sand and sage dunes";material->roughness=.92f;
    material->layers.enabled=true;material->layers.weights=weights;material->layers.weightScale={3.f/256,3.f/256};
    // The generated albedo supplies the warm sand hue. Tints only distinguish
    // damp shoreline and sparse sage dunes; roughness is authored separately.
    material->layers.layers[0].color=sand?engine::Vec3{1,1,1}:engine::Vec3{.90f,.60f,.28f};
    material->layers.layers[1].color={.57f,.55f,.47f};material->layers.layers[1].roughness=.7f;
    material->layers.layers[2].color={.45f,.57f,.33f};
    for(auto& layer:material->layers.layers)layer.texture=sand?sand:sandTexture();
    data.material=material;
    auto terrain=engine::makeTerrainActor("Beach sand and dunes",engine::TerrainAsset::create(std::move(data)));
    terrain->transform.position={-128,0,-128};terrain->setSavedId("beach.terrain");
    for(auto* chunk:terrain->getComponents<engine::TerrainComponent>()){chunk->color={1,1,1};chunk->castsShadow=false;}
    scene.adoptChild(std::move(terrain));
#else
    (void)boardWidth;(void)boardDepth;
    auto& a=scene.spawn("Beach sand fallback");a.transform.position.y=beachGroundHeight;
    auto& m=a.addComponent<engine::MeshComponent>("Sand",engine::Mesh::plane(256));m.color={.86f,.70f,.46f};m.castsShadow=false;
#endif
    water(scene,false);if(runnel)water(scene,true);
    addSurf(scene,std::ceil(boardDepth*.5f+1.f));
}
}
