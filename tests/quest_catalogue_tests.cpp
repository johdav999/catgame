#include "release1/progression/codec.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace paws;
using release1::Verb;
namespace p=release1::progression;
namespace {
unsigned checks=0;
void check(bool ok,const std::string& why){++checks;if(!ok)throw std::runtime_error(why);}
std::string read(const std::filesystem::path& path){std::ifstream in(path,std::ios::binary);check(bool(in),"Open fixture");return {std::istreambuf_iterator<char>(in),{}};}
void reject(Afternoon& a,release1::Intent command){const auto before=a.encode();check(!a.dispatchIntent(command).accepted,"Reject invalid intent");check(a.encode()==before,"Rejected intent leaves all kernel/game state unchanged");}
void step(Afternoon& a,const release1::rules::Step& s){const auto wasEarned=a.rewarded(a.active());auto c=a.intent(s.verb,s.object,s.target);c.direction=s.direction;c.destination=s.destination;const auto r=a.dispatchIntent(c);check(r.accepted,"Authored step: "+r.message);const bool first=a.puzzleView().completed&&!wasEarned;const auto& k=a.questMigrationView();check(k.authoritative==p::questCount&&k.matched,"Full kernel authority");check(k.newReceipts.size()==(first?1u:0u),"Exactly one first-completion receipt");if(first)check(k.newReceipts.front()==c.quest+"/first-completion","Stable receipt ID");check(a.lastRewardGranted()==(first&&p::quests[a.active()].pack.empty()),"Reward presentation policy retained");}
void rejectSave(const Afternoon& a,const std::string& bad){auto before=a.encode();auto candidate=p::inspectSave(bad);check(!candidate.value&&candidate.original==bad&&!candidate.error.empty(),"Inconsistent kernel save rejects atomically");check(a.encode()==before,"Import validation preserves live state");}
void recordEndings(Afternoon& a){
    const auto s=a.session().state();check(a.dispatchGarden({s.attempt,s.revision,Afternoon::GardenAction::RecordEnding,{},{}}).accepted,"Historical MVP ending retained");
    check(a.dispatchIntent(a.intent(Verb::RecordPicnic)).accepted&&a.picnicSeen(),"Garden picnic retained");
    check(a.dispatchIntent(a.intent(Verb::SelectQuest,{},"beach_picnic")).accepted,"Visit beach picnic");check(a.dispatchIntent(a.intent(Verb::RecordBeachEnding)).accepted&&a.beachEndingSeen(),"Independent beach ending retained");
    check(a.returnToGarden(a.intent(Verb::Select)).accepted,"Return after endings");
}
}
int main(int argc,char** argv){try{
    const auto fixtures=std::filesystem::path(PAWS_ROOT)/"tests/fixtures";
    unsigned imports=0;
    for(auto corpus:{"quest-migration-m0","quest-migration-m5"})for(const auto& file:std::filesystem::directory_iterator(fixtures/corpus))if(file.path().extension()==".save"){
        auto a=Afternoon::decode(read(file.path()));const auto bytes=a.encode();check(bytes.starts_with("PAWS_AFTERNOON 14\n"),"Old schema upgrades to 14");check(Afternoon::decode(bytes).encode()==bytes,"Historical content and all kernel records roundtrip");check(a.questMigrationView().newReceipts.empty()&&!a.lastRewardGranted(),"Old saves emit no completion effects");++imports;
    }
    Afternoon a;a.setBeachDevelopmentAccess(true);std::ostringstream trace;
    std::filesystem::path output;if(argc==2){output=argv[1];std::filesystem::create_directories(output);}
    unsigned commands=0;
    for(std::size_t q=0;q<p::quests.size();++q){const auto& policy=p::quests[q];
        auto old=a.intent(Verb::Hint);check(a.dispatchIntent(a.intent(Verb::SelectQuest,{},std::string(policy.id))).accepted,"Select current quest");reject(a,old);
        check(a.setUserPaused(true,a.intent(Verb::Select)).accepted,"Pause");reject(a,a.intent(Verb::Restart));check(a.setUserPaused(false,a.intent(Verb::Select)).accepted,"Resume");
        if(!policy.pack.empty()){a.setBeachDevelopmentAccess(false);reject(a,a.intent(Verb::Hint));a.setBeachDevelopmentAccess(true);}
        auto invalid=a.intent(Verb::Hint);invalid.object=std::string(129,'x');reject(a,invalid);
        const auto definition=release1::rules::quest(policy.id,a.puzzleView().content);const int coins=a.coins();
        for(std::size_t n=0;n<definition.solution.size();++n){step(a,definition.solution[n]);++commands;
            const auto bytes=a.encode();auto loaded=Afternoon::decode(bytes);check(loaded.encode()==bytes,"Every authored step retains exact full-catalogue save");loaded.setBeachDevelopmentAccess(true);a=std::move(loaded);
            trace<<policy.id<<'\t'<<n<<'\t'<<a.puzzleView().completed<<'\t'<<a.rewarded(q)<<'\t'<<a.coins()<<'\t'<<a.chapter();for(auto cat:p::catIds)trace<<'\t'<<a.friendship(cat);trace<<'\n';
        }
        check(a.puzzleView().completed&&a.rewarded(q),"Current solution completes through kernel");check(a.coins()==coins+(policy.pack.empty()?20:0),"Base/beach wallet isolation");
        step(a,{Verb::Undo});check(!a.puzzleView().completed&&a.rewarded(q),"Completion undo retains durable receipt");
        step(a,{Verb::Restart});for(const auto& s:definition.solution)step(a,s);check(a.coins()==coins+(policy.pack.empty()?20:0),"Practice grants no extra coins");
        check(a.returnToGarden(a.intent(Verb::Select)).accepted,"Travel to garden");check(Afternoon::decode(a.encode()).encode()==a.encode(),"Travel save roundtrip");
        if(q==p::questIndex("garden_picnic")){
            auto finale=a;for(auto cat:{"mabel","button","pepper"})check(finale.introduce(cat,finale.intent(Verb::Select)).accepted,"Picnic chapter introductions");
            finale.setItemAvailable("garden_cushion",true);
            check(finale.dispatchIntent(finale.intent(Verb::Buy,"garden_cushion")).accepted,"Picnic furnishing purchase");
            check(finale.dispatchIntent(finale.intent(Verb::PlaceFurnishing,"garden_cushion","quiet_corner")).accepted,"Picnic furnishing placement");
            check(finale.dispatchIntent(finale.intent(Verb::RecordPicnic)).accepted,"Original ten-quest picnic recorded");
            const auto bytes=finale.encode();check(Afternoon::decode(bytes).encode()==bytes,"Original picnic save roundtrip");
            if(!output.empty())std::ofstream(output/"garden-ending.save",std::ios::binary)<<bytes;
        }
        if(!output.empty()){std::ofstream(output/(std::string(policy.id)+".save"),std::ios::binary)<<a.encode();}
    }
    check(a.coins()==int(p::baseQuestCount)*20&&a.beachComplete(),"All quests preserve final wallet and beach completion");
    for(auto cat:{"mabel","button","pepper"})check(a.introduce(cat,a.intent(Verb::Select)).accepted,"Retain completed chapter introductions");
    check(a.dispatchIntent(a.intent(Verb::SelectQuest,{},"pip_missing_ball")).accepted,"Select legacy home");check(a.returnToGarden(a.intent(Verb::Select)).accepted,"Return to legacy home");
    for(const auto& item:p::items)a.setItemAvailable(item.id,true);
    for(std::size_t n=0;n<p::items.size();++n){check(a.dispatchIntent(a.intent(Verb::Buy,std::string(p::items[n].id))).accepted,"Purchase remains game policy");const auto spot=n==0?"sunny_corner":n==1?"quiet_corner":p::spots[n+1].id;check(a.dispatchIntent(a.intent(Verb::PlaceFurnishing,std::string(p::items[n].id),std::string(spot))).accepted,"Furnishing placement retained");
        // A separate save exercises the beach ending independently of the full shop.
        if(n==0){auto ending=a;recordEndings(ending);const auto bytes=ending.encode();check(Afternoon::decode(bytes).encode()==bytes,"Ending fixture roundtrip");if(!output.empty())std::ofstream(output/"ending.save",std::ios::binary)<<bytes;}
    }
    recordEndings(a);const auto complete=a.encode();check(Afternoon::decode(complete).encode()==complete,"Purchases, placements and all endings roundtrip");
    const auto kernel=complete.find("QUEST_KERNEL ");
    {const auto start=complete.find('\n',kernel)+1,end=complete.find("END\n",start);std::istringstream input(complete.substr(start,end-start));std::vector<std::string> rows;std::string row;while(std::getline(input,row))rows.push_back(row);std::reverse(rows.begin(),rows.end());std::string reordered;for(const auto& value:rows)reordered+=value+'\n';auto bytes=complete;bytes.replace(start,end-start,reordered);check(Afternoon::decode(bytes).encode()==complete,"Full kernel section is ID-keyed, independent of file order");}
    for(const auto& policy:p::quests){const auto id=std::string(policy.id);const auto receipt=id+"/first-completion";const auto at=complete.find(receipt,kernel);check(at!=std::string::npos,"Every quest has one persisted receipt");auto bad=complete;bad.replace(at,receipt.size(),id+"/wrong");rejectSave(a,bad);}
    auto missing=complete;missing.erase(kernel,missing.find("END\n",kernel)-kernel);rejectSave(a,missing);
    if(!output.empty()){std::ofstream(output/"catalogue.trace",std::ios::binary)<<trace.str();std::ofstream(output/"complete.save",std::ios::binary)<<complete;}
    std::cout<<checks<<" M6 checks, "<<p::quests.size()<<" current quests, "<<commands<<" authored commands, "<<imports<<" historical imports passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"After "<<checks<<" checks: "<<e.what()<<'\n';return 1;}}
