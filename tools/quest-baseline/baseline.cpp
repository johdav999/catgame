#include "afternoon.hpp"
#include "release1/rules/content.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
using namespace paws;
namespace p=paws::release1::progression;
using release1::Verb;
namespace {
bool currentBaseline=false;
void require(bool ok,const std::string& message){if(!ok)throw std::runtime_error(message);}
std::string read(const std::filesystem::path& path){std::ifstream f(path,std::ios::binary);require(bool(f),"Cannot read "+path.string());return {std::istreambuf_iterator<char>(f),{}};}
// Convert only the additive M6 section/header for strict comparison with the
// exact M5 writer captured before cutover. All journals and metadata stay intact.
std::string pilotEnvelope(std::string bytes) {
    if(bytes.starts_with("PAWS_AFTERNOON 14\n")) {
        bytes.replace(0,std::string("PAWS_AFTERNOON 14").size(),"PAWS_AFTERNOON 13");
        const auto at=bytes.find("QUEST_KERNEL "),end=bytes.find("END\n",at);require(at!=std::string::npos&&end!=std::string::npos,"Missing kernel section");
        std::istringstream section(bytes.substr(at,end-at));std::string tag,line;unsigned schema,count;section>>tag>>schema>>count;std::getline(section,line);
        std::map<std::string,std::string> rows;while(std::getline(section,line)){std::istringstream row(line);std::string id;row>>std::quoted(id);rows[id]=line;}
        const auto replacement="QUEST_KERNEL 1 2\n"+rows.at("pip_missing_ball")+'\n'+rows.at("beach_fresh_water")+'\n';
        bytes.replace(at,end-at,replacement);
    }return bytes;
}
// Compare the unchanged legacy envelope separately from additive kernel state.
// Frozen files remain exact import inputs; never regenerate them for migration.
std::string legacyEnvelope(std::string bytes){
    bytes=pilotEnvelope(std::move(bytes));
    if(bytes.starts_with("PAWS_AFTERNOON 13\n")){
        bytes.replace(0,std::string("PAWS_AFTERNOON 13").size(),"PAWS_AFTERNOON 12");
        const auto at=bytes.find("QUEST_KERNEL ");require(at!=std::string::npos,"Missing pilot section");
        bytes.erase(at,bytes.find("END\n",at)-at);
    }return bytes;
}
// These two layouts advanced before M5. Keep their historical save inputs and
// trace, allowing only their journal sections and resulting owner revision to
// differ after an explicit restart opts into the current supported layout.
std::string historicalComparison(std::string bytes) {
    bytes=legacyEnvelope(std::move(bytes));if(!bytes.starts_with("PAWS_AFTERNOON 12\n"))return bytes;
    const auto metadata=bytes.find("\nR1 ")+1;const auto endMetadata=bytes.find('\n',metadata);
    std::istringstream fields(bytes.substr(metadata,endMetadata-metadata));std::string tag,schema,active;unsigned revision;bool garden,picnic;int coins;
    fields>>tag>>std::quoted(schema)>>std::quoted(active)>>revision>>garden>>picnic>>coins;
    std::ostringstream normalized;normalized<<"R1 "<<std::quoted(schema)<<' '<<std::quoted(active)<<" <journal-revision> "<<garden<<' '<<picnic<<' '<<coins;
    bytes.replace(metadata,endMetadata-metadata,normalized.str());
    for(auto id:{"beach_windbreak","beach_shell_encore"}) {
        const auto at=bytes.find("\n\""+std::string(id)+"\" ")+1;const auto lineEnd=bytes.find('\n',at);
        require(at>0,"Historical comparison missing quest");std::istringstream line(bytes.substr(at,lineEnd-at));
        std::string quest,content;bool earned;unsigned attempt,length;line>>std::quoted(quest)>>std::quoted(content)>>earned>>attempt>>length;
        auto end=lineEnd+1;for(unsigned n=0;n<length;++n)end=bytes.find('\n',end)+1;
        std::ostringstream row;row<<std::quoted(quest)<<" <supported-layout> "<<earned<<' '<<attempt<<'\n';bytes.replace(at,end-at,row.str());
    }return bytes;
}
void artifact(const std::filesystem::path& root,const std::string& name,const std::string& bytes,bool write){
    if(write){std::ofstream out(root/name,std::ios::binary);out<<bytes;require(bool(out),"Cannot write "+name);}
    else {const auto expected=read(root/name);require(name.ends_with(".save")?(currentBaseline?expected==pilotEnvelope(bytes):historicalComparison(expected)==historicalComparison(bytes)):expected==bytes,"Baseline changed: "+name+" (review explicitly; do not silently regenerate)");}
}
void step(Afternoon& a,const release1::rules::Step& s){auto c=a.intent(s.verb,s.object,s.target);c.direction=s.direction;c.destination=s.destination;const auto r=a.dispatchIntent(c);require(r.accepted,"Solution step rejected: "+r.message);}
void restoreCheck(const Afternoon& a){const auto bytes=a.encode();require(Afternoon::decode(bytes).encode()==bytes,"Roundtrip mismatch");}
// Synthetic old envelopes exercise the released structural schemas; they are
// explicitly not claimed to be saves captured from historical executables.
std::string oldEnvelope(const Afternoon& a,int version){
    const unsigned counts[]={10,12,15,18,19,24,29};
    const unsigned count=counts[version-6];std::istringstream in(legacyEnvelope(a.encode()));
    std::string magic,tag,legacy;int current;in>>magic>>current>>tag>>std::quoted(legacy);
    require(magic=="PAWS_AFTERNOON"&&current==12&&tag=="LEGACY","Update baseline generator for new schema");
    std::ostringstream out;out<<magic<<' '<<version<<"\nLEGACY "<<std::quoted(legacy)<<'\n';
    std::string line;std::getline(in,line);
    while(std::getline(in,line)){
        if(line.starts_with("HOME ")&&version<10)continue;
        if(line.starts_with("BEACH_END ")&&version<11)continue;
        if(line.starts_with("QUESTS ")){
            out<<"QUESTS "<<count<<'\n';
            for(unsigned q=0;q<p::questCount;++q){require(bool(std::getline(in,line)),"Missing quest record");if(q<count)out<<line<<'\n';}
        }else out<<line<<'\n';
    }
    return out.str();
}
}
int main(int argc,char** argv){try{
    require(argc==3&&(std::string(argv[1])=="--write"||std::string(argv[1])=="--verify"||std::string(argv[1])=="--verify-current"),"Usage: baseline --write|--verify|--verify-current directory");
    currentBaseline=std::string(argv[1])=="--verify-current";
    const bool write=std::string(argv[1])=="--write";const std::filesystem::path root=argv[2];
    if(write)std::filesystem::create_directories(root);
    require(p::quests.size()==29,"M0 catalogue changed; review baseline contract");
    require(p::validatePolicies(p::quests).empty(),"Invalid quest catalogue");
    for(int v=1;v<=5;++v){auto bytes=read(std::filesystem::path(PAWS_ROOT)/"tests/release1/progression/fixtures"/("mvp-v"+std::to_string(v)+".save"));auto decoded=Afternoon::decode(bytes);restoreCheck(decoded);artifact(root,"legacy-v"+std::to_string(v)+".save",bytes,write);}
    auto fresh=write?Afternoon{}:Afternoon::decode(read(root/"synthetic-fresh-v12.save"));
    for(int v=6;v<=12;++v){auto bytes=oldEnvelope(fresh,v);auto decoded=Afternoon::decode(bytes);restoreCheck(decoded);artifact(root,"synthetic-fresh-v"+std::to_string(v)+".save",bytes,write);}
    Afternoon a=fresh;a.setBeachDevelopmentAccess(true);
    std::ostringstream catalogue,trace;
    catalogue<<"id\tcontent\tpack\tchapter\trequesters\tprerequisites\tsolution_steps\n";
    trace<<"quest\tcommand\tcompleted\trewarded\tcoins\tchapter\tfriendship_pip\tfriendship_mabel\tfriendship_button\tfriendship_pepper\n";
    auto record=[&](std::string_view id,std::string command){trace<<id<<'\t'<<command<<'\t'<<a.puzzleView().completed<<'\t'<<a.rewarded(a.active())<<'\t'<<a.coins()<<'\t'<<a.chapter();for(auto cat:p::catIds)trace<<'\t'<<a.friendship(cat);trace<<'\n';};
    std::size_t commands=0;
    for(std::size_t q=0;q<p::quests.size();++q){
        const auto& policy=p::quests[q];
        std::string content(policy.content);if(!write&&!currentBaseline&&policy.id=="beach_windbreak")content="beach-windbreak-v1";if(!write&&!currentBaseline&&policy.id=="beach_shell_encore")content="beach-shell-encore-v1";
        const auto d=release1::rules::quest(policy.id,content);
        catalogue<<policy.id<<'\t'<<d.content<<'\t'<<policy.pack<<'\t'<<policy.chapter<<'\t'<<policy.requesters<<'\t';
        for(std::size_t j=0;j<policy.prerequisites.size();++j){if(j)catalogue<<',';catalogue<<policy.prerequisites[j];}catalogue<<'\t'<<d.solution.size()<<'\n';
        require(a.dispatchIntent(a.intent(Verb::SelectQuest,{},std::string(policy.id))).accepted,"Cannot select "+std::string(policy.id));record(policy.id,"selected");
        require(!d.solution.empty(),"Missing solution");
        for(std::size_t n=0;n<d.solution.size();++n){step(a,d.solution[n]);++commands;record(policy.id,std::to_string(n));
            if(n==d.solution.size()/2){restoreCheck(a);artifact(root,std::string(policy.id)+"-partial.save",a.encode(),write);}}
        require(a.puzzleView().completed&&a.rewarded(q),"Solution did not complete/commit "+std::string(policy.id));restoreCheck(a);
        artifact(root,std::string(policy.id)+"-completed.save",a.encode(),write);
        const int coins=a.coins();std::array<int,4> friendship{};for(unsigned i=0;i<4;++i)friendship[i]=a.friendship(p::catIds[i]);
        auto undo=a.dispatchIntent(a.intent(Verb::Undo));require(undo.accepted,"Undo completion rejected");record(policy.id,"undo");restoreCheck(a);
        require(a.rewarded(q),"Undo revoked durable completion");
        artifact(root,std::string(policy.id)+"-undo.save",a.encode(),write);
        require(a.dispatchIntent(a.intent(Verb::Restart)).accepted,"Restart rejected");record(policy.id,"restart");
        for(const auto& s:release1::rules::quest(policy.id,a.puzzleView().content).solution)step(a,s);record(policy.id,"practice-complete");
        require(a.coins()==coins,"Practice duplicated reward");for(unsigned i=0;i<4;++i)require(a.friendship(p::catIds[i])==friendship[i],"Practice duplicated friendship");restoreCheck(a);
    }
    artifact(root,"catalogue.tsv",catalogue.str(),write);artifact(root,"trace.tsv",trace.str(),write);
    std::cout<<"M0: 29 quests, "<<commands<<" solution commands, undo/restart/practice and versions 1-12 PASS\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
