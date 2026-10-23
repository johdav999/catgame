#include "owner.hpp"
#include "release1/rules/content.hpp"
#include <algorithm>
#include <functional>
#include <limits>
#include <set>
#include <stdexcept>

namespace paws::release1::progression {
const std::array<QuestPolicy,questCount> quests{{
 {"pip_missing_ball","missing-ball-v2",1,1,{}},
 {"pip_tidy_toys","tidy-toys-v2",1,1,{"pip_missing_ball"}},
 {"pip_stuck_gate","stuck-gate-v2",1,1,{"pip_tidy_toys"}},
 {"button_mouse_bench","button-mouse-bench-v2",2,4,{"pip_stuck_gate"}},
 {"mabel_sunny_spot","mabel-sunny-spot-v2",2,2,{"pip_stuck_gate"}},
 {"button_cushion_bed","button-cushion-bed-v1",2,4,{"pip_stuck_gate"}},
 {"mabel_drinking_route","mabel-drinking-route-v1",2,2,{"pip_stuck_gate"}},
 {"pepper_garden_bells","pepper-garden-bells-v1",3,8,{"button_mouse_bench","mabel_sunny_spot","button_cushion_bed","mabel_drinking_route"}},
 {"pip_pepper_playhouse","pip-pepper-playhouse-v2",3,9,{"button_mouse_bench","mabel_sunny_spot","button_cushion_bed","mabel_drinking_route"}},
 {"garden_picnic","garden-picnic-v1",3,15,{"pepper_garden_bells","pip_pepper_playhouse"}},
 {"pepper_scent_trail","pepper-scent-trail-v1",3,8,{"pepper_garden_bells"}},
 {"pip_wobbly_perch","pip-wobbly-perch-v1",3,1,{"pip_pepper_playhouse"}},
 {"pip_friend_ball","pip-friend-ball-v1",1,1,{"pip_missing_ball"}},
 {"pip_friend_view","pip-friend-view-v1",1,1,{"pip_friend_ball"}},
 { "pip_friend_room","pip-friend-room-v1",1,1,{"pip_friend_view"}},
 {"mabel_friend_sun","mabel-friend-sun-v1",2,2,{"pip_stuck_gate","pip_friend_room"}},
 {"mabel_friend_water","mabel-friend-water-v1",2,2,{"mabel_friend_sun"}},
 {"mabel_friend_share","mabel-friend-share-v1",2,3,{"mabel_friend_water"}},
 {beachQuestId,"beach-pip-ball-v2",1,1,{},beachPackId},
 {"beach_button_bridge","beach-button-bridge-v2",1,4,{beachQuestId},beachPackId},
 {"beach_mabel_shade","beach-mabel-shade-v2",1,2,{"beach_button_bridge"},beachPackId},
 {"beach_pepper_basket","beach-pepper-basket-v2",1,8,{"beach_mabel_shade"},beachPackId},
 {"beach_shell_chimes","beach-shell-chimes-v2",1,8,{"beach_pepper_basket"},beachPackId},
 {"beach_picnic","beach-picnic-v2",1,15,{"beach_shell_chimes"},beachPackId},
 {"beach_tidepool_crossing","beach-tidepool-crossing-v1",1,4,{"beach_picnic"},beachPackId},
 {"beach_fresh_water","beach-fresh-water-v1",1,2,{"beach_tidepool_crossing"},beachPackId},
 {"beach_windbreak","beach-windbreak-v2",1,6,{"beach_fresh_water"},beachPackId},
 {"beach_shell_encore","beach-shell-encore-v2",1,8,{"beach_windbreak"},beachPackId},
 {"beach_lantern_balance","beach-lantern-balance-v1",1,1,{"beach_shell_encore"},beachPackId}
}};
const std::array<ItemPolicy,8> items{{
 {"garden_cushion","Garden cushion","garden/cushion@1","pip_missing_ball",30,.94f,1.18f,{"rest","quiet"}},
 {"drinking_bowl","Drinking bowl","garden/bowl@1","pip_missing_ball",25,.56f,.56f,{"drink","sunny","quiet"}},
 {"scratching_post","Scratching post","release1/props/scratching_post@1","pip_tidy_toys",20,.65f,.65f,{"scratch"}},
 {"sunny_perch","Sunny perch","release1/props/sunny_perch@1","mabel_sunny_spot",20,.8f,1.4f,{"perch"}},
 {"hammock","Hammock","release1/props/hammock@1","button_cushion_bed",25,1.1f,1.6f,{"hammock"}},
 {"playhouse","Playhouse","assembly:playhouse_furnishing","pip_pepper_playhouse",30,1.5f,1.6f,{"playhouse"}},
 {"low_picnic_table","Low picnic table","release1/props/low_picnic_table@1","chapter2_complete",20,1.2f,1.4f,{"picnic_table"}},
 {"picnic_mat","Picnic mat","release1/props/picnic_mat@1","chapter2_complete",20,1.5f,1.5f,{"picnic_mat"}}
}};
const std::array<SpotPolicy,9> spots{{
 {"bench_corner",1.1f,1.35f,{"rest","drink"}}, {"sunny_corner",1.1f,1.35f,{"rest","sunny","drink"}},
 {"quiet_corner",1.1f,1.35f,{"rest","quiet","drink"}}, {"scratch_corner",.8f,.8f,{"scratch"}},
 {"perch_corner",.9f,1.5f,{"perch"}}, {"hammock_nook",1.2f,1.7f,{"hammock"}},
 {"playhouse_corner",1.6f,1.7f,{"playhouse"}}, {"picnic_table_spot",1.3f,1.5f,{"picnic_table"}},
 {"picnic_mat_spot",1.6f,1.6f,{"picnic_mat"}}
}};
std::size_t questIndex(std::string_view id) {for(std::size_t i=0;i<quests.size();++i)if(quests[i].id==id)return i;return quests.size();}
std::size_t itemIndex(std::string_view id) {for(std::size_t i=0;i<items.size();++i)if(items[i].id==id)return i;return items.size();}
std::size_t spotIndex(std::string_view id) {for(std::size_t i=0;i<spots.size();++i)if(spots[i].id==id)return i;return spots.size();}
bool compatible(const ItemPolicy& item,const SpotPolicy& spot) {
    return item.width<=spot.width && item.depth<=spot.depth && std::any_of(item.roles.begin(),item.roles.end(),[&](auto r){return std::find(spot.roles.begin(),spot.roles.end(),r)!=spot.roles.end();});
}
std::string validatePolicies(std::span<const QuestPolicy> all) {
    std::set<std::string_view> ids,contents;
    for(const auto& q:all)if(q.id.empty()||q.content.empty()||!ids.insert(q.id).second||!contents.insert(q.content).second||q.chapter<1||q.chapter>3||!q.requesters||q.requesters>15)return "Invalid quest identity or reward policy.";
    std::vector<int> marks(all.size());
    std::function<bool(std::size_t)> visit=[&](std::size_t i){
        if(marks[i]==1)return false;if(marks[i]==2)return true;marks[i]=1;
        std::set<std::string_view> seen;
        for(auto id:all[i].prerequisites){
            auto p=std::find_if(all.begin(),all.end(),[&](const auto& q){return q.id==id;});
            if(p==all.end()||!seen.insert(id).second||p->chapter>all[i].chapter||!visit(std::size_t(p-all.begin())))return false;
        }
        marks[i]=2;return true;
    };
    for(std::size_t i=0;i<all.size();++i)if(!visit(i))return "Invalid prerequisite graph.";
    return {};
}
}

