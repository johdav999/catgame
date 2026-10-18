#include "cat_collision.hpp"
#include "pip_asset.hpp"
#include "engine/mesh_component.hpp"
#include <algorithm>
#include <cmath>
#include <queue>
#include <tuple>
#include <unordered_map>

namespace paws {
namespace {
constexpr float clearance=.003f;
const auto volume=engine::CollisionShape::capsule(.06f,.22f);
float length(engine::Vec3 d){return std::sqrt(engine::dot(d,d));}
bool solid(std::string_view name){
    // Water, paving, clue marks, foliage and previews are presentation surfaces.
    return name=="ball"||name=="mouse"||name=="fish"||name=="bench"||name=="bench_cushion"
        ||name=="cushion"||name=="blocking_cushion"||name.starts_with("cushion_")
        ||name=="basket"||name=="bowl"||name=="crate"||name.starts_with("pot_")
        ||name=="scratching_post"||name=="sunny_perch"||name=="hammock"
        ||name.starts_with("playhouse_")||name=="low_picnic_table"||name=="picnic_mat"
        ||name=="platform"||name.starts_with("plank_")||name.starts_with("channel_")
        ||name.starts_with("gate_")||name=="fence"||name=="edge"||name.ends_with("_enclosure")
        ||name=="bell_support"||name=="sequence_blackboard"||name=="water_support"
        ||name=="rain_barrel"||name=="planting"||name=="herb_pot"||name=="empty_pot"||name=="stone_border";
}
bool preview(const engine::Node& n){for(auto* p=&n;p;p=p->parent())if(p->savedId().starts_with("preview."))return true;return false;}
}
engine::Transform CatCollision::pose(engine::Vec3 foot){engine::Transform p;p.position=foot+engine::Vec3{0,.305f,0};return p;}
engine::PhysicsQueryFilter CatCollision::filter(){engine::PhysicsQueryFilter f;f.layers=1u<<14;f.includeSensors=false;return f;}
std::shared_ptr<const engine::CollisionShape> CatCollision::shape(const engine::Mesh& mesh,const engine::Mat4& basis,bool sphere,bool padded){
    for(const auto& s:shapes_)if(s.mesh==&mesh&&s.basis.m==basis.m&&s.padded==padded)return s.shape;
    std::vector<engine::Vec3> points;points.reserve(mesh.vertices.size());
    for(const auto& v:mesh.vertices)points.push_back(engine::transformPoint(basis,v.position));
    std::shared_ptr<const engine::CollisionShape> result;
    if(sphere){
        const auto& b=mesh.localBounds();auto ext=b.maximum-b.minimum;
        const float scale=std::max({length({basis.m[0],basis.m[1],basis.m[2]}),length({basis.m[4],basis.m[5],basis.m[6]}),length({basis.m[8],basis.m[9],basis.m[10]})});
        result=engine::CollisionShape::sphere(std::max({ext.x,ext.y,ext.z})*.5f*scale);
    }else if(padded){
        if(points.empty())return {};
        auto low=points.front(),high=low;
        for(const auto p:points){low={std::min(low.x,p.x),std::min(low.y,p.y),std::min(low.z,p.z)};high={std::max(high.x,p.x),std::max(high.y,p.y),std::max(high.z,p.z)};}
        // The root capsule adds another .22 m. Together this keeps the .675 m
        // low animation silhouette outside the basket while approaching/turning,
        // not just once the final .75 m stand-off has been reached.
        constexpr float padding=.5f;std::vector<engine::Vec3> corners;
        for(float x:{low.x-padding,high.x+padding})for(float y:{low.y,high.y})for(float z:{low.z-padding,high.z+padding})corners.push_back({x,y,z});
        result=engine::CollisionShape::convexHull(corners);
    }else{
        // Remove collapsed triangles from imported/generated geometry before Jolt cooking.
        std::vector<std::uint32_t> indices;
        for(std::size_t i=0;i+2<mesh.indices.size();i+=3){auto a=mesh.indices[i],b=mesh.indices[i+1],c=mesh.indices[i+2];
            const auto normal=engine::cross(points[b]-points[a],points[c]-points[a]);
            if(engine::dot(normal,normal)>=1.01e-12f)indices.insert(indices.end(),{a,b,c});}
        if(indices.empty())return {};
        result=engine::CollisionShape::triangleMesh(points,indices);
    }
    // Moving gate rotations must not retain an unbounded history of cooked meshes.
    if(shapes_.size()>=128)shapes_.erase(shapes_.begin());
    shapes_.push_back({&mesh,basis,result,padded});return result;
}
void CatCollision::build(engine::Scene& scene,const GardenMeshes& meshes,engine::ActorId pawClearanceTarget){
    if(!proxies_.empty())return;
    std::unordered_map<const engine::Mesh*,bool> solids;
    for(const auto& [name,mesh]:meshes)if(mesh&&solid(name))solids[mesh.get()]=name=="ball";
    std::vector<std::tuple<engine::ComponentId,bool,bool>> sources;
    // Include currently disabled owned furnishings, but never their drag previews.
    auto collect=[&](auto&& self,const engine::Node& node)->void{
        if(auto* m=dynamic_cast<const engine::MeshComponent*>(&node);m&&m->mesh&&!m->mesh->skeleton&&!preview(node)&&(!m->owner()||!m->owner()->name.ends_with("home outline")))
            if(auto it=solids.find(m->mesh.get());it!=solids.end())sources.emplace_back(m->id(),it->second,m->owner()&&m->owner()->id()==pawClearanceTarget);
        for(const auto& child:node.children())self(self,*child);
    };collect(collect,scene);
    for(auto [source,sphere,padded]:sources){auto& a=scene.spawn("Cat collision solid");engine::RigidBodySettings settings;settings.motion=engine::BodyMotion::Static;settings.collisionLayer=14;
        a.addComponent<engine::RigidBodyComponent>("Authored solid",settings);
        a.enabled=false;proxies_.push_back({source,a.id(),{},sphere,padded});}
    synchronize(scene);
}
void CatCollision::synchronize(engine::Scene& scene){
    bool changed=false;
    for(auto& p:proxies_){auto* a=scene.findActor(p.actor);auto* m=dynamic_cast<engine::MeshComponent*>(scene.findComponent(p.source));if(!a)continue;
        bool active=m&&m->mesh&&m->isActiveInHierarchy();if(a->enabled!=active){a->enabled=active;changed=true;}if(!active)continue;
        auto world=m->worldMatrix();engine::Transform at;at.position={world.m[12],world.m[13],world.m[14]};
        auto basis=world;basis.m[12]=basis.m[13]=basis.m[14]=0;
        if(p.sphere){const auto& b=m->mesh->localBounds();at.position=engine::transformPoint(world,(b.minimum+b.maximum)*.5f);
            const float scale=std::max({length({world.m[0],world.m[1],world.m[2]}),length({world.m[4],world.m[5],world.m[6]}),length({world.m[8],world.m[9],world.m[10]})});
            basis=engine::Mat4::identity();basis.m[0]=basis.m[5]=basis.m[10]=scale;}
        auto* collider=a->getComponent<engine::ColliderComponent>();
        if(!collider||basis.m!=p.basis.m||p.mesh!=m->mesh.get()){auto geometry=shape(*m->mesh,basis,p.sphere,p.padded);if(!geometry){a->enabled=false;continue;}
            if(collider)collider->setShape(geometry);else a->addComponent<engine::ColliderComponent>("Solid surface",geometry);p.basis=basis;p.mesh=m->mesh.get();changed=true;}
        auto* body=a->getComponent<engine::RigidBodyComponent>();
        if(body->hasBody()){if(length(a->transform.position-at.position)>1e-6f)body->teleport(at);}else{a->transform=at;changed=true;}
    }
    if(changed)scene.synchronize();
}
bool CatCollision::clear(engine::Scene& scene,engine::Vec3 foot)const{
    if(proxies_.empty())return true;
    for(const auto& h:scene.physics().overlap(*volume,pose(foot),filter()))if(h.penetrationDepth>clearance)return false;
    return true;
}
engine::Vec3 CatCollision::constrain(engine::Scene& scene,engine::Vec3 from,engine::Vec3 to)const{
    auto d=to-from;float distance=length(d);if(distance<1e-6f||proxies_.empty())return to;
    auto hit=scene.physics().sweep(*volume,pose(from),d,distance,filter());
    if(!hit)return to;
    // Jolt reports a millimetre contact tolerance at a supporting tangent.
    // Accept only a clear endpoint within that skin; never cross a solid volume.
    if(distance-hit->distance<=.005f&&clear(scene,to))return to;
    // Resting support at the capsule's lower tangent may be reported as an
    // initial overlap. Only a separating/tangential move can leave that contact.
    if(hit->startedOverlapping&&hit->penetrationDepth<=clearance&&engine::dot(d,hit->normal)>=-1e-6f){
        auto probe=pose(from);probe.position=probe.position+hit->normal*.005f;
        auto next=scene.physics().sweep(*volume,probe,d,distance,filter());
        if(!next)return clear(scene,to)?to:from;
        hit=next;
    }
    return from+d*(std::max(0.f,hit->distance-clearance)/distance);
}
bool CatCollision::clearSegment(engine::Scene& scene,engine::Vec3 from,engine::Vec3 to)const{return length(constrain(scene,from,to)-to)<.0001f&&clear(scene,to);}
std::vector<engine::Vec3> CatCollision::route(engine::Scene& scene,engine::Vec3 from,engine::Vec3 to)const{
    if(clearSegment(scene,from,to))return {from,to};
    if(std::abs(from.y-to.y)>.001f||!clear(scene,from)||!clear(scene,to))return {};
    constexpr float step=.20f;
    const float minX=std::min(from.x,to.x)-2.f,minZ=std::min(from.z,to.z)-2.f;
    const int width=int(std::ceil((std::abs(to.x-from.x)+4)/step))+1,height=int(std::ceil((std::abs(to.z-from.z)+4)/step))+1;
    if(width*height>20000)return {};
    auto point=[&](int i){return engine::Vec3{minX+(i%width)*step,from.y,minZ+(i/width)*step};};
    auto index=[&](engine::Vec3 p){return std::clamp(int(std::lround((p.x-minX)/step)),0,width-1)+width*std::clamp(int(std::lround((p.z-minZ)/step)),0,height-1);};
    std::vector<int> previous(width*height,-1);std::vector<signed char> vacant(width*height,-1);std::queue<int> queue;
    // Connect exact endpoints to nearby clear grid nodes; never snap through geometry.
    int start=index(from),goal=index(to);
    for(int z=-2;z<=2;++z)for(int x=-2;x<=2;++x){int n=start+x+z*width;if(n<0||n>=width*height||std::abs(n%width-start%width)>2)continue;
        if(clearSegment(scene,from,point(n))){previous[n]=n;queue.push(n);}}
    int finish=-1;
    while(!queue.empty()){
        int n=queue.front();queue.pop();auto at=point(n);
        if(std::abs(n%width-goal%width)<=2&&std::abs(n/width-goal/width)<=2&&clearSegment(scene,at,to)){finish=n;break;}
        for(auto [dx,dz]:{std::pair{1,0},std::pair{-1,0},std::pair{0,1},std::pair{0,-1},std::pair{1,1},std::pair{1,-1},std::pair{-1,1},std::pair{-1,-1}}){
            int x=n%width+dx,z=n/width+dz;if(x<0||x>=width||z<0||z>=height)continue;int next=x+z*width;if(previous[next]>=0)continue;
            if(vacant[next]<0)vacant[next]=clear(scene,point(next));
            if(!vacant[next]||!clearSegment(scene,at,point(next)))continue;
            previous[next]=n;queue.push(next);
        }
    }
    if(finish<0)return {};
    std::vector<engine::Vec3> raw{to};for(int n=finish;;n=previous[n]){raw.push_back(point(n));if(previous[n]==n)break;}raw.push_back(from);std::reverse(raw.begin(),raw.end());
    std::vector<engine::Vec3> result{from};for(std::size_t n=0;n+1<raw.size();){std::size_t last=n+1;while(last+1<raw.size()&&clearSegment(scene,raw[n],raw[last+1]))++last;result.push_back(raw[last]);n=last;}return result;
}
std::vector<engine::Vec3> CatCollision::supportedRoute(engine::Scene& scene,const std::vector<engine::Vec3>& authored)const{
    if(authored.empty())return {};
    std::vector<engine::Vec3> result{authored.front()};
    for(std::size_t i=1;i<authored.size();++i){auto from=result.back(),to=authored[i];
        // Sparse step markers describe a supported climb, rather than a full
        // body clearance envelope. Lift intermediate markers over a higher tread.
        if(i+1<authored.size()&&to.y>0&&!clear(scene,to))for(int n=1;n<=24;++n){auto raised=to;raised.y+=n*.025f;if(clear(scene,raised)){to=raised;break;}}
        if(clearSegment(scene,from,to)){result.push_back(to);continue;}
        // Raise before crossing a support edge, then lower onto the measured
        // contact. All legs are swept, including ceilings and neighbouring props.
        bool found=false;
        for(float extra:{0.f,.05f,.10f,.20f,.35f}){if(std::abs(from.y-to.y)<.001f)break;float y=std::max(from.y,to.y)+extra;auto up=from,over=to;up.y=over.y=y;
            if(clearSegment(scene,from,up)&&clearSegment(scene,up,over)&&clearSegment(scene,over,to)){result.push_back(up);result.push_back(over);result.push_back(to);found=true;break;}}
        if(!found){auto detour=route(scene,from,to);if(detour.empty())return {};result.insert(result.end(),detour.begin()+1,detour.end());}
    }
    return result;
}
bool CatCollision::follow(engine::Scene& scene,engine::Actor& actor,std::vector<engine::Vec3>& path,std::size_t& segment,float budget)const{
    bool faced=false;
    while(segment+1<path.size()){
        auto from=actor.transform.position,to=path[segment+1],d=to-from;float distance=length(d);
        if(distance<.0001f){++segment;continue;}
        auto requested=from+d*(std::min(budget,distance)/distance),actual=constrain(scene,from,requested);
        actor.transform.position=actual;
        if(!faced&&d.x*d.x+d.z*d.z>1e-8f){turnCatTowards(actor,std::atan2(d.x,d.z),budget/pipWalkSpeed);faced=true;}
        if(length(actual-requested)>.0001f)return false;
        budget-=std::min(budget,distance);if(length(actual-to)>.0001f)return false;++segment;
        if(budget<=0)return segment+1>=path.size();
    }return true;
}
bool CatCollision::sample(engine::Scene& scene,engine::Actor& actor,std::vector<engine::Vec3>& path,std::size_t& segment,float distance)const{
    if(path.empty())return false;
    float travelled=0;for(std::size_t i=0;i<segment&&i+1<path.size();++i)travelled+=length(path[i+1]-path[i]);
    if(segment+1<path.size())travelled+=length(actor.transform.position-path[segment]);
    return follow(scene,actor,path,segment,std::max(0.f,distance-travelled));
}
}
