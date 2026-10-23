#include "codec.hpp"
#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace {
using paws::release1::Verb;
constexpr std::array<std::pair<Verb,std::string_view>,17> verbs{{
 {Verb::Select,"select"},{Verb::Slide,"slide"},{Verb::Rotate,"rotate"},{Verb::Roll,"roll"},
 {Verb::PickUp,"pick_up"},{Verb::Fit,"fit"},{Verb::Connect,"connect"},{Verb::Retrieve,"retrieve"},
 {Verb::TapSymbol,"tap_symbol"},{Verb::AssignCat,"assign_cat"},{Verb::Undo,"undo"},{Verb::Restart,"restart"},{Verb::Hint,"hint"},{Verb::ReplaySequence,"replay_sequence"},{Verb::Inspect,"inspect"},{Verb::FollowTrail,"follow_trail"},{Verb::TestBalance,"test_balance"}
}};
std::string_view code(Verb verb){for(auto [v,s]:verbs)if(v==verb)return s;throw std::invalid_argument("Unsaved puzzle verb");}
Verb parseVerb(std::string_view name){for(auto [v,s]:verbs)if(s==name)return v;throw std::invalid_argument("Unknown semantic command");}
void require(bool condition,const char* message){if(!condition)throw std::invalid_argument(message);}
void tag(std::istream& in,std::string_view expected){std::string s;require(bool(in>>s)&&s==expected,"Invalid save section");}
}
namespace paws {
namespace p=release1::progression;
// Version 13 persisted only these two projections. This table is a compatibility
// reader contract, never a live migration/authority selector.
static constexpr std::array<std::string_view,2> version13Quests{"pip_missing_ball","beach_fresh_water"};
std::string Afternoon::encode() const {
    std::ostringstream out;
    // Reuse the exact supported legacy section and validator. It still owns the
    // three original sessions, journals, settings, recovery data and old ending.
    out<<"PAWS_AFTERNOON 14\nLEGACY "<<std::quoted(encodeLegacy())<<'\n';
    out<<"R1 "<<std::quoted(std::string(release1::contractId))<<' '<<std::quoted(std::string(p::quests[activeQuest_].id))
       <<' '<<ownerRevision_<<' '<<garden_<<' '<<picnicSeen_<<' '<<coins()<<'\n';
    out<<"HOME "<<std::quoted(std::string(p::quests[homeQuest_].id))<<'\n';
    out<<"BEACH_END "<<beachEndingSeen_<<'\n';
    out<<"CATS 4\n";
    for(std::size_t i=0;i<4;++i)out<<std::quoted(std::string(p::catIds[i]))<<' '<<friendship(p::catIds[i])<<' '<<introduced_[i]<<'\n';
    out<<"ITEMS 8\n";
    for(std::size_t i=0;i<8;++i)out<<std::quoted(std::string(p::items[i].id))<<' '<<owned_[i]<<' '<<std::quoted(placements_[i])<<'\n';
    out<<"QUESTS "<<p::quests.size()<<'\n';
    for(std::size_t q=0;q<p::quests.size();++q){
        out<<std::quoted(std::string(p::quests[q].id))<<' '<<std::quoted(q<3?sessions_[q].content().contentId:extra_[q-3]->journalContent())<<' '<<rewarded_[q]<<' '
           <<(q<3?sessions_[q].state().attempt:extraIds_[q-3])<<' '<<(q<3?0:extraRecords_[q-3].size())<<'\n';
        if(q>=3)for(const auto& s:extraRecords_[q-3])out<<std::quoted(std::string(code(s.verb)))<<' '<<std::quoted(s.object)<<' '<<std::quoted(s.target)<<' '<<int(s.direction)<<' '<<s.destination.x<<' '<<s.destination.y<<'\n';
    }
    out<<questAdapter_->saveSection(questFacts());
    out<<"END\n";auto bytes=out.str();if(bytes.size()>16*1024*1024)throw std::invalid_argument("Save is too large.");return bytes;
}
Afternoon Afternoon::decode(std::string_view bytes) {
    require(bytes.size()<=16*1024*1024,"Save is too large.");
    std::istringstream in{std::string(bytes)};std::string magic;int version=0;
    require(bool(in>>magic>>version)&&magic=="PAWS_AFTERNOON","Unsupported save header");
    if(version>=1&&version<=5)return decodeLegacy(bytes);
    require(version>=6&&version<=14,"Unsupported future save version");
    tag(in,"LEGACY");std::string legacy;require(bool(in>>std::quoted(legacy)),"Missing legacy section");
    // Prevent recursive v6 nesting: only the old, independently validated codec.
    Afternoon candidate=decodeLegacy(legacy);
    candidate.replayingSave_=true;
    // Replay validates content history independently of current access. This is
    // discarded before returning: importing a save can never grant ownership.
    candidate.beachDevelopmentAccess_=true;
    const unsigned savedQuestCount=version==6?10u:version==7?12u:version==8?15u:version==9?18u:version==10?19u:version==11?24u:unsigned(p::questCount);
    tag(in,"R1");std::string schema,active;Revision revision=0;bool garden=false,picnic=false;int balance=0;
    require(bool(in>>std::quoted(schema)>>std::quoted(active)>>revision>>garden>>picnic>>balance)&&schema==release1::contractId&&revision<std::numeric_limits<Revision>::max()-16&&balance>=0&&balance<=360,"Invalid release-one metadata");
    const auto activeIndex=p::questIndex(active);require(activeIndex<savedQuestCount,"Unknown selected quest");
    require(p::quests[activeIndex].pack.empty()||!garden,"Adventure cannot be active in the home garden");
    if(version>=10){tag(in,"HOME");std::string home;require(bool(in>>std::quoted(home)),"Missing home request");candidate.homeQuest_=p::questIndex(home);require(candidate.homeQuest_<p::baseQuestCount,"Invalid home request");}
    else candidate.homeQuest_=activeIndex;
    if(version>=11){tag(in,"BEACH_END");require(bool(in>>candidate.beachEndingSeen_),"Missing beach ending record");}
    tag(in,"CATS");unsigned count=0;require(bool(in>>count)&&count==4,"Invalid cat count");
    std::array<bool,4> catSeen{},introductions{};std::array<int,4> friendship{};
    for(unsigned n=0;n<4;++n){std::string id;int f=0;bool introduced=false;require(bool(in>>std::quoted(id)>>f>>introduced),"Invalid cat record");auto at=std::find(p::catIds.begin(),p::catIds.end(),id);require(at!=p::catIds.end(),"Unknown cat");auto i=std::size_t(at-p::catIds.begin());require(!catSeen[i]&&f>=0&&f<=10,"Duplicate/invalid cat");catSeen[i]=true;friendship[i]=f;introductions[i]=introduced;}
    tag(in,"ITEMS");require(bool(in>>count)&&count==8,"Invalid item count");
    std::array<bool,8> itemSeen{},owned{};std::array<std::string,8> placements;
    for(unsigned n=0;n<8;++n){std::string id,spot;bool own=false;require(bool(in>>std::quoted(id)>>own>>std::quoted(spot))&&spot.size()<=128,"Invalid item record");auto i=p::itemIndex(id);require(i<8&&!itemSeen[i],"Unknown/duplicate item");itemSeen[i]=true;owned[i]=own;placements[i]=std::move(spot);}
    // Old purchases and positions cannot be contradicted by the extension.
    for(unsigned i=0;i<2;++i)require(owned[i]==candidate.owned_[i]&&placements[i]==candidate.placements_[i],"Legacy furnishing mismatch");
    tag(in,"QUESTS");require(bool(in>>count)&&count==savedQuestCount,"Invalid quest count");
    std::array<bool,p::questCount> seen{},ledger{};std::array<AttemptId,p::questCount> attempts{};
    std::array<std::vector<release1::rules::Step>,p::questCount-3> journals;
    for(unsigned n=0;n<count;++n){
        std::string id,content;bool reward=false;AttemptId attempt=0;unsigned length=0;
        require(bool(in>>std::quoted(id)>>std::quoted(content)>>reward>>attempt>>length),"Invalid quest record");
        const auto q=p::questIndex(id);require(q<count&&!seen[q]&&length<=100000&&attempt>0&&attempt<std::numeric_limits<AttemptId>::max()-16&&attempt<=candidate.epoch_,"Unknown/duplicate/invalid quest or attempt");
        require(!content.empty(),"Missing saved content identity");
        auto definition=release1::rules::quest(id,content);
        if(q<3)require(content==candidate.sessions_[q].content().contentId,"Legacy content mismatch");
        else candidate.extra_[q-3]=std::make_unique<release1::rules::Attempt>(std::move(definition));
        seen[q]=true;ledger[q]=reward;attempts[q]=attempt;
        if(q<3){require(length==0&&reward==candidate.rewarded_[q]&&attempt==candidate.sessions_[q].state().attempt,"Legacy quest mismatch");continue;}
        auto& journal=journals[q-3];journal.reserve(length);
        for(unsigned j=0;j<length;++j){std::string verb,object,target;int direction=0,x=0,y=0;
            require(bool(in>>std::quoted(verb)>>std::quoted(object)>>std::quoted(target)>>direction>>x>>y)&&object.size()<=128&&target.size()<=128&&direction>=0&&direction<4&&x>=-1024&&x<=1024&&y>=-1024&&y<=1024,"Invalid semantic command");
            journal.push_back({parseVerb(verb),std::move(object),std::move(target),Direction(direction),{x,y}});
        }
    }
    std::string kernelSection;
    if(version>=13) {
        tag(in,"QUEST_KERNEL");unsigned schema=0,entries=0;
        require(bool(in>>schema>>entries)&&schema==(version==13?1u:2u)&&entries==(version==13?version13Quests.size():p::quests.size()),"Unsupported quest kernel section");
        std::map<std::string,std::string> records;
        for(unsigned n=0;n<entries;++n) {
            std::string id,content,stage,receipt;AttemptId attempt=0;bool earned=false,complete=false,checkpoint=false;
            require(bool(in>>std::quoted(id)>>std::quoted(content)>>attempt>>earned>>complete>>checkpoint>>std::quoted(stage)>>std::quoted(receipt))&&
                id.size()<=128&&content.size()<=128&&stage.size()<=128&&receipt.size()<=128,"Invalid quest kernel record");
            require(version==13?std::find(version13Quests.begin(),version13Quests.end(),id)!=version13Quests.end():p::questIndex(id)<p::quests.size(),"Unknown quest kernel entry");
            require(!records.contains(id),"Duplicate quest kernel entry");
            std::ostringstream record;record<<std::quoted(id)<<' '<<std::quoted(content)<<' '<<attempt<<' '<<earned<<' '<<complete<<' '<<checkpoint<<' '<<std::quoted(stage)<<' '<<std::quoted(receipt)<<'\n';records[id]=record.str();
        }
        std::ostringstream section;section<<"QUEST_KERNEL "<<schema<<' '<<entries<<'\n';
        if(version==13)for(auto id:version13Quests)section<<records.at(std::string(id));
        else for(const auto& quest:p::quests)section<<records.at(std::string(quest.id));
        kernelSection=section.str();
    }
    tag(in,"END");in>>std::ws;require(in.eof(),"Trailing save data");
    std::size_t commands=0;for(auto& journal:candidate.records_)commands+=journal.size();
    // Frozen graph is topologically ordered; all journal sections were ID-keyed
    // above, so file order is irrelevant. Replay the real Prompt 4 command path.
    for(std::size_t q=3;q<count;++q){
        auto& attempt=*candidate.extra_[q-3];auto& journal=journals[q-3];
        require(journal.empty()||candidate.unlocked(q),"Saved attempt predates prerequisites");
        bool earned=false;unsigned restarts=0;
        for(const auto& step:journal){
            require(attempt.dispatch(attempt.command(step)).accepted,"Illegal saved puzzle command");
            if(step.verb==Verb::Restart)++restarts;
            earned=earned||attempt.view().completed;
        }
        require(earned==ledger[q]&&attempts[q]>=restarts+1,"Reward or attempt does not match replay");
        candidate.rewarded_[q]=earned;candidate.extraIds_[q-3]=attempts[q];commands+=journal.size();candidate.extraRecords_[q-3]=std::move(journal);
    }
    require(revision>=commands&&candidate.unlocked(activeIndex),"Invalid owner revision or locked active quest");
    require(candidate.unlocked(candidate.homeQuest_),"Locked home request");
    candidate.owned_=owned;candidate.placements_.fill({});
    for(std::size_t i=0;i<8;++i){
        require(!owned[i]||candidate.itemUnlocked(i),"Saved purchase predates unlock");
        if(!placements[i].empty()){require(candidate.placementBlockage(p::items[i].id,placements[i]).empty(),"Invalid saved placement");candidate.placements_[i]=placements[i];}
    }
    require(candidate.coins()==balance&&balance>=0,"Wallet does not match reward/purchase ledger");
    require(!picnic||candidate.picnicEligible(),"Ineligible picnic ending");
    require(introductions[0],"Pip introduction is missing");
    for(std::size_t i=0;i<4;++i){require(friendship[i]==candidate.friendship(p::catIds[i]),"Friendship does not match reward ledger");require(!introductions[i]||candidate.chapter()>=(i==0?1:i==3?3:2),"Introduction predates chapter");}
    require(activeIndex>=3||garden==candidate.sessions_[activeIndex].state().inGarden,"Legacy garden state mismatch");
    if(activeIndex<3)require(candidate.active_==activeIndex,"Legacy active quest mismatch");
    candidate.activeQuest_=activeIndex;candidate.ownerRevision_=revision;candidate.garden_=garden;candidate.picnicSeen_=picnic;candidate.introduced_=introductions;candidate.lastReward_=false;
    require(!candidate.beachEndingSeen_||candidate.beachPicnicComplete(),"Beach ending predates completed picnic");
    candidate.beachDevelopmentAccess_=false;
    candidate.replayingSave_=false;candidate.questAdapter_.reset();candidate.reconcileQuests(true);
    if(version>=13) {
        auto expected=candidate.questAdapter_->saveSection(candidate.questFacts());
        if(version==13){expected="QUEST_KERNEL 1 2\n";const auto facts=candidate.questFacts();for(auto id:version13Quests)expected+=candidate.questAdapter_->projectionRecord(facts[p::questIndex(id)]);}
        require(expected==kernelSection,"Quest kernel state disagrees with validated puzzle history");
    }
    return candidate;
}
}
namespace paws::release1::progression {
SaveCandidate inspectSave(std::string bytes) {
    SaveCandidate result;result.original=std::move(bytes);
    try{result.value=Afternoon::decode(result.original);}catch(const std::exception& e){result.error=e.what();}
    return result;
}
}
