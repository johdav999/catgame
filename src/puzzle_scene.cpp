#include "puzzle_scene.hpp"
#include "route_traversal.hpp"
#include "engine/mesh_component.hpp"
#include "engine/light_component.hpp"
#include "engine/interaction_component.hpp"
#include <algorithm>
#include <cmath>
namespace paws {
namespace {
engine::Vec3 position(Cell p,float height=0) { return {(p.x-2)*1.45f,height,(p.y-1)*1.45f}; }
void placeBall(engine::Actor& ball,engine::Vec3 routePose,bool production) {
    // The imported ball is grounded at its base. Keep its centre over the
    // channel surface while rotating, rather than orbiting the base pivot.
    const auto centre=production?gardenAnchor("ball","centre"):engine::Vec3{};
    auto rotation=ball.transform;rotation.position={};
    ball.transform.position=routePose+centre-engine::transformDirection(rotation.matrix(),centre);
}
engine::Actor& box(engine::Scene& scene,const std::shared_ptr<const engine::Mesh>& cube,
    const std::string& name,engine::Vec3 p,engine::Vec3 scale,engine::Vec3 color) {
    auto& actor=scene.spawn(name); actor.transform.position=p; actor.transform.scale=scale;
    actor.addComponent<engine::MeshComponent>(name,cube).color=color; return actor;
}
float endingLength(int kind){return kind==1?3.2f:kind==2?5.4f:3.6f;}
engine::Vec3 basketApproach(engine::Scene& scene,const SceneBindings::QuestEnding& ending,bool left){
    auto* basket=scene.findActor(ending.basket);
    auto* mesh=basket?basket->getComponent<engine::MeshComponent>():nullptr;
    if(!mesh||!mesh->mesh)return ending.home;
    const auto world=mesh->worldMatrix();float edge=basket->transform.position.x;
    for(const auto& v:mesh->mesh->vertices){const float x=engine::transformPoint(world,v.position).x;edge=left?std::min(edge,x):std::max(edge,x);}
    // Root-centred body collision does not cover the forward paws. Reserve the
    // complete low silhouette of walk/quest_basket_finish, including the pat,
    // outside the visible rim. Actual skinned vertices verify this clearance.
    constexpr float pawStandOff=.75f;
    return {edge+(left?-pawStandOff:pawStandOff),0,.15f};
}
void endingPose(engine::Scene& scene,SceneBindings& bindings,float seconds,float delta=0){
    auto& e=bindings.ending;auto* actor=scene.findActor(e.actor);
    auto* skin=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(bindings.cat));
    if(!actor||!skin){e.active=false;return;}
    const float finalYaw=e.kind==3||(e.kind==2&&e.target.x<e.toyStart.x)?engine::pi/2:-engine::pi/2;
    const bool travelling=e.segment+1<e.route.size();
    bool reached=bindings.collision.sample(scene,*actor,e.route,e.segment,std::min(seconds,e.walkSeconds)*pipWalkSpeed);
    if(reached){
        if(travelling&&delta>0){e.elapsed=e.walkSeconds;sampleCatWalk(*actor,*skin,seconds,true);return;}
        if(delta>0&&!turnCatTowards(*actor,finalYaw,delta)){
            e.elapsed=e.walkSeconds;sampleCatWalk(*actor,*skin,skin->animation().time()+delta,true);return;
        }
        actor->transform.rotation.y=finalYaw;
    }
    const float walk=reached?1.f:std::min(.9999f,e.walkSeconds>0?seconds/e.walkSeconds:0.f);
    if(!reached)e.elapsed=std::min(e.elapsed,e.walkSeconds-.0001f);
    std::string_view action="garden_walk";float clipTime=seconds;
    if(walk>=1){action=e.kind==1?"quest_ball_play":e.kind==2?"quest_basket_finish":"quest_gate_collect";clipTime=std::clamp(seconds-e.walkSeconds,0.f,endingLength(e.kind));}
    // Explicit sampling keeps contacts and props on the same presentation clock.
    const auto& clips=skin->mesh->skeleton->clips;
    auto at=std::find_if(clips.begin(),clips.end(),[&](const auto& c){return c.name==action;});
    if(walk<1)sampleCatWalk(*actor,*skin,seconds,true);
    else if(at!=clips.end()){skin->animation().play(pipClip(*skin->mesh,action),false);skin->animation().seek(clipTime);skin->animation().pause();}
    if(e.kind==1){
        const float push=std::clamp((clipTime-1.2f)/.5f,0.f,1.f);
        if(walk>=1){
            if(auto* ball=scene.findActor(bindings.ball))placeBall(*ball,e.toyStart+engine::Vec3{-.20f*push,0,0},true);
            auto target=e.target;target.x-=.18f*std::clamp((clipTime-2.f)/1.2f,0.f,1.f);
            bindings.collision.synchronize(scene);actor->transform.position=bindings.collision.constrain(scene,actor->transform.position,target);
        }
    }
    if(e.kind==3&&walk>=1&&clipTime>=(isToonCat(*skin->mesh)?1.8f:1.f))if(auto* toy=scene.findActor(e.toy)){
        // Skin the authored muzzle contact with the actual head pose; the toy
        // follows the head through lifting and looking back, not an actor offset.
        const auto& skeleton=*skin->mesh->skeleton;auto pose=skin->skinPose();
        for(std::size_t i=1;i<skeleton.bindings.size();++i)if(skeleton.joints[skeleton.bindings[i].joint].name=="head"){
            const auto mouth=engine::transformPoint(pose->bones[i].transform,catMuzzleBind(*skin->mesh));
            toy->transform.position=engine::transformPoint(actor->transform.matrix(),mouth)-engine::Vec3{0,.035f,0};
            toy->transform.rotation.y=actor->transform.rotation.y;break;
        }
    }
    if(e.kind==2&&walk>=1)if(auto* toy=scene.findActor(e.toy)){
        const float pat=clipTime>2&&clipTime<2.8f?std::sin((clipTime-2)*engine::pi/.8f):0;
        toy->transform.position=e.toyStart+engine::Vec3{0,.025f*pat,0};
    }
}
}
PuzzleScene makePuzzleScene(const PawsSession& session,const GardenMeshes* meshes) {
    PuzzleScene out; out.scene=std::make_unique<engine::Scene>(session.content().title+" - integration fixture");
    out.bindings.questId=session.content().questId;out.bindings.production=meshes!=nullptr;
    auto& scene=*out.scene; const auto cube=engine::Mesh::cube();
    scene.ambient={0.38f,0.38f,0.36f};
    if(!meshes) box(scene,cube,"Board",{0,-0.22f,0},{7.8f,0.25f,4.9f},{0.55f,0.55f,0.51f});
    const auto& c=session.content();
    const auto prop=[&](const std::string& asset,engine::Vec3 p)->engine::Actor& {
        auto& a=scene.spawn(asset);a.transform.position=p;
        a.addComponent<engine::MeshComponent>(asset,meshes->at(asset)).color={1,1,1};return a;
    };
    engine::ActorId boundaryGate;
    const engine::Vec3 switchPosition{2,0,0};
    if(meshes) {
        for(auto name:{"quest_ball_play","quest_basket_finish","quest_gate_collect"})pipClip(*meshes->at("pip"),name);
        for(int y=-1;y<c.height+1;++y) for(int x=-1;x<6;++x) prop("paving",{float(x-2),-.01f,float(y-1)});
        boundaryGate=addGardenEnclosure(scene,*meshes,-3.5f,3.5f,-2.5f,float(c.height)-.5f);
        // Extend the final channel to the inset pressure switch, leaving the
        // boundary gate and its swing unobstructed.
        if(c.goal==Goal::Retrieve)prop("channel_straight",{1,0,0});
        prop("bench",{2.35f,0,-2.85f});prop("planting",{2.6f,0,-1.7f});
        prop("bench_cushion",{2.35f,.49f,-2.82f});
        auto& cat=scene.spawn("Pip");cat.transform.position={-2.4f,0,.5f};
        if(c.goal==Goal::Route) {
            // Wait outside the route, facing the return spot rather than the source ball.
            cat.transform.position={float(c.destination.x-2)+.85f,0,float(c.destination.y-1)};
            cat.transform.rotation.y=-engine::pi/2;
        }
        auto& skin=cat.addComponent<engine::SkeletalMeshComponent>("Pip",meshes->at("pip"));skin.color={1,1,1};out.bindings.cat=skin.id();presentPip(skin,0,false);
        if(c.goal==Goal::Tidy){
            // Stand on the rules' access side, with room for Pip's body behind
            // the entrance, facing the cushion and toys.
            cat.transform.position={float(c.access.x-2)-.4f,0,float(c.access.y-1)};
            cat.transform.rotation.y=engine::pi/2;
        }
        if(c.goal==Goal::Retrieve){cat.transform.position=c.keepBallOnObstacleMove?engine::Vec3{-2.4f,0,-1.f}:engine::Vec3{2.7f,0,-1.f};cat.transform.rotation.y=engine::pi/2;}
        out.bindings.ending.actor=cat.id();out.bindings.ending.home=cat.transform.position;out.bindings.ending.homeRotation=cat.transform.rotation;
        if(c.goal==Goal::Tidy) {
            auto& basket=prop("basket",{c.packedToysBlockAccess?2.f:1.f,0,0});basket.transform.rotation.y=-engine::pi/2;if(c.packedToysBlockAccess)basket.transform.scale.x=1.9f;
            out.bindings.ending.basket=basket.id();
            // Low stone borders define one entrance into the cushion's sliding
            // lane. The toy area cannot be entered by walking around the ends.
            const auto border=[&](engine::Vec3 from,engine::Vec3 to){
                const auto d=to-from;const float length=std::sqrt(engine::dot(d,d));
                const int pieces=int(std::ceil(length/1.1f));
                const auto& bounds=meshes->at("stone_border")->localBounds();
                for(int i=0;i<pieces;++i){auto& wall=prop("stone_border",from+d*((float(i)+.5f)/pieces));
                    wall.name="Toy passage stone border";wall.transform.rotation.y=std::atan2(-d.z,d.x);
                    wall.transform.scale.x=length/pieces/(bounds.maximum.x-bounds.minimum.x);
                }
            };
            constexpr float left=-1.58f,right=2.72f,back=-1.95f,front=1.95f,gap=.42f;
            border({left,0,back},{right,0,back});border({left,0,front},{right,0,front});
            border({right,0,back},{right,0,front});
            border({left,0,back},{left,0,-gap});border({left,0,gap},{left,0,front});
            border({-.42f,0,-.58f},{-.42f,0,.58f});
        }
        if(c.goal==Goal::Route) box(scene,cube,"Pip's ball destination",{float(c.destination.x-2),.012f,float(c.destination.y-1)},{.5f,.025f,.5f},{.16f,.40f,.36f});
    } else for(int y=0;y<c.height;++y) for(int x=0;x<c.width;++x)
        box(scene,cube,"Authored slot",position({x,y},-0.06f),{1.32f,0.08f,1.32f},{0.76f,0.74f,0.66f});
    if(!meshes) box(scene,cube,"Ball source",position(c.source,0.02f),{1,0.1f,1},{0.77f,0.58f,0.36f});
    if(!meshes) box(scene,cube,"Destination",position(c.destination,0.02f),{1,0.1f,1},{0.20f,0.52f,0.48f});
    for(const auto& o:c.objects) {
        ObjectBinding binding; binding.object=o.id;
        if(meshes) {
            std::string asset;
            switch(o.kind) {
                case Kind::Pot:asset=o.id=="pot_b"?"pot_fluted":"pot_round";break;
                case Kind::Crate:asset="crate";break;
                case Kind::Catch:asset="bowl";break;
                case Kind::Cushion:asset="blocking_cushion";break;
                case Kind::Straight:asset="channel_straight";break;
                case Kind::Curve:asset="channel_curve";break;
                case Kind::Toy:asset=o.shape==Shape::Ball?"ball":o.shape==Shape::Mouse?"mouse":"fish";break;
                case Kind::Gate:binding.actor=boundaryGate;break;
                case Kind::Switch:asset="switch_plate";prop("switch_base",switchPosition);break;
                case Kind::Socket:break;
            }
            if(asset.empty()&&!binding.actor) binding.actor=scene.spawn(o.name).id(); // Basket mesh supplies the three actual shaped recesses.
            else if(!asset.empty()) binding.actor=prop(asset,{}).id();
        } else if(!channel(o.kind)) {
            auto& actor=box(scene,cube,o.name,position(o.initial,0.48f),{0.85f,0.85f,0.85f},{0.48f,0.47f,0.43f});
            binding.actor=actor.id();
            // Symbolic integration fixtures until the phase-4 production kit arrives.
            if(o.kind==Kind::Cushion) actor.transform.scale={1.1f,.3f,1.1f};
            if(o.kind==Kind::Gate) actor.transform.scale={.18f,1.5f,1.2f};
            if(o.kind==Kind::Switch||o.kind==Kind::Socket) actor.transform.scale={.8f,.12f,.8f};
            if(o.kind==Kind::Toy) actor.transform.scale={.45f,.3f,.45f};
            auto* mesh=actor.getComponent<engine::MeshComponent>();
            if(o.shape==Shape::Ball) {mesh->mesh=engine::Mesh::sphere(16,12);mesh->color={.85f,.32f,.23f};}
            if(o.shape==Shape::Mouse) {actor.transform.scale.x*=.65f;mesh->color={.45f,.55f,.7f};}
            if(o.shape==Shape::Fish) {actor.transform.scale.z*=.55f;mesh->color={.28f,.62f,.5f};}
        } else {
            binding.actor=box(scene,cube,o.name,position(o.initial,0.05f),{0.96f,0.08f,0.96f},{0.38f,0.40f,0.38f}).id();
            for(auto& arm:binding.arms) arm=box(scene,cube,o.name+" opening",{}, {1,1,1},{0.9f,0.83f,0.62f}).id();
        }
        auto& interaction=scene.findActor(binding.actor)->addComponent<engine::InteractionComponent>();
        if(o.kind==Kind::Socket) interaction.proxyBounds=engine::MeshBounds{{-.18f,0,-.18f},{.18f,.12f,.18f},true};
        out.bindings.objects.push_back(binding);
    }
    // Contextual parking outlines are GUI geometry, not selectable props. Only
    // the selected obstacle's reachable spaces are enabled by projectPuzzle.
    auto cream=std::make_shared<engine::Material>();cream->shader=engine::MaterialShader::Unlit;cream->baseColor={1,.973f,.914f};
    auto teal=std::make_shared<engine::Material>();teal->shader=engine::MaterialShader::Unlit;teal->baseColor={.157f,.4f,.365f};
    const float spacing=meshes?1.f:1.45f;
    for(std::size_t i=0;i<c.objects.size();++i)if(sliding(c.objects[i].kind))for(auto cell:c.objects[i].slots){
        auto& guide=scene.spawn("Reachable parking outline");guide.enabled=false;
        guide.transform.position={(cell.x-2)*spacing,.03f,(cell.y-1)*spacing};
        for(int edge=0;edge<4;++edge)for(bool border:{true,false}){
            auto& line=guide.addComponent<engine::MeshComponent>("Parking outline",cube);
            const float half=.44f*spacing;line.transform.position={edge<2?0:(edge==2?-half:half),border?0:.008f,edge<2?(edge==0?-half:half):0};
            const float width=(border?.085f:.04f)*spacing;
            line.transform.scale=edge<2?engine::Vec3{2*half+width,.008f,width}:engine::Vec3{width,.008f,2*half+width};
            line.setMaterial(0,engine::MaterialInstance{border?teal:cream});line.color={1,1,1};line.castsShadow=false;
        }
        out.bindings.slideGuides.push_back({i,cell,guide.id()});
    }
    if(meshes&&c.contentId.ends_with("-v2")) {
        const auto mark=[&](engine::Vec3 from,engine::Vec3 to,engine::Vec3 color,float width=.035f){
            const auto d=to-from;auto& a=box(scene,cube,"Puzzle paving inlay",(from+to)*.5f+engine::Vec3{0,.014f,0},
                {width,.008f,std::sqrt(engine::dot(d,d))},color);a.transform.rotation.y=std::atan2(d.x,d.z);
            a.getComponent<engine::MeshComponent>()->castsShadow=false;
        };
        const auto at=[](Cell cell){return engine::Vec3{float(cell.x-2),0,float(cell.y-1)};};
        for(const auto& o:c.objects)if(sliding(o.kind)) {
            for(std::size_t n=1;n<o.slots.size();++n)mark(at(o.slots[n-1]),at(o.slots[n]),{.55f,.43f,.28f});
            for(auto cell:o.slots)for(float side:{-.36f,.36f}){const auto p=at(cell);
                mark(p+engine::Vec3{-.36f,0,side},p+engine::Vec3{.36f,0,side},{.55f,.43f,.28f});
                mark(p+engine::Vec3{side,0,-.36f},p+engine::Vec3{side,0,.36f},{.55f,.43f,.28f});}
        }
        if(c.goal==Goal::Tidy) {
            // Match the rules' open side aisle instead of implying that the
            // upper recess is the only entrance to the whole basket.
            if(c.packedToysBlockAccess) {
                for(int row=0;row<3;++row)mark(at({2,row}),at({4,row}),{.16f,.40f,.36f});
            }else {mark(at({2,0}),at({4,0}),{.16f,.40f,.36f});mark(at({4,0}),at({4,2}),{.16f,.40f,.36f});}
            for(auto cell:{Cell{1,0},Cell{1,2}}){auto& approach=prop("paw_mat",at(cell));approach.transform.scale={.5f,1,.5f};}
            for(const auto& o:c.objects)if(o.kind==Kind::Toy){
                auto& home=prop(o.shape==Shape::Ball?"ball":o.shape==Shape::Mouse?"mouse":"fish",at(o.initial));
                home.name=o.name+" home outline";home.transform.scale={1.2f,.035f,1.2f};
                auto* mesh=home.getComponent<engine::MeshComponent>();mesh->color={.63f,.58f,.46f};mesh->castsShadow=false;
            }
        }
        if(c.goal==Goal::Retrieve)mark(at(c.access),gardenGatePosition(3.5f,-2.5f,2.5f),{.55f,.43f,.28f});
    }
    auto& ball=scene.spawn("Source ball"); ball.addComponent<engine::MeshComponent>("Ball",meshes?meshes->at("ball"):engine::Mesh::sphere(16,12)).color=meshes?engine::Vec3{1,1,1}:engine::Vec3{0.88f,0.32f,0.21f};
    if(c.goal!=Goal::Tidy) ball.addComponent<engine::InteractionComponent>();
    ball.transform.scale=meshes?engine::Vec3{1,1,1}:engine::Vec3{.36f,.36f,.36f}; out.bindings.ball=ball.id();
    out.bindings.selection=box(scene,cube,"Selected object marker",{0,-1,0},{1.23f,0.04f,1.23f},{0.12f,0.62f,0.59f}).id();
    auto& light=scene.spawn("Light").addComponent<engine::LightComponent>("Sun",engine::LightType::Directional);
    light.direction={-0.5f,-1,-0.3f}; light.intensity=1.5f;light.color={1,.95f,.87f};
    light.shadow.enabled=meshes!=nullptr;light.shadow.halfExtent=6;light.shadow.halfDepth=10;light.shadow.normalBias=.01f;
    auto& camera=scene.spawn("View").addComponent<engine::CameraComponent>("Puzzle camera");
    if(meshes)camera.setPerspective(engine::pi*35/180,.05f,80);
    camera.transform.position={0,9,6}; camera.lookAt({0,0,0}); out.camera=&camera;
    if(meshes)out.bindings.collision.build(scene,*meshes,out.bindings.ending.basket);
    projectPuzzle(scene,out.bindings,session); return out;
}
bool projectPuzzle(engine::Scene& scene,SceneBindings& bindings,const PawsSession& session,bool animate) {
    const auto& s=session.state(); const auto& c=session.content();
    if(bindings.questId!=c.questId || s.attempt<bindings.attempt || (s.attempt==bindings.attempt && s.revision<bindings.revision) || bindings.objects.size()!=c.objects.size()) return false;
    auto* ball=scene.findActor(bindings.ball); auto* selection=scene.findActor(bindings.selection);
    if(!ball || !selection) return false;
    for(std::size_t i=0;i<c.objects.size();++i) {
        const auto& binding=bindings.objects[i];
        if(binding.object!=c.objects[i].id || !scene.findActor(binding.actor)) return false;
        if(!bindings.production&&channel(c.objects[i].kind)) for(auto id:binding.arms) if(!scene.findActor(id)) return false;
    }
    bindings.motion.clear();bindings.arrivalMotion.clear();bindings.elapsed=0;
    const bool arrived=s.puzzle.completed||s.puzzle.ballOnSwitch;
    const bool rollingToSwitch=animate&&s.puzzle.ballOnSwitch&&
        (!bindings.ballAtDestination||s.attempt!=bindings.attempt||!bindings.ballRoute.empty());
    auto place=[&](engine::Actor& actor,engine::Vec3 p,engine::Vec3 rotation=engine::Vec3{}) {
        if(animate) bindings.motion.push_back({actor.id(),actor.transform.position,p,actor.transform.rotation,rotation});
        else {actor.transform.position=p;actor.transform.rotation=rotation;}
    };
    const auto world=[&](Cell cell,float height=0.f) {
        if(bindings.production&&c.goal==Goal::Retrieve&&cell==c.destination)return engine::Vec3{2,height,0};
        return bindings.production?engine::Vec3{float(cell.x-2),height,float(cell.y-1)}:position(cell,height);
    };
    const auto socketPosition=[&](Shape shape) {auto a=gardenAnchor("basket",shape==Shape::Ball?"ball_socket":shape==Shape::Mouse?"mouse_socket":"fish_socket");return engine::Vec3{c.packedToysBlockAccess?2.f:1.f,a.y,a.x*(c.packedToysBlockAccess?1.9f:1.f)};};
    selection->transform.position={0,-1,0};
    for(std::size_t i=0;i<c.objects.size();++i) {
        const auto& o=c.objects[i]; const auto& state=s.puzzle.objects[i]; const auto& binding=bindings.objects[i];
        auto p=position(state.cell,sliding(o.kind)?(o.kind==Kind::Pot?.48f:.22f):o.kind==Kind::Toy?.35f:.05f);
        if(o.kind==Kind::Gate) {p.y=.75f;if(gateOpen(c,s.puzzle,o.id)) p.z-=1.25f;}
        if(o.kind==Kind::Switch) p.y=switchActive(c,s.puzzle,o.id)?0:.12f;
        if(state.location==Location::Held) p=position(c.access,.8f);
        if(state.location==Location::Collected) p=position(c.access,.35f);
        engine::Vec3 rotation{};
        if(bindings.production) {
            p=world(state.cell);
            if(channel(o.kind)) rotation.y=(o.kind==Kind::Straight?1-state.orientation:3-state.orientation)*engine::pi/2;
            if(o.kind==Kind::Socket||state.location==Location::Socket) {p=socketPosition(o.shape);rotation.y=-engine::pi/2;}
            if(o.kind==Kind::Switch) p=p+gardenAnchor("switch_base",switchActive(c,s.puzzle,o.id)?"plate_pressed":"plate_unpressed");
            if(o.kind==Kind::Gate) {auto hinge=gardenAnchor("gate_frame","hinge");p=gardenGatePosition(3.5f,-2.5f,2.5f)+engine::Vec3{hinge.z,hinge.y,-hinge.x};rotation.y=engine::pi/2+(gateOpen(c,s.puzzle,o.id)?100*engine::pi/180:0);}
            if(o.kind==Kind::Toy&&!o.link.empty()&&o.link==c.gateRole)p=gardenGatePosition(3.5f,-2.5f,2.5f)+engine::Vec3{.8f,0,0};
            if(state.location==Location::Held) p=world(c.access,.45f);
            if(state.location==Location::Collected) p=c.goal==Goal::Retrieve?gardenGatePosition(3.5f,-2.5f,2.5f)+engine::Vec3{.8f,0,0}:engine::Vec3{-2.1f,.02f,.55f};
        }
        if(rollingToSwitch&&(o.kind==Kind::Switch||o.kind==Kind::Gate)) {
            auto* actor=scene.findActor(binding.actor);
            bindings.arrivalMotion.push_back({actor->id(),actor->transform.position,p,actor->transform.rotation,rotation});
        } else place(*scene.findActor(binding.actor),p,rotation);
        if(s.selected==o.id) {selection->transform.position=bindings.production?engine::Vec3{p.x,.015f,p.z}:position(state.cell,.005f);selection->transform.scale=bindings.production?engine::Vec3{.6f,.012f,.6f}:engine::Vec3{1.23f,.04f,1.23f};}
        const auto openings=ports(o.kind,state.orientation);
        for(std::size_t j=0;!bindings.production&&j<openings.size();++j) {
            auto* arm=scene.findActor(binding.arms[j]); auto p=position(state.cell,0.13f);
            const auto d=openings[j]; const auto next=position(adjacent(state.cell,d));
            p.x+=(next.x-p.x)*0.25f; p.z+=(next.z-p.z)*0.25f; place(*arm,p);
            arm->transform.scale=(d==Direction::North || d==Direction::South)?engine::Vec3{0.25f,0.08f,0.8f}:engine::Vec3{0.8f,0.08f,0.25f};
        }
    }
    auto ballPose=world((s.puzzle.completed||s.puzzle.ballOnSwitch)?c.destination:c.source,bindings.production?.06f:.38f);
    if(bindings.production&&s.puzzle.ballOnSwitch) ballPose=world(c.destination)+gardenAnchor("switch_base","ball_root_pressed");
    // Selection/hints must not restart an in-flight roll. Undo, restart and
    // nonanimated restoration cancel it and project the committed pose.
    const bool keepRoll=animate&&arrived&&bindings.ballAtDestination&&s.attempt==bindings.attempt;
    if(!keepRoll) {bindings.ballRoute.clear();bindings.ballElapsed=0;ball->transform.rotation={};}
    if(animate&&c.goal!=Goal::Tidy&&arrived&&(!bindings.ballAtDestination||s.attempt!=bindings.attempt)) {
        const auto route=evaluateRoute(c,s.puzzle).path;
        for(std::size_t n=0;n<route.size();++n) {
            const auto cell=route[n];bool curved=false;
            if(bindings.production&&n>0) for(std::size_t j=0;j<c.objects.size();++j) if(c.objects[j].kind==Kind::Curve&&s.puzzle.objects[j].cell==cell) {
                // Sample the authored quarter-circle channel, choosing its entry from the previous cell.
                const float angle=(3-s.puzzle.objects[j].orientation)*engine::pi/2;
                auto transform=[&](engine::Vec3 p){return world(cell)+engine::Vec3{std::cos(angle)*p.x+std::sin(angle)*p.z,p.y,-std::sin(angle)*p.x+std::cos(angle)*p.z};};
                const auto a=transform({-.5f,.06f,0}),b=transform({0,.06f,-.5f}),previous=world(route[n-1],.06f);
                const bool reverse=engine::dot(b-previous,b-previous)<engine::dot(a-previous,a-previous);
                for(int sample=0;sample<=8;++sample) {
                    float t=float(reverse?8-sample:sample)/8,theta=-engine::pi/2+t*engine::pi/2;
                    bindings.ballRoute.push_back(transform({-.5f+.5f*std::cos(theta),.06f,-(.5f+.5f*std::sin(theta))}));
                }
                curved=true;break;
            }
            if(!curved) bindings.ballRoute.push_back(world(cell,bindings.production?.06f:.38f));
        }
        if(!bindings.ballRoute.empty()) bindings.ballRoute.back()=ballPose;
    }
    // The ball has exactly one transform writer while following the route.
    if(bindings.ballRoute.empty()) placeBall(*ball,c.goal==Goal::Tidy?engine::Vec3{0,-3,0}:ballPose,bindings.production);
    else if(!keepRoll) placeBall(*ball,bindings.ballRoute.front(),bindings.production);
    bindings.ballAtDestination=arrived;
    const bool newEnding=s.puzzle.completed&&(!bindings.completed||bindings.attempt!=s.attempt);
    if(bindings.completed!=s.puzzle.completed) {
        if(auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(bindings.cat))) presentPip(*cat,s.puzzle.completed&&bindings.ballRoute.empty()?2:0,!animate);
        bindings.completed=s.puzzle.completed;
    }
    auto& ending=bindings.ending;
    if(bindings.production&&ending.actor){
        if(!s.puzzle.completed){
            ending.active=ending.finished=false;ending.elapsed=0;
            if(auto* cat=scene.findActor(ending.actor)){cat->transform.position=ending.home;cat->transform.rotation=ending.homeRotation;}
        }else if(newEnding){
            ending.kind=c.goal==Goal::Route?1:c.goal==Goal::Tidy?2:3;
            ending.start=ending.home;ending.toyStart=ballPose;
            ending.target=ending.kind==1?ballPose+engine::Vec3{.47f,-ballPose.y,.137f}:ending.kind==2?basketApproach(scene,ending,c.packedToysBlockAccess):gardenGatePosition(3.5f,-2.5f,2.5f)+engine::Vec3{.16f,0,0};
            for(std::size_t i=0;i<c.objects.size();++i)if(c.objects[i].id==c.requestedItem)ending.toy=bindings.objects[i].actor;
            if(ending.kind==2)for(std::size_t i=0;i<c.objects.size();++i)if(c.objects[i].kind==Kind::Toy&&c.objects[i].shape==Shape::Mouse){ending.toy=bindings.objects[i].actor;ending.toyStart=socketPosition(Shape::Mouse);}
            bindings.collision.synchronize(scene);
            ending.route=animate?std::vector<engine::Vec3>{}:bindings.collision.route(scene,ending.start,ending.target);ending.segment=0;
            ending.walkSeconds=0;for(std::size_t n=1;n<ending.route.size();++n){auto d=ending.route[n]-ending.route[n-1];ending.walkSeconds+=std::sqrt(engine::dot(d,d))/pipWalkSpeed;}
            ending.elapsed=0;ending.active=animate;ending.finished=!animate;
            if(!animate)endingPose(scene,bindings,ending.walkSeconds+endingLength(ending.kind));
        }else if(ending.finished)endingPose(scene,bindings,ending.walkSeconds+endingLength(ending.kind));
    }
    for(const auto& guide:bindings.slideGuides)if(auto* actor=scene.findActor(guide.actor)){
        actor->enabled=false;
        if(!s.paused&&!s.inGarden&&!s.puzzle.completed&&guide.object<c.objects.size()&&s.selected==c.objects[guide.object].id){
            const auto routes=slidingRoutes(c,s.puzzle,guide.object);
            actor->enabled=std::any_of(routes.begin(),routes.end(),[&](const auto& route){return !route.moves.empty()&&route.cell==guide.cell;});
        }
    }
    bindings.collision.synchronize(scene);
    bindings.attempt=s.attempt; bindings.revision=s.revision; return true;
}
void animatePuzzle(engine::Scene& scene,SceneBindings& bindings,float delta,bool reducedMotion) {
    bindings.elapsed+=std::max(0.f,delta);
    float t=reducedMotion?1.f:std::min(1.f,bindings.elapsed/.25f);t=t*t*(3-2*t);
    for(const auto& m:bindings.motion) if(auto* actor=scene.findActor(m.actor)) {
        actor->transform.position=m.from+(m.to-m.from)*t;
        actor->transform.rotation=m.rotationFrom+(m.rotationTo-m.rotationFrom)*t;
    }
    if(bindings.reducedMotion!=reducedMotion) {
        bindings.reducedMotion=reducedMotion;
        if(bindings.ending.finished)endingPose(scene,bindings,bindings.ending.walkSeconds+endingLength(bindings.ending.kind));
        else if(!bindings.ending.active)if(auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(bindings.cat))) presentPip(*cat,bindings.completed?2:0,reducedMotion);
    }
    if(t>=1) bindings.motion.clear();
    if(!bindings.ballRoute.empty()) {
        const auto previous=sampleRoute(bindings.ballRoute,bindings.ballElapsed*1.6f).position;
        bindings.ballElapsed+=std::max(0.f,delta);
        const auto& route=bindings.ballRoute;
        // 1.6 metres/second keeps the whole journey readable. Sampling by
        // distance avoids slowing at densely sampled curved channel sections.
        const auto sample=reducedMotion?RouteSample{route.back(),true}:sampleRoute(route,bindings.ballElapsed*1.6f);
        if(auto* ball=scene.findActor(bindings.ball)) {
            const auto movement=sample.position-previous;
            // Radius of the production ball is 0.105 m; greybox radius is 0.18 m.
            const float radius=bindings.production?.105f:.18f;
            ball->transform.rotation.x+=movement.z/radius;
            ball->transform.rotation.z-=movement.x/radius;
            placeBall(*ball,sample.position,bindings.production);
        }
        if(sample.finished) {
            bindings.ballRoute.clear();
            bindings.motion.insert(bindings.motion.end(),bindings.arrivalMotion.begin(),bindings.arrivalMotion.end());
            bindings.arrivalMotion.clear();bindings.elapsed=0;
            if(reducedMotion) {
                for(const auto& m:bindings.motion) if(auto* actor=scene.findActor(m.actor)) {
                    actor->transform.position=m.to;actor->transform.rotation=m.rotationTo;
                }
                bindings.motion.clear();
            }
            if(bindings.completed) if(auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(bindings.cat)))
                presentPip(*cat,2,reducedMotion);
        }
    }
    auto& ending=bindings.ending;
    bindings.collision.synchronize(scene);
    // Plan after the committed prop/gate/ball projection reaches its destination,
    // so transient visual positions cannot strand an otherwise legal ending.
    if(ending.active&&ending.route.empty()&&bindings.ballRoute.empty()&&bindings.motion.empty()&&bindings.arrivalMotion.empty()){
        if(auto* cat=scene.findActor(ending.actor))ending.route=bindings.collision.route(scene,cat->transform.position,ending.target);
        ending.walkSeconds=0;for(std::size_t n=1;n<ending.route.size();++n){auto d=ending.route[n]-ending.route[n-1];ending.walkSeconds+=std::sqrt(engine::dot(d,d))/pipWalkSpeed;}
    }
    bool followArrival=false;
    if(ending.active&&ending.kind==1&&!bindings.ballRoute.empty()){
        float length=0;for(std::size_t n=1;n<bindings.ballRoute.size();++n){auto d=bindings.ballRoute[n]-bindings.ballRoute[n-1];length+=std::sqrt(engine::dot(d,d));}
        followArrival=length/1.6f-bindings.ballElapsed<=ending.walkSeconds;
    }
    if(ending.active&&!ending.route.empty()&&(bindings.ballRoute.empty()||followArrival)&&bindings.motion.empty()&&bindings.arrivalMotion.empty()){
        ending.elapsed=reducedMotion?ending.walkSeconds+endingLength(ending.kind):ending.elapsed+std::max(0.f,delta);
        if(!bindings.ballRoute.empty())ending.elapsed=std::min(ending.elapsed,ending.walkSeconds-.001f);
        endingPose(scene,bindings,ending.elapsed,reducedMotion?0:delta);
        if(ending.elapsed>=ending.walkSeconds+endingLength(ending.kind)){ending.active=false;ending.finished=true;}
    }
}
}

