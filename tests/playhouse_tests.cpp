#include "afternoon.hpp"
#include "release1/rules/content.hpp"
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
using namespace paws;using namespace paws::release1;using namespace paws::release1::rules;
namespace {
int checks=0;
void check(bool value,std::string why){++checks;if(!value)throw std::runtime_error(why);}
void go(Attempt& a,Step s){auto r=a.dispatch(a.command(s));check(r.accepted,r.message);}
void blocked(Attempt& a,Step s,std::string_view reason){auto state=a.logicalState();auto v=a.view();auto r=a.dispatch(a.command(s));check(!r.accepted&&r.message.find(reason)!=std::string::npos,"Expected rejection: "+r.message);check(state==a.logicalState()&&v.revision==a.view().revision,"Rejected move changed state");}
bool active(const PuzzleView& v,std::string_view id){for(auto& o:v.objects)if(o.id==id)return o.active;throw std::runtime_error("Missing view object");}
void go(Afternoon& a,Step s){auto c=a.intent(s.verb,s.object,s.target);c.direction=s.direction;c.destination=s.destination;auto r=a.dispatchIntent(c);check(r.accepted,r.message);}
void select(Afternoon& a,std::size_t q){auto r=a.dispatchIntent(a.intent(Verb::SelectQuest,{},std::string(progression::quests[q].id)));check(r.accepted,r.message);}
}
int main(int argc,char** argv){try{
 const auto d=quest("pip_pepper_playhouse");check(validate(d).empty()&&cooperativePlayhouse(d),"Valid new playhouse");
 Attempt a(d);
 blocked(a,{Verb::AssignCat,"pepper","storage"},"side gate is closed");
 blocked(a,{Verb::Slide,"crate","",Direction::South},"side gate is closed");
 blocked(a,{Verb::AssignCat,"pip","inside_latch"},"through the doorway");
 blocked(a,{Verb::AssignCat,"pip","pepper_switch"},"own marked");
 go(a,{Verb::AssignCat,"pip","pip_switch"});
 blocked(a,{Verb::Slide,"crate","",Direction::South},"Move Pepper");
 go(a,{Verb::AssignCat,"pepper","pepper_switch"});
 check(active(a.view(),"playhouse_door")&&!a.view().completed,"Two switches only open the doorway");
 blocked(a,{Verb::Connect},"both kittens inside");
 blocked(a,{Verb::AssignCat,"pip","inside_entry"},"would close");
 go(a,{Verb::AssignCat,"pepper","storage"});check(!active(a.view(),"playhouse_door"),"Star releases immediately");
 go(a,{Verb::Slide,"crate","",Direction::South});
 go(a,{Verb::AssignCat,"pip","waiting"});check(!active(a.view(),"side_gate"),"Side gate closes without weight");
 blocked(a,{Verb::AssignCat,"pepper","pepper_switch"},"side gate is closed");
 go(a,{Verb::Undo});check(active(a.view(),"side_gate"),"Undo restores side gate");go(a,{Verb::AssignCat,"pip","waiting"});
 for(std::size_t n=4;n<=8;++n)go(a,d.solution[n]);
 check(pawSwitchHeld(d,a.logicalState())&&active(a.view(),"pip_switch")&&active(a.view(),"side_gate"),"Crate presses paw switch");
 go(a,{Verb::AssignCat,"pepper","pepper_switch"});go(a,{Verb::AssignCat,"pip","inside_entry"});
 blocked(a,{Verb::AssignCat,"pepper","pepper_inside"},"would close");
 const auto preLatch=a.logicalState();go(a,{Verb::AssignCat,"pip","inside_latch"});check(a.logicalState().playhouseLatched,"Latch commits");
 go(a,{Verb::Undo});check(a.logicalState()==preLatch&&!active(a.view(),"inside_latch"),"Undo releases latch");
 go(a,{Verb::AssignCat,"pip","inside_latch"});go(a,{Verb::AssignCat,"pepper","pepper_waiting"});
 check(!heldSwitches(d,a.logicalState())&&playhouseOpen(d,a.logicalState())&&!a.view().completed,"Latch holds while Pepper steps off; neither switch count nor latch alone completes");
 go(a,{Verb::AssignCat,"pepper","pepper_inside"});check(a.view().completed,"Both cats inside complete");
 go(a,{Verb::Undo});check(!a.view().completed&&a.logicalState().playhouseLatched,"Undo completion retains earlier latch");
 go(a,{Verb::Restart});check(!a.logicalState().playhouseLatched&&!active(a.view(),"side_gate")&&!active(a.view(),"playhouse_door"),"Restart resets all state");
 for(auto s:d.solution)go(a,s);check(a.view().completed,"Full solution replay");
 Attempt occupied(d);for(std::size_t n=0;n<3;++n)go(occupied,d.solution[n]);
 for(std::size_t n=4;n<8;++n)go(occupied,d.solution[n]);blocked(occupied,d.solution[8],"Pip occupies");
 Afternoon owner;
 for(std::size_t q=0;q<7;++q){select(owner,q);for(auto s:quest(progression::quests[q].id).solution)go(owner,s);}
 select(owner,8);check(owner.coins()==140,"Prerequisites earned");
 if(argc==3&&std::string_view(argv[1])=="--fixtures") {std::filesystem::create_directories(argv[2]);std::ofstream(std::filesystem::path(argv[2])/"playhouse.save")<<owner.encode();}
 auto oldBytes=owner.encode();auto pos=oldBytes.find("pip-pepper-playhouse-v2");check(pos!=std::string::npos,"Old fixture content found");oldBytes.replace(pos,23,"pip-pepper-playhouse-v1");
 auto old=Afternoon::decode(oldBytes);check(old.puzzleView().content=="pip-pepper-playhouse-v1","Old unplayed save retains content");
 for(auto s:quest("pip_pepper_playhouse","pip-pepper-playhouse-v1").solution)go(old,s);
 check(old.coins()==160,"Old completion earns once");
 if(argc==3&&std::string_view(argv[1])=="--fixtures")std::ofstream(std::filesystem::path(argv[2])/"playhouse-v1-complete.save")<<old.encode();
 old=Afternoon::decode(old.encode());go(old,{Verb::Restart});
 check(old.puzzleView().content==d.content&&old.coins()==160&&!old.puzzleView().completed,"Explicit restart upgrades without resetting progress");
 auto upgraded=old.encode();check(Afternoon::decode(upgraded).encode()==upgraded,"Mixed old/new journal roundtrip");
 for(auto s:d.solution){go(owner,s);auto bytes=owner.encode();check(Afternoon::decode(bytes).encode()==bytes,"Mid-step save roundtrip");go(old,s);old=Afternoon::decode(old.encode());}
 check(owner.coins()==160&&old.coins()==160,"New completion and old practice rewards once");
 go(owner,{Verb::Undo});owner=Afternoon::decode(owner.encode());check(!owner.puzzleView().completed&&owner.coins()==160,"Undo/reload preserves ledger");
 go(owner,d.solution.back());check(owner.coins()==160&&!owner.lastRewardGranted(),"Recompletion pays nothing");
 go(owner,{Verb::Restart});check(!active(owner.puzzleView(),"inside_latch"),"Owner restart unlatches");
 std::cout<<checks<<" playhouse checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
