#include "release1/progression/owner.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
using namespace paws;
std::string legacySection(const std::string& bytes) {
    std::istringstream input(bytes);std::string magic,tag,legacy;int version=0;
    input>>magic>>version;
    if(version<6)return bytes;
    input>>tag>>std::quoted(legacy);
    if(tag!="LEGACY"||!input)throw std::runtime_error("Missing legacy section");
    return legacy;
}

void check(bool value,const char* text){if(!value)throw std::runtime_error(text);}
CommandResult garden(Afternoon& a,Afternoon::GardenAction action,std::string item={},std::string spot={}) {
    const auto s=a.session().state();return a.dispatchGarden({s.attempt,s.revision,action,std::move(item),std::move(spot)});
}
void complete(Afternoon& a,std::size_t q) {
    check(a.selectQuest(q),"quest unlock");
    const auto s=a.session().state();if(s.inGarden)check(a.dispatch({s.attempt,s.revision,Action::Resume,{}}).accepted,"resume");
    for(const auto& step:a.session().content().solution) {
        const auto st=a.session().state();check(a.dispatch({st.attempt,st.revision,step.action,step.object,step.direction,step.target}).accepted,"solution");
    }
    const auto st=a.session().state();check(a.dispatch({st.attempt,st.revision,Action::Garden,{}}).accepted,"garden");
}
void rejectedWithoutMutation(Afternoon& a,Afternoon::GardenAction action,std::string item={},std::string spot={}) {
    auto before=a.encode();check(!garden(a,action,item,spot).accepted,"expected rejection");check(a.encode()==before,"rejection leaves coherent model unchanged");
}
int main(){try {
    using G=Afternoon::GardenAction;Afternoon a;
    rejectedWithoutMutation(a,G::Buy,"garden_cushion");
    auto s=a.session().state();a.dispatch({s.attempt,s.revision,Action::Garden,{}});
    rejectedWithoutMutation(a,G::Buy,"garden_cushion");
    complete(a,0);check(a.shopOpen()&&a.coins()==20,"first quest unlocks shop");
    rejectedWithoutMutation(a,G::Buy,"garden_cushion");rejectedWithoutMutation(a,G::Buy,"drinking_bowl");
    rejectedWithoutMutation(a,G::Buy,"unlisted_item");rejectedWithoutMutation(a,G::Place,"garden_cushion","sunny_corner");
    complete(a,1);check(garden(a,G::Buy,"garden_cushion").accepted&&a.coins()==10,"purchase spends exactly 30");
    auto old=a.session().state();rejectedWithoutMutation(a,G::Buy,"garden_cushion");
    check(garden(a,G::Place,"garden_cushion","bench_corner").accepted,"place owned cushion");
    auto current=a.encode();check(!a.dispatchGarden({old.attempt,old.revision,G::Buy,"drinking_bowl",{}}).accepted&&a.encode()==current,"stale purchase atomic");
    rejectedWithoutMutation(a,G::Place,"garden_cushion","unknown_spot");
    check(!a.endingEligible(),"two quests insufficient for ending");rejectedWithoutMutation(a,G::RecordEnding);
    complete(a,2);check(a.coins()==30&&a.friendship()==3,"third reward preserves spending");
    check(garden(a,G::Buy,"drinking_bowl").accepted&&a.coins()==5,"both items leave five coins");
    rejectedWithoutMutation(a,G::Place,"drinking_bowl","bench_corner");
    check(garden(a,G::Place,"drinking_bowl","quiet_corner").accepted,"second item independent placement");
    check(garden(a,G::Place,"garden_cushion","sunny_corner").accepted&&a.coins()==5,"free move");
    check(a.placement("garden_cushion")=="sunny_corner"&&a.placement("drinking_bowl")=="quiet_corner","move preserves other item");
    check(a.endingEligible()&&garden(a,G::RecordEnding).accepted&&a.endingSeen(),"ending records once");rejectedWithoutMutation(a,G::RecordEnding);
    auto bytes=a.encode();auto restored=Afternoon::decode(bytes);check(restored.encode()==bytes&&restored.coins()==5&&restored.endingSeen(),"coherent ownership placements ending restore");
    auto expectBad=[](std::string bytes){bool bad=false;try{Afternoon::decode(bytes);}catch(...){bad=true;}check(bad,"corrupt furnishings rejected");};
    bytes=legacySection(bytes); // Keep historical corruption fixtures on their original wire format.
    auto bad=bytes;auto at=bad.find("\"quiet_corner\"");bad.replace(at,14,"\"sunny_corner\"");expectBad(bad);
    bad=bytes;at=bad.find("\"sunny_corner\"");bad.replace(at,14,"\"removed_spot\"");expectBad(bad);
    bad=bytes;at=bad.find("1 1 \"sunny_corner\"");bad[at]='0';expectBad(bad);
    auto legacy=Afternoon::decode("PAWS_AFTERNOON 2 0 0 0 0 70 60 80 1 0 0 0 0 0\n\"missing-ball-v1\" 0 1 0\n\"tidy-toys-v1\" 0 1 0\n\"stuck-gate-v1\" 0 1 0\n");
    check(legacy.coins()==0&&!legacy.owned("garden_cushion")&&!legacy.endingSeen(),"version two migrates without invented items");
    // Geometric compatibility belongs to the same portable contract as runtime.
    check(fits(shopItems[0],gardenSpots[0])&&!fits(shopItems[0],{"small","Small",{},.5f,.5f}),"footprints enforce compatibility");
    std::cout<<"Phase 7 atomic purchases, free moves, occupancy, stale commands, ending and codec passed\n";
return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
