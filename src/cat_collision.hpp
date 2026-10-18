#pragma once
#include "garden_kit.hpp"
#include "engine/physics.hpp"
#include "engine/collider_component.hpp"

namespace paws {
// Game-owned collision policy for authored cat travel. Physics queries use the
// same solid geometry for planning and each actual move; puzzle rules stay pure.
class CatCollision {
public:
    // A low target that must stay outside the cat's paws can reserve a larger
    // horizontal footprint; other props retain their exact imported surfaces.
    void build(engine::Scene&,const GardenMeshes&,engine::ActorId pawClearanceTarget={});
    void synchronize(engine::Scene&);
    bool clear(engine::Scene&,engine::Vec3 foot) const;
    bool clearSegment(engine::Scene&,engine::Vec3 from,engine::Vec3 to) const;
    engine::Vec3 constrain(engine::Scene&,engine::Vec3 from,engine::Vec3 to) const;
    std::vector<engine::Vec3> route(engine::Scene&,engine::Vec3 from,engine::Vec3 to) const;
    // Preserve authored elevation/contact endpoints, adding checked lift/landing
    // segments at steps instead of moving diagonally through their risers.
    std::vector<engine::Vec3> supportedRoute(engine::Scene&,const std::vector<engine::Vec3>&) const;
    bool follow(engine::Scene&,engine::Actor&,std::vector<engine::Vec3>&,std::size_t& segment,float distance) const;
    bool sample(engine::Scene&,engine::Actor&,std::vector<engine::Vec3>&,std::size_t& segment,float distance) const;
    std::size_t solids() const { return proxies_.size(); }
private:
    struct Proxy { engine::ComponentId source; engine::ActorId actor; engine::Mat4 basis; bool sphere=false,padded=false; const engine::Mesh* mesh=nullptr; };
    struct Shape { const engine::Mesh* mesh; engine::Mat4 basis; std::shared_ptr<const engine::CollisionShape> shape; bool padded=false; };
    std::vector<Proxy> proxies_;
    std::vector<Shape> shapes_;
    std::shared_ptr<const engine::CollisionShape> shape(const engine::Mesh&,const engine::Mat4&,bool sphere,bool padded);
    static engine::Transform pose(engine::Vec3 foot);
    static engine::PhysicsQueryFilter filter();
};
}
