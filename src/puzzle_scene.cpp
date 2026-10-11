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
engine::Actor& box(engine::Scene& scene,const std::shared_ptr<const engine::Mesh>& cube,
    const std::string& name,engine::Vec3 p,engine::Vec3 scale,engine::Vec3 color) {
    auto& actor=scene.spawn(name); actor.transform.position=p; actor.transform.scale=scale;
    actor.addComponent<engine::MeshComponent>(name,cube).color=color; return actor;
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
    if(meshes) {
        for(int y=-1;y<4;++y) for(int x=-1;x<6;++x) prop("paving",{float(x-2),-.01f,float(y-1)});
        prop("bench",{-2.4f,0,-1.8f});prop("planting",{2.6f,0,-1.7f});
        auto& cat=scene.spawn("Pip");cat.transform.position={-2.4f,0,.5f};
        if(c.goal==Goal::Route) {
            // Wait outside the route, facing the return spot rather than the source ball.
            cat.transform.position={float(c.destination.x-2)+.85f,0,float(c.destination.y-1)};
            cat.transform.rotation.y=-engine::pi/2;
        }
        auto& skin=cat.addComponent<engine::SkeletalMeshComponent>("Pip",meshes->at("pip"));skin.color={1,1,1};out.bindings.cat=skin.id();presentPip(skin,0,false);
        if(c.goal==Goal::Tidy) {auto& basket=prop("basket",{1,0,0});basket.transform.rotation.y=-engine::pi/2;}
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
                case Kind::Cushion:asset="blocking_cushion";break;
                case Kind::Straight:asset="channel_straight";break;
                case Kind::Curve:asset="channel_curve";break;
                case Kind::Toy:asset=o.shape==Shape::Ball?"ball":o.shape==Shape::Mouse?"mouse":"fish";break;
                case Kind::Gate:asset="gate_leaf";prop("gate_frame",{float(o.initial.x-2),0,float(o.initial.y-1)}).transform.rotation.y=engine::pi/2;break;
                case Kind::Switch:asset="switch_plate";prop("switch_base",{float(o.initial.x-2),0,float(o.initial.y-1)});break;
                case Kind::Socket:break;
            }
            if(asset.empty()) binding.actor=scene.spawn(o.name).id(); // Basket mesh supplies the three actual shaped recesses.
            else binding.actor=prop(asset,{}).id();
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
    auto& ball=scene.spawn("Source ball"); ball.addComponent<engine::MeshComponent>("Ball",meshes?meshes->at("ball"):engine::Mesh::sphere(16,12)).color=meshes?engine::Vec3{1,1,1}:engine::Vec3{0.88f,0.32f,0.21f};
    if(c.goal!=Goal::Tidy) ball.addComponent<engine::InteractionComponent>();
    ball.transform.scale=meshes?engine::Vec3{1,1,1}:engine::Vec3{.36f,.36f,.36f}; out.bindings.ball=ball.id();
    out.bindings.selection=box(scene,cube,"Selected object marker",{0,-1,0},{1.23f,0.04f,1.23f},{0.12f,0.62f,0.59f}).id();
    auto& light=scene.spawn("Light").addComponent<engine::LightComponent>("Sun",engine::LightType::Directional);
    light.direction={-0.5f,-1,-0.3f}; light.intensity=1.5f;
    auto& camera=scene.spawn("View").addComponent<engine::CameraComponent>("Puzzle camera");
    camera.transform.position={0,9,6}; camera.lookAt({0,0,0}); out.camera=&camera;
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
    bindings.motion.clear();bindings.elapsed=0;
    auto place=[&](engine::Actor& actor,engine::Vec3 p,engine::Vec3 rotation=engine::Vec3{}) {
        if(animate) bindings.motion.push_back({actor.id(),actor.transform.position,p,actor.transform.rotation,rotation});
        else {actor.transform.position=p;actor.transform.rotation=rotation;}
    };
    const auto world=[&](Cell cell,float height=0.f) {return bindings.production?engine::Vec3{float(cell.x-2),height,float(cell.y-1)}:position(cell,height);};
    const auto socketPosition=[&](Shape shape) {auto a=gardenAnchor("basket",shape==Shape::Ball?"ball_socket":shape==Shape::Mouse?"mouse_socket":"fish_socket");return engine::Vec3{1,a.y,a.x};};
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
            if(o.kind==Kind::Gate) {auto hinge=gardenAnchor("gate_frame","hinge");p=p+engine::Vec3{hinge.z,hinge.y,-hinge.x};rotation.y=engine::pi/2+(gateOpen(c,s.puzzle,o.id)?100*engine::pi/180:0);}
            if(state.location==Location::Held) p=world(c.access,.45f);
            if(state.location==Location::Collected) p={-2.1f,.02f,.55f};
        }
        place(*scene.findActor(binding.actor),p,rotation);
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
    const bool arrived=s.puzzle.completed||s.puzzle.ballOnSwitch;
    // Selection/hints must not restart an in-flight roll. Undo, restart and
    // nonanimated restoration cancel it and project the committed pose.
    const bool keepRoll=animate&&arrived&&bindings.ballAtDestination&&s.attempt==bindings.attempt;
    if(!keepRoll) {bindings.ballRoute.clear();bindings.ballElapsed=0;}
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
    if(bindings.ballRoute.empty()) ball->transform.position=c.goal==Goal::Tidy?engine::Vec3{0,-3,0}:ballPose;
    else if(!keepRoll) ball->transform.position=bindings.ballRoute.front();
    bindings.ballAtDestination=arrived;
    if(bindings.completed!=s.puzzle.completed) {
        if(auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(bindings.cat))) presentPip(*cat,s.puzzle.completed&&bindings.ballRoute.empty()?2:0,!animate);
        bindings.completed=s.puzzle.completed;
    }
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
        if(auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(bindings.cat))) presentPip(*cat,bindings.completed?2:0,reducedMotion);
    }
    if(t>=1) bindings.motion.clear();
    if(!bindings.ballRoute.empty()) {
        bindings.ballElapsed+=std::max(0.f,delta);
        const auto& route=bindings.ballRoute;
        // 1.6 metres/second keeps the whole journey readable. Sampling by
        // distance avoids slowing at densely sampled curved channel sections.
        const auto sample=reducedMotion?RouteSample{route.back(),true}:sampleRoute(route,bindings.ballElapsed*1.6f);
        if(auto* ball=scene.findActor(bindings.ball)) {
            const auto movement=sample.position-ball->transform.position;
            ball->transform.position=sample.position;
            // Radius of the production ball is 0.105 m; greybox radius is 0.18 m.
            const float radius=bindings.production?.105f:.18f;
            ball->transform.rotation.x+=movement.z/radius;
            ball->transform.rotation.z-=movement.x/radius;
        }
        if(sample.finished) {
            bindings.ballRoute.clear();
            if(bindings.completed) if(auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(scene.findComponent(bindings.cat)))
                presentPip(*cat,2,reducedMotion);
        }
    }
}
}

