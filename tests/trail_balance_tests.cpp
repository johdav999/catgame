#include "afternoon.hpp"
#include "release1/rules/content.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace paws;using namespace paws::release1;using namespace paws::release1::rules;
namespace {
int checks=0;
void check(bool b,const std::string& message){++checks;if(!b)throw std::runtime_error(message);}
void go(Attempt& a,Step s){auto r=a.dispatch(a.command(s));check(r.accepted,r.message);}
void go(Afternoon& a,Step s){auto c=a.intent(s.verb,s.object,s.target);c.direction=s.direction;c.destination=s.destination;auto r=a.dispatchIntent(c);check(r.accepted,r.message);}
void blocked(Attempt& a,Step s){const auto before=a.logicalState();const auto rev=a.view().revision;check(!a.dispatch(a.command(s)).accepted,"Expected rejected action");check(before==a.logicalState()&&rev==a.view().revision,"Rejection mutated attempt");}
void select(Afternoon& a,std::size_t q){go(a,{Verb::SelectQuest,"",std::string(progression::quests[q].id)});}
}
int main(int argc,char** argv){try{
 const auto scent=quest("pepper_scent_trail"),balance=quest("pip_wobbly_perch");
 check(validate(scent).empty()&&validate(balance).empty(),"Both authored solutions validate");
 Attempt trail(scent);blocked(trail,{Verb::FollowTrail,"trail_0"});
 for(int n=0;n<3;++n)go(trail,{Verb::Inspect,"clue_"+std::to_string(n)});
 blocked(trail,{Verb::FollowTrail,"trail_1"});auto first=trail.logicalState();
 go(trail,{Verb::FollowTrail,"trail_0"});check(trail.logicalState().stage==1&&trail.logicalState().inspected==0,"Next stop clears discoveries");
 go(trail,{Verb::Undo});check(trail.logicalState()==first,"Undo restores evidence and previous stop");
 go(trail,{Verb::Restart});for(auto s:scent.solution)go(trail,s);check(trail.view().completed,"Six stops completed");
 go(trail,{Verb::Undo});check(!trail.view().completed&&trail.logicalState().stage==5,"Undo last trail choice");
 Attempt beam(balance);blocked(beam,{Verb::TestBalance});
 go(beam,{Verb::Fit,"weight_1","beam_1"});blocked(beam,{Verb::Fit,"weight_2","beam_1"});
 blocked(beam,{Verb::Fit,"weight_1","home_2"});go(beam,{Verb::Fit,"weight_1","home_1"});
 go(beam,{Verb::Restart});
 for(int n=1;n<=4;++n)go(beam,{Verb::Fit,"weight_"+std::to_string(n),"beam_"+std::to_string(n==4?5:n)});
 blocked(beam,{Verb::TestBalance});go(beam,{Verb::Restart});
 for(auto s:balance.solution){auto stale=beam.command(s);go(beam,s);check(!beam.dispatch(stale).accepted,"Stale balance action rejected");}
 check(beam.view().completed&&balanceTorque(balance,beam.logicalState())==0,"Four actual torque equations solved");
 go(beam,{Verb::Undo});check(!beam.view().completed&&beam.logicalState().stage==3,"Undo balance completion");
 auto malformed=balance;malformed.balances[0].pivot=99;check(!validate(malformed).empty(),"Reject malformed support");
 Afternoon owner;check(!owner.unlocked(10)&&!owner.unlocked(11),"New quests respect prerequisites");
 for(std::size_t q=0;q<9;++q){select(owner,q);for(auto s:quest(progression::quests[q].id).solution)go(owner,s);}
 check(owner.unlocked(10)&&owner.unlocked(11),"Both later quests available");
 if(argc==2){std::filesystem::create_directories(argv[1]);std::ofstream(std::filesystem::path(argv[1])/"ready.save")<<owner.encode();}
 // A real version-six envelope has exactly the original ten records. Keep its
 // reward and ending policy while adding two fresh optional attempts on import.
 if(argc==2){for(std::size_t q=10;q<12;++q){select(owner,q);std::ofstream(std::filesystem::path(argv[1])/(q==10?"scent.save":"balance.save"))<<owner.encode();}select(owner,8);}
 auto old=owner.encode();old.replace(0,old.find("\n"),"PAWS_AFTERNOON 6");auto pos=old.find("QUESTS "+std::to_string(progression::quests.size()));old.replace(pos,9,"QUESTS 10");pos=old.find("\"pepper_scent_trail\"",old.find("QUESTS 10"));old.erase(pos,old.find("END\n",pos)-pos);
 if(auto home=old.find("HOME ");home!=std::string::npos)old.erase(home,old.find("\n",home)-home+1);if(auto end=old.find("BEACH_END ");end!=std::string::npos)old.erase(end,old.find("\n",end)-end+1);auto migrated=Afternoon::decode(old);check(migrated.coins()==180&&migrated.questCount()==progression::questCount&&!migrated.rewarded(10),"Version-six migration preserves progress");
 for(std::size_t q=10;q<12;++q){select(owner,q);auto d=quest(progression::quests[q].id);
   for(auto s:d.solution){go(owner,s);auto bytes=owner.encode();check(Afternoon::decode(bytes).encode()==bytes,"Every new command roundtrips");}
   check(owner.rewarded(q)&&owner.lastRewardGranted(),"Completion grants reward");const int coins=owner.coins();
   go(owner,{Verb::Undo});owner=Afternoon::decode(owner.encode());go(owner,d.solution.back());check(owner.coins()==coins&&!owner.lastRewardGranted(),"Undo/reload/recompletion cannot pay again");
   go(owner,{Verb::Restart});owner=Afternoon::decode(owner.encode());for(auto s:d.solution)go(owner,s);check(owner.coins()==coins,"Practice cannot pay again");
 }
 check(owner.coins()==220,"Two new quests award 40 coins total");
 select(owner,9);for(auto s:quest("garden_picnic").solution)go(owner,s);
 check(owner.coins()==240&&Afternoon::decode(owner.encode()).friendship("pip")==6,"Full twelve-quest wallet and friendship restore");
 std::cout<<checks<<" trail and balance checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
