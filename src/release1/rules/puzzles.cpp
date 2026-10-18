#include "puzzles.hpp"
#include "content.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <stdexcept>

namespace paws::release1::rules {
namespace {
template<class T> bool has(const std::vector<T>& v,const T& x) {return std::find(v.begin(),v.end(),x)!=v.end();}
template<class T> std::size_t index(const std::vector<T>& v,const std::string& id) {
    for(std::size_t i=0;i<v.size();++i)if(v[i].id==id)return i;return v.size();
}
bool inside(const Definition& d,Cell c) {return c.x>=0&&c.y>=0&&c.x<d.width&&c.y<d.height;}
int distance(Cell a,Cell b) {return std::abs(a.x-b.x)+std::abs(a.y-b.y);}
State initial(const Definition& d) {
    State s;for(auto& o:d.objects)s.objects.push_back({o.initial,o.orientation});return s;
}
bool reachableSocket(const Definition& d,const State& s,Cell target) {
    std::vector<Cell> pending{d.source},visited;
    while(!pending.empty()) {
        auto p=pending.back();pending.pop_back();
        if(has(visited,p)||!has(d.walkable,p))continue;
        visited.push_back(p);bool blocked=false;
        for(std::size_t i=0;i<d.objects.size();++i)if(d.objects[i].kind==Piece::Crate&&s.objects[i].cell==p)blocked=true;
        if(blocked)continue;if(p==target)return true;
        for(int n=0;n<4;++n)pending.push_back(adjacent(p,Direction(n)));
    }
    return false;
}
std::optional<Action> legacyAction(Verb v) {
    switch(v) {
        case Verb::Select:return Action::Select;case Verb::Slide:return Action::Move;
        case Verb::Rotate:return Action::Rotate;case Verb::Roll:return Action::Roll;
        case Verb::PickUp:return Action::PickUp;case Verb::Fit:return Action::Place;
        case Verb::Retrieve:return Action::PickUp;case Verb::Undo:return Action::Undo;
        case Verb::Restart:return Action::Restart;case Verb::Hint:return Action::Hint;
        default:return {};
    }
}
std::string label(const Step& s,const Definition& d) {
    const auto i=index(d.objects,s.object),j=index(d.sockets,s.target);
    std::string name=i<d.objects.size()?d.objects[i].name:s.object;
    if(d.legacy)for(auto& o:d.legacy->objects)if(o.id==s.object)name=o.name;
    switch(s.verb) {
        case Verb::Select:return "Select "+name;
        case Verb::Slide:return "Slide "+name+" "+std::array{"north","east","south","west"}[int(s.direction)];
        case Verb::Rotate:return "Rotate "+name+" clockwise";
        case Verb::PickUp:return "Pick up "+name;
        case Verb::Retrieve:return "Retrieve "+name+" from "+(j<d.sockets.size()?d.sockets[j].name:s.target);
        case Verb::TapSymbol:return "Tap "+name;
        case Verb::ReplaySequence:return "Replay pictures";
        case Verb::Inspect:return "Inspect "+name;
        case Verb::FollowTrail:return "Follow "+name;
        case Verb::TestBalance:return "Test this balance";
        case Verb::AssignCat:return "Assign "+name+" to "+d.sockets[j].name;
        case Verb::Fit: {
            auto text="Place "+name+" in "+(j<d.sockets.size()?d.sockets[j].name:s.target);
            if(d.family==Family::Shapes)text+=" at ("+std::to_string(s.destination.x)+","+std::to_string(s.destination.y)+")";
            return text;
        }
        case Verb::Roll:return d.family==Family::Water?"Check water flow":"Roll ball";
        case Verb::Connect:return d.family==Family::Shade?"Check shade":d.family==Family::Planks?"Check Mabel's route":d.family==Family::Switches?"Open doorway":"Check picnic clues";
        case Verb::Undo:return "Undo";case Verb::Restart:return "Restart / clear sequence";case Verb::Hint:return "Free hint";
        default:return "Unsupported action";
    }
}
}
std::vector<Cell> footprint(const Object& o,const ObjectState& s) {
    if(o.shape.empty())return {s.cell};
    auto cells=o.shape;
    for(int r=0;r<s.orientation;++r)for(auto& p:cells)p={-p.y,p.x};
    int x=cells[0].x,y=cells[0].y;
    for(auto p:cells){x=std::min(x,p.x);y=std::min(y,p.y);}
    for(auto& p:cells){p.x+=s.cell.x-x;p.y+=s.cell.y-y;}return cells;
}
RouteResult plankRoute(const Definition& d,const State& s) {
    RouteResult result;
    if(s.objects.size()!=d.objects.size()){result.message="Invalid plank state.";return result;}
    std::vector<std::vector<Cell>> pending{{d.source}};std::vector<Cell> visited;
    while(!pending.empty()) {
        auto path=pending.back();pending.pop_back();auto p=path.back();
        if(has(visited,p))continue;visited.push_back(p);
        const bool blocked=std::any_of(d.objects.begin(),d.objects.end(),[&](const auto& o){auto i=index(d.objects,o.id);return o.kind==Piece::Crate&&s.objects[i].cell==p;});
        if(blocked)continue;
        if(p==d.destination){result.reached=true;result.path=std::move(path);result.message=d.quest=="pip_friend_view"?"There! Beyond the fence! I knew there was something.":"Mabel has a supported route to the sunny perch.";return result;}
        for(std::size_t i=0;i<d.objects.size();++i) {
            const auto& o=d.objects[i];const auto& st=s.objects[i];
            if(o.kind!=Piece::Plank||st.location!=Location::Socket)continue;
            const auto j=index(d.sockets,st.socket);if(j==d.sockets.size())continue;
            const auto& t=d.sockets[j];
            if(!t.supported||o.length!=t.length||st.orientation%2!=t.orientation%2)continue;
            if(t.cell==p||t.end==p){auto next=path;next.push_back(t.cell==p?t.end:t.cell);pending.push_back(std::move(next));}
        }
    }
    result.message="The supported planks do not connect the start to the sunny perch.";return result;
}
bool cooperativePlayhouse(const Definition& d) {
    return d.family==Family::Switches&&index(d.objects,"crate")<d.objects.size();
}
bool pawSwitchHeld(const Definition& d,const State& s) {
    if(d.objects.size()!=s.objects.size())return false;
    const auto p=index(d.objects,"pip"),c=index(d.objects,"crate"),t=index(d.sockets,"pip_switch");
    return (p<d.objects.size()&&s.objects[p].socket=="pip_switch")||
        (c<d.objects.size()&&t<d.sockets.size()&&s.objects[c].cell==d.sockets[t].cell);
}
bool heldSwitches(const Definition& d,const State& s) {
    if(d.objects.size()!=s.objects.size())return false;
    const auto a=index(d.objects,"pip"),b=index(d.objects,"pepper");
    return a<d.objects.size()&&b<d.objects.size()&&pawSwitchHeld(d,s)&&
        s.objects[b].location==Location::Socket&&s.objects[b].socket=="pepper_switch";
}
bool playhouseOpen(const Definition& d,const State& s) {
    return s.playhouseLatched||heldSwitches(d,s);
}
std::string picnicBlockage(const Definition& d,const State& s) {
    if(s.objects.size()!=d.objects.size())return "Invalid picnic state.";
    if(d.beachPicnic){
        if(d.objects.size()!=4)return "Missing beach picnic mats.";
        for(const auto& o:s.objects)if(o.location!=Location::Socket)return "Place all four named mats first.";
        for(const auto& o:s.objects)if(has(d.walkable,o.cell))return "Leave the marked approach clear.";
        const auto& b=s.objects[index(d.objects,"button_mat")];const auto& m=s.objects[index(d.objects,"mabel_mat")];
        const auto& p=s.objects[index(d.objects,"pip_mat")];const auto& e=s.objects[index(d.objects,"pepper_mat")];
        if(!has(d.shade,b.cell)||!has(d.quiet,b.cell))return "Button needs both shade and quiet.";
        if(!has(d.shade,m.cell)||distance(b.cell,m.cell)!=1)return "Mabel needs shade beside Button.";
        if(has(d.shade,p.cell))return "Pip would like a sunny place.";
        if(distance(p.cell,e.cell)!=1)return "Pepper wants a place beside Pip.";
        return {};
    }
    const auto bed=index(d.objects,"bed"),bowl=index(d.objects,"bowl");
    if(bed==d.objects.size()||bowl==d.objects.size())return "Missing picnic props.";
    const auto& a=s.objects[bed];const auto& b=s.objects[bowl];
    if(d.sunnyNook){
        if(a.location!=Location::Socket||b.location!=Location::Socket)return "Place both cushions in the sunny nook.";
        if(has(d.walkable,a.cell)||has(d.walkable,b.cell))return "Keep the marked middle path clear.";
        if(!has(d.shade,a.cell)||!has(d.shade,b.cell))return "Both cushions need a golden sunshine mark.";
        if(!has(d.quiet,a.cell))return "Mabel prefers her familiar rear-left sunny spot.";
        if(a.cell.y!=b.cell.y||std::abs(a.cell.x-b.cell.x)!=2)return "Leave one clear path cell between the two cushions.";
        return {};
    }
    if(a.location!=Location::Socket||b.location!=Location::Socket)return "Place both supplied picnic props first.";
    if(!has(d.shade,a.cell))return "Button's bed needs a SHADE spot.";
    if(!has(d.quiet,a.cell))return "Button's bed needs a QUIET spot as well as shade.";
    if(!has(d.shade,b.cell))return "Mabel's bowl needs a SHADE spot.";
    if(distance(a.cell,b.cell)!=1)return "The bowl must share an edge with the bed; diagonals do not count.";
    return {};
}
std::string validate(const Definition& d,bool replay) {
    if(int(d.family)<0||int(d.family)>int(Family::Shade)||d.quest.empty()||d.content.empty()||d.title.empty()||d.objective.empty())return "Invalid definition identity/family.";
    if(d.width<1||d.width>16||d.height<1||d.height>16||d.portraitMargin<24||d.landscapeMargin<24)return "Invalid board/framing bounds.";
    for(auto& h:d.hints)if(h.empty())return "Three progressive hints are required.";
    if(d.family==Family::Legacy||d.family==Family::Water) {
        if(!d.legacy||d.legacy->questId!=d.quest||d.legacy->contentId!=d.content||d.width!=d.legacy->width||d.height!=d.legacy->height)return "Invalid legacy/route definition.";
        if(auto e=validateContent(*d.legacy,replay);!e.empty())return e;
    } else {
        if(d.legacy||d.objects.empty()||d.objects.size()>64||d.sockets.size()>64)return "Invalid typed object count.";
        std::set<std::string> ids;
        for(auto& o:d.objects) {
            if(o.id.empty()||o.name.empty()||o.role.empty()||!ids.insert(o.id).second||!inside(d,o.initial)||o.orientation<0||o.orientation>3||int(o.kind)<0||int(o.kind)>int(Piece::Marker))return "Invalid/duplicate object.";
            if(o.kind==Piece::Plank&&(o.length<1||o.length>2))return "Unsupported plank length.";
            if(o.kind==Piece::Crate&&(o.slots.empty()||!has(o.slots,o.initial)))return "Crate needs authored sliding slots.";
            std::set<std::pair<int,int>> slots;
            for(auto p:o.slots)if(!inside(d,p)||!slots.emplace(p.x,p.y).second)return "Invalid sliding slot.";
            std::set<std::pair<int,int>> shape;
            for(auto p:o.shape)if(p.x<0||p.y<0||p.x>2||p.y>2||!shape.emplace(p.x,p.y).second)return "Invalid shape footprint.";
            if(o.kind==Piece::Shape) {
                const std::vector<Cell> expected=o.role=="cushion_square"?std::vector<Cell>{{0,0},{1,0},{0,1},{1,1}}:
                    o.role=="cushion_bar"?std::vector<Cell>{{0,0},{1,0},{2,0}}:
                    o.role=="cushion_elbow"?std::vector<Cell>{{0,0},{1,0},{0,1}}:std::vector<Cell>{};
                if(o.shape!=expected||expected.empty())return "Shape violates frozen footprint contract.";
            }
        }
        std::set<std::pair<int,int>> socketCells;
        for(auto& t:d.sockets) {
            if(t.id.empty()||t.name.empty()||t.role.empty()||!ids.insert(t.id).second||!inside(d,t.cell))return "Invalid/duplicate socket.";
            if(!socketCells.emplace(t.cell.x,t.cell.y).second&&d.family!=Family::Planks)return "Socket destinations overlap.";
            if((d.family==Family::Planks||d.family==Family::Shade)&&(!inside(d,t.end)||distance(t.cell,t.end)!=t.length||
                (t.cell.x!=t.end.x&&t.cell.y!=t.end.y)||t.length<1||t.length>2||t.orientation<0||t.orientation>3||
                (t.cell.y==t.end.y?0:1)!=t.orientation%2))return "Invalid plank connection endpoints.";
        }
        for(auto cells:{d.walkable,d.basket,d.shade,d.quiet}) {
            std::set<std::pair<int,int>> unique;
            for(auto c:cells)if(!inside(d,c)||!unique.emplace(c.x,c.y).second)return "Invalid/duplicate zone cell.";
        }
        auto count=[&](Piece k){return std::count_if(d.objects.begin(),d.objects.end(),[&](auto& o){return o.kind==k;});};
        if(d.family==Family::Bench) {
            auto i=index(d.objects,d.requested);
            if(i==d.objects.size()||d.objects[i].kind!=Piece::Toy||!has(d.walkable,d.source)||!has(d.walkable,d.reach)||distance(d.reach,d.objects[i].initial)!=1)return "Invalid restricted retrieval reference/reach.";
            if(std::none_of(d.sockets.begin(),d.sockets.end(),[&](auto& s){return s.cell==d.reach&&s.role=="reach_socket";}))return "Missing reach socket.";
        }
        if(d.family==Family::Planks&&(count(Piece::Plank)+count(Piece::Crate)!=int(d.objects.size())||count(Piece::Plank)==0||d.sockets.empty()||!inside(d,d.source)||!inside(d,d.destination)||d.source==d.destination))return "Invalid plank route roles.";
        if(d.family==Family::Shapes) {
            if(count(Piece::Shape)!=3||d.basket.size()!=10||d.sockets.size()!=1||d.sockets[0].role!="cushion_basket")return "Invalid basket roles/area.";
            std::set<std::string> roles;for(auto& o:d.objects)roles.insert(o.role);
            if(roles!=std::set<std::string>{"cushion_square","cushion_bar","cushion_elbow"})return "Basket needs one cushion of each frozen shape.";
        }
        if(d.family==Family::Bells) {
            if(count(Piece::Bell)<2||count(Piece::Bell)>8||d.sequence.empty()||d.sequence.size()>8||!std::isfinite(d.pictureSeconds)||d.pictureSeconds<.25||d.pictureSeconds>10)return "Sequence needs 1–8 pictures, 2–8 responses and a valid duration.";
            std::set<std::string> symbols;
            for(auto& o:d.objects)if(o.kind==Piece::Bell&&(!symbols.insert(o.symbol).second||o.symbol.empty()))return "Invalid/duplicate response picture.";
            for(auto& s:d.sequence)if(!symbols.contains(s))return "Unknown sequence picture.";
        }
        if(d.family==Family::Switches) {
            if(count(Piece::Cat)!=2||index(d.objects,"pip")==d.objects.size()||index(d.objects,"pepper")==d.objects.size())return "Missing kitten roles.";
            for(auto id:{"pip_switch","pepper_switch","waiting"})if(index(d.sockets,id)==d.sockets.size())return "Missing switch/release reference.";
            if(d.sockets[index(d.sockets,"pip_switch")].cell==d.sockets[index(d.sockets,"pepper_switch")].cell)return "Switches need distinct cells.";
            if(cooperativePlayhouse(d)) {
                if(count(Piece::Crate)!=1||d.objects[index(d.objects,"crate")].kind!=Piece::Crate||!inside(d,d.source)||!inside(d,d.destination))return "Invalid cooperation crate/gates.";
                for(auto id:{"storage","inside_entry","inside_latch","pepper_inside","pepper_waiting"})if(index(d.sockets,id)==d.sockets.size())return "Missing cooperation destination.";
                const auto& crate=d.objects[index(d.objects,"crate")];
                if(!has(crate.slots,d.source)||!has(crate.slots,d.sockets[index(d.sockets,"pip_switch")].cell))return "Crate track must connect storage to paw switch.";
            }
        }
        if(d.family==Family::Picnic&&!d.beachPicnic&&(count(Piece::Placement)!=2||index(d.objects,"bed")==d.objects.size()||index(d.objects,"bowl")==d.objects.size()||d.shade.empty()||d.quiet.empty()||d.sockets.empty()))return "Missing picnic clue roles.";
        if(d.beachPicnic){
            if(d.family!=Family::Picnic||count(Piece::Placement)!=4||d.shade.empty()||d.quiet.empty()||d.sockets.empty())return "Invalid beach picnic roles.";
            for(auto id:{"button_mat","mabel_mat","pip_mat","pepper_mat"})if(index(d.objects,id)==d.objects.size())return "Missing named beach mat.";
        }
        if(d.family==Family::Shade){
            if(count(Piece::Plank)!=2||d.objects.size()!=2||d.shade.size()!=4||d.sockets.empty())return "Shade needs two panels and four mat cells.";
            for(const auto& o:d.objects)if(o.role!="shade_panel"||o.length!=2)return "Invalid shade panel.";
            for(const auto& t:d.sockets)if(t.role!="shade_socket")return "Invalid shade supports.";
        }
        if(d.family==Family::Scent) {
            if(d.trail.empty()||d.trail.size()>12||d.objects.size()!=6)return "Invalid scent trail stages.";
            for(int n=0;n<3;++n)for(auto prefix:{"clue_","trail_"})if(index(d.objects,std::string(prefix)+std::to_string(n))==d.objects.size())return "Missing trail clue or choice.";
            for(const auto& stage:d.trail){if(stage.answer>2)return "Invalid trail answer.";for(int n=0;n<3;++n)if(stage.clues[n].empty()||stage.choices[n].empty())return "Missing trail evidence.";}
        }
        if(d.family==Family::Balance) {
            if(d.balances.empty()||d.balances.size()>8||count(Piece::Placement)!=(d.sharedLookout?6:4))return "Invalid balance rounds.";
            if(d.sharedLookout){
                for(auto id:{"pip_cushion","your_cushion"}){const auto i=index(d.objects,id);if(i==d.objects.size()||d.objects[i].role!="lookout_cushion")return "Missing shared-lookout cushion.";}
                for(auto id:{"pip_seat","your_seat","approach"})if(index(d.sockets,id)==d.sockets.size())return "Missing shared-lookout seat or approach.";
            }
            for(const auto& r:d.balances)if(r.pivot<1||r.pivot>=d.width||r.catWeight<0||r.catWeight>8||std::abs(r.catArm)>4)return "Invalid balance load.";
            for(int n=1;n<=4;++n){const auto i=index(d.objects,"weight_"+std::to_string(n));if(i==d.objects.size()||d.objects[i].length!=n||index(d.sockets,"home_"+std::to_string(n))==d.sockets.size())return "Missing numbered balance weight/home.";}
        }
        auto start=initial(d);
        for(std::size_t i=0;i<d.objects.size();++i)for(std::size_t j=i+1;j<d.objects.size();++j)
            if(d.objects[i].kind!=Piece::Marker&&d.objects[j].kind!=Piece::Marker&&start.objects[i].cell==start.objects[j].cell)return "Initial pieces overlap.";
    }
    if(replay) {
        if(d.solution.empty()||d.solution.size()>128)return "Missing/bounded known solution.";
        Attempt a(d);
        for(auto& s:d.solution) {
            if(s.verb==Verb::Undo||s.verb==Verb::Restart||s.verb==Verb::Hint||s.verb==Verb::Select)return "Solution must contain puzzle operations.";
            auto result=a.dispatch(a.command(s));if(!result.accepted)return "Illegal known solution: "+result.message;
        }
        if(!a.view().completed)return "Known solution does not complete this layout.";
    }
    return {};
}
Attempt::Attempt(Definition d,std::uint64_t generation):definition_(std::move(d)),generation_(generation) {
    if(auto error=validate(definition_,false);!error.empty())throw std::invalid_argument(error);
    if(definition_.legacy)legacy_=std::make_unique<PawsSession>(*definition_.legacy);
    journalContent_=definition_.content;state_=initial(definition_);feedback_=definition_.objective;
}
Attempt::Attempt(const Attempt& o):definition_(o.definition_),journalContent_(o.journalContent_),state_(o.state_),
    legacy_(o.legacy_?std::make_unique<PawsSession>(*o.legacy_):nullptr),undo_(o.undo_),
    generation_(o.generation_),attempt_(o.attempt_),revision_(o.revision_),hints_(o.hints_),
    paused_(o.paused_),selected_(o.selected_),feedback_(o.feedback_) {}
Attempt& Attempt::operator=(const Attempt& o) {
    if(this!=&o){Attempt candidate(o);*this=std::move(candidate);}return *this;
}
void Attempt::activate(std::uint64_t generation) {generation_=generation;++revision_;if(legacy_)legacy_->invalidateCommands(attempt_);}
void Attempt::setPaused(bool value) {if(paused_!=value){paused_=value;++revision_;}}
Intent Attempt::command(const Step& s) const {
    Intent c;c.generation=generation_;c.attempt=attempt_;c.revision=revision_;c.quest=definition_.quest;
    c.object=s.object;c.target=s.target;c.verb=s.verb;c.direction=s.direction;c.destination=s.destination;return c;
}
CommandResult Attempt::dispatch(const Intent& c) {return apply(c,true);}
int balanceTorque(const Definition& d,const State& s) {
    if(d.family!=Family::Balance||d.balances.empty()||s.objects.size()!=d.objects.size())return 0;
    const auto& r=d.balances[std::min<std::size_t>(s.stage,d.balances.size()-1)];
    int torque=r.catWeight*r.catArm;
    for(std::size_t n=0;n<d.objects.size();++n)if(d.objects[n].kind==Piece::Placement&&s.objects[n].socket.starts_with("beam_"))torque+=d.objects[n].length*(s.objects[n].cell.x-r.pivot);
    return torque;
}
CommandResult Attempt::apply(const Intent& c,bool record) {
    if(c.generation!=generation_||c.attempt!=attempt_||c.revision!=revision_||c.quest!=definition_.quest)return {false,"That action belongs to an older puzzle state."};
    if(paused_)return {false,"Resume the puzzle first."};
    if(revision_==std::numeric_limits<Revision>::max()||attempt_==std::numeric_limits<AttemptId>::max())return {false,"Attempt identity limit reached. Start a new session."};
    auto accept=[&](std::string message) {if(record){++revision_;feedback_=message;}return CommandResult{true,std::move(message)};};
    if(legacy_) {
        const auto v=definition_.family==Family::Water&&c.verb==Verb::Connect?Verb::Roll:c.verb;
        auto action=legacyAction(v);if(!action)return {false,"Unsupported action for this puzzle."};
        auto candidate=*legacy_;
        auto result=candidate.dispatch({candidate.state().attempt,candidate.state().revision,*action,c.object,c.direction,c.target});
        if(result.accepted) {
            if(definition_.family==Family::Water) {
                if(v==Verb::Roll)result.message=candidate.state().puzzle.completed?"The water reaches Mabel's bowl. Bowl filled!":"The catch basin is full. Turn the remaining gutters, keeping the cushion dry.";
                else if(v==Verb::Rotate)result.message="Gutter turned clockwise. Check the visible openings.";
            }
            if(record){*legacy_=std::move(candidate);if(v==Verb::Restart){++attempt_;hints_=0;}if(v==Verb::Hint)hints_=std::min(3u,hints_+1);}
            return accept(result.message);
        }
        // Water feedback comes from the same port tracer; adapt its subject.
        if(definition_.family==Family::Water) {
            std::size_t pos=0;while((pos=result.message.find("ball",pos))!=std::string::npos){result.message.replace(pos,4,"water");pos+=5;}
        }
        return result;
    }
    if(c.verb==Verb::Restart) {
        // A deliberate restart may opt an old playhouse journal into the new
        // layout. The codec retains its original content ID and this explicit
        // restart target, so old moves still replay against their original rules.
        const bool upgrade=definition_.content=="pip-pepper-playhouse-v1"&&c.target=="pip-pepper-playhouse-v2";
        if(!c.target.empty()&&!upgrade)return {false,"Unsupported restart layout."};
        if(record&&upgrade)definition_=quest(definition_.quest,c.target);
        if(record){++attempt_;state_=initial(definition_);undo_.clear();hints_=0;selected_.clear();}return accept("Fresh attempt. Pictures, pieces and switches reset.");
    }
    if(c.verb==Verb::Hint) {
        auto h=std::min(3u,hints_+1);if(record)hints_=h;return accept(definition_.hints[h-1]);
    }
    if(c.verb==Verb::Undo) {
        if(undo_.empty())return {false,"Nothing to undo yet."};
        if(record){state_=undo_.back();undo_.pop_back();}return accept("Last committed change undone.");
    }
    if(c.verb==Verb::ReplaySequence){
        if(definition_.family!=Family::PictureSequence||state_.completed)return {false,"There is no picture sequence to replay."};
        if(record){undo_.push_back(state_);if(undo_.size()>128)undo_.erase(undo_.begin());state_.entered.clear();++state_.sequenceReplay;}
        return accept("Watch the blackboard again. Your answer starts fresh.");
    }
    const auto& d=definition_;const auto i=index(d.objects,c.object),t=index(d.sockets,c.target);
    if(c.verb==Verb::Select) {
        if(i==d.objects.size())return {false,"Choose a named puzzle object."};
        if(record)selected_=c.object;return accept(d.objects[i].name+" selected.");
    }
    if(state_.completed)return {false,"Puzzle complete. Undo or restart to play again."};
    auto next=state_;std::string message;
    if(c.verb==Verb::Inspect||c.verb==Verb::FollowTrail) {
        if(d.family!=Family::Scent)return {false,"There is no scent trail here."};
        if(i==d.objects.size())return {false,"Choose a clue or trail."};
        const auto& stage=d.trail[next.stage];
        if(c.verb==Verb::Inspect){
            if(!c.object.starts_with("clue_")||c.object.size()!=6||c.object[5]<'0'||c.object[5]>'2')return {false,"Choose one of the three evidence markers."};
            const unsigned n=unsigned(c.object[5]-'0');next.inspected|=1u<<n;message=stage.clues[n];
        }else{
            if(next.inspected!=7)return {false,"Inspect all three clues before choosing the next trail."};
            if(!c.object.starts_with("trail_")||c.object.size()!=7||c.object[6]<'0'||c.object[6]>'2')return {false,"Choose a marked trail."};
            if(unsigned(c.object[6]-'0')!=stage.answer)return {false,"That place does not fit all three clues. Your discoveries are kept; compare them again."};
            ++next.stage;next.inspected=0;next.completed=next.stage==d.trail.size();
            message=next.completed?(d.quest=="beach_pepper_basket"?"Pepper found the picnic basket beside the shell. All four dune stops solved!":"Pepper found her catnip pouch in the leaf cushion. All six trail stops solved!"):"A fresh trail! Inspect the new evidence before choosing again.";
        }
    } else if(c.verb==Verb::TestBalance) {
        if(d.family!=Family::Balance)return {false,"There is no balance to test here."};
        for(std::size_t n=0;n<d.objects.size();++n)if(d.objects[n].role=="balance_weight"&&!next.objects[n].socket.starts_with("beam_"))return {false,"Put all four numbered weights on different beam positions first."};
        if(d.sharedLookout&&next.stage+1==d.balances.size()&&(next.objects[index(d.objects,"pip_cushion")].socket!="pip_seat"||next.objects[index(d.objects,"your_cushion")].socket!="your_seat"))return {false,"Place both cushions on their seats, leaving the middle approach clear."};
        const int torque=balanceTorque(d,next);
        if(torque)return {false,torque<0?"The left side pulls harder. Move weight closer to the support on the left, or farther out on the right.":"The right side pulls harder. Move weight closer to the support on the right, or farther out on the left."};
        ++next.stage;next.completed=next.stage==d.balances.size();
        if(!next.completed){auto fresh=initial(d);next.objects=std::move(fresh.objects);}
        message=d.sharedLookout?(next.completed?"This one is yours. I saved it.":"Steady! Now balance the flowerpots with Pip aboard, and place both cushions."):(next.completed?"All four setups balance. Pip's perch is steady!":"Steady! The next setup changes the support or Pip's load. Cushions returned to storage.");
    } else if(c.verb==Verb::Connect) {
        if(d.family==Family::Shade){
            for(const auto& o:next.objects)if(o.location!=Location::Socket)return {false,"Fit both shade panels between firm supports first."};
            for(auto cell:d.shade){bool covered=false;for(const auto& o:next.objects){
                const auto t=index(d.sockets,o.socket);if(t==d.sockets.size())continue;const auto& support=d.sockets[t];
                const int dx=(support.end.x>support.cell.x)-(support.end.x<support.cell.x),dy=(support.end.y>support.cell.y)-(support.end.y<support.cell.y);
                for(int n=0;n<support.length;++n)covered=covered||cell==Cell{support.cell.x+n*dx,support.cell.y+n*dy};
            }if(!covered)return {false,"Part of Mabel's marked mat is still in the sun."};}
            message="Both panels are supported. Mabel's whole mat is shaded.";
        }
        else if(d.family==Family::Planks) {auto r=plankRoute(d,next);if(!r.reached)return {false,r.message};message=r.message;}
        else if(d.family==Family::Switches) {
            if(cooperativePlayhouse(d))return {false,"Bring both kittens inside. Opening the doorway is only the first part."};
            if(!heldSwitches(d,next))return {false,"Pip and Pepper must hold their own switches together now. Earlier presses do not count."};message="Both switches held. The doorway is open!";}
        else if(d.family==Family::Picnic) {auto e=picnicBlockage(d,next);if(!e.empty())return {false,e};message=d.beachPicnic?"Everyone has a place. Our seaside picnic is ready!":d.sunnyNook?"Two cushions in the sunshine, with a clear path between them. Mabel saved Pip a place.":"Shade, quiet and beside clues all fit. Ready for the picnic!";}
        else return {false,"This puzzle has no route or clue check."};
        next.completed=true;
    } else {
        if(i==d.objects.size())return {false,"Choose a named puzzle object."};
        const auto& o=d.objects[i];auto& st=next.objects[i];
        if(c.verb==Verb::Slide) {
            if(o.kind!=Piece::Crate)return {false,"Only the crates slide in this puzzle."};
            if(int(c.direction)<0||int(c.direction)>3)return {false,"Invalid slide direction."};
            auto p=adjacent(st.cell,c.direction);
            if(!has(o.slots,p))return {false,"There is no authored sliding slot in that direction."};
            if(cooperativePlayhouse(d)) {
                const auto crateHome=o.initial;
                if(st.cell==crateHome||p==crateHome) {
                    if(!pawSwitchHeld(d,state_))return {false,"The side gate is closed. Hold the paw switch first."};
                    if(next.objects[index(d.objects,"pepper")].socket!="storage")return {false,"Move Pepper through the side gate to help slide the crate out."};
                }
                for(std::size_t j=0;j<d.objects.size();++j)if(j!=i&&next.objects[j].cell==p)return {false,d.objects[j].name+" occupies that spot. Move them clear first."};
            }
            for(std::size_t j=0;j<d.objects.size();++j)if(j!=i&&d.objects[j].kind==Piece::Crate&&next.objects[j].cell==p)return {false,d.objects[j].name+" occupies that slot. Move it first."};
            st.cell=p;message=o.name+" slid one slot.";
        } else if(c.verb==Verb::Retrieve) {
            if(d.family!=Family::Bench||c.object!=d.requested)return {false,"Choose Button's requested mouse."};
            if(t==d.sockets.size()||d.sockets[t].role!="reach_socket")return {false,"Choose a named bench reach socket."};
            const auto p=d.sockets[t].cell;
            if(p!=d.reach||distance(p,st.cell)!=1)return {false,"That socket cannot reach the mouse. Use the near socket; cats do not crawl under the bench."};
            if(!reachableSocket(d,next,p))return {false,"The crates block the path to the near reach socket."};
            st.location=Location::Collected;next.completed=true;message="Button's mouse retrieved from beside the bench.";
        } else if(c.verb==Verb::PickUp) {
            if(o.kind!=Piece::Shape&&o.kind!=Piece::Plank)return {false,"Choose a cushion or plank to pick up."};
            for(auto& s:next.objects)if(s.location==Location::Held)return {false,"Place the piece you are holding first."};
            st.location=Location::Held;st.socket.clear();message=o.name+" held. Choose rotation and destination.";
        } else if(c.verb==Verb::Rotate) {
            if(o.kind!=Piece::Shape&&o.kind!=Piece::Plank)return {false,"Only cushions and planks rotate here."};
            // A direct click can turn a loose or fitted piece. A fitted piece
            // leaves its crossing/basket until the player drops it again.
            if(st.location==Location::Socket){st.location=Location::Ground;st.socket.clear();}
            st.orientation=(st.orientation+1)%4;message=o.name+" turned clockwise.";
        } else if(c.verb==Verb::Fit) {
            if(t==d.sockets.size())return {false,"Choose a named destination socket."};
            const auto& socket=d.sockets[t];
            if(d.family==Family::Balance&&o.role=="lookout_cushion") {
                if(next.stage+1!=d.balances.size())return {false,"Make the empty lookout steady first."};
                const auto seat=c.object=="pip_cushion"?"pip_seat":"your_seat";
                if(socket.id!=seat)return {false,"Keep the middle approach clear and use the seat marked for this cushion."};
                if(st.socket==seat)return {false,"That cushion is already in place."};
                st.cell=socket.cell;st.socket=socket.id;st.location=Location::Socket;message="A place saved. Test the lookout with Pip aboard.";
            } else if(d.family==Family::Balance) {
                if(o.kind!=Piece::Placement||(socket.role!="balance_socket"&&socket.id!="home_"+c.object.substr(7)))return {false,"Choose a beam position or this weight's own storage mat."};
                if(st.socket==socket.id)return {false,"That weight is already there."};
                bool swapped=false;
                for(std::size_t j=0;j<next.objects.size();++j)if(j!=i&&next.objects[j].cell==socket.cell){
                    if(!d.sharedLookout||d.objects[j].role!="balance_weight"||socket.role!="balance_socket")return {false,"That position is occupied. Return a weight to storage to make room."};
                    auto& other=next.objects[j];
                    if(st.socket.starts_with("beam_"))other=st;
                    else {
                        const auto home=index(d.sockets,"home_"+d.objects[j].id.substr(7));
                        if(home==d.sockets.size())return {false,"That flowerpot has no storage mat."};
                        other.cell=d.sockets[home].cell;other.socket=d.sockets[home].id;other.location=Location::Socket;
                    }
                    swapped=true;
                }
                st.cell=socket.cell;st.socket=socket.id;st.location=Location::Socket;
                message=swapped?"Flowerpots switched. Test the balance when you're ready.":o.name+" placed. Tap the paw mat when all four weights balance Pip's load.";
            } else if(d.family==Family::Shapes) {
                if(o.kind!=Piece::Shape||st.location!=Location::Held)return {false,"Pick up a cushion first."};
                if(socket.role!="cushion_basket"||!inside(d,c.destination))return {false,"Choose a basket cell destination."};
                st.cell=c.destination;
                for(auto p:footprint(o,st)) {
                    if(!has(d.basket,p))return {false,"The rotated cushion extends outside the basket cells."};
                    for(std::size_t j=0;j<d.objects.size();++j)if(j!=i&&next.objects[j].location==Location::Socket&&has(footprint(d.objects[j],next.objects[j]),p))return {false,"That cushion overlaps "+d.objects[j].name+". Choose another destination."};
                }
                st.location=Location::Socket;st.socket=socket.id;
                next.completed=std::all_of(next.objects.begin(),next.objects.end(),[](auto& s){return s.location==Location::Socket;});
                message=next.completed?"All ten basket cells covered. Button's bed fits!":o.name+" fitted without overlap.";
            } else if(d.family==Family::Planks||d.family==Family::Shade) {
                if(o.kind!=Piece::Plank||st.location!=Location::Held)return {false,"Pick up a plank first."};
                if(socket.role!=(d.family==Family::Shade?"shade_socket":"plank_socket")||o.length!=socket.length)return {false,"The plank length does not match this crossing."};
                if(!socket.supported)return {false,"This gap has no supported landing. Choose a supported crossing."};
                if(st.orientation%2!=socket.orientation%2)return {false,"Rotate the plank so both endpoints face the crossing supports."};
                for(std::size_t j=0;j<d.objects.size();++j)if(d.objects[j].kind==Piece::Crate&&(next.objects[j].cell==socket.cell||next.objects[j].cell==socket.end))return {false,"The planter blocks this landing. Slide it clear of both route platforms."};
                for(auto& s:next.objects)if(s.location==Location::Socket&&s.socket==c.target)return {false,"That crossing already has a plank."};
                st.cell=socket.cell;st.socket=socket.id;st.location=Location::Socket;message=o.name+" connected to both supports.";
            } else if(d.family==Family::Picnic) {
                if(o.kind!=Piece::Placement||socket.role!="picnic_spot")return {false,"Choose a supplied picnic prop and garden spot."};
                for(std::size_t j=0;j<next.objects.size();++j)if(j!=i&&next.objects[j].location==Location::Socket&&next.objects[j].cell==socket.cell)return {false,"The other picnic prop occupies that spot."};
                if((d.sunnyNook||d.beachPicnic)&&has(d.walkable,socket.cell))return {false,"Leave the marked middle path clear between the cushions."};
                st.cell=socket.cell;st.socket=socket.id;st.location=Location::Socket;message=o.name+" placed. Check all clues together.";
            } else return {false,"This puzzle does not fit objects into sockets."};
        } else if(c.verb==Verb::TapSymbol) {
            if(d.family!=Family::Bells||o.kind!=Piece::Bell)return {false,"Choose a picture bell."};
            if(o.symbol!=d.sequence[next.entered.size()]) {next.entered.clear();message="Different picture. Your answer is cleared. Try again or replay the blackboard.";}
            else {next.entered.push_back(o.symbol);next.completed=next.entered==d.sequence;message=next.completed?"Picture sequence complete!":"Picture matched. Keep playing the remembered order.";}
        } else if(c.verb==Verb::AssignCat) {
            if(d.family!=Family::Switches||o.kind!=Piece::Cat||t==d.sockets.size())return {false,"Choose Pip or Pepper and a named switch or waiting spot."};
            const auto& socket=d.sockets[t];
            if(cooperativePlayhouse(d)) {
                const bool pip=c.object=="pip";
                if((socket.id=="pip_switch"&&!pip)||(socket.id=="pepper_switch"&&pip))return {false,"Each kitten holds its own marked switch."};
                if((pip&&(socket.id=="storage"||socket.id=="pepper_inside"||socket.id=="pepper_waiting"))||
                   (!pip&&(socket.id=="inside_entry"||socket.id=="inside_latch"||socket.id=="waiting")))return {false,"Use the destination marked for this kitten."};
                if(st.socket==socket.id)return {false,o.name+" is already there."};
                if((socket.id=="storage"||st.socket=="storage")&&!pawSwitchHeld(d,state_))return {false,"The side gate is closed. Put Pip or the crate on the paw switch."};
                const auto inside=[](std::string_view id){return id=="inside_entry"||id=="inside_latch"||id=="pepper_inside";};
                if(socket.id=="inside_latch"&&!inside(st.socket))return {false,"Move Pip through the doorway before using the inside latch."};
                for(std::size_t j=0;j<next.objects.size();++j)if(j!=i&&next.objects[j].cell==socket.cell)return {false,d.objects[j].name+" occupies that spot. Move them clear first."};
                const bool wasInside=inside(st.socket),willBeInside=inside(socket.id);
                st.socket=socket.id;st.cell=socket.cell;st.location=Location::Socket;
                if(wasInside!=willBeInside&&(!playhouseOpen(d,state_)||!playhouseOpen(d,next)))return {false,"The doorway would close when that kitten leaves its switch. Use the crate, then the inside latch."};
                if(socket.id=="inside_latch")next.playhouseLatched=true;
                next.completed=next.playhouseLatched&&inside(next.objects[index(d.objects,"pip")].socket)&&inside(next.objects[index(d.objects,"pepper")].socket);
                message=next.completed?"Both kittens are inside! The playhouse is ready.":socket.id=="inside_latch"?"Pip secured the inside latch. Pepper can leave the star switch and come inside.":socket.id=="storage"?"Pepper reached the storage alcove. Drag the crate out through the gate.":socket.id=="inside_entry"?"Pip is inside. Move him onto the latch tile to hold the door open.":socket.id=="pip_switch"?"Pip holds the paw switch. The side gate is open for Pepper.":socket.id=="pepper_switch"?"Pepper holds the star switch. With the paw switch held, Pip can enter.":"Waiting place reached. Unweighted switches release immediately.";
            } else {
                if(socket.role!="waiting"&&socket.id!=c.object+"_switch")return {false,"Each kitten holds its own marked switch."};
                for(std::size_t j=0;j<next.objects.size();++j)if(j!=i&&next.objects[j].location==Location::Socket&&next.objects[j].cell==socket.cell)return {false,"The other kitten occupies that spot."};
                st.socket=socket.id;st.cell=socket.cell;st.location=Location::Socket;message=socket.role=="waiting"?o.name+" stepped away. Switch released.":o.name+" is holding the switch now.";
            }
        } else return {false,"Unsupported action for this puzzle."};
    }
    if(record){if(undo_.size()==128)undo_.erase(undo_.begin());undo_.push_back(state_);state_=std::move(next);}return accept(message);
}
std::vector<Step> Attempt::candidates() const {
    std::vector<Step> actions={{Verb::Undo},{Verb::Restart},{Verb::Hint}};
    const auto& d=definition_;
    if(d.family==Family::PictureSequence)actions.push_back({Verb::ReplaySequence});
    if(d.family==Family::Scent)for(int n=0;n<3;++n){actions.push_back({Verb::Inspect,"clue_"+std::to_string(n)});actions.push_back({Verb::FollowTrail,"trail_"+std::to_string(n)});}
    if(d.family==Family::Balance)actions.push_back({Verb::TestBalance});
    if(d.legacy) {
        for(auto& o:d.legacy->objects) {
            actions.push_back({Verb::Select,o.id});
            if(sliding(o.kind))for(int n=0;n<4;++n)actions.push_back({Verb::Slide,o.id,"",Direction(n)});
            if(channel(o.kind))actions.push_back({Verb::Rotate,o.id});
            if(o.kind==Kind::Toy) {
                actions.push_back({Verb::PickUp,o.id});
                for(auto& t:d.legacy->objects)if(t.kind==Kind::Socket)actions.push_back({Verb::Fit,o.id,t.id});
                if(d.legacy->packedToysBlockAccess)actions.push_back({Verb::Fit,o.id,"home"});
            }
        }
        if(d.legacy->goal!=Goal::Tidy)actions.push_back({Verb::Roll});return actions;
    }
    for(auto& o:d.objects) {
        actions.push_back({Verb::Select,o.id});
        if(o.kind==Piece::Crate)for(int n=0;n<4;++n)actions.push_back({Verb::Slide,o.id,"",Direction(n)});
        if(o.kind==Piece::Toy)for(auto& t:d.sockets)if(t.role=="reach_socket")actions.push_back({Verb::Retrieve,o.id,t.id});
        if(o.kind==Piece::Plank||o.kind==Piece::Shape) {
            actions.push_back({Verb::PickUp,o.id});actions.push_back({Verb::Rotate,o.id});
            for(auto& t:d.sockets) {
                if(o.kind==Piece::Shape)for(auto p:d.basket)actions.push_back({Verb::Fit,o.id,t.id,Direction::North,p});
                else actions.push_back({Verb::Fit,o.id,t.id});
            }
        }
        if(o.kind==Piece::Placement)for(auto& t:d.sockets)actions.push_back({Verb::Fit,o.id,t.id});
        if(o.kind==Piece::Bell)actions.push_back({Verb::TapSymbol,o.id});
        if(o.kind==Piece::Cat)for(auto& t:d.sockets)actions.push_back({Verb::AssignCat,o.id,t.id});
    }
    if(d.family==Family::Planks||d.family==Family::Shade||d.family==Family::Switches||d.family==Family::Picnic)actions.push_back({Verb::Connect});return actions;
}
PuzzleView Attempt::view() const {
    PuzzleView v;const auto& d=definition_;v.quest=d.quest;v.content=d.content;v.title=d.title;v.objective=d.objective;
    v.generation=generation_;v.attempt=attempt_;v.revision=revision_;v.feedback=feedback_;v.hints=d.hints;v.revealedHints=hints_;
    v.completed=legacy_?legacy_->state().puzzle.completed:state_.completed;v.canUndo=legacy_?legacy_->canUndo():!undo_.empty();
    v.symbols=d.sequence;v.enteredSymbols=state_.entered;v.pictureSequence=d.family==Family::PictureSequence;v.sequenceReplay=state_.sequenceReplay;
    if(legacy_) {
        const auto& s=legacy_->state().puzzle;
        for(std::size_t i=0;i<d.legacy->objects.size();++i) {
            auto& o=d.legacy->objects[i];auto& st=s.objects[i];
            std::string role=channel(o.kind)?"channel":sliding(o.kind)?"pot":o.kind==Kind::Toy?"toy":o.kind==Kind::Socket?"basket_socket":o.kind==Kind::Gate?"gate":o.kind==Kind::Catch?"catch_tray":"switch";
            if(d.family==Family::Water&&channel(o.kind))role="gutter";
            if(d.quest=="beach_pip_ball"&&o.kind==Kind::Crate)role="crate";
            if(d.family==Family::Water&&o.kind==Kind::Catch)role=o.id=="dry_cushion"?"dry_cushion":"water_basin";
            else if(o.kind==Kind::Cushion)role="blocking_cushion";
            v.objects.push_back({o.id,o.name,role,st.cell,st.orientation,{st.cell},st.socket,st.location==Location::Held,
                o.kind==Kind::Switch?switchActive(*d.legacy,s,o.id):o.kind==Kind::Gate?gateOpen(*d.legacy,s,o.id):false,st.location==Location::Collected});
        }
        if(d.family==Family::Water)for(auto& o:v.objects)if(o.role=="water_basin")o.active=s.checkpointReached;
        if(d.legacy->goal!=Goal::Tidy)v.route=evaluateRoute(*d.legacy,s).path;
        const auto ballCell=d.legacy->goal==Goal::Relay?(s.completed?d.destination:s.checkpointReached?d.legacy->checkpoint:d.source):d.source;
        if(d.family!=Family::Water&&d.legacy->goal==Goal::Relay)v.objective=s.checkpointReached?"The ball is safe. Turn the four channels beyond the tray, then roll it home to Pip.":"Move the planters and turn the first four channels to reach the catch tray.";
        if(d.family==Family::Water&&d.legacy->goal==Goal::Relay)v.objective=s.checkpointReached?"Basin filled. Turn the remaining gutters to Mabel's bowl; keep the cushion dry. Tap the basin to check.":d.objective;
        v.objects.push_back({d.legacy->sourceId,d.family==Family::Water?"Rain barrel outlet":"Ball source",d.family==Family::Water?"rain_barrel":"ball",d.family==Family::Water?d.source:ballCell,0,{d.family==Family::Water?d.source:ballCell}});
        v.objects.push_back({d.legacy->destinationId,d.family==Family::Water?"Mabel's bowl":"Destination",d.family==Family::Water?"drinking_bowl":"destination",d.destination,0,{d.destination},"",false,v.completed,false});
    } else {
        for(std::size_t i=0;i<d.objects.size();++i) {
            const auto& o=d.objects[i];const auto& st=state_.objects[i];
            v.objects.push_back({o.id,o.name,o.role,st.cell,st.orientation,footprint(o,st),st.socket,st.location==Location::Held,
                o.kind==Piece::Cat&&st.socket==o.id+"_switch",st.location==Location::Collected});
            if(o.kind==Piece::Plank&&st.location==Location::Socket) {
                const auto t=index(d.sockets,st.socket);if(t<d.sockets.size()) {
                    const auto& socket=d.sockets[t];auto p=socket.cell;v.objects.back().footprint={p};
                    const int dx=(socket.end.x>p.x)-(socket.end.x<p.x),dy=(socket.end.y>p.y)-(socket.end.y<p.y);
                    for(int n=0;n<socket.length;++n){p.x+=dx;p.y+=dy;v.objects.back().footprint.push_back(p);}
                }
            }
        }
        for(auto& t:d.sockets) {
            bool active=false;for(auto& o:state_.objects)if(o.location==Location::Socket&&o.socket==t.id)active=true;
            if(cooperativePlayhouse(d)&&t.id=="pip_switch")active=pawSwitchHeld(d,state_);
            if(cooperativePlayhouse(d)&&t.id=="inside_latch")active=state_.playhouseLatched;
            v.objects.push_back({t.id,t.name,t.role,t.cell,t.orientation,{t.cell},"",false,active,false});
        }
        if(d.family==Family::Scent){
            const auto stage=std::min<std::size_t>(state_.stage,d.trail.size()-1);
            v.objective=v.completed?(d.quest=="beach_pepper_basket"?"The picnic basket and blanket are safe!":"Pepper's catnip pouch is safe!"):"Trail stop "+std::to_string(stage+1)+" / "+std::to_string(d.trail.size())+". Inspect the three clue markers, then choose a trail. ";
            for(int n=0;n<3;++n){
                auto& clue=v.objects[index(d.objects,"clue_"+std::to_string(n))];clue.active=(state_.inspected&(1u<<n))!=0;
                clue.name=clue.active?d.trail[stage].clues[n]:(d.quest=="beach_pepper_basket"?std::array{"Paw prints: inspect","Blanket threads: inspect","Picnic scent: inspect"}[n]:std::array{"Paw prints: inspect","Snagged fluff: inspect","Catnip scent: inspect"}[n]);
                v.objects[index(d.objects,"trail_"+std::to_string(n))].name=d.trail[stage].choices[n];
                if(clue.active&&!v.completed)v.objective+=clue.name+" ";
            }
        }
        if(d.family==Family::Balance){
            const auto& round=d.balances[std::min<std::size_t>(state_.stage,d.balances.size()-1)];
            const auto torque=balanceTorque(d,state_);
            v.objective=v.completed?"Four steady setups! Pip can settle on his perch.":"Setup "+std::to_string(state_.stage+1)+" / "+std::to_string(d.balances.size())+". Support: "+std::to_string(round.pivot)+". Pip: "+std::to_string(round.catWeight)+" weight x "+std::to_string(std::abs(round.catArm))+" steps "+(round.catArm<0?"left":"right")+". Pull: "+(torque==0?"even":torque<0?"left +":"right +")+(torque==0?"":std::to_string(std::abs(torque)))+". Beam positions: 1,2,3,5,6,7. Cushions start 1,2,3,4 from the left. Place all four, then test.";
            if(d.sharedLookout)v.objective=v.completed?"Two places, one steady lookout. Pip saved you a seat.":state_.stage==0?"Balance the four numbered flowerpots on the empty lookout, then test.":"Pip adds 3 weights at position 7. Balance the pots, then put both cushions on their marked seats. Keep the approach clear.";
            for(auto& t:v.objects)if(t.role=="balance_socket")t.name="Position "+std::to_string(t.cell.x)+" ("+std::to_string(std::abs(t.cell.x-round.pivot))+" steps "+(t.cell.x<round.pivot?"left":"right")+")";
            v.objects.push_back({"balance_support","Support","balance_support",{round.pivot,2}});
            v.objects.push_back({"balance_beam",std::to_string(torque),"balance_beam",{4,2},torque<0?1:torque>0?2:0});
            v.objects.push_back({"balance_pip","Pip's load","balance_pip",round.catWeight?Cell{round.pivot+round.catArm,2}:Cell{8,3},0,{},"",false,round.catWeight>0,false});
            v.objects.push_back({"balance_pad","Pip's perch platform","balance_pad",{round.pivot+round.catArm,2},0,{},"",false,round.catWeight>0,false});
        }
        if(d.family==Family::Planks)v.route=plankRoute(d,state_).path;
        if(d.family==Family::Planks) {
            v.objects.push_back({"mabel_start","Mabel's start platform","platform",d.source,0,{d.source}});
            v.objects.push_back({"sunny_perch","Sunny perch destination","sunny_perch",d.destination,0,{d.destination}});
        }
        if(d.family==Family::Shapes)for(auto& o:v.objects)if(o.role=="cushion_basket")o.footprint=d.basket;
        if(d.family==Family::Picnic&&!d.sunnyNook) {
            v.objects.push_back({"shade_clue","SHADE cells","shade_clue",d.shade.front(),0,d.shade});
            v.objects.push_back({"quiet_clue","QUIET cells","quiet_clue",d.quiet.front(),0,d.quiet});
            const auto& bed=state_.objects[index(d.objects,d.beachPicnic?"button_mat":"bed")];const auto& bowl=state_.objects[index(d.objects,d.beachPicnic?"mabel_mat":"bowl")];
            v.objects.push_back({"adjacency_clue","Bowl beside bed (shared edge)","adjacency_clue",bed.cell,0,{bed.cell,bowl.cell},"",false,
                bed.location==Location::Socket&&bowl.location==Location::Socket&&distance(bed.cell,bowl.cell)==1,false});
        }
        if(d.family==Family::Switches) {
            const auto door=cooperativePlayhouse(d)?d.destination:Cell{2,0};
            v.objects.push_back({"playhouse_door","Playhouse doorway","playhouse_door",door,0,{door},"",false,playhouseOpen(d,state_),false});
            if(cooperativePlayhouse(d)) {
                v.objects.push_back({"side_gate","Storage side gate","side_gate",d.source,0,{d.source},"",false,pawSwitchHeld(d,state_),false});
                const auto& pip=state_.objects[index(d.objects,"pip")];const auto& pepper=state_.objects[index(d.objects,"pepper")];
                const auto& crate=state_.objects[index(d.objects,"crate")];
                v.objective=state_.playhouseLatched?"Door latched open! Drag Pepper off the star switch and into the playhouse.":
                    pip.socket=="inside_entry"?"Pip is inside. Drag him onto the inside latch tile to secure the door.":
                    heldSwitches(d,state_)?"The doorway is open. Use the crate to keep the paw switch held, then drag Pip inside.":
                    pawSwitchHeld(d,state_)&&crate.cell==d.sockets[index(d.sockets,"pip_switch")].cell?"The crate holds the paw switch. Drag Pepper to the star switch to open the playhouse.":
                    crate.cell!=d.objects[index(d.objects,"crate")].initial?"Park Pip on his waiting mat, then drag the crate along its track onto the paw switch.":
                    pepper.socket=="storage"?"Pepper can help now. Drag the crate out through the open side gate.":
                    pawSwitchHeld(d,state_)?"The crate needs Pepper's help. Drag Pepper to the white-marked storage alcove beside it first.":d.objective;
                if(v.completed)v.objective="Both kittens are inside. A little teamwork made room for everyone.";
            }
        }
    }
    // Preflight is the exact commit processor on a local copy. The GUI receives
    // both enabled actions and descriptive reasons; it never duplicates legality.
    Attempt probe(d,generation_);probe.state_=state_;probe.undo_=undo_;probe.attempt_=attempt_;probe.revision_=revision_;
    probe.hints_=hints_;probe.paused_=paused_;if(legacy_)*probe.legacy_=*legacy_;
    for(auto& s:candidates()) {auto intent=command(s);auto result=probe.apply(intent,false);auto text=label(s,d);
        if(s.verb==Verb::Inspect||s.verb==Verb::FollowTrail){auto o=std::find_if(v.objects.begin(),v.objects.end(),[&](const auto& o){return o.id==s.object;});if(o!=v.objects.end())text=(s.verb==Verb::Inspect?"Inspect: ":"Follow: ")+o->name;}
        v.actions.push_back({intent,text,result.accepted?"":result.message,result.accepted});}
    return v;
}
}
