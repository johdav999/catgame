#include "afternoon.hpp"
#include "release1/rules/content.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace paws;using namespace paws::release1;using namespace paws::release1::rules;
void check(bool b,const std::string& why){if(!b)throw std::runtime_error(why);}
void go(Afternoon& a,Step s){auto c=a.intent(s.verb,s.object,s.target);c.direction=s.direction;c.destination=s.destination;auto r=a.dispatchIntent(c);check(r.accepted,r.message);}
int main(int argc,char** argv){try{
 {
  Attempt pots(quest("pip_friend_room"));
  auto fit=[&](const char* pot,const char* slot){check(pots.dispatch(pots.command({Verb::Fit,pot,slot})).accepted,"Flowerpot placement accepted");};
  fit("weight_1","beam_1");fit("weight_2","beam_2");
  const auto before=pots.logicalState();fit("weight_1","beam_2");
  auto slot=[&](const char* id){for(const auto& o:pots.view().objects)if(o.id==id)return o.socket;return std::string{};};
  check(slot("weight_1")=="beam_2"&&slot("weight_2")=="beam_1","Occupied beam drop swaps both flowerpots");
  check(pots.dispatch(pots.command({Verb::Undo})).accepted&&pots.logicalState()==before,"One undo restores both flowerpots");
  fit("weight_3","beam_1");
  check(slot("weight_3")=="beam_1"&&slot("weight_1")=="home_1","Storage drop returns displaced pot to its own mat");
 }
 auto malformed=quest("pip_friend_room");malformed.objects.pop_back();check(!validate(malformed).empty(),"Missing story cushion is rejected");
 Afternoon a;check(!a.unlocked(12)&&!a.unlocked(13)&&!a.unlocked(14),"Story requires first request");
 for(auto s:quest("pip_missing_ball").solution)go(a,s);
 auto old=a.encode();old.replace(0,old.find("\n"),"PAWS_AFTERNOON 7");auto at=old.find("QUESTS "+std::to_string(progression::quests.size()));old.replace(at,9,"QUESTS 12");at=old.find("\"pip_friend_ball\"",at);old.erase(at,old.find("END\n",at)-at);
 if(auto home=old.find("HOME ");home!=std::string::npos)old.erase(home,old.find("\n",home)-home+1);if(auto end=old.find("BEACH_END ");end!=std::string::npos)old.erase(end,old.find("\n",end)-end+1);auto migrated=Afternoon::decode(old);check(migrated.coins()==20&&migrated.unlocked(12)&&!migrated.rewarded(12),"V7 migration preserves ledger and unlocks story");
 for(std::size_t q=12;q<15;++q){go(a,{Verb::SelectQuest,"",std::string(progression::quests[q].id)});auto d=quest(progression::quests[q].id);
  if(argc==2){std::filesystem::create_directories(argv[1]);std::ofstream(std::filesystem::path(argv[1])/(std::to_string(q)+".save"))<<a.encode();}
  if(q==12){auto c=a.intent(Verb::Rotate,"channel_e");check(!a.dispatchIntent(c).accepted,"Second route stays locked before checkpoint");}
  if(q==14){check(!a.dispatchIntent(a.intent(Verb::Fit,"pip_cushion","pip_seat")).accepted,"Empty lookout must be balanced first");}
  for(std::size_t n=0;n<d.solution.size();++n){auto s=d.solution[n];go(a,s);auto bytes=a.encode();check(Afternoon::decode(bytes).encode()==bytes,"Every story step survives exact save roundtrip");
   if(q==12&&n==7){check(!a.puzzleView().completed&&!a.rewarded(q),"Checkpoint is not quest completion");go(a,{Verb::Undo});check(!a.dispatchIntent(a.intent(Verb::Rotate,"channel_e")).accepted,"Undo restores checkpoint gate");go(a,s);}
   if(q==14&&n==4){check(!a.dispatchIntent(a.intent(Verb::Fit,"pip_cushion","approach")).accepted,"Approach must remain clear");check(!a.dispatchIntent(a.intent(Verb::Fit,"pip_cushion","your_seat")).accepted,"Seats are distinct");}
  }
  check(a.rewarded(q)&&a.puzzleView().completed,"Story completion rewards");const auto coins=a.coins();go(a,{Verb::Undo});a=Afternoon::decode(a.encode());go(a,d.solution.back());check(a.coins()==coins&&!a.lastRewardGranted(),"Undo and reload cannot duplicate reward");
  go(a,{Verb::Restart});for(auto s:d.solution)go(a,s);check(a.coins()==coins,"Practice cannot duplicate reward");
 }
 check(a.coins()==80&&a.friendship("pip")==4,"Story is independent of later main quests");
 check(a.returnToGarden(a.intent(Verb::Select)).accepted,"Garden return");auto restored=Afternoon::decode(a.encode());check(restored.rewarded(14),"Permanent lookout entitlement survives reload");
 if(argc==2)std::ofstream(std::filesystem::path(argv[1])/"lookout.save")<<a.encode();
 std::cout<<"Pip story: prerequisites, checkpoint, undo, seating, V7 migration, all-step saves, once-only rewards passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
