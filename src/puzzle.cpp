#include "puzzle.hpp"
#include <algorithm>
#include <set>
#include <stdexcept>
#include <tuple>

namespace paws {
namespace {
Direction opposite(Direction d) { return Direction((int(d)+2)%4); }
bool inside(const PuzzleDefinition& c, Cell p) { return p.x>=0 && p.y>=0 && p.x<c.width && p.y<c.height; }
std::size_t indexOf(const PuzzleDefinition& c, const ObjectId& id) {
    for(std::size_t i=0;i<c.objects.size();++i) if(c.objects[i].id==id) return i;
    return c.objects.size();
}
}
bool sliding(Kind k) { return k==Kind::Pot || k==Kind::Cushion; }
bool channel(Kind k) { return k==Kind::Straight || k==Kind::Curve; }
Cell adjacent(Cell p, Direction d) {
    switch(d) { case Direction::North: --p.y; break; case Direction::East: ++p.x; break;
        case Direction::South: ++p.y; break; case Direction::West: --p.x; break; }
    return p;
}
std::vector<Direction> ports(Kind k, int r) {
    if(!channel(k)) return {};
    return {Direction(r%4), Direction((r+(k==Kind::Straight?2:1))%4)};
}
PuzzleState initialState(const PuzzleDefinition& c) {
    PuzzleState s;
    for(const auto& o:c.objects) s.objects.push_back({o.initial,o.orientation});
    return s;
}
std::string moveBlockage(const PuzzleDefinition& c, const PuzzleState& s, std::size_t i, Direction d) {
    if(i>=c.objects.size() || s.objects.size()!=c.objects.size()) return "Unknown object.";
    const auto& o=c.objects[i];
    if(!sliding(o.kind)) return "Select a pot or cushion to slide it.";
    const auto target=adjacent(s.objects[i].cell,d);
    if(std::find(o.slots.begin(),o.slots.end(),target)==o.slots.end()) return "No sliding slot in that direction.";
    for(std::size_t j=0;j<c.objects.size();++j)
        if(i!=j && sliding(c.objects[j].kind) && s.objects[j].cell==target) return "Another object occupies that slot.";
    return {};
}
bool switchActive(const PuzzleDefinition& c,const PuzzleState& s,const ObjectId& id) {
    const auto i=indexOf(c,id);
    return i<c.objects.size() && c.objects[i].kind==Kind::Switch &&
        c.objects[i].initial==c.destination && s.ballOnSwitch;
}
bool gateOpen(const PuzzleDefinition& c,const PuzzleState& s,const ObjectId& id) {
    const auto i=indexOf(c,id);
    return i<c.objects.size() && c.objects[i].kind==Kind::Gate && switchActive(c,s,c.objects[i].link);
}
bool reachable(const PuzzleDefinition& c,const PuzzleState& s,Cell target) {
    if(c.walkable.empty()) return true;
    std::vector<Cell> pending{c.access},visited;
    while(!pending.empty()) {
        auto p=pending.back();pending.pop_back();
        if(std::find(visited.begin(),visited.end(),p)!=visited.end() ||
           std::find(c.walkable.begin(),c.walkable.end(),p)==c.walkable.end()) continue;
        visited.push_back(p);bool blocked=false;
        for(std::size_t i=0;i<c.objects.size();++i) if(s.objects[i].cell==p &&
            (sliding(c.objects[i].kind)||(c.objects[i].kind==Kind::Gate&&!gateOpen(c,s,c.objects[i].id)))) blocked=true;
        if(blocked) continue;
        if(p==target) return true;
        for(int d=0;d<4;++d) pending.push_back(adjacent(p,Direction(d)));
    }
    return false;
}
std::string pickupBlockage(const PuzzleDefinition& c,const PuzzleState& s,std::size_t i) {
    if(i>=c.objects.size()||c.objects[i].kind!=Kind::Toy) return "Choose a toy to pick up.";
    if(s.objects[i].location==Location::Collected) return "Pip already has this toy.";
    for(const auto& o:s.objects) if(o.location==Location::Held) return "Place the toy you are holding first.";
    if(!c.objects[i].link.empty()&&!gateOpen(c,s,c.objects[i].link)) return "The gate is closed. Keep the ball on the switch.";
    if(!reachable(c,s,s.objects[i].cell)) return "The way is blocked. Slide the cushion aside first.";
    return {};
}
std::string placeBlockage(const PuzzleDefinition& c,const PuzzleState& s,std::size_t i,const ObjectId& target) {
    auto j=indexOf(c,target);
    if(i>=c.objects.size()||c.objects[i].kind!=Kind::Toy||s.objects[i].location!=Location::Held) return "Pick up a toy first.";
    if(j>=c.objects.size()||c.objects[j].kind!=Kind::Socket) return "Choose a basket recess.";
    if(!reachable(c,s,c.objects[j].initial)) return "The basket is blocked. Slide the cushion aside first.";
    for(const auto& o:s.objects) if(o.location==Location::Socket&&o.socket==target) return "That recess is occupied.";
    if(c.objects[i].shape!=c.objects[j].shape) return "That shape does not fit this recess.";
    return {};
}
RouteResult traceRoute(const PuzzleDefinition& c, const PuzzleState& s, Cell cell, Direction entry) {
    RouteResult result;
    if(s.objects.size()!=c.objects.size()) { result.message="Invalid puzzle state."; return result; }
    std::set<std::tuple<int,int,int>> visited;
    while(inside(c,cell)) {
        if(!visited.emplace(cell.x,cell.y,int(entry)).second) { result.message="The route loops back on itself."; return result; }
        result.path.push_back(cell);
        if(cell==c.destination) {
            result.reached=entry==c.destinationEntry;
            result.message=result.reached?"Pip's ball is home! Puzzle complete.":"The ball reaches the wrong side of the destination.";
            return result;
        }
        std::size_t channel=c.objects.size();
        for(std::size_t i=0;i<c.objects.size();++i) if(s.objects[i].cell==cell) {
            if(sliding(c.objects[i].kind)) { result.message=c.objects[i].name+" blocks the ball. Slide it aside."; return result; }
            if(paws::channel(c.objects[i].kind)) channel=i;
        }
        if(channel==c.objects.size()) { result.message="The ball meets an empty slot. Connect the channels."; return result; }
        const auto p=ports(c.objects[channel].kind,s.objects[channel].orientation);
        const auto found=std::find(p.begin(),p.end(),entry);
        if(found==p.end()) { result.message=c.objects[channel].name+" has no opening toward the ball."; return result; }
        const auto exit=p[0]==entry?p[1]:p[0];
        cell=adjacent(cell,exit); entry=opposite(exit);
    }
    result.message="The channel sends the ball off the board.";
    return result;
}
RouteResult evaluateRoute(const PuzzleDefinition& c, const PuzzleState& s) {
    auto result=traceRoute(c,s,adjacent(c.source,c.sourceExit),opposite(c.sourceExit));
    result.path.insert(result.path.begin(),c.source);
    return result;
}
std::string validateContent(const PuzzleDefinition& c, bool replay) {
    if((c.schemaVersion!=1 && c.schemaVersion!=2) || c.contentId.empty() || c.questId.empty()) return "Unsupported content identity/version.";
    if(c.schemaVersion==1 && c.goal!=Goal::Route) return "New goals require schema 2.";
    if(c.goal!=Goal::Route&&c.goal!=Goal::Tidy&&c.goal!=Goal::Retrieve) return "Unknown goal.";
    if(c.width<1 || c.height<1 || c.width>16 || c.height>16 || !inside(c,c.source) || !inside(c,c.destination) || c.source==c.destination) return "Invalid board endpoints.";
    if(int(c.sourceExit)<0 || int(c.sourceExit)>3 || int(c.destinationEntry)<0 || int(c.destinationEntry)>3) return "Invalid endpoint direction.";
    if(c.objects.empty() || c.objects.size()>64) return "Invalid object count.";
    std::set<std::string> ids;
    if(c.sourceId.empty() || c.destinationId.empty() || c.sourceId==c.destinationId) return "Invalid endpoint identity.";
    ids.insert(c.sourceId); ids.insert(c.destinationId);
    std::set<std::tuple<int,int,int>> occupancy;
    for(const auto& o:c.objects) {
        if(o.id.empty() || o.name.empty() || !ids.insert(o.id).second) return "Invalid or duplicate object identity.";
        if(!inside(c,o.initial) || (c.goal==Goal::Route&&(o.initial==c.source||o.initial==c.destination)) || o.orientation<0 || o.orientation>3) return "Invalid object placement.";
        if(int(o.kind)<0||int(o.kind)>int(Kind::Gate)||(c.schemaVersion==1&&int(o.kind)>2)) return "Unsupported object kind.";
        if(!occupancy.emplace(o.initial.x,o.initial.y,sliding(o.kind)?0:channel(o.kind)?1:2).second) return "Overlapping objects on the same occupancy layer.";
        if(sliding(o.kind)) {
            if(std::find(o.slots.begin(),o.slots.end(),o.initial)==o.slots.end()) return "Pot must begin on its sliding slots.";
            std::set<std::pair<int,int>> slots;
            for(auto p:o.slots) if(!inside(c,p) || p==c.source || p==c.destination || !slots.emplace(p.x,p.y).second) return "Invalid sliding slot.";
        } else if(!o.slots.empty()) return "Only obstacles can slide.";
        if((o.kind==Kind::Toy||o.kind==Kind::Socket)&&(int(o.shape)<1||int(o.shape)>3)) return "Missing toy/socket shape.";
    }
    for(const auto& o:c.objects) if(!o.link.empty()) {
        const auto target=indexOf(c,o.link);
        if(target==c.objects.size() || (o.kind==Kind::Gate?c.objects[target].kind!=Kind::Switch:
            o.kind!=Kind::Toy||c.objects[target].kind!=Kind::Gate)) return "Invalid linked role.";
    }
    if(c.goal!=Goal::Route) {
        if(c.hints.size()!=3||c.walkable.empty()||std::find(c.walkable.begin(),c.walkable.end(),c.access)==c.walkable.end()) return "Missing hints or access layout.";
        std::set<std::pair<int,int>> cells;
        for(auto p:c.walkable) if(!inside(c,p)||!cells.emplace(p.x,p.y).second) return "Invalid access layout.";
        if(c.goal==Goal::Tidy) {
            for(auto shape:{Shape::Ball,Shape::Mouse,Shape::Fish}) {
                int toys=0,sockets=0;
                for(const auto& o:c.objects) if(o.shape==shape) {toys+=o.kind==Kind::Toy;sockets+=o.kind==Kind::Socket;}
                if(toys!=1||sockets!=1) return "Tidy requires one toy and recess of each shape.";
            }
        } else {
            const auto toy=indexOf(c,c.requestedItem),gate=indexOf(c,c.gateRole),sw=indexOf(c,c.switchRole);
            if(toy==c.objects.size()||gate==c.objects.size()||sw==c.objects.size()||c.objects[toy].kind!=Kind::Toy||
               c.objects[toy].link!=c.gateRole||c.objects[gate].kind!=Kind::Gate||c.objects[gate].link!=c.switchRole||
               c.objects[sw].kind!=Kind::Switch||c.objects[sw].initial!=c.destination) return "Invalid retrieval roles.";
        }
    }
    if(replay) {
        if(c.solution.empty()) return "Missing known solution.";
        PawsSession session(c); // Constructor performs structural validation only.
        for(const auto& step:c.solution) {
            if(step.action!=Action::Move && step.action!=Action::Rotate && step.action!=Action::Roll && step.action!=Action::PickUp && step.action!=Action::Place) return "Invalid solution action.";
            auto result=session.dispatch({session.state().attempt,session.state().revision,step.action,step.object,step.direction,step.target});
            if(!result.accepted) return "Known solution contains an illegal command: "+result.message;
        }
        if(!session.state().puzzle.completed) return "Known solution does not reach destination.";
    }
    return {};
}
PawsSession::PawsSession(PuzzleDefinition c):content_(std::move(c)) {
    if(auto error=validateContent(content_,false); !error.empty()) throw std::invalid_argument(error);
    state_.questId=content_.questId; state_.puzzle=initialState(content_);
}
CommandResult PawsSession::dispatch(const Command& c) {
    if(c.attempt!=state_.attempt || c.revision!=state_.revision) return {false,"That action belongs to an older puzzle state."};
    auto accept=[&](std::string message) { ++state_.revision; return CommandResult{true,std::move(message)}; };
    if(c.action==Action::Resume) { state_.paused=false; state_.inGarden=false; return accept(content_.objective); }
    if(c.action==Action::Pause) { state_.paused=!state_.paused; if(!state_.paused) state_.inGarden=false; return accept(state_.paused?"Puzzle paused.":"Puzzle resumed."); }
    if(c.action==Action::Garden) { state_.inGarden=true; state_.paused=true; return accept("Your attempt is kept. Continue when ready."); }
    if(state_.paused) return {false,"Resume the puzzle first."};
    if(c.action==Action::Restart) {
        ++state_.attempt; state_.puzzle=initialState(content_); state_.selected.clear(); state_.hint=0; undo_.clear();
        return accept("A fresh attempt. Everything is back in place.");
    }
    if(c.action==Action::Hint) {
        if(content_.hints.empty()) return {false,"No hint is available."};
        state_.hint=std::min(state_.hint+1,unsigned(content_.hints.size()));
        return accept(content_.hints[state_.hint-1]);
    }
    if(c.action==Action::Undo) {
        if(undo_.empty()) return {false,"Nothing to undo yet."};
        state_.puzzle=undo_.back(); undo_.pop_back(); return accept("Last change undone.");
    }
    const auto i=indexOf(content_,c.object);
    if(c.action==Action::Select) {
        if(i==content_.objects.size()) return {false,"Select a puzzle object."};
        state_.selected=c.object; return accept(content_.objects[i].name+" selected.");
    }
    if(state_.puzzle.completed) return {false,"Puzzle complete. Undo or restart to play again."};
    auto next=state_.puzzle;
    std::string message;
    if(c.action==Action::Roll) {
        if(content_.goal==Goal::Tidy) return {false,"Pick up the toys and place them in their matching recesses."};
        if(next.ballOnSwitch) return {false,"The ball is already holding the switch down."};
        auto route=evaluateRoute(content_,next);
        if(!route.reached) return {false,route.message+" The ball stays at its source."};
        next.completed=content_.goal==Goal::Route;
        next.ballOnSwitch=content_.goal==Goal::Retrieve;
        message=next.ballOnSwitch?"Switch pressed. The gate is open while the ball stays here.":route.message;
    } else if(c.action==Action::ReturnBall) {
        if(!next.ballOnSwitch) return {false,"The ball is already at its source."};
        next.ballOnSwitch=false;message="Ball returned. The switch is empty and the gate is closed.";
    } else if(c.action==Action::PickUp) {
        if(auto reason=pickupBlockage(content_,next,i);!reason.empty()) return {false,reason};
        next.objects[i].location=content_.goal==Goal::Retrieve&&c.object==content_.requestedItem?Location::Collected:Location::Held;
        next.objects[i].socket.clear();
        next.completed=next.objects[i].location==Location::Collected;
        message=next.completed?"You found Pip's mouse!":content_.objects[i].name+" picked up. Choose its matching recess.";
    } else if(c.action==Action::Place) {
        if(auto reason=placeBlockage(content_,next,i,c.target);!reason.empty()) return {false,reason};
        next.objects[i].location=Location::Socket;next.objects[i].socket=c.target;
        next.objects[i].cell=content_.objects[indexOf(content_,c.target)].initial;
        next.completed=true;
        for(std::size_t j=0;j<content_.objects.size();++j) if(content_.objects[j].kind==Kind::Toy&&next.objects[j].location!=Location::Socket) next.completed=false;
        message=next.completed?"All three toys are tucked away. Lovely!":"A perfect fit.";
    } else if(c.action==Action::Move || c.action==Action::Rotate) {
        if(i==content_.objects.size()) return {false,"Select a puzzle object."};
        if(c.action==Action::Move) {
            if(int(c.direction)<0 || int(c.direction)>3) return {false,"Invalid direction."};
            if(auto error=moveBlockage(content_,next,i,c.direction); !error.empty()) return {false,error};
            next.objects[i].cell=adjacent(next.objects[i].cell,c.direction); message=content_.objects[i].name+" moved to the next slot.";
        } else {
            if(!channel(content_.objects[i].kind)) return {false,"Only channels rotate."};
            next.objects[i].orientation=(next.objects[i].orientation+1)%4; message="Channel rotated clockwise.";
        }
        if(next.ballOnSwitch) {next.ballOnSwitch=false;message+=" Ball returned to source; the gate closes.";}
    } else return {false,"Unknown puzzle action."};
    if(undo_.size()==128) undo_.erase(undo_.begin());
    undo_.push_back(state_.puzzle); state_.puzzle=std::move(next); return accept(message);
}
}
