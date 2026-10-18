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
 Afternoon a;a.settings.quality=1;check(!a.unlocked(15),"Sun story starts locked");
 for(auto q:{0,1,2,12,13,14}){go(a,{Verb::SelectQuest,"",std::string(progression::quests[q].id)});for(auto s:quest(progression::quests[q].id).solution)go(a,s);}
 check(a.introduce("mabel",a.intent(Verb::Select)).accepted,"Introduce Mabel");
 auto old=a.encode();old.replace(0,old.find("\n"),"PAWS_AFTERNOON 8");auto at=old.find("QUESTS "+std::to_string(progression::quests.size()));old.replace(at,9,"QUESTS 15");at=old.find("\"mabel_friend_sun\"",at);old.erase(at,old.find("END\n",at)-at);
 if(auto home=old.find("HOME ");home!=std::string::npos)old.erase(home,old.find("\n",home)-home+1);if(auto end=old.find("BEACH_END ");end!=std::string::npos)old.erase(end,old.find("\n",end)-end+1);auto migrated=Afternoon::decode(old);check(migrated.coins()==120&&migrated.unlocked(15)&&!migrated.rewarded(15),"V8 migration preserves ledger");
 for(std::size_t q=15;q<18;++q){go(a,{Verb::SelectQuest,"",std::string(progression::quests[q].id)});auto d=quest(progression::quests[q].id);
  if(argc==2){std::filesystem::create_directories(argv[1]);std::ofstream(std::filesystem::path(argv[1])/(std::to_string(q)+".save"))<<a.encode();}
  if(q==17)check(!a.dispatchIntent(a.intent(Verb::Fit,"bed","spot_2_0")).accepted,"Shared path stays clear");
  for(std::size_t n=0;n<d.solution.size();++n){auto s=d.solution[n];go(a,s);auto bytes=a.encode();check(Afternoon::decode(bytes).encode()==bytes,"Exact step roundtrip");
   if(q==16&&n==7){check(!a.puzzleView().completed,"Basin is halfway");check(!a.dispatchIntent(a.intent(Verb::Roll)).accepted,"Reject wet cushion branch");go(a,{Verb::Undo});check(!a.dispatchIntent(a.intent(Verb::Rotate,"channel_e")).accepted,"Undo restores basin gate");go(a,s);}
  }
  check(a.rewarded(q)&&a.puzzleView().completed,"Story rewards");auto coins=a.coins();go(a,{Verb::Undo});a=Afternoon::decode(a.encode());go(a,d.solution.back());check(a.coins()==coins,"No duplicate reward after undo reload");
 }
 check(a.returnToGarden(a.intent(Verb::Select)).accepted,"Garden return");check(Afternoon::decode(a.encode()).rewarded(17),"Nook survives reload");
 if(argc==2)std::ofstream(std::filesystem::path(argv[1])/"nook.save")<<a.encode();
 std::cout<<"Mabel story, dry cushion, clear approach, V8 migration, step saves and rewards passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
