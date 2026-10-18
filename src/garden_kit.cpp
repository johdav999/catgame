#include "garden_kit.hpp"
#include "garden_terrain.hpp"
#include "engine/light_component.hpp"
#if PAWS_TERRAIN
#include "engine/terrain.hpp"
#endif
#include <cmath>
#include <stdexcept>
namespace paws {
namespace {
#include "garden_manifest.inc"
engine::Vec3 reviewPosition(std::string_view name){
    for(const auto& instance:reviewInstances)if(name==instance.asset)return instance.position;
    throw std::invalid_argument("Missing garden review instance");
}
}
std::span<const GardenAsset> gardenAssets(){return assetSpecs;}
std::span<const GardenInstance> gardenReviewInstances(){return reviewInstances;}
void prepareGardenPresentationMeshes(GardenMeshes& meshes){
    const auto& source=meshes.at("paving");
    if(!source->lods.empty())return;
    auto paving=std::make_shared<engine::Mesh>(*source);
    // The slab is viewed from above ground. Preserve its rounded outline and
    // original maps while avoiding shading its enclosed reverse faces.
    for(auto& material:paving->materials){
        auto surface=std::make_shared<engine::Material>(*material);
        surface->doubleSided=false;material=std::move(surface);
    }
    for(const auto& [name,pixels]:{std::pair{"paving_lod1",180.f},std::pair{"paving_lod2",80.f}}){
        auto level=std::make_shared<engine::Mesh>(*meshes.at(name));
        level->materials=paving->materials;
        paving->lods.push_back({std::move(level),pixels});
    }
    meshes["paving"]=std::move(paving);
}
engine::Vec3 gardenGatePosition(float right,float back,float front){
    return {right+.24f,0,back+std::min(1.5f,(front-back)*.5f)};
}
engine::ActorId addGardenEnclosure(engine::Scene& scene,const GardenMeshes& meshes,float left,float right,float back,float front,engine::Actor* parent){
    // Derived geometry is shared by every shrub in this scene. Keep materials
    // (and their textures) shared across levels; publish only immutable meshes.
    auto foliage=std::make_shared<engine::Mesh>(*meshes.at("exterior_foliage"));
    for(const auto& [name,pixels]:{std::pair{"exterior_foliage_lod1",240.f},std::pair{"exterior_foliage_lod2",90.f}}){
        auto level=std::make_shared<engine::Mesh>(*meshes.at(name));
        if(level->parts.size()!=foliage->parts.size())throw std::invalid_argument("Foliage LOD parts differ");
        for(std::size_t i=0;i<level->parts.size();++i)
            if(level->parts[i].materialIndex!=foliage->parts[i].materialIndex)throw std::invalid_argument("Foliage LOD material slots differ");
        level->materials=foliage->materials;
        foliage->lods.push_back({std::move(level),pixels});
    }
    // Repeat the broad front panels instead of stretching their leaves and
    // pickets across a whole courtyard. Side panels keep their authored slope.
    unsigned panelIndex=0;
    const auto panel=[&](const char* name,engine::Vec3 at,float length,float yaw){
        auto& a=parent?parent->addChild<engine::Actor>(name):scene.spawn(name);a.transform.position=at;a.transform.rotation.y=yaw;
        if(!parent)a.setSavedId("garden.enclosure."+std::to_string(panelIndex++));
        a.transform.scale={length/3.2f,1,1};
        a.addComponent<engine::MeshComponent>(name,meshes.at(name)).color={1,1,1};
    };
    const float width=right-left+.48f,depth=front-back;
    const int count=std::max(1,int(std::round(width/3.2f)));
    for(int i=0;i<count;++i){
        const float x=(left+right-width)*.5f+(i+.5f)*width/count;
        panel("front_enclosure",{x,0,front+.24f},width/count+.035f,0);
    }
    panel("left_enclosure",{left-.24f,0,(back+front)*.5f},depth+.12f,engine::pi/2);
    const auto gatePosition=gardenGatePosition(right,back,front);
    // Two side sections leave the frame and the complete leaf swing clear.
    constexpr float gateHalfGap=.85f;
    for(auto range:{std::pair{back-.06f,gatePosition.z-gateHalfGap},std::pair{gatePosition.z+gateHalfGap,front+.06f}}){
        const auto [start,end]=range;
        if(end>start)panel("right_enclosure",{right+.24f,0,(start+end)*.5f},end-start,-engine::pi/2);
    }
    auto& frame=parent?parent->addChild<engine::Actor>("Garden gate frame"):scene.spawn("Garden gate frame");
    frame.transform.position=gatePosition;frame.transform.rotation.y=engine::pi/2;
    frame.addComponent<engine::MeshComponent>("Gate frame",meshes.at("gate_frame")).color={1,1,1};
    auto& gate=parent?parent->addChild<engine::Actor>("Garden gate"):scene.spawn("Garden gate");
    const auto hinge=gardenAnchor("gate_frame","hinge");
    gate.transform.position=gatePosition+engine::Vec3{hinge.z,hinge.y,-hinge.x};gate.transform.rotation.y=engine::pi/2;
    gate.addComponent<engine::MeshComponent>("Gate leaf",meshes.at("gate_leaf")).color={1,1,1};
    if(!parent){frame.setSavedId("garden.gate.frame");gate.setSavedId("garden.gate");}
    // A short path beyond the gate keeps its exit and the quest toy visible.
    for(int i=0;i<2;++i){
        auto& tile=parent?parent->addChild<engine::Actor>("Gate path"):scene.spawn("Gate path");
        tile.transform.position={right+.5f+i,-.01f,gatePosition.z};
        tile.addComponent<engine::MeshComponent>("Paving",meshes.at("paving")).color={1,1,1};
    }
    // The house faces +Z, towards the courtyard. Its front steps remain behind
    // the clear floor; one shared mesh follows the active chapter boundary.
    const auto& houseBounds=meshes.at("house")->localBounds();
    auto& house=parent?parent->addChild<engine::Actor>("Garden background house"):scene.spawn("Garden background house");
    if(!parent)house.setSavedId("garden.house");
    house.transform.position={(left+right)*.5f,-houseBounds.minimum.y,back-.2f-houseBounds.maximum.z};
    house.addComponent<engine::MeshComponent>("House",meshes.at("house")).color={1,1,1};
    const float houseLeft=house.transform.position.x+houseBounds.minimum.x;
    const float houseRight=house.transform.position.x+houseBounds.maximum.x;
    const float houseBack=house.transform.position.z+houseBounds.minimum.z;
    // Reference-board details occupy the decorative perimeter, not the central
    // puzzle floor. No SelectionComponent: these never intercept puzzle input.
    unsigned detailIndex=0;
    const auto detail=[&](const char* name,engine::Vec3 at,float yaw=0.f){
        auto& a=parent?parent->addChild<engine::Actor>(name):scene.spawn(name);
        if(!parent)a.setSavedId("garden.detail."+std::to_string(detailIndex++));
        a.transform.position=at;a.transform.rotation.y=yaw;
        a.addComponent<engine::MeshComponent>(name,meshes.at(name)).color={1,1,1};
    };
    // Small asymmetric clusters, with depth and orientation varied within the
    // one-metre apron. Keep every mesh outside the playable rectangle.
    detail("watering_can",{left+.36f,0,front-1.25f},-.65f);
    detail("empty_pot",{left+.62f,0,front-.62f},.35f);
    detail("stone_border",{left+1.15f,0,back+.12f},-.08f);
    detail("herb_pot",{left+.48f,0,back+.5f},.2f);
    // Follow the bench beside the entrance as the house shifts with expansion.
    const float benchX=(left+right)*.5f+2.35f;
    detail("book_stack",{benchX+.94f,0,back-.32f},-.2f);
    detail("lantern",{benchX+.61f,0,back+.08f},.18f);
    detail("paw_mat",{(left+right)*.5f+.58f,.005f,back+.42f},-.12f);
    detail("garden_sign",{left+2.25f,.17f,front+.3f},-.12f);
    detail("climbing_ivy",{left+.09f,.12f,back+2.65f},engine::pi/2);
    detail("climbing_ivy",{right-.09f,.12f,front-.9f},-engine::pi/2);
    detail("moss_flowers",{left+.26f,.002f,front-.23f},.4f);
    detail("moss_flowers",{left+.57f,.002f,back+1.03f},-.6f);
    detail("moss_flowers",{right-.32f,.002f,front-.4f},1.1f);
    detail("moss_flowers",{right-.65f,.002f,front-.25f},-.2f);
    // A planted world continues beyond the fence. Keep this under the chapter
    // boundary parent so expansion replaces the old border and its vegetation.
    unsigned greeneryIndex=0;
    const auto decoration=[&](const char* name){
        auto& a=parent?parent->addChild<engine::Actor>(name):scene.spawn(name);
        if(!parent)a.setSavedId("garden.greenery."+std::to_string(greeneryIndex++));
        return &a;
    };
    // Fruit trees frame the house and the back half of each side. Whole crown
    // bounds remain outside the courtyard, including after chapter expansion.
    const auto tree=[&](const char* meshName,float x,float z,float scale,float yaw){
        auto& a=parent?parent->addChild<engine::Actor>(meshName):scene.spawn(meshName);
        a.transform.scale={scale,scale,scale};a.transform.rotation.y=yaw;
        a.transform.position={x,-meshes.at(meshName)->localBounds().minimum.y*scale,z};
        a.addComponent<engine::MeshComponent>("Fruit tree",meshes.at(meshName)).color={1,1,1};
        return &a;
    };
    unsigned treeIndex=0;
    const auto placeTree=[&](const char* name,float x,float z,float scale,float yaw){
        auto* a=tree(name,x,z,scale,yaw);
        if(!parent)a->setSavedId("garden.tree."+std::to_string(treeIndex++));
    };
    placeTree("apple_tree",houseLeft-1.9f,houseBack-2.3f,1.08f,.3f);
    placeTree("pear_tree",houseRight+1.6f,houseBack-2.2f,1.06f,-.5f);
    placeTree("pear_tree",left-2.6f,back+.25f,.78f,.9f);
    placeTree("apple_tree",left-3.0f,back+(front-back)*.54f,.72f,-.4f);
    placeTree("apple_tree",right+2.7f,back+.1f,.8f,-.8f);
    placeTree("pear_tree",right+3.0f,back+(front-back)*.54f,.72f,.4f);
#if PAWS_TERRAIN
    const float flatLeft=std::min(left-8.f,houseLeft-5.f),flatRight=std::max(right+8.f,houseRight+5.f);
    const float flatBack=std::min(back-8.f,houseBack-6.f);
    auto* terrain=addGardenTerrain(scene,meshes,flatLeft,flatRight,flatBack,front+8.f,parent);
    const auto& terrainAsset=terrain->getComponent<engine::TerrainComponent>()->asset();
    // Loose groves on the rising side and rear hills, with open meadow between.
    // Sample the actual source triangles in enclosure space, so chapter parents
    // and the three-metre heightfield grid cannot leave floating trunks.
    const engine::Vec3 groves[]={
        {flatLeft-11.f,0,back-2.f},{flatRight+12.f,0,back-4.f},
        {houseLeft-9.f,0,flatBack-13.f},{houseRight+10.f,0,flatBack-16.f},
        {(houseLeft+houseRight)*.5f,0,flatBack-27.f}};
    struct GroveTree {float x,z,scale,yaw;};
    constexpr GroveTree groveTrees[]={{-3.1f,0,1.45f,.3f},{1.7f,-2.8f,1.8f,-.8f},
        {3.9f,2.3f,1.35f,1.7f},{-.5f,4.1f,1.65f,2.5f}};
    unsigned hillIndex=0;
    for(const auto& grove:groves)for(const auto& offset:groveTrees){
        const float x=grove.x+offset.x,z=grove.z+offset.z;
        const auto ground=terrainAsset->ground(x-terrain->transform.position.x,z-terrain->transform.position.z);
        if(!ground)throw std::runtime_error("Hillside grove outside terrain");
        auto* a=tree(hillIndex%2?"pear_tree":"apple_tree",x,z,offset.scale,offset.yaw+hillIndex*.37f);
        a->transform.position.y+=ground->position.y-.04f;
        if(!parent)a->setSavedId("garden.hill-tree."+std::to_string(hillIndex));
        auto* mesh=a->getComponent<engine::MeshComponent>();
        mesh->allowInstanceReordering=true;
        mesh->castsShadow=false;
        ++hillIndex;
    }
#else
    auto soil=std::make_shared<engine::Material>();soil->name="Shaded garden understory";
    soil->baseColor={.095f,.16f,.065f};soil->roughness=1;soil->shader=engine::MaterialShader::Unlit;
    // Continue the lawn beyond the camera's 80 m far plane, including the
    // pulled-back portrait framing and orbit views. This only enlarges the
    // existing four slabs; world-space UVs keep the grass at the same scale.
    const float outer=160.f;
    const auto bed=[&](float x,float z,float w,float d){
        auto& a=*decoration("Shaded planting bed");a.transform.position={x,-.14f,z};a.transform.scale={w,.12f,d};
        const auto grass=meshes.find("grass_ground");
        auto& m=a.addComponent<engine::MeshComponent>("Understory",grass!=meshes.end()?grass->second:engine::Mesh::cube());
        engine::MaterialInstance surface{grass!=meshes.end()?grass->second->materials.at(0):soil};
        // One texture tile covers two metres; align adjoining slabs in world space.
        surface.uvScale={w/2.f,d/2.f};surface.uvOffset={(x-w/2.f)/2.f,(z-d/2.f)/2.f};
        m.setMaterial(0,std::move(surface));m.color={1,1,1};m.castsShadow=false;
    };
    bed((left+right)*.5f,back-outer*.5f,right-left+2*outer,outer);
    bed((left+right)*.5f,front+outer*.5f,right-left+2*outer,outer);
    bed(left-outer*.5f,(back+front)*.5f,outer,front-back);
    bed(right+outer*.5f,(back+front)*.5f,outer,front-back);
#endif
    // Deterministic offsets avoid a visible planted grid and never affect rules.
    unsigned seed=731;
    const auto random=[&](){seed=1664525u*seed+1013904223u;return float((seed>>8)&65535u)/65535.f;};
    // Keep the inner two metres dense. Alternate outer cells give ~1.5m
    // diagonal spacing; broader outer clumps retain cover with fewer overlaps.
    struct Shrub {float x,z,gapX,gapZ,jitterX,jitterZ,yaw,size,height,tint;bool outer;unsigned savedIndex;};
    std::vector<Shrub> shrubs;
    for(float z=back-6.f;z<=front+6.f;z+=1.05f)for(float x=left-6.f;x<=right+6.f;x+=1.05f){
        const float gapX=std::max({left-x,0.f,x-right}),gapZ=std::max({back-z,0.f,z-front});
        if(std::max(gapX,gapZ)<.55f)continue;
        if(x>right&&x<right+2.8f&&std::abs(z-gatePosition.z)<1.45f)continue;
        // Leave the facade and footprint free of the dense exterior planting.
        if(x>houseLeft-1.f&&x<houseRight+1.f&&z>houseBack-1.f&&z<back)continue;
        // Consume the same seeded samples even for omitted outer shrubs, so
        // the retained inner planting keeps its established pose and tint.
        const float jitterX=random(),jitterZ=random(),yaw=random(),size=random(),height=random(),tint=random();
        const bool outerPlanting=std::max(gapX,gapZ)>2.f;
        const int column=int(std::round((x-(left-6.f))/1.05f));
        const unsigned row=unsigned(std::round((z-(back-6.f))/1.05f));
        if(outerPlanting&&((column+row)&1u))continue;
        shrubs.push_back({x,z,gapX,gapZ,jitterX,jitterZ,yaw,size,height,tint,outerPlanting,greeneryIndex++});
    }
    // Keep the exact seeded poses, density and saved identities. The elevated
    // garden camera starts on +Z: nearer opaque clumps establish depth first,
    // reducing shading of leaves hidden behind them within each instance batch.
    for(auto it=shrubs.rbegin();it!=shrubs.rend();++it){
        const auto& [x,z,gapX,gapZ,jitterX,jitterZ,yaw,size,height,tint,outerPlanting,savedIndex]=*it;
        auto& a=parent?parent->addChild<engine::Actor>("Dense exterior foliage"):scene.spawn("Dense exterior foliage");
        if(!parent)a.setSavedId("garden.greenery."+std::to_string(savedIndex));
        a.transform.position={x+(jitterX-.5f)*.35f,-.12f,z+(jitterZ-.5f)*.35f};
        a.transform.rotation.y=yaw*2*engine::pi;
        const float spread=outerPlanting?3.3f+size*.5f:2.7f+size*.65f;
        const bool foreground=z>front-1.f;
        a.transform.scale={spread,foreground?1.05f+height*.5f:2.f+height*1.3f,spread};
        auto& m=a.addComponent<engine::MeshComponent>("Leafy garden border",foliage);
        // The two opaque material parts may batch together without draw-order
        // semantics; tint and transform remain per instance.
        m.allowInstanceReordering=true;
        const auto& b=m.mesh->localBounds();
        float minX=1e6f,maxX=-1e6f,minZ=1e6f,maxZ=-1e6f;
        for(float px:{b.minimum.x,b.maximum.x})for(float pz:{b.minimum.z,b.maximum.z}){
            auto p=engine::transformPoint(a.transform.matrix(),{px,0,pz});
            minX=std::min(minX,p.x);maxX=std::max(maxX,p.x);minZ=std::min(minZ,p.z);maxZ=std::max(maxZ,p.z);
        }
        if(maxX>left&&minX<right&&maxZ>back&&minZ<front){
            if(gapX>gapZ)a.transform.position.x+=x<left?left-maxX-.04f:right-minX+.04f;
            else a.transform.position.z+=z<back?back-maxZ-.04f:front-minZ+.04f;
        }
        const float shade=.75f+tint*.23f;m.color={shade*.86f,shade,shade*.75f};
        m.castsShadow=false;
    }
    return gate.id();
}
engine::Vec3 gardenAnchor(std::string_view asset,std::string_view anchor){
    for(const auto& a:anchorSpecs)if(asset==a.asset&&anchor==a.name)return a.position;
    throw std::invalid_argument("Missing garden anchor");
}
void validateGardenAsset(const GardenAsset& spec,const engine::Mesh& mesh){
    if(mesh.vertices.empty()||mesh.indices.empty()||!mesh.warnings.empty())throw std::invalid_argument(std::string(spec.name)+": empty or unsupported GLB");
    const auto& b=mesh.localBounds();
    const auto deltaMin=b.minimum-spec.minimum,deltaMax=b.maximum-spec.maximum;
    if(!b.valid||engine::dot(deltaMin,deltaMin)>.000064f||engine::dot(deltaMax,deltaMax)>.000064f)
        throw std::invalid_argument(std::string(spec.name)+": export bounds differ from contract");
    if(mesh.indices.size()/3!=spec.triangles)throw std::invalid_argument(std::string(spec.name)+": triangle budget changed");
}
GardenPreview makeGardenPreview(const GardenMeshes& meshes,std::shared_ptr<const engine::Mesh> pip){
    for(const auto& spec:gardenAssets())validateGardenAsset(spec,*meshes.at(spec.name));
    validatePip(*pip);
    GardenPreview result;result.scene.scene=std::make_unique<engine::Scene>("Garden kit workshop");
    auto& scene=*result.scene.scene;scene.ambient={.33f,.37f,.40f};scene.environmentIntensity=.65f;
    for(const auto& instance:gardenReviewInstances()){
        auto& a=scene.spawn(instance.asset);a.transform.position=instance.position;
        a.addComponent<engine::MeshComponent>(instance.asset,meshes.at(instance.asset)).color={1,1,1};
        const std::string_view name=instance.asset;
        if(name=="gate_leaf")result.gate=a.id();if(name=="switch_plate")result.plate=a.id();
        if(name=="ball")result.ball=a.id();if(name=="mouse")result.mouse=a.id();if(name=="fish")result.fish=a.id();
    }
    // Shared immutable paving mesh, rather than enlarged stone texture coverage.
    for(int x=-3;x<3;++x)for(int z=-2;z<3;++z){
        auto& a=scene.spawn("Courtyard paving");a.transform.position={x+.5f,-.005f,z-.5f};
        a.addComponent<engine::MeshComponent>("Sandstone",meshes.at("paving")).color={1,1,1};
    }
    auto& catActor=scene.spawn("Pip scale reference");catActor.transform.position=reviewPosition("cushion")+gardenAnchor("cushion","rest_root");result.pip=catActor.id();
    auto& cat=catActor.addComponent<engine::SkeletalMeshComponent>("Pip",std::move(pip));cat.color={1,1,1};result.cat=cat.id();presentPip(cat,5,true);
    auto& light=scene.spawn("Garden sun").addComponent<engine::LightComponent>("Sun",engine::LightType::Directional);
    light.direction={-.5f,-1,-.6f};light.color={1,.95f,.87f};light.intensity=1.7f;light.shadow.enabled=true;light.shadow.halfExtent=5;light.shadow.halfDepth=8;light.shadow.normalBias=.02f;
    auto& camera=scene.spawn("Workshop camera").addComponent<engine::CameraComponent>("Camera");
    camera.setPerspective(engine::pi*35/180,.05f,60);camera.transform.position={6,7,9};camera.lookAt({0,.15f,0});result.scene.camera=&camera;
    return result;
}
void poseGardenPreview(engine::Scene& scene,const GardenPreview& b,int mode){
    if(mode<0||mode>3)throw std::out_of_range("Unknown workshop pose");
    // Reset every moving part to its authored transform before projecting a pose.
    for(auto [name,id]:{std::pair{"gate_leaf",b.gate},{"switch_plate",b.plate},{"ball",b.ball},{"mouse",b.mouse},{"fish",b.fish}}){
        if(auto* a=scene.findActor(id))for(const auto& i:gardenReviewInstances())if(std::string_view(i.asset)==name){a->transform.position=i.position;a->transform.rotation={};}
    }
    auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(b.cat));auto* actor=scene.findActor(b.pip);
    if(cat&&actor){actor->transform.position=reviewPosition("cushion")+gardenAnchor("cushion","rest_root");presentPip(*cat,5,true);}
    if(mode==1){
        if(auto* a=scene.findActor(b.gate))a->transform.rotation.y=gateOpenDegrees*engine::pi/180;
        if(auto* a=scene.findActor(b.plate))a->transform.position=reviewPosition("switch_base")+gardenAnchor("switch_base","plate_pressed");
        if(auto* a=scene.findActor(b.ball))a->transform.position=reviewPosition("switch_base")+gardenAnchor("switch_base","ball_root_pressed");
    }else if(mode==2){
        const auto basket=reviewPosition("basket");
        for(auto [name,id]:{std::pair{"ball_socket",b.ball},{"mouse_socket",b.mouse},{"fish_socket",b.fish}})
            if(auto* a=scene.findActor(id))a->transform.position=basket+gardenAnchor("basket",name);
    }else if(mode==3&&cat&&actor){
        actor->transform.position=reviewPosition("bowl")+gardenAnchor("bowl","drink_root");presentPip(*cat,7,true);
    }
}
}
