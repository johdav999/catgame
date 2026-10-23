#pragma once
#include "release1/contracts.hpp"
#include <memory>
#include <optional>

namespace paws::release1::rules {
enum class Family { Legacy, Bench, Planks, Shapes, Water, PictureSequence, Bells=PictureSequence, Switches, Picnic, Scent, Balance, Shade };
enum class Piece { Crate, Toy, Plank, Shape, Bell, Cat, Placement, Marker };
struct Object {
    std::string id,name,role;
    Piece kind=Piece::Marker;
    Cell initial;
    int orientation=0,length=1;
    std::vector<Cell> slots,shape;
    std::string symbol;
};
struct Socket {
    std::string id,name,role;
    Cell cell,end;
    int length=1,orientation=0;
    bool supported=true;
};
struct Step {
    Verb verb;
    std::string object,target;
    Direction direction=Direction::North;
    Cell destination;
};
struct TrailStage {
    std::array<std::string,3> clues,choices;
    unsigned answer=0;
};
struct BalanceRound { int pivot=4,catWeight=0,catArm=0; };
// Definition is copied into an attempt and only exposed as const. No mesh poses.
struct Definition {
    Family family=Family::Legacy;
    std::string quest,content,title,objective;
    int width=0,height=0;
    std::vector<Object> objects;
    std::vector<Socket> sockets;
    std::vector<Cell> walkable,basket,shade,quiet;
    Cell source,destination,reach;
    std::string requested;
    std::vector<std::string> sequence;
    double pictureSeconds=1;
    std::array<std::string,3> hints;
    std::vector<Step> solution;
    std::vector<TrailStage> trail;
    std::vector<BalanceRound> balances;
    std::optional<PuzzleDefinition> legacy;
    bool sharedLookout=false,sunnyNook=false;
    bool beachPicnic=false;
    // Both aspect ratios must show every cell, socket and clue; safe margins
    // are logical pixels and can grow for larger-text UI, never crop targets.
    int portraitMargin=24,landscapeMargin=24;
};
struct State {
    std::vector<ObjectState> objects;
    std::vector<std::string> entered;
    unsigned sequenceReplay=0;
    bool playhouseLatched=false;
    bool completed=false;
    unsigned stage=0,inspected=0;
    bool operator==(const State&) const=default;
};
std::vector<Cell> footprint(const Object&,const ObjectState&);
RouteResult plankRoute(const Definition&,const State&);
bool heldSwitches(const Definition&,const State&);
bool cooperativePlayhouse(const Definition&);
bool pawSwitchHeld(const Definition&,const State&);
bool playhouseOpen(const Definition&,const State&);
std::string picnicBlockage(const Definition&,const State&);
int balanceTorque(const Definition&,const State&);
// Distinct target cells shaded by committed, fitted panels (not drag previews).
std::vector<Cell> shadedCells(const Definition&,const PuzzleView&);
std::string validate(const Definition&,bool replay=true);
class Attempt {
public:
    explicit Attempt(Definition,std::uint64_t generation=1);
    Attempt(const Attempt&);
    Attempt& operator=(const Attempt&);
    Attempt(Attempt&&) noexcept=default;
    Attempt& operator=(Attempt&&) noexcept=default;
    const Definition& definition() const { return definition_; }
    const std::string& journalContent() const { return journalContent_; }
    const State& logicalState() const { return state_; }
    PuzzleView view() const;
    CommandResult dispatch(const Intent&);
    Intent command(const Step&) const;
    // Called by Afternoon/host on activation, restore or suspension boundary.
    // Does not reset an attempt or grant anything; invalidates delayed commands.
    void activate(std::uint64_t generation);
    void setPaused(bool);
private:
    CommandResult apply(const Intent&,bool record);
    std::vector<Step> candidates() const;
    Definition definition_;
    std::string journalContent_;
    State state_;
    std::unique_ptr<PawsSession> legacy_;
    std::vector<State> undo_;
    std::uint64_t generation_=1;
    AttemptId attempt_=1;
    Revision revision_=0;
    unsigned hints_=0;
    bool paused_=false;
    std::string selected_,feedback_;
};
}
