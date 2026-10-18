#pragma once
#include "garden_kit.hpp"
#include "cat_collision.hpp"
#include "release1/contracts.hpp"
#include <map>
#include <optional>

namespace paws::release1::garden {
// Presentation contacts must come from the accepted mesh/action handoff. These
// are application data, never persisted placements or a second legality model.
struct Contact {
    engine::Vec3 approach,root;
    float yaw=0;
    std::string enter,loop,leave;
    double enterSeconds=0,leaveSeconds=0,still=0;
    // Measured supported root samples for an elevation/entry sequence, in local
    // furnishing coordinates. Empty means no world movement during that clip.
    std::vector<engine::Vec3> supportedRoots;
    bool verified=false;
};
struct CatModel {
    std::shared_ptr<const engine::Mesh> mesh;
    std::map<std::string,Contact,std::less<>> contacts;
    bool naturalCurl=false;
};
struct Assets { GardenMeshes props; std::array<CatModel,4> cats; };
struct Spot { std::string_view id; engine::Vec3 position; float width,depth; int chapter; };
std::span<const Spot> spots();
engine::Vec3 home(std::size_t cat);
// Reuse the accepted MVP contact pair; the old low resting pose is explicitly
// not a natural sleeping curl. Additional contacts arrive with Prompt 2.
CatModel legacyPip(std::shared_ptr<const engine::Mesh>);
struct Stamp { std::uint64_t generation=0; Revision revision=0; bool operator==(const Stamp&)const=default; };
struct Ticket { Stamp owner; std::uint64_t scene=0,serial=0; std::size_t cat=4; };
enum class Phase { Idle, Queued, Approach, Enter, Use, Leave, Return, Attention, Reaction, ArrivalQueued, Arrival, Wander, WanderPause, RestEnter, Resting, RestLeave, WanderHome, WelcomeWalk, WelcomeHello, WelcomeHome };
bool ambient(Phase);
struct CatBinding {
    engine::ActorId actor;
    engine::ActorId questMarker;
    engine::ComponentId mesh;
    Phase phase=Phase::Idle;
    std::string item,spot,reason;
    Ticket ticket;
    Contact contact;
    std::vector<engine::Vec3> path;
    std::vector<engine::Vec3> entryPath;
    std::size_t entrySegment=0;
    std::size_t segment=0;
    double elapsed=0,quiet=0,entryTravel=0;
    std::uint32_t randomState=1;
    unsigned outings=0;
    double ambientHold=0;
    bool introduced=false;
    bool eligible=false,arrivalComplete=false;
};
struct ItemBinding { engine::ActorId actor,preview; std::string spot; };
class Presentation {
public:
    void project(engine::Scene&,const ProgressView&,bool reducedMotion);
    std::optional<Ticket> use(engine::Scene&,std::string_view item,std::size_t cat,Stamp,std::string& reason);
    // Atomic group request; seats are independently reserved. Small mat uses
    // one centre seat and grounded companion seats outside its footprint.
    std::vector<Ticket> gather(engine::Scene&,std::string_view item,Stamp,std::string& reason,bool stagedArrival=false);
    bool celebrationProps(engine::Scene&,Stamp,bool enabled);
    bool cancel(engine::Scene&,const Ticket&);
    void cancelAll(engine::Scene&);
    bool attention(engine::Scene&,std::size_t cat,Stamp);
    void react(engine::Scene&,std::size_t cat,Stamp);
    // Called exactly once from Application's simulation callback. No scene tick.
    void update(engine::Scene&,double simulationDelta);
    void previewPlacement(engine::Scene&,std::string_view item,std::string_view spot,bool legal);
    std::vector<QuestView> requests(std::size_t cat) const;
    std::string useBlockage(std::string_view item,std::size_t cat)const;
    Stamp stamp()const {return stamp_;}
    const std::array<CatBinding,4>& cats()const {return cats_;}
    const std::array<ItemBinding,8>& items()const {return items_;}
    bool reduced()const{return reduced_;}
    bool nookReady()const;
    bool sunnyVisit(engine::Scene&,Stamp,std::string&,bool pinned=true);
    engine::ActorId nookSeat()const{return nookSeat_;}
    static engine::Vec3 nookPosition(){return {3.2f,0,4.3f};}
    bool lookoutReady()const;
    engine::ActorId lookoutSeat()const{return lookoutSeat_;}
    static engine::Vec3 lookoutPosition(){return {-1.1f,0,-1.65f};}
private:
    friend struct Built;
    friend struct Builder;
    Assets assets_;
    CatCollision collision_;
    ProgressView view_;
    Stamp stamp_;
    std::uint64_t scene_=0,serial_=0;
    bool projected_=false,reduced_=false;
    std::array<CatBinding,4> cats_;
    std::array<ItemBinding,8> items_;
    std::array<engine::ActorId,3> areas_;
    std::array<engine::ActorId,3> enclosures_;
    std::array<engine::ActorId,2> supplied_;
    engine::ActorId bench_,benchCushion_,lookout_,lookoutSeat_,nook_,nookSeat_;
    bool sunnyPinned_=false;
    double sunnyNext_=45,sunnyRemaining_=0;
    enum class NapPhase { Sleeping, Rising, Awake, Settling };
    engine::ActorId napActor_;
    engine::ComponentId napMesh_;
    NapPhase napPhase_=NapPhase::Sleeping;
    double napElapsed_=0;
    unsigned napCycle_=0;
    bool celebrating_=false;
    std::string groupItem_;
    bool stagedArrival_=false;
    std::string placement(std::string_view item)const;
    void idle(engine::Scene&,std::size_t);
    void updateNap(engine::Scene&,double);
    bool startWelcome(engine::Scene&,std::size_t,engine::Vec3,engine::Vec3);
    void updateWelcome(engine::Scene&,double);
    bool startSocial(engine::Scene&);
    bool social_=false;
    double socialQuiet_=30;
    std::size_t newcomer_=4,greeter_=4;
    int welcomeStage_=0;
    std::array<bool,2> welcomeReady_{};
    double welcomeTime_=0;
    std::vector<engine::Vec3> welcomeRoute_;
    void updateAmbient(engine::Scene&,std::size_t,double);
    bool wander(engine::Scene&,std::size_t,bool);
    void yieldAmbient(engine::Scene&);
    void play(engine::Scene&,std::size_t,std::string_view,bool=false,double=0);
    bool move(engine::Scene&,std::size_t,double);
    bool laneBusy(std::size_t)const;
    std::vector<engine::Vec3> pathTo(engine::Scene&,std::size_t,engine::Vec3,std::string_view,bool fromCurrent=false)const;
};
struct Built { StarterScene scene; Presentation presentation; };
// Strong candidate construction: missing props or malformed supplied cats throw
// before the host replaces its valid outgoing scene. Absent cats remain absent.
Built build(const Assets&,const ProgressView&,bool reducedMotion);
}