namespace paws {
namespace p=release1::progression;
using release1::Verb;
namespace {
std::optional<Action> legacyAction(Verb v,std::string_view object,const PawsSession& s) {
    switch(v){
    case Verb::Select:return Action::Select;case Verb::Slide:return Action::Move;case Verb::Rotate:return Action::Rotate;
    case Verb::Roll:return Action::Roll;case Verb::PickUp:return Action::PickUp;case Verb::Fit:return Action::Place;
    case Verb::Retrieve:return object==s.content().sourceId?Action::ReturnBall:Action::PickUp;
    case Verb::Undo:return Action::Undo;case Verb::Restart:return Action::Restart;case Verb::Hint:return Action::Hint;
    default:return {};
    }
}
}
void Afternoon::initializeRelease() {
    if(auto e=p::validatePolicies(p::quests);!e.empty())throw std::invalid_argument(e);
    for(std::size_t i=0;i<extra_.size();++i){auto d=release1::rules::quest(p::quests[i+3].id);if(d.content!=p::quests[i+3].content)throw std::invalid_argument("Content policy mismatch");extra_[i]=std::make_unique<release1::rules::Attempt>(std::move(d));}
    reconcileQuests(true);
}
release1::progression::QuestFacts Afternoon::questFacts() const {
    p::QuestFacts facts;
    for(std::size_t n=0;n<facts.size();++n) {
        auto& f=facts[n];f.id=p::quests[n].id;f.earned=rewarded_[n];
        if(n<3){f.content=sessions_[n].content().contentId;f.attempt=sessions_[n].state().attempt;f.completed=sessions_[n].state().puzzle.completed;}
        else {const auto view=extra_[n-3]->view();f.content=extra_[n-3]->definition().content;f.attempt=extraIds_[n-3];f.completed=view.completed;
            if(f.id=="beach_fresh_water")for(const auto& object:view.objects)if(object.role=="water_basin")f.checkpoint=object.active;}
    }
    return facts;
}
void Afternoon::reconcileQuests(bool replay) {
    auto candidate=p::QuestAdapter::reconcile(questFacts(),questAdapter_,replay);
    for(std::size_t n=0;n<p::quests.size();++n)rewarded_[n]=candidate->earned(p::quests[n].id);
    lastReward_=!replay&&p::quests[activeQuest_].pack.empty()&&
        std::find(candidate->diagnostics().newReceipts.begin(),candidate->diagnostics().newReceipts.end(),
            std::string(p::quests[activeQuest_].id)+"/first-completion")!=candidate->diagnostics().newReceipts.end();
    questAdapter_=std::move(candidate);
}
void Afternoon::publishQuestCandidate(Afternoon&& candidate) noexcept {
    // Existing callers may iterate the borrowed immutable legacy solution while
    // dispatching moves. Retain its allocations across atomic state publication.
    for(std::size_t n=0;n<sessions_.size();++n)
        if(sessions_[n].content_.contentId==candidate.sessions_[n].content_.contentId)
            std::swap(sessions_[n].content_,candidate.sessions_[n].content_);
    *this=std::move(candidate);
}
// Speculative input checks need a live snapshot, not save validation/replay of
// every quest. Preserve transient command stamps and independent undo owners.
Afternoon::Afternoon(const Afternoon& o):settings(o.settings),saveRevision(o.saveRevision),saveSession(o.saveSession),userPaused(o.userPaused),
    sessions_(o.sessions_),records_(o.records_),rewarded_(o.rewarded_),active_(o.active_),epoch_(o.epoch_),lastReward_(o.lastReward_),
    owned_(o.owned_),placements_(o.placements_),endingSeen_(o.endingSeen_),extraRecords_(o.extraRecords_),extraIds_(o.extraIds_),
    introduced_(o.introduced_),available_(o.available_),activeQuest_(o.activeQuest_),homeQuest_(o.homeQuest_),
    ownerRevision_(o.ownerRevision_),generation_(o.generation_),garden_(o.garden_),picnicSeen_(o.picnicSeen_),suspended_(o.suspended_),
    beachDevelopmentAccess_(o.beachDevelopmentAccess_),beachEndingSeen_(o.beachEndingSeen_),questAdapter_(o.questAdapter_),replayingSave_(o.replayingSave_) {
    for(std::size_t n=0;n<extra_.size();++n)
        if(o.extra_[n])extra_[n]=std::make_unique<release1::rules::Attempt>(*o.extra_[n]);
}
Afternoon& Afternoon::operator=(const Afternoon& o) {if(this!=&o){Afternoon candidate(o);*this=std::move(candidate);}return *this;}
bool Afternoon::canAdvance() const {
    return ownerRevision_<std::numeric_limits<Revision>::max()-16&&epoch_<std::numeric_limits<AttemptId>::max()-16&&generation_<std::numeric_limits<std::uint64_t>::max()-16&&
        std::all_of(sessions_.begin(),sessions_.end(),[](const auto& s){return s.state().revision<std::numeric_limits<Revision>::max()-16&&s.state().attempt<std::numeric_limits<AttemptId>::max()-16;});
}
int Afternoon::legacyFriendship() const {return int(std::count(rewarded_.begin(),rewarded_.begin()+3,true));}
int Afternoon::legacyCoins() const {return 20*legacyFriendship()-(owned_[0]?30:0)-(owned_[1]?25:0);}
int Afternoon::friendship() const {return friendship("pip");}
int Afternoon::friendship(std::string_view cat) const {
    auto it=std::find(p::catIds.begin(),p::catIds.end(),cat);if(it==p::catIds.end())return 0;
    const unsigned bit=1u<<unsigned(it-p::catIds.begin());int value=0;
    for(std::size_t i=0;i<p::quests.size();++i)if(p::quests[i].pack.empty()&&rewarded_[i]&&(p::quests[i].requesters&bit))++value;return value;
}
int Afternoon::coins() const {int balance=0;for(std::size_t i=0;i<p::quests.size();++i)if(p::quests[i].pack.empty()&&rewarded_[i])balance+=20;for(std::size_t i=0;i<8;++i)if(owned_[i])balance-=p::items[i].price;return balance;}
bool Afternoon::unlocked(std::size_t i) const {
    if(i>=p::quests.size())return false;
    if(p::quests[i].pack==p::beachPackId&&!beachDevelopmentAccess_)return false;
    if(!replayingSave_&&questAdapter_)return questAdapter_->available(p::quests[i].id);
    return std::all_of(p::quests[i].prerequisites.begin(),p::quests[i].prerequisites.end(),[&](auto id){const auto n=p::questIndex(id);return n<p::quests.size()&&rewarded_[n];});
}
int Afternoon::chapter() const {return unlocked(7)?3:unlocked(3)?2:1;}
bool Afternoon::itemUnlocked(std::size_t i) const {if(i>=8)return false;return p::items[i].unlock=="chapter2_complete"?chapter()==3:rewarded_[p::questIndex(p::items[i].unlock)];}
bool Afternoon::owned(std::string_view id) const {auto i=p::itemIndex(id);return i<8&&owned_[i];}
std::string_view Afternoon::placement(std::string_view id) const {auto i=p::itemIndex(id);return i<8?std::string_view(placements_[i]):std::string_view{};}
void Afternoon::setItemAvailable(std::string_view id,bool ready) {auto i=p::itemIndex(id);if(i>=8)throw std::invalid_argument("Unknown furnishing");available_[i]=ready;}
std::string Afternoon::purchaseBlockage(std::string_view id) const {
    const auto i=p::itemIndex(id);if(i>=8)return "That furnishing is not in the catalogue.";
    if(owned_[i])return "Already yours. Moving it is free.";
    if(!itemUnlocked(i))return "Locked until its related request milestone.";
    if(!available_[i])return "This furnishing's presentation assets are unavailable.";
    if(coins()<p::items[i].price)return "You need "+std::to_string(p::items[i].price-coins())+" more coins.";
    return {};
}
std::string Afternoon::placementBlockage(std::string_view id,std::string_view spot) const {
    const auto i=p::itemIndex(id),s=p::spotIndex(spot);
    if(i>=8)return "Choose a known furnishing.";if(!owned_[i])return "Buy this furnishing before placing it.";
    if(s>=9)return "Choose an authored garden spot.";
    if(!p::compatible(p::items[i],p::spots[s]))return "The furnishing's role or footprint does not fit this spot.";
    for(std::size_t n=0;n<8;++n)if(n!=i&&placements_[n]==spot)return "Another furnishing occupies this spot.";
    return {};
}
bool Afternoon::endingEligible() const {return legacyFriendship()==3&&(!placements_[0].empty()||!placements_[1].empty());}
bool Afternoon::picnicEligible() const {return std::all_of(rewarded_.begin(),rewarded_.begin()+10,[](bool b){return b;})&&std::any_of(placements_.begin(),placements_.end(),[](auto& s){return !s.empty();});}
AttemptId Afternoon::currentAttempt() const {return activeQuest_<3?sessions_[activeQuest_].state().attempt:extraIds_[activeQuest_-3];}
bool Afternoon::current(const release1::Intent& c) const {return c.generation==generation_&&c.attempt==currentAttempt()&&c.revision==ownerRevision_&&c.quest==p::quests[activeQuest_].id;}
release1::Intent Afternoon::intent(Verb verb,std::string object,std::string target) const {
    release1::Intent c;c.generation=generation_;c.attempt=currentAttempt();c.revision=ownerRevision_;c.quest=p::quests[activeQuest_].id;c.verb=verb;c.object=std::move(object);c.target=std::move(target);return c;
}
bool Afternoon::selectQuest(std::size_t i) {
    if(!unlocked(i)||!canAdvance())return false;
    if(p::quests[i].pack.empty())homeQuest_=i;
    else if(p::quests[activeQuest_].pack.empty())homeQuest_=activeQuest_;
    for(const auto& s:sessions_)epoch_=std::max(epoch_,s.state().attempt);
    if(i<3){if(i!=activeQuest_)sessions_[i].invalidateCommands(++epoch_);else sessions_[i].invalidateCommands(sessions_[i].state().attempt);active_=i;garden_=sessions_[i].state().inGarden;}
    else {if(i!=activeQuest_)extraIds_[i-3]=++epoch_;garden_=false;}
    activeQuest_=i;++generation_;++ownerRevision_;lastReward_=false;return true;
}
CommandResult Afternoon::dispatchIntent(const release1::Intent& c) {
    if(replayingSave_)return dispatchIntentImpl(c);
    Afternoon candidate(*this);auto result=candidate.dispatchIntentImpl(c);
    if(result.accepted){candidate.reconcileQuests();publishQuestCandidate(std::move(candidate));}
    return result;
}
CommandResult Afternoon::dispatchIntentImpl(const release1::Intent& c) {
    if(!current(c)||!canAdvance())return {false,"That action belongs to an earlier view."};
    if(c.object.size()>128||c.target.size()>128||int(c.direction)<0||int(c.direction)>3||c.orientation!=0||c.destination.x<-1024||c.destination.x>1024||c.destination.y<-1024||c.destination.y>1024)return {false,"Invalid command fields."};
    if(userPaused||suspended_)return {false,"Resume the afternoon first."};
    if(c.verb==Verb::RecordBeachEnding){
        if(!inBeach()||activeQuest_!=p::questIndex("beach_picnic")||!beachDevelopmentAccess_||!beachPicnicComplete()||!puzzleView().completed)return {false,"Finish the beach picnic before watching the ending."};
        beachEndingSeen_=true;++ownerRevision_;lastReward_=false;return {true,"A seaside afternoon to remember."};
    }
    if(c.verb==Verb::SelectQuest) {
        auto i=p::questIndex(c.target);if(!unlocked(i))return {false,"That request is locked."};
        const auto revision=ownerRevision_;
        // Resume is journaled before selection only after checking all preconditions.
        if(i<3&&sessions_[i].state().inGarden&&records_[i].size()>=100000)return {false,"This request's journal is full."};
        selectQuest(i);
        if(i<3&&session().state().inGarden){auto s=session().state();auto r=dispatchLegacyImpl({s.attempt,s.revision,Action::Resume,{}});if(!r.accepted)throw std::logic_error("Validated resume failed");}
        garden_=false;ownerRevision_=revision+1;return {true,"Request selected."};
    }
    if(c.verb==Verb::Buy||c.verb==Verb::PlaceFurnishing||c.verb==Verb::RecordPicnic||c.verb==Verb::ReplayPicnic) {
        if(!garden_)return {false,"Return to the garden first."};
        if(c.verb==Verb::RecordPicnic){if(picnicSeen_||!picnicEligible())return {false,"Complete all ten requests and place an owned furnishing first; the ending records once."};picnicSeen_=true;}
        else if(c.verb==Verb::ReplayPicnic){if(!picnicSeen_)return {false,"The picnic ending has not been recorded."};++generation_;}
        else {
            auto reason=c.verb==Verb::Buy?purchaseBlockage(c.object):placementBlockage(c.object,c.target);
            if(!reason.empty())return {false,std::move(reason)};
            const auto i=p::itemIndex(c.object);
            if(!available_[i])return {false,"This furnishing's presentation assets are unavailable."};
            if(c.verb==Verb::Buy)owned_[i]=true;
            else {auto prepared=c.target;placements_[i].swap(prepared);}
        }
        ++ownerRevision_;lastReward_=false;return {true,"Garden change committed."};
    }
    if(garden_)return {false,"Select a request before arranging puzzle pieces."};
    if(inBeach()&&!beachDevelopmentAccess_)return {false,"This preview needs development beach access."};
    if(activeQuest_<3){auto action=legacyAction(c.verb,c.object,session());if(!action)return {false,"Unsupported puzzle operation."};auto s=session().state();return dispatchLegacyImpl({s.attempt,s.revision,*action,c.object,c.direction,c.target});}
    const auto i=activeQuest_-3;
    if(extraRecords_[i].size()>=100000)return {false,"This request's journal is full."};
    if(extraRecords_[i].size()==extraRecords_[i].capacity())extraRecords_[i].reserve(std::min<std::size_t>(100000,std::max<std::size_t>(16,extraRecords_[i].capacity()*2)));
    release1::rules::Step step{c.verb,c.object,c.target,c.direction,c.destination};
    if(c.verb==Verb::Restart&&extra_[i]->definition().content=="pip-pepper-playhouse-v1")step.target="pip-pepper-playhouse-v2";
    if(c.verb==Verb::Restart&&inBeach()&&extra_[i]->definition().content!=p::quests[activeQuest_].content)step.target=p::quests[activeQuest_].content;
    auto command=extra_[i]->command(step);command.orientation=c.orientation;
    auto result=extra_[i]->dispatch(command);if(!result.accepted)return result;
    extraRecords_[i].push_back(std::move(step));
    if(c.verb==Verb::Restart)extraIds_[i]=++epoch_;
    lastReward_=false;
    if(replayingSave_&&extra_[i]->view().completed&&!rewarded_[activeQuest_]){rewarded_[activeQuest_]=true;lastReward_=p::quests[activeQuest_].pack.empty();}
    ++ownerRevision_;return result;
}
CommandResult Afternoon::returnToGarden(const release1::Intent& c) {
    if(!current(c)||!canAdvance())return {false,"That action belongs to an earlier view."};
    if(!p::quests[activeQuest_].pack.empty()&&!selectQuest(homeQuest_))return {false,"The garden could not be restored."};
    if(activeQuest_<3&&!session().state().inGarden){auto s=session().state();auto r=dispatch({s.attempt,s.revision,Action::Garden,{}});if(!r.accepted)return r;}
    else ++ownerRevision_;
    garden_=true;++generation_;lastReward_=false;return {true,"Back in the garden."};
}
std::vector<std::string> Afternoon::pendingIntroductions() const {
    std::vector<std::string> result;const int ch=chapter();
    for(std::size_t i=0;i<4;++i)if(!introduced_[i]&&ch>=(i==0?1:i==3?3:2))result.emplace_back(p::catIds[i]);return result;
}
CommandResult Afternoon::introduce(std::string_view cat,const release1::Intent& c) {
    if(!current(c)||!canAdvance())return {false,"That dialogue belongs to an earlier view."};
    auto it=std::find(p::catIds.begin(),p::catIds.end(),cat);if(it==p::catIds.end())return {false,"Unknown cat."};
    const auto i=std::size_t(it-p::catIds.begin());if(introduced_[i]||chapter()<(i==0?1:i==3?3:2))return {false,"That introduction is not pending."};
    introduced_[i]=true;++ownerRevision_;lastReward_=false;return {true,"Introduction remembered."};
}
void Afternoon::setSuspended(bool on) {if(suspended_!=on){if(!canAdvance())throw std::invalid_argument("Command identity limit");suspended_=on;++generation_;}}
CommandResult Afternoon::setUserPaused(bool on,const release1::Intent& c) {
    if(!current(c)||!canAdvance())return {false,"That pause command belongs to an earlier view."};
    userPaused=on;++ownerRevision_;++generation_;lastReward_=false;return {true,on?"Paused.":"Resumed."};
}
Afternoon Afternoon::restartedAdventure() const {
    Afternoon fresh;
    fresh.beachDevelopmentAccess_=beachDevelopmentAccess_;
    fresh.settings=settings;
    fresh.invalidateImportedCommands(*this);
    fresh.saveSession=saveSession;
    fresh.returnToGarden(fresh.intent(release1::Verb::Select));
    return fresh;
}
void Afternoon::invalidateImportedCommands(const Afternoon& previous) {
    AttemptId next=std::max(epoch_,previous.epoch_);
    for(const auto& s:previous.sessions_)next=std::max(next,s.state().attempt);
    for(const auto& s:sessions_)next=std::max(next,s.state().attempt);
    const auto rev=std::max(ownerRevision_,previous.ownerRevision_);const auto gen=std::max(generation_,previous.generation_);
    if(!canAdvance()||next>=std::numeric_limits<AttemptId>::max()-32||rev>=std::numeric_limits<Revision>::max()-32||gen>=std::numeric_limits<std::uint64_t>::max()-32)throw std::invalid_argument("Save identity limit reached.");
    for(auto& s:sessions_)s.invalidateCommands(++next);
    for(auto& id:extraIds_)id=++next;
    epoch_=next;ownerRevision_=rev+1;generation_=gen+1;saveRevision=std::max(saveRevision,previous.saveRevision);lastReward_=false;
}
release1::ProgressView Afternoon::progressView() const {
    release1::ProgressView v;v.generation=generation_;v.attempt=currentAttempt();v.revision=ownerRevision_;v.coins=coins();v.chapter=chapter();
    v.beachDevelopmentAccess=beachDevelopmentAccess_;
    v.beachEndingSeen=beachEndingSeen_;v.beachComplete=beachComplete();
    v.mvpEndingSeen=endingSeen_;v.picnicEndingSeen=picnicSeen_;v.picnicEligible=picnicEligible();
    for(std::size_t i=0;i<4;++i){auto f=friendship(p::catIds[i]);int m=int(std::count_if(p::milestones[i].begin(),p::milestones[i].end(),[&](int n){return f>=n;}));v.cats.push_back({std::string(p::catIds[i]),std::string(p::catNames[i]),f,m,introduced_[i]});}
    for(std::size_t i=0;i<p::quests.size();++i){const bool completed=!replayingSave_&&questAdapter_?questAdapter_->completed(p::quests[i].id):i<3?sessions_[i].state().puzzle.completed:extra_[i-3]->view().completed;v.quests.push_back({std::string(p::quests[i].id),release1::rules::quest(p::quests[i].id).title,p::quests[i].chapter,unlocked(i),completed,rewarded_[i]});}
    for(std::size_t i=0;i<8;++i){auto& item=p::items[i];v.items.push_back({std::string(item.id),std::string(item.name),std::string(item.asset),placements_[i],!available_[i]&&itemUnlocked(i)?"Presentation assets unavailable.":purchaseBlockage(item.id),item.price,itemUnlocked(i),owned_[i],available_[i]});}
    for(auto& spot:p::spots){std::string occupant;for(std::size_t i=0;i<8;++i)if(placements_[i]==spot.id)occupant=p::items[i].id;v.spots.push_back({std::string(spot.id),std::string(spot.roles[0]),occupant,"Choose a furnishing to check compatibility.",false});}
    return v;
}
std::vector<release1::SpotView> Afternoon::placementView(std::string_view item) const {
    auto result=progressView().spots;const auto i=p::itemIndex(item);
    for(auto& spot:result){spot.reason=placementBlockage(item,spot.id);
        if(spot.reason.empty()&&i<8&&!available_[i])spot.reason="Presentation assets unavailable.";
        if(spot.reason.empty()&&(userPaused||suspended_||!garden_))spot.reason= garden_?"Resume the afternoon first.":"Return to the garden first.";
        spot.compatible=spot.reason.empty();}
    return result;
}
std::vector<release1::Intent> Afternoon::movementOptions(std::string_view object) const {
    using V=release1::Verb;
    std::vector<release1::Intent> result;
    if(inGarden()||userPaused||suspended_)return result;
    const auto initial=puzzleView();if(initial.completed)return result;
    const auto at=std::find_if(initial.objects.begin(),initial.objects.end(),[&](const auto& o){return o.id==object;});
    if(at==initial.objects.end())return result;
    if(at->role=="crate"|| (std::any_of(initial.actions.begin(),initial.actions.end(),[&](const auto& a){return a.intent.object==object&&a.intent.verb==V::Slide;}))){
        struct Node {Afternoon owner;Cell cell;};
        std::vector<Node> queue{{*this,at->cell}};std::vector<Cell> seen{at->cell};
        for(std::size_t n=0;n<queue.size();++n)for(int dir=0;dir<4;++dir){
            auto next=queue[n].owner;auto command=next.intent(V::Slide,std::string(object));command.direction=Direction(dir);
            if(!next.dispatchIntent(command).accepted)continue;
            const auto view=next.puzzleView();const auto o=std::find_if(view.objects.begin(),view.objects.end(),[&](const auto& o){return o.id==object;});
            if(o==view.objects.end()||std::find(seen.begin(),seen.end(),o->cell)!=seen.end())continue;
            seen.push_back(o->cell);command.destination=o->cell;result.push_back(command);queue.push_back({std::move(next),o->cell});
        }
        return result;
    }
    // Sources and rotation-only objects have no movement destinations.
    if(std::none_of(initial.actions.begin(),initial.actions.end(),[&](const auto& a){return a.intent.object==object&&(a.intent.verb==V::PickUp||a.intent.verb==V::Fit||a.intent.verb==V::AssignCat);}))return result;
    auto candidate=*this;
    if(!at->held&&std::any_of(initial.actions.begin(),initial.actions.end(),[&](const auto& a){return a.intent.object==object&&a.intent.verb==V::PickUp;}))
        if(!candidate.dispatchIntent(candidate.intent(V::PickUp,std::string(object))).accepted)return result;
    for(const auto& action:candidate.puzzleView().actions)if(action.enabled&&action.intent.object==object&&(action.intent.verb==V::Fit||action.intent.verb==V::AssignCat))result.push_back(action.intent);
    return result;
}
release1::PuzzleView Afternoon::puzzleView() const {
    release1::PuzzleView v;
    if(activeQuest_>=3)v=extra_[activeQuest_-3]->view();
    else {
        const auto& s=session();const auto& d=s.content();const auto& state=s.state();
        v.quest=d.questId;v.content=d.contentId;v.title=d.title;v.objective=release1::rules::quest(d.questId,d.contentId).objective;v.completed=state.puzzle.completed;v.canUndo=s.canUndo();v.revealedHints=state.hint;
        for(std::size_t n=0;n<3;++n)v.hints[n]=d.hints[n];
        auto add=[&](Verb verb,std::string object={},Direction direction=Direction::North,std::string target={}){
            auto c=intent(verb,object,target);c.direction=direction;auto action=legacyAction(verb,object,s);auto probe=s;
            auto r=action?probe.dispatch({state.attempt,state.revision,*action,object,direction,target}):CommandResult{false,"Unsupported action"};
            auto name=[&](const std::string& id){for(auto& o:d.objects)if(o.id==id)return o.name;return id==d.sourceId?std::string("ball"):id;};
            static constexpr std::array<std::string_view,4> directions{"north","east","south","west"};
            v.actions.push_back({c,verb==Verb::Select?"Select "+name(object):verb==Verb::Slide?"Move "+name(object)+" "+std::string(directions[int(direction)]):verb==Verb::Rotate?"Rotate "+name(object):verb==Verb::Fit?"Fit "+name(object)+" into "+name(target):verb==Verb::Undo?"Undo":verb==Verb::Restart?"Restart":verb==Verb::Hint?"Hint":verb==Verb::Roll?"Roll":"Retrieve "+name(object),r.accepted?"":r.message,r.accepted});
        };
        for(std::size_t i=0;i<d.objects.size();++i){const auto& o=d.objects[i];const auto& st=state.puzzle.objects[i];
            const std::string role=channel(o.kind)?"channel":o.kind==Kind::Cushion?"blocking_cushion":o.kind==Kind::Crate?"crate":o.kind==Kind::Catch?"catch_tray":sliding(o.kind)?"pot":o.kind==Kind::Toy?"toy":o.kind==Kind::Socket?"basket_socket":o.kind==Kind::Gate?"gate":"switch";
            v.objects.push_back({o.id,o.name,role,st.cell,st.orientation,{st.cell},st.socket,st.location==Location::Held,o.kind==Kind::Switch?switchActive(d,state.puzzle,o.id):o.kind==Kind::Gate?gateOpen(d,state.puzzle,o.id):false,st.location==Location::Collected});
            add(Verb::Select,o.id);if(sliding(o.kind))for(int k=0;k<4;++k)add(Verb::Slide,o.id,Direction(k));if(channel(o.kind))add(Verb::Rotate,o.id);
            if(o.kind==Kind::Toy){add(Verb::PickUp,o.id);for(auto& t:d.objects)if(t.kind==Kind::Socket)add(Verb::Fit,o.id,Direction::North,t.id);if(d.packedToysBlockAccess)add(Verb::Fit,o.id,Direction::North,"home");}
        }
        if(d.goal!=Goal::Tidy){v.route=evaluateRoute(d,state.puzzle).path;add(Verb::Roll);add(Verb::Retrieve,d.sourceId);}
        v.objects.push_back({d.sourceId,"Ball source","ball",d.source});v.objects.push_back({d.destinationId,"Destination","destination",d.destination});
        add(Verb::Undo);add(Verb::Restart);add(Verb::Hint);
    }
    for(std::size_t n=0;n<p::beachQuestIds.size();++n)if(rewarded_[p::baseQuestCount+n])v.beachProgress|=1u<<n;
    if(!replayingSave_&&questAdapter_)v.completed=questAdapter_->completed(v.quest);
    v.generation=generation_;v.attempt=currentAttempt();v.revision=ownerRevision_;
    for(auto& a:v.actions){a.intent.generation=v.generation;a.intent.attempt=v.attempt;a.intent.revision=v.revision;
        if(userPaused||suspended_||garden_){a.enabled=false;a.reason=garden_?"Select a request first.":"Resume the afternoon first.";}}
    return v;
}
}
