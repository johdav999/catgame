#include "afternoon.hpp"
#include <iostream>
#include <stdexcept>
using namespace paws;using namespace paws::release1;
namespace {
int checks=0;
void check(bool b,const std::string& why){++checks;if(!b)throw std::runtime_error(why);}
void go(Afternoon& a,const rules::Step& s){auto c=a.intent(s.verb,s.object,s.target);c.direction=s.direction;c.destination=s.destination;auto r=a.dispatchIntent(c);check(r.accepted,s.object+": "+r.message);}
void select(Afternoon& a,std::string_view id){go(a,{Verb::SelectQuest,"",std::string(id)});}
void home(Afternoon& a){check(a.returnToGarden(a.intent(Verb::Select)).accepted,"Return to remembered garden");}
Afternoon reload(const Afternoon& a){auto bytes=a.encode();auto b=Afternoon::decode(bytes);check(bytes==b.encode(),"Exact story save replay");check(!b.beachDevelopmentAccess(),"Save is not ownership");b.setBeachDevelopmentAccess(true);return b;}
void rejected(Afternoon& a,const rules::Step& s){auto bytes=a.encode();auto c=a.intent(s.verb,s.object,s.target);c.direction=s.direction;check(!a.dispatchIntent(c).accepted,"Expected blocked move");check(a.encode()==bytes,"Blocked move preserves journal and ledger");}
}
int main(){try{
 Afternoon a;a.setBeachDevelopmentAccess(true);home(a);
 check(!a.dispatchIntent(a.intent(Verb::RecordBeachEnding)).accepted,"Ending cannot be recorded before the picnic");
 for(std::size_t stage=0;stage<6;++stage){
  const auto id=progression::beachQuestIds[stage];
  if(stage+1<6)check(!a.unlocked(progression::baseQuestCount+stage+1),"Next stage remains locked");
  select(a,id);const auto d=rules::quest(id);check(rules::validate(d).empty(),"Versioned beach layout validates");
  if(stage==2){rejected(a,{Verb::Connect});go(a,{Verb::PickUp,"front_panel"});rejected(a,{Verb::Fit,"front_panel","loose_support"});go(a,{Verb::Restart});}
  if(stage==3){rejected(a,{Verb::FollowTrail,"trail_0"});for(int n=0;n<3;++n)go(a,{Verb::Inspect,"clue_"+std::to_string(n)});rejected(a,{Verb::FollowTrail,"trail_0"});go(a,{Verb::Restart});}
  if(stage==5){rejected(a,{Verb::Connect});go(a,{Verb::Fit,"button_mat","spot_1_0"});rejected(a,{Verb::Fit,"mabel_mat","spot_1_0"});rejected(a,{Verb::Fit,"pip_mat","spot_2_1"});go(a,{Verb::Restart});}
  for(std::size_t n=0;n<d.solution.size();++n){go(a,d.solution[n]);a=reload(a);
   if(n==d.solution.size()/2){const auto before=a.puzzleView().objects;const auto stale=a.intent(Verb::Hint);home(a);a=reload(a);check(a.inGarden()&&a.active()==0,"Travel restores original base quest");select(a,id);
    const auto after=a.puzzleView().objects;check(after.size()==before.size(),"Revisit retains read model");for(std::size_t i=0;i<after.size();++i)check(after[i].cell==before[i].cell&&after[i].socket==before[i].socket&&after[i].active==before[i].active,"Revisit preserves every piece and clue");check(!a.dispatchIntent(stale).accepted,"Travel invalidates delayed intents");
   }
  }
  check(a.puzzleView().completed&&a.rewarded(progression::baseQuestCount+stage),"Stage completion retained");
  check(a.puzzleView().beachProgress==((1u<<(stage+1))-1),"Cove changes reflect only earned beach milestones");
  go(a,{Verb::Undo});check(!a.puzzleView().completed,"Completion undo restored after travel/reload");go(a,d.solution.back());
  go(a,{Verb::Restart});check(!a.puzzleView().completed,"Explicit restart starts fresh");for(auto s:d.solution)go(a,s);
  check(a.coins()==0&&a.friendship("pip")==0&&a.friendship("mabel")==0&&a.chapter()==1&&!a.picnicEligible(),"Beach cannot change base progression");
  if(stage==0){
   auto v10=a.encode();v10.replace(0,v10.find('\n'),"PAWS_AFTERNOON 10");auto at=v10.find("BEACH_END ");v10.erase(at,v10.find('\n',at)-at+1);at=v10.find("QUESTS 24");v10.replace(at,9,"QUESTS 19");at=v10.find("\"beach_button_bridge\"",v10.find("QUESTS 19"));v10.erase(at,v10.find("END\n",at)-at);
   auto old=Afternoon::decode(v10);check(old.inBeach()&&old.puzzleView().completed&&old.rewarded(18)&&!old.rewarded(19)&&!old.beachEndingSeen(),"Version-10 beach completion migrates without resetting or inventing ending");
  }
  home(a);a=reload(a);select(a,id);check(a.puzzleView().completed,"Completed revisit does not reset a quest");
 }
 check(a.beachComplete()&&!a.beachEndingSeen(),"The ending awaits explicit viewing");
 go(a,{Verb::RecordBeachEnding});check(a.beachEndingSeen(),"Beach ending recorded independently");a=reload(a);go(a,{Verb::RecordBeachEnding});check(a.beachEndingSeen()&&a.coins()==0&&!a.picnicSeen(),"Ending replay has no base reward or garden ending effects");
 auto bad=a.encode();auto at=bad.find("BEACH_END 1");check(at!=std::string::npos,"Ending record serialized");
 auto fresh=Afternoon{}.encode();at=fresh.find("BEACH_END 0");fresh.replace(at,11,"BEACH_END 1");bool failed=false;try{Afternoon::decode(fresh);}catch(...){failed=true;}check(failed,"Reject ending record without completed quests");
 std::cout<<checks<<" complete beach story checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
