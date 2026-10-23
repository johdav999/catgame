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
std::string read(const std::filesystem::path& file){std::ifstream in(file,std::ios::binary);check(bool(in),"Open fixture");return {std::istreambuf_iterator<char>(in),{}};}
std::string pilots(const Afternoon& a){auto bytes=a.encode();auto at=bytes.find("QUEST_KERNEL ");return bytes.substr(at,bytes.find("END\n",at)-at);}
void reject(const Afternoon& live,std::string bad){auto before=live.encode();auto inspected=p::inspectSave(bad);check(!inspected.value&&!inspected.error.empty()&&inspected.original==bad,"Reject inconsistent candidate preserving exact bytes");check(live.encode()==before,"Rejection preserves live state");}
void command(Afternoon& a,const release1::rules::Step& step){auto c=a.intent(step.verb,step.object,step.target);c.direction=step.direction;c.destination=step.destination;auto n=a.questMigrationView().comparisons;auto r=a.dispatchIntent(c);check(r.accepted,"Pilot command: "+r.message);check(a.questMigrationView().matched&&a.questMigrationView().comparisons==n+1,"One shadow comparison per commit");}
}
int main(int argc,char** argv){try{
    const std::filesystem::path root=std::filesystem::path(PAWS_ROOT)/"tests/fixtures/quest-migration-m0";
    unsigned imports=0;
    for(const auto& file:std::filesystem::directory_iterator(root))if(file.path().extension()==".save") {
        auto original=read(file.path());auto a=Afternoon::decode(original);auto encoded=a.encode();
        check(encoded.starts_with("PAWS_AFTERNOON 14\n"),"Convert old schema to 13");
        check(Afternoon::decode(encoded).encode()==encoded,"Old journal roundtrips new kernel state exactly");
        check(a.questMigrationView().newReceipts.empty()&&!a.lastRewardGranted(),"Import never emits pilot rewards");++imports;
    }
    check(imports==99,"All 99 frozen old-save fixtures imported");
    std::ostringstream trace;
    for(auto id:std::array<std::string_view,2>{"pip_missing_ball","beach_fresh_water"}) {
        Afternoon a=id=="pip_missing_ball"?Afternoon{}:Afternoon::decode(read(root/"beach_fresh_water-partial.save"));
        a.setBeachDevelopmentAccess(true);
        check(a.dispatchIntent(a.intent(Verb::SelectQuest,{},std::string(id))).accepted,"Select pilot");
        command(a,{Verb::Restart});const auto definition=release1::rules::quest(id,a.puzzleView().content);
        const int before=a.coins();const auto stale=a.intent(Verb::Hint);
        bool basin=false;
        for(std::size_t n=0;n<definition.solution.size();++n) {
            command(a,definition.solution[n]);
            if(id=="beach_fresh_water"&&definition.solution[n].verb==Verb::Roll&&!a.puzzleView().completed) {
                basin=true;check(pilots(a).find("\"puzzle\"")!=std::string::npos,"Kernel observes filled basin");
                command(a,{Verb::Undo});check(pilots(a).find("\"basin\"")!=std::string::npos,"Undo regresses basin stage");
                command(a,definition.solution[n]);
            }
            trace<<id<<'\t'<<n<<'\t'<<a.puzzleView().completed<<'\t'<<a.rewarded(a.active())<<'\t'<<a.coins()<<'\n'<<pilots(a);
            auto saved=a.encode();auto restored=Afternoon::decode(saved);check(restored.encode()==saved,"Every pilot step survives exact save/load");
            restored.setBeachDevelopmentAccess(true);check(restored.questMigrationView().newReceipts.empty(),"Reload emits no effects");
            a=std::move(restored);
        }
        check(a.puzzleView().completed&&a.rewarded(a.active()),"Pilot completion committed");
        check(a.coins()==before+(id=="pip_missing_ball"?20:0),"Pack-isolated first reward");
        check(id!="beach_fresh_water"||basin,"Multi-stage water exercised");
        const auto saved=a.encode();auto invalid=stale;invalid.generation=a.generation();
        check(!a.dispatchIntent(invalid).accepted&&a.encode()==saved,"Stale pilot command does not mutate save");
        command(a,{Verb::Undo});check(!a.puzzleView().completed&&a.rewarded(a.active()),"Undo retains durable kernel receipt");
        if(argc==2)std::ofstream(std::string(argv[1])+"."+std::string(id)+".undo.save",std::ios::binary)<<a.encode();
        command(a,{Verb::Restart});const int coins=a.coins();
        for(const auto& step:definition.solution){command(a,step);check(a.questMigrationView().newReceipts.empty(),"Practice cannot issue a second receipt");}
        check(a.coins()==coins,"Practice cannot change wallet");
        check(a.returnToGarden(a.intent(Verb::Select)).accepted&&a.coins()==coins,"Travel retains completion outside scene");
        auto snapshot=a.encode();check(Afternoon::decode(snapshot).encode()==snapshot,"Garden reload preserves pilot state");
        auto kernel= snapshot.find("QUEST_KERNEL ");
        for(const auto& [from,to]:std::vector<std::pair<std::string,std::string>>{{"QUEST_KERNEL 2 29","QUEST_KERNEL 9 29"},{"/first-completion","/other-receipt"},{"\"puzzle\"","\"bogus-stage\""},{"\"missing-ball-v2\"","\"missing-ball-v999\""}}) {
            auto bad=snapshot;auto at=bad.find(from,kernel);if(at!=std::string::npos){bad.replace(at,from.size(),to);reject(a,bad);}
        }
        auto missing=snapshot;missing.erase(kernel,missing.find("END\n",kernel)-kernel);reject(a,missing);
        const auto rowStart=snapshot.find('\n',kernel)+1,rowEnd=snapshot.find('\n',rowStart);
        const auto row=snapshot.substr(rowStart,rowEnd-rowStart);
        std::istringstream input(row);std::string quest,content,stage,receipt;unsigned attempt;bool earned,complete,checkpoint;
        input>>std::quoted(quest)>>std::quoted(content)>>attempt>>earned>>complete>>checkpoint>>std::quoted(stage)>>std::quoted(receipt);
        for(int field=0;field<4;++field){std::ostringstream altered;altered<<std::quoted(quest)<<' '<<std::quoted(content)<<' '<<(field==0?attempt+1:attempt)<<' '<<(field==1?!earned:earned)<<' '<<(field==2?!complete:complete)<<' '<<(field==3?!checkpoint:checkpoint)<<' '<<std::quoted(stage)<<' '<<std::quoted(receipt);auto bad=snapshot;bad.replace(rowStart,row.size(),altered.str());reject(a,bad);}
        auto duplicate=snapshot;const auto secondStart=rowEnd+1,secondEnd=snapshot.find('\n',secondStart);duplicate.replace(secondStart,secondEnd-secondStart,row);reject(a,duplicate);
        auto future=snapshot;future.replace(0,std::string("PAWS_AFTERNOON 14").size(),"PAWS_AFTERNOON 15");reject(a,future);
        reject(a,snapshot+"trailing");
    }
    // No second completion path: the kernel checks prerequisites, but account
    // access remains transient Paws policy and cannot come from imported bytes.
    auto beach=Afternoon::decode(read(root/"beach_fresh_water-completed.save"));
    check(!beach.beachDevelopmentAccess()&&!beach.unlocked(p::questIndex("beach_fresh_water")),"Save cannot grant pack access");
    if(argc==2)std::ofstream(argv[1],std::ios::binary)<<trace.str();
    std::cout<<checks<<" M5 pilot, shadow, migration and rejection checks; "<<imports<<" old saves passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"After "<<checks<<" checks: "<<e.what()<<'\n';return 1;}}
