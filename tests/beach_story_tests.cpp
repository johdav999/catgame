#include "afternoon.hpp"
#include "historical_save.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <bit>
#include <algorithm>
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
int main(int argc,char** argv){try{
 Afternoon a;a.setBeachDevelopmentAccess(true);home(a);
 check(!a.dispatchIntent(a.intent(Verb::RecordBeachEnding)).accepted,"Ending cannot be recorded before the picnic");
 for(std::size_t stage=0;stage<6;++stage){
  const auto id=progression::beachQuestIds[stage];
  if(stage+1<6)check(!a.unlocked(progression::baseQuestCount+stage+1),"Next stage remains locked");
  select(a,id);const auto d=rules::quest(id);check(rules::validate(d).empty(),"Versioned beach layout validates");
  if(stage==1){rejected(a,{Verb::Slide,"landing_basket","",Direction::South});go(a,{Verb::PickUp,"long_plank"});go(a,{Verb::Rotate,"long_plank"});rejected(a,{Verb::Fit,"long_plank","long_socket"});go(a,{Verb::Restart});}
  if(stage==2){rejected(a,{Verb::Connect});go(a,{Verb::PickUp,"front_panel"});rejected(a,{Verb::Fit,"front_panel","loose_support"});go(a,{Verb::Restart});}
  if(stage==2){for(std::size_t n=0;n<d.solution.size()-2;++n)go(a,d.solution[n]);go(a,{Verb::Fit,"corner_panel","outside_support"});rejected(a,{Verb::Connect});go(a,{Verb::Restart});}
  if(stage==3){rejected(a,{Verb::FollowTrail,"trail_0"});for(int n=0;n<3;++n)go(a,{Verb::Inspect,"clue_"+std::to_string(n)});rejected(a,{Verb::FollowTrail,"trail_0"});go(a,{Verb::Restart});}
  if(stage==5){rejected(a,{Verb::Connect});rejected(a,{Verb::Fit,"button_mat","spot_1_0"});for(std::size_t n=0;n<8;++n)go(a,d.solution[n]);go(a,{Verb::Fit,"button_mat","spot_1_0"});rejected(a,{Verb::Fit,"mabel_mat","spot_1_0"});rejected(a,{Verb::Fit,"pip_mat","spot_2_1"});go(a,{Verb::Fit,"pepper_mat","spot_3_1"});rejected(a,{Verb::Slide,"picnic_basket","",Direction::West});go(a,{Verb::Restart});}
  for(std::size_t n=0;n<d.solution.size();++n){if(d.solution[n].verb==Verb::FollowTrail)rejected(a,{Verb::FollowTrail,"trail_"+std::to_string((d.trail[n/4].answer+1)%3)});go(a,d.solution[n]);a=reload(a);
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
   auto v10=a.encode();v10.replace(0,v10.find('\n'),"PAWS_AFTERNOON 10");auto at=v10.find("BEACH_END ");v10.erase(at,v10.find('\n',at)-at+1);at=v10.find("QUESTS 29");v10.replace(at,9,"QUESTS 19");at=v10.find("\"beach_button_bridge\"",v10.find("QUESTS 19"));v10.erase(at,v10.find("END\n",at)-at);
   auto old=Afternoon::decode(v10);check(old.inBeach()&&old.puzzleView().completed&&old.rewarded(18)&&!old.rewarded(19)&&!old.beachEndingSeen(),"Version-10 beach completion migrates without resetting or inventing ending");
  }
  home(a);a=reload(a);select(a,id);check(a.puzzleView().completed,"Completed revisit does not reset a quest");
 }
 check(a.beachPicnicComplete()&&!a.beachComplete()&&!a.beachEndingSeen(),"The ending awaits explicit viewing");
 go(a,{Verb::RecordBeachEnding});check(a.beachEndingSeen(),"Beach ending recorded independently");a=reload(a);go(a,{Verb::RecordBeachEnding});check(a.beachEndingSeen()&&a.coins()==0&&!a.picnicSeen(),"Ending replay has no base reward or garden ending effects");
 auto bad=a.encode();auto at=bad.find("BEACH_END 1");check(at!=std::string::npos,"Ending record serialized");
 auto fresh=Afternoon{}.encode();at=fresh.find("BEACH_END 0");fresh.replace(at,11,"BEACH_END 1");bool failed=false;try{Afternoon::decode(fresh);}catch(...){failed=true;}check(failed,"Reject ending record without completed quests");
 // A completed v11 picnic keeps its postcard; the five outings initialize fresh.
 auto v11=a.encode();v11.replace(0,v11.find('\n'),"PAWS_AFTERNOON 11");
 at=v11.find("QUESTS 29");v11.replace(at,9,"QUESTS 24");
 at=v11.find("\"beach_tidepool_crossing\"",v11.find("QUESTS 24"));v11.erase(at,v11.find("END\n",at)-at);
 auto migrated=Afternoon::decode(v11);migrated.setBeachDevelopmentAccess(true);
 check(migrated.beachEndingSeen()&&migrated.beachPicnicComplete()&&!migrated.beachComplete(),"v11 postcard survives the extension");
 check(migrated.unlocked(24)&&!migrated.unlocked(25),"Only the first new outing unlocks after picnic migration");
 a=std::move(migrated);
 if(argc==2){auto fixture=a;fixture.saveRevision=10000;fixture.saveSession="beach-outings-fixture";std::ofstream(argv[1])<<fixture.encode();}
 const std::size_t expectedSteps[]={23,15,21,8,20};
 for(std::size_t stage=6;stage<progression::beachQuestIds.size();++stage){
  const auto id=progression::beachQuestIds[stage];select(a,id);const auto d=rules::quest(id);
  const auto error=rules::validate(d);check(error.empty(),std::string(id)+": "+error);
  check(d.solution.size()==expectedSteps[stage-6],"New outings retain authored multi-move solutions");
  if(stage==6){rejected(a,{Verb::Slide,"parking","",Direction::East});rejected(a,{Verb::Slide,"landing","",Direction::South});rejected(a,{Verb::Connect});}
  if(stage==8){
   check(d.objects.size()==8&&d.shade.size()==16&&d.sockets.size()==14,"Workshop has eight panels and competing support pairs");
   std::vector<unsigned> coverage;
   for(const auto& socket:d.sockets){unsigned mask=0;for(int k=0;k<2;++k){Cell cell{socket.cell.x+(socket.orientation?0:k),socket.cell.y+(socket.orientation?k:0)};auto found=std::find(d.shade.begin(),d.shade.end(),cell);check(found!=d.shade.end(),"Every offered pair shades real workshop cells");mask|=1u<<std::distance(d.shade.begin(),found);}coverage.push_back(mask);}
   unsigned solutions=0;
   for(unsigned selection=0;selection<(1u<<coverage.size());++selection){if(std::popcount(selection)!=8)continue;unsigned covered=0;for(unsigned k=0;k<coverage.size();++k)if(selection&(1u<<k))covered|=coverage[k];if(covered==0xffff)++solutions;}
   check(solutions==1,"Independent exhaustive cover search proves one complete arrangement");
   rejected(a,{Verb::Connect});
   for(auto step:d.solution){if(step.verb==Verb::Connect)break;if(step.verb==Verb::Fit&&step.object=="panel_2")step.target="alternative_0";go(a,step);}
   rejected(a,{Verb::Connect}); // All eight panels fitted, but a plausible crossing wastes one cell.
   go(a,{Verb::PickUp,"panel_2"});go(a,{Verb::Fit,"panel_2","support_2"});go(a,{Verb::Connect});
   check(a.puzzleView().completed,"A mistaken crossing can be repaired without restarting");go(a,{Verb::Restart});
  }
  if(stage==9)check(d.sequence.size()==8&&a.puzzleView().symbols.size()==8,"Current encore requires exactly eight pictures");
  if(stage==10)rejected(a,{Verb::TestBalance});
  for(std::size_t n=0;n<d.solution.size();++n){go(a,d.solution[n]);a=reload(a);
   if(n==d.solution.size()/2){go(a,{Verb::Undo});go(a,d.solution[n]);a=reload(a);home(a);a=reload(a);select(a,id);}
  }
  check(a.puzzleView().completed&&a.rewarded(progression::baseQuestCount+stage),"Outing complete and saved");
  if(stage==7&&argc==2){auto fixture=a;fixture.saveRevision=10000;fixture.saveSession="workshop-fixture";std::ofstream(std::string(argv[1])+".workshop")<<fixture.encode();}
  if(stage==9&&argc==2){auto fixture=a;fixture.saveRevision=10000;fixture.saveSession="lantern-fixture";std::ofstream(std::string(argv[1])+".lantern")<<fixture.encode();}
  check(a.puzzleView().beachProgress==((1u<<(stage+1))-1),"New beach milestone bit retained");
  check(a.coins()==0&&a.friendship("pip")==0&&a.chapter()==1&&!a.picnicSeen(),"New outings leave garden rewards untouched");
  go(a,{Verb::Restart});for(const auto& step:d.solution)go(a,step);a=reload(a);
 }
 check(a.beachComplete()&&a.beachEndingSeen(),"All eleven quests complete without losing the original ending");
 select(a,"beach_picnic");go(a,{Verb::RecordBeachEnding});a=reload(a);
 // Every saved v1 board retains its exact rules until an explicit restart.
 auto legacyBytes=paws::tests::version12Envelope(Afternoon{}.encode());
 {const auto pos=legacyBytes.find("beach-shell-encore-v2");check(pos!=std::string::npos,"Fresh encore uses v2");legacyBytes.replace(pos,std::string_view("beach-shell-encore-v2").size(),"beach-shell-encore-v1");}
 {const auto pos=legacyBytes.find("beach-windbreak-v2");check(pos!=std::string::npos,"Fresh workshop uses v2");legacyBytes.replace(pos,18,"beach-windbreak-v1");}
 for(auto id:std::span(progression::beachQuestIds).first(progression::originalBeachQuestCount)){const auto current=rules::quest(id);auto oldContent=current.content;oldContent.back()='1';const auto pos=legacyBytes.find(current.content);check(pos!=std::string::npos,"Fresh save has beach content identity");legacyBytes.replace(pos,current.content.size(),oldContent);}
 auto legacy=Afternoon::decode(legacyBytes);legacy.setBeachDevelopmentAccess(true);home(legacy);
 for(auto id:std::span(progression::beachQuestIds).first(progression::originalBeachQuestCount)){
  const auto current=rules::quest(id);auto oldContent=current.content;oldContent.back()='1';const auto previous=rules::quest(id,oldContent);
  check(current.family==previous.family&&current.solution.size()>previous.solution.size(),"Every beach quest keeps its family and gains required steps");
  select(legacy,id);check(legacy.puzzleView().content==oldContent,"Old save retains original layout");
  for(const auto& step:previous.solution){go(legacy,step);legacy=reload(legacy);}
  check(legacy.puzzleView().completed,"Original beach solution still completes after reload");
  go(legacy,{Verb::Restart});legacy=reload(legacy);check(legacy.puzzleView().content==current.content&&!legacy.puzzleView().completed,"Explicit restart upgrades and persists longer layout");
  for(const auto& step:current.solution){go(legacy,step);legacy=reload(legacy);}
  check(legacy.puzzleView().completed&&legacy.coins()==0,"Upgraded solution completes without reward inflation");home(legacy);
 }
 for(auto id:{"beach_tidepool_crossing","beach_fresh_water"}){select(legacy,id);for(const auto& step:rules::quest(id).solution)go(legacy,step);home(legacy);}
 select(legacy,"beach_windbreak");check(legacy.puzzleView().content=="beach-windbreak-v1","Old workshop journal keeps its six-panel layout");
 for(const auto& step:rules::quest("beach_windbreak","beach-windbreak-v1").solution){go(legacy,step);legacy=reload(legacy);}
 check(legacy.puzzleView().completed,"Original workshop still completes");
 go(legacy,{Verb::Restart});legacy=reload(legacy);check(legacy.puzzleView().content=="beach-windbreak-v2"&&!legacy.puzzleView().completed,"Restart upgrades old workshop without losing completion credit");
 check(legacy.rewarded(progression::questIndex("beach_windbreak")),"Workshop completion credit survives upgrade");
 for(const auto& step:rules::quest("beach_windbreak").solution){go(legacy,step);legacy=reload(legacy);}
 check(legacy.puzzleView().completed&&legacy.coins()==0,"Revised workshop completes without reward inflation");
 select(legacy,"beach_shell_encore");check(legacy.puzzleView().symbols.size()==12,"Old encore journal retains twelve pictures");
 for(const auto& step:rules::quest("beach_shell_encore","beach-shell-encore-v1").solution){go(legacy,step);legacy=reload(legacy);}
 check(legacy.puzzleView().completed,"Old twelve-picture encore replays safely");
 if(argc==2){auto fixture=legacy;fixture.saveRevision=10000;fixture.saveSession="encore-fixture";std::ofstream(std::string(argv[1])+".encore-v1")<<fixture.encode();}
 go(legacy,{Verb::Restart});legacy=reload(legacy);
 check(legacy.puzzleView().symbols.size()==8&&legacy.puzzleView().content=="beach-shell-encore-v2","Restart upgrades to eight pictures");
 check(legacy.rewarded(progression::questIndex("beach_shell_encore")),"Encore completion credit survives upgrade");
 for(const auto& step:rules::quest("beach_shell_encore").solution){go(legacy,step);legacy=reload(legacy);}
 check(legacy.puzzleView().completed&&legacy.coins()==0,"Eight-picture encore completes without extra rewards");
 std::cout<<checks<<" complete beach story checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
