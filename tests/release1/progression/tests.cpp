#include "release1/progression/codec.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
namespace legacyPhase7 {
#define main run
#include "legacy_phase7_tests.inc"
#undef main
}
namespace legacyPhase8 {
#define main run
#include "legacy_phase8_tests.inc"
#undef main
}
using namespace paws;
namespace p=paws::release1::progression;
using paws::release1::Verb;
int checks=0;
void check(bool ok,const std::string& why){++checks;if(!ok)throw std::runtime_error(why);}
std::string read(const std::filesystem::path& file){std::ifstream in(file);check(bool(in),"Fixture opens: "+file.string());return {std::istreambuf_iterator<char>(in),{}};}
void go(Afternoon& a,std::size_t q){auto r=a.dispatchIntent(a.intent(Verb::SelectQuest,{},std::string(p::quests[q].id)));check(r.accepted,"Select quest: "+r.message);}
void step(Afternoon& a,const release1::rules::Step& s){auto c=a.intent(s.verb,s.object,s.target);c.direction=s.direction;c.destination=s.destination;auto r=a.dispatchIntent(c);check(r.accepted,"Step "+std::to_string(int(s.verb))+": "+r.message);}
void solve(Afternoon& a,std::size_t q){go(a,q);for(auto& s:release1::rules::quest(p::quests[q].id).solution)step(a,s);check(a.rewarded(q),"Committed reward");}
void garden(Afternoon& a){check(a.returnToGarden(a.intent(Verb::Select)).accepted,"Return to garden");}
void reject(Afternoon& a,release1::Intent c){auto before=a.encode();check(!a.dispatchIntent(c).accepted,"Reject invalid transaction");check(a.encode()==before,"Rejected transaction leaves complete owner unchanged");}
void mutateBad(const Afternoon& a,std::string bytes){auto before=a.encode();auto candidate=p::inspectSave(bytes);check(!candidate.value&&candidate.original==bytes&&!candidate.error.empty(),"Reject candidate and preserve exact original bytes");check(a.encode()==before,"Candidate validation never publishes");}
void replaceLast(std::string& s,const std::string& from,const std::string& to){auto at=s.rfind(from);check(at!=std::string::npos,"Mutation target exists");s.replace(at,from.size(),to);}
int main(){try{
    {
        Afternoon source;
        source.settings.reducedMotion=true;source.saveRevision=17;source.saveSession="snapshot-test";
        source.setBeachDevelopmentAccess(true);
        auto command=source.intent(Verb::Rotate,"channel_b");
        Afternoon copy(source);
        check(copy.encode()==source.encode(),"Snapshot retains complete persistent state");
        check(copy.beachDevelopmentAccess(),"Snapshot retains transient development access");
        const auto unchanged=source.encode();
        check(copy.dispatchIntent(command).accepted,"Snapshot accepts original live command stamp");
        check(source.encode()==unchanged,"Snapshot mutation cannot change original session or journal");
        check(copy.dispatchIntent(copy.intent(Verb::Undo)).accepted,"Snapshot retains independent legacy Undo");
        source.setSuspended(true);Afternoon suspended(source);
        check(!suspended.dispatchIntent(suspended.intent(Verb::Rotate,"channel_b")).accepted,"Snapshot retains suspension fence");
        source.setSuspended(false);solve(source,0);solve(source,1);solve(source,2);go(source,3);
        const auto stepToCopy=release1::rules::quest(p::quests[3].id).solution.front();
        step(source,stepToCopy);const auto extraBefore=source.encode();
        copy=source;
        check(copy.encode()==extraBefore,"Copy assignment retains extra quest journal");
        check(copy.dispatchIntent(copy.intent(Verb::Undo)).accepted,"Snapshot retains extra quest Undo stack");
        check(source.encode()==extraBefore,"Extra quest Undo cannot mutate source attempt");
    }
    check(legacyPhase7::run()==0,"Legacy furnishing transactions on the extended owner");
    check(legacyPhase8::run()==0,"Legacy recovery and migration regressions on the extended owner");
    check(p::validatePolicies(p::quests).empty(),"Valid frozen graph");
    auto graph=p::quests;graph[0].prerequisites={"garden_picnic"};check(!p::validatePolicies(graph).empty(),"Reject graph cycle");
    graph=p::quests;graph[2].prerequisites={"missing"};check(!p::validatePolicies(graph).empty(),"Reject dangling prerequisite");
    graph=p::quests;graph[2].id=graph[1].id;check(!p::validatePolicies(graph).empty(),"Reject duplicate stable ID");
    int cost=0;for(auto& i:p::items)cost+=i.price;check(cost==190,"Frozen economy");
    const auto root=std::filesystem::path(__FILE__).parent_path()/"fixtures";
    for(int version=1;version<=5;++version){
        auto bytes=read(root/("mvp-v"+std::to_string(version)+".save"));auto a=Afternoon::decode(bytes);
        check(a.chapter()==2&&a.friendship("pip")==3&&a.friendship("mabel")==0&&a.friendship("button")==0&&a.friendship("pepper")==0,"Old Pip friendship maps only to Pip");
        check(a.coins()==(version>=3?5:60)&&a.owned("garden_cushion")==bool(version>=3),"Legacy earned coins and purchases retained");
        check(a.endingSeen()==bool(version>=3)&&!a.picnicSeen()&&!a.picnicEligible(),"Separate ending identities");
        check(a.saveRevision==(version>=4?42u:0u)&&a.userPaused==bool(version>=4)&&a.settings.quality==(version==5?2:0),"Recovery/settings migration");
        for(std::size_t i=0;i<3;++i)check(a.quest(i).canUndo()&&a.quest(i).state().attempt>0&&a.quest(i).state().revision>0,"Legacy attempts/history preserved");
        check(a.pendingIntroductions()==std::vector<std::string>({"mabel","button"}),"Migrated chapter derives pending new introductions without rewards");
        check(Afternoon::decode(a.encode()).encode()==a.encode(),"Migrated v6 exact roundtrip");
        for(std::size_t q=3;q<10;++q)check(!a.rewarded(q),"No retroactive new rewards");
    }
    Afternoon a;garden(a);reject(a,a.intent(Verb::Buy,"garden_cushion"));reject(a,a.intent(Verb::SelectQuest,{},"garden_picnic"));
    auto malformed=a.intent(Verb::Hint);malformed.object=std::string(129,'x');reject(a,malformed);
    solve(a,0);check(a.coins()==20&&a.friendship()==1&&a.lastRewardGranted(),"First atomic reward");
    auto stale=a.intent(Verb::Hint);step(a,{Verb::Hint});reject(a,stale);
    step(a,{Verb::Undo});step(a,release1::rules::quest(p::quests[0].id).solution.back());check(a.coins()==20&&!a.lastRewardGranted(),"Undo/recompletion pays once");
    step(a,{Verb::Restart});for(auto& s:release1::rules::quest(p::quests[0].id).solution)step(a,s);check(a.coins()==20,"Practice pays nothing");
    solve(a,1);solve(a,2);check(a.chapter()==2&&a.coins()==60,"All chapter two quests open together");
    for(std::size_t i=3;i<7;++i)check(a.unlocked(i),"Unordered chapter two unlock");
    check(a.introduce("mabel",a.intent(Verb::Select)).accepted,"Introduction acknowledged");
    auto prior=a.encode();check(!a.introduce("mabel",a.intent(Verb::Select)).accepted&&a.encode()==prior,"Once-only introduction");
    check(!a.introduce("pepper",a.intent(Verb::Select)).accepted,"Locked introduction rejected");
    // Two independently interrupted journals and hints survive a save/restore.
    go(a,3);auto bench=release1::rules::quest(p::quests[3].id);step(a,bench.solution[0]);step(a,{Verb::Hint});
    auto old=a.intent(Verb::Slide,"crate_a");go(a,4);reject(a,old);
    auto planks=release1::rules::quest(p::quests[4].id);step(a,planks.solution[0]);
    a.saveRevision=78;a.saveSession="release-test";a.settings.effects=23;
    auto bytes=a.encode();auto restored=Afternoon::decode(bytes);check(restored.encode()==bytes,"Interrupted journals exact roundtrip");
    auto oldCommand=a.intent(Verb::Hint);restored.invalidateImportedCommands(a);reject(restored,oldCommand);
    check(restored.saveRevision==78&&restored.settings.effects==23,"Import rebase retains recovery/settings");
    go(restored,3);check(restored.puzzleView().revealedHints==1&&restored.puzzleView().canUndo,"Bench history and hints restore independently");
    for(std::size_t i=1;i<bench.solution.size();++i)step(restored,bench.solution[i]);
    go(restored,4);for(std::size_t i=1;i<planks.solution.size();++i)step(restored,planks.solution[i]);
    solve(restored,6);solve(restored,5);check(restored.chapter()==3&&restored.unlocked(7)&&restored.unlocked(8)&&!restored.unlocked(9),"Chapter three branch unlocks");
    go(restored,7);step(restored,{Verb::TapSymbol,"leaf"});step(restored,{Verb::ReplaySequence});
    auto replayed=Afternoon::decode(restored.encode());check(replayed.puzzleView().enteredSymbols.empty()&&replayed.puzzleView().sequenceReplay==1,"sequence replay save lost the fresh answer");
    step(replayed,{Verb::Undo});check(replayed.puzzleView().enteredSymbols==std::vector<std::string>{"leaf"},"saved sequence replay undo lost prior prefix");
    garden(restored);auto model=restored.progressView();check(model.items[6].unlocked&&!model.items[6].available,"Unlocked but unavailable presentation is truthful");
    reject(restored,restored.intent(Verb::Buy,"low_picnic_table"));
    solve(restored,8);check(restored.friendship("pip")==4&&restored.friendship("pepper")==1,"Joint reward distributes to both cats");
    solve(restored,7);solve(restored,9);check(restored.coins()==200&&!restored.picnicEligible(),"Ten quests without purchases; ending waits for placement");
    model=restored.progressView();check(model.cats[0].friendship==5&&model.cats[1].friendship==3&&model.cats[2].friendship==3&&model.cats[3].friendship==3,"Final friendship distribution");
    for(auto& cat:model.cats)check(cat.milestone==3,"Per-cat at-home milestone");
    garden(restored);reject(restored,restored.intent(Verb::RecordPicnic));
    for(auto& item:p::items)restored.setItemAvailable(item.id,true); // Logical fixture only, no art acceptance.
    for(auto& item:p::items){auto before=restored.ownerRevision();check(restored.dispatchIntent(restored.intent(Verb::Buy,std::string(item.id))).accepted,"Buy unlocked affordable item");check(restored.ownerRevision()==before+1,"Purchase commits exactly one owner revision");}
    check(restored.coins()==10,"Eight items cost 190 exactly");reject(restored,restored.intent(Verb::Buy,"garden_cushion"));
    check(restored.dispatchIntent(restored.intent(Verb::PlaceFurnishing,"garden_cushion","bench_corner")).accepted,"Place owned cushion");
    auto options=restored.placementView("hammock");check(options[5].compatible&&!options[0].compatible,"Placement read model uses authoritative role/footprint checks");
    reject(restored,restored.intent(Verb::PlaceFurnishing,"drinking_bowl","bench_corner"));
    reject(restored,restored.intent(Verb::PlaceFurnishing,"hammock","sunny_corner"));
    check(restored.dispatchIntent(restored.intent(Verb::PlaceFurnishing,"garden_cushion","sunny_corner")).accepted&&restored.coins()==10,"Free move");
    for(std::size_t i=1;i<8;++i){auto spot=i==1?"quiet_corner":std::string(p::spots[i+1].id);check(restored.dispatchIntent(restored.intent(Verb::PlaceFurnishing,std::string(p::items[i].id),spot)).accepted,"Compatible authored placement");}
    check(restored.picnicEligible()&&!restored.picnicSeen(),"Eligible new finale independent of MVP flag");
    check(restored.dispatchIntent(restored.intent(Verb::RecordPicnic)).accepted,"Record picnic");reject(restored,restored.intent(Verb::RecordPicnic));
    check(restored.dispatchIntent(restored.intent(Verb::ReplayPicnic)).accepted&&restored.coins()==10,"Replay preserves rewards");
    bytes=restored.encode();check(Afternoon::decode(bytes).encode()==bytes,"Complete catalog/finale roundtrip");
    auto bad=bytes;replaceLast(bad,"\"garden_picnic\" \"garden-picnic-v1\"","\"garden_picnic\" \"garden-picnic-v999\"");mutateBad(restored,bad);
    bad=bytes;replaceLast(bad,"\"pepper\" 3", "\"pepper\" 4");mutateBad(restored,bad);
    bad=bytes;replaceLast(bad,"\"hammock_nook\"", "\"quiet_corner\"");mutateBad(restored,bad);
    bad=bytes;replaceLast(bad,"\"garden_picnic\" \"garden-picnic-v1\" 1", "\"garden_picnic\" \"garden-picnic-v1\" 0");mutateBad(restored,bad);
    mutateBad(restored,bytes+"extra");mutateBad(restored,bytes.substr(0,bytes.size()/2));mutateBad(restored,"PAWS_AFTERNOON 99\n");
    mutateBad(restored,std::string(16*1024*1024+1,'x'));
    bad=bytes;replaceLast(bad,"\"pepper\" 3", "\"pip\" 3");mutateBad(restored,bad);
    // ID-keyed item and cat tables accept a different file order, normalizing on write.
    auto reverseRows=[](std::string data,const std::string& marker,int count){auto start=data.find(marker)+marker.size();auto end=start;std::vector<std::string> rows;for(int i=0;i<count;++i){auto next=data.find('\n',end)+1;rows.push_back(data.substr(end,next-end));end=next;}std::reverse(rows.begin(),rows.end());std::string replacement;for(auto& r:rows)replacement+=r;data.replace(start,end-start,replacement);return data;};
    check(Afternoon::decode(reverseRows(reverseRows(bytes,"CATS 4\n",4),"ITEMS 8\n",8)).encode()==bytes,"Stable IDs make file ordering irrelevant");
    auto migrated=Afternoon::decode(read(root/"mvp-v3.save"));for(std::size_t q=3;q<10;++q)solve(migrated,q);garden(migrated);
    check(migrated.endingSeen()&&!migrated.picnicSeen()&&migrated.picnicEligible(),"Old seen ending does not suppress release-one eligibility");
    check(migrated.dispatchIntent(migrated.intent(Verb::RecordPicnic)).accepted&&migrated.coins()==145,"Migrated finale records without duplicate old rewards");
    go(restored,9);check(restored.setUserPaused(true,restored.intent(Verb::Select)).accepted,"Explicit pause");
    restored.setSuspended(true);restored.setSuspended(false);reject(restored,restored.intent(Verb::Restart));check(restored.userPaused,"Background return preserves explicit pause");
    check(restored.setUserPaused(false,restored.intent(Verb::Select)).accepted,"Explicit resume");
    auto direct=restored;go(direct,4);step(direct,{Verb::Restart});
    step(direct,{Verb::Rotate,"long_plank"});step(direct,{Verb::Rotate,"step_plank"});
    const auto directBytes=direct.encode();auto directRestored=Afternoon::decode(directBytes);
    check(directRestored.encode()==directBytes&&directRestored.coins()==direct.coins(),"Direct loose rotations survive journal replay without changing rewards");
    restored.settings.reducedMotion=true;restored.settings.music=30;restored.settings.quality=2;
    restored.saveRevision=500;
    auto beforeAdventure=restored.intent(Verb::Restart);
    auto fresh=restored.restartedAdventure();
    check(fresh.coins()==0&&fresh.chapter()==1&&fresh.inGarden()&&!fresh.userPaused,"Adventure restart returns to a fresh garden");
    check(fresh.settings.reducedMotion&&fresh.settings.music==30&&fresh.settings.quality==2,"Adventure restart preserves preferences");
    check(fresh.saveRevision>=500&&!fresh.picnicSeen()&&!fresh.endingSeen(),"Restart preserves save ordering and clears finales");
    for(std::size_t q=0;q<10;++q)check(!fresh.rewarded(q)&&fresh.unlocked(q)==(q==0),"Restart clears every reward and relocks later quests");
    for(auto& item:p::items)check(!fresh.owned(item.id)&&fresh.placement(item.id).empty(),"Restart clears purchases and placements");
    reject(fresh,beforeAdventure);
    auto freshReload=Afternoon::decode(fresh.encode());
    check(freshReload.coins()==0&&freshReload.chapter()==1&&freshReload.inGarden(),"Fresh campaign survives save roundtrip");
    solve(freshReload,0);check(freshReload.coins()==20,"First reward is earnable again after campaign restart");
    // All 24 chapter-two orders and both chapter-three orders stay purchase-free.
    std::array<std::size_t,4> order{3,4,5,6};int variants=0;
    do{Afternoon b;for(std::size_t q=0;q<3;++q)solve(b,q);for(auto q:order)solve(b,q);solve(b,variants%2?7:8);solve(b,variants%2?8:7);solve(b,9);check(b.coins()==200,"Every branch order rewards 200 without purchases");++variants;}while(std::next_permutation(order.begin(),order.end()));
    check(variants==24,"Every chapter-two permutation tested");
    std::cout<<checks<<" progression, migration, replay, transaction and branch-order assertions passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"After "<<checks<<" assertions: "<<e.what()<<'\n';return 1;}}

