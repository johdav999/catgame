#include "afternoon.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace paws;
using namespace paws::release1;
namespace {
int checks=0;
void check(bool ok,const std::string& message){++checks;if(!ok)throw std::runtime_error(message);}
void go(Afternoon& a,const rules::Step& step){auto c=a.intent(step.verb,step.object,step.target);c.direction=step.direction;check(a.dispatchIntent(c).accepted,"Rejected beach step: "+step.object);}
void travel(Afternoon& a){go(a,{Verb::SelectQuest,"",std::string(progression::beachQuestId)});}
void home(Afternoon& a){check(a.returnToGarden(a.intent(Verb::Select)).accepted,"Return to garden");}
Afternoon reload(const Afternoon& a){auto restored=Afternoon::decode(a.encode());check(restored.encode()==a.encode(),"Exact save replay");check(!restored.beachDevelopmentAccess(),"Save cannot grant access");restored.setBeachDevelopmentAccess(true);return restored;}
}
int main(){try{
    const auto definition=rules::quest(progression::beachQuestId);
    check(rules::validate(definition).empty(),"Authored beach layout and solution validate");
    Afternoon a;const auto q=progression::questIndex(progression::beachQuestId);
    check(!a.unlocked(q)&&!a.selectQuest(q),"Development preview defaults locked");
    a.setBeachDevelopmentAccess(true);
    go(a,{Verb::Slide,"pot_a","",Direction::North});
    const auto base=a.puzzleView().objects;
    home(a);const int coins=a.coins(),friendship=a.friendship("pip"),chapter=a.chapter();
    travel(a);check(a.inBeach()&&!a.inGarden(),"Travel selects independent adventure");
    const auto stale=a.intent(Verb::Rotate,"channel_a");
    const auto unchanged=a.encode();
    auto blocked=a.intent(Verb::Slide,"basket_a");blocked.direction=Direction::North;
    check(!a.dispatchIntent(blocked).accepted&&a.encode()==unchanged,"Shared parking blocks front basket without mutation");
    for(std::size_t n=0;n<3;++n)go(a,definition.solution[n]);
    const auto halfway=a.puzzleView();
    a=reload(a);check(a.inBeach()&&a.puzzleView().objects[0].cell==halfway.objects[0].cell,"Reload retains active beach and baskets");
    home(a);check(a.active()==0&&a.inGarden(),"Travel home remembers base request");
    check(a.puzzleView().objects[0].cell==base[0].cell,"Base attempt preserved");
    a=reload(a);travel(a);
    check(a.puzzleView().objects[0].cell==halfway.objects[0].cell,"Revisit resumes beach attempt");
    check(!a.dispatchIntent(stale).accepted,"Travel invalidates stale commands");
    go(a,{Verb::Undo});check(a.puzzleView().objects[0].cell==Cell{1,2},"Undo restored after reload and travel");
    go(a,definition.solution[2]);
    for(std::size_t n=3;n<definition.solution.size();++n){go(a,definition.solution[n]);a=reload(a);}
    check(a.puzzleView().completed&&a.rewarded(q),"Completion is persisted");
    check(a.coins()==coins&&a.friendship("pip")==friendship&&a.chapter()==chapter&&!a.picnicEligible(),"Pack cannot alter base rewards or finale");
    check(!a.lastRewardGranted(),"No misleading base reward receipt");
    auto locked=Afternoon::decode(a.encode());check(!locked.dispatchIntent(locked.intent(Verb::Restart)).accepted,"Loaded progress cannot bypass access");home(locked);
    go(a,{Verb::Undo});go(a,definition.solution.back());check(a.coins()==coins,"Recompletion cannot mint coins");
    go(a,{Verb::Restart});check(!a.puzzleView().completed,"Explicit restart resets board");
    for(const auto& step:definition.solution)go(a,step);
    home(a);a=reload(a);travel(a);check(a.puzzleView().completed,"Completed visit preserves board");
    auto bad=a.encode();auto at=bad.rfind("beach-pip-ball-v2");bad.replace(at,17,"beach-pip-ball-v9");
    bool rejected=false;try{Afternoon::decode(bad);}catch(...){rejected=true;}check(rejected,"Unknown content rejected");
    // A v9 file has no HOME row and no beach record. It opens with a fresh,
    // locked adventure while retaining all base progress.
    Afternoon original;auto old=original.encode();old.replace(0,old.find('\n'),"PAWS_AFTERNOON 9");
    at=old.find("HOME ");old.erase(at,old.find('\n',at)-at+1);
    at=old.find("QUESTS "+std::to_string(progression::questCount));old.replace(at,9,"QUESTS 18");
    at=old.find("\"beach_pip_ball\"",old.find("QUESTS 18"));old.erase(at,old.find("END\n",at)-at);
    at=old.find("BEACH_END ");old.erase(at,old.find('\n',at)-at+1);
    auto migrated=Afternoon::decode(old);check(!migrated.unlocked(q)&&!migrated.rewarded(q)&&migrated.coins()==0,"v9 migration preserves base and locks fresh pack");
    auto copied=a;check(copied.beachDevelopmentAccess()&&copied.inBeach(),"Detached candidates retain transient host access");
    std::cout<<checks<<" beach adventure checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
