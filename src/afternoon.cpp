#include "afternoon.hpp"
#include <algorithm>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace paws {
Afternoon::Afternoon():sessions_{PawsSession(missingBall()),PawsSession(tidyToys()),PawsSession(stuckGate())} {}
int Afternoon::friendship() const { return int(std::count(rewarded_.begin(),rewarded_.end(),true)); }
int Afternoon::coins() const {
    int balance=20*friendship();
    for(std::size_t i=0;i<shopItems.size();++i) if(owned_[i])balance-=shopItems[i].price;
    return balance;
}
bool Afternoon::owned(std::string_view item) const {
    for(std::size_t i=0;i<shopItems.size();++i) if(shopItems[i].id==item)return owned_[i];
    return false;
}
std::string_view Afternoon::placement(std::string_view item) const {
    for(std::size_t i=0;i<shopItems.size();++i) if(shopItems[i].id==item)return placements_[i];
    return {};
}
std::string Afternoon::purchaseBlockage(std::string_view id) const {
    const auto* item=shopItem(id);
    if(!item)return "That item is not in Pip's shop.";
    if(!shopOpen())return "Help Pip with his first request to open the shop.";
    if(owned(id))return "Already yours. You can move it without buying again.";
    if(coins()<item->price)return "You need "+std::to_string(item->price-coins())+" more coins.";
    return {};
}
std::string Afternoon::placementBlockage(std::string_view itemId,std::string_view spotId) const {
    const auto* item=shopItem(itemId);const auto* spot=gardenSpot(spotId);
    if(!item)return "Choose a garden furnishing.";
    if(!owned(itemId))return "Buy this furnishing before placing it.";
    if(!spot)return "Choose a garden spot.";
    if(!fits(*item,*spot))return "This furnishing does not fit this spot.";
    for(std::size_t i=0;i<placements_.size();++i)
        if(shopItems[i].id!=itemId&&placements_[i]==spotId)return std::string(shopItems[i].name)+" already occupies this spot.";
    return {};
}
bool Afternoon::endingEligible() const {
    return friendship()==3&&std::any_of(placements_.begin(),placements_.end(),[](const auto& p){return !p.empty();});
}
CommandResult Afternoon::dispatchGarden(const GardenCommand& command) {
    const auto& s=session().state();
    if(command.attempt!=s.attempt||command.revision!=s.revision)return {false,"That action belongs to an earlier view. Please try again."};
    if(!s.inGarden)return {false,"Return to the garden first."};
    if(s.revision>=std::numeric_limits<Revision>::max()-1)return {false,"This afternoon's command limit has been reached."};
    CommandResult success{true,command.action==GardenAction::Buy?"Already yours. Choose a spot for Pip.":command.action==GardenAction::Place?"A lovely spot for Pip.":"Pip feels at home."};
    if(command.action==GardenAction::RecordEnding) {
        if(endingSeen_)return {false,"This afternoon's moment is already remembered."};
        if(!endingEligible())return {false,"Help Pip with all three requests and place a garden comfort first."};
        endingSeen_=true;
    } else {
        if(command.action!=GardenAction::Buy&&command.action!=GardenAction::Place)return {false,"Unknown garden action."};
        auto reason=command.action==GardenAction::Buy?purchaseBlockage(command.item):placementBlockage(command.item,command.spot);
        if(!reason.empty())return {false,std::move(reason)};
        std::size_t index=0;while(shopItems[index].id!=command.item)++index;
        if(command.action==GardenAction::Buy)owned_[index]=true;
        else {
            // Allocate before changing either placement or revision.
            std::string prepared=command.spot;placements_[index].swap(prepared);
        }
    }
    sessions_[active_].invalidateCommands(s.attempt);lastReward_=false;
    return success;
}
bool Afternoon::selectQuest(std::size_t i) {
    if(!unlocked(i)) return false;
    if(i==active_) {
        // Scene/menu changes invalidate queued revisions while preserving this attempt.
        sessions_[i].invalidateCommands(sessions_[i].state().attempt);lastReward_=false;return true;
    }
    for(const auto& s:sessions_) epoch_=std::max(epoch_,s.state().attempt);
    active_=i;sessions_[i].invalidateCommands(++epoch_);lastReward_=false;return true;
}
CommandResult Afternoon::dispatch(const Command& command) {
    // Resource limits protect both hostile import and pathological sessions.
    if(records_[active_].size()>=100000) return {false,"This afternoon's save is full."};
    // Allocate the journal entry before any gameplay/reward mutation. Committing
    // the moved record and the once-only ledger then cannot allocate.
    SolutionStep record{command.action,command.object,command.direction,command.target};
    auto& records=records_[active_];
    if(records.size()==records.capacity()) records.reserve(std::min<std::size_t>(100000,std::max<std::size_t>(16,records.capacity()*2)));
    auto result=sessions_[active_].dispatch(command);
    if(!result.accepted) return result;
    epoch_=std::max(epoch_,session().state().attempt);
    records.push_back(std::move(record));
    lastReward_=false;
    if(session().state().puzzle.completed&&!rewarded_[active_]) {
        rewarded_[active_]=true;lastReward_=true;
    }
    return result;
}
std::string Afternoon::encode() const {
    std::ostringstream out;
    out<<"PAWS_AFTERNOON 5 "<<active_<<' '<<settings.largerText<<' '<<settings.reducedMotion<<' '<<settings.muted
       <<' '<<settings.music<<' '<<settings.ambience<<' '<<settings.effects<<' '<<epoch_
       <<' '<<coins()<<' '<<friendship()<<' '<<rewarded_[0]<<' '<<rewarded_[1]<<' '<<rewarded_[2]<<'\n';
    out<<saveRevision<<' '<<std::quoted(saveSession)<<' '<<userPaused<<"\n";
    out<<settings.quality<<'\n';
    out<<owned_[0]<<' '<<owned_[1]<<' '<<std::quoted(placements_[0])<<' '<<std::quoted(placements_[1])<<' '<<endingSeen_<<'\n';
    for(std::size_t i=0;i<3;++i) {
        out<<std::quoted(sessions_[i].content().contentId)<<' '<<records_[i].size()
           <<' '<<sessions_[i].state().attempt<<' '<<sessions_[i].state().revision<<'\n';
        for(const auto& step:records_[i]) out<<int(step.action)<<' '<<std::quoted(step.object)<<' '<<int(step.direction)<<' '<<std::quoted(step.target)<<'\n';
    }
    return out.str();
}
Afternoon Afternoon::decode(std::string_view bytes) {
    if(bytes.size()>16*1024*1024) throw std::invalid_argument("Save is too large.");
    std::istringstream in{std::string(bytes)};std::string magic;int version=0,active=0;
    Afternoon candidate;
    auto& s=candidate.settings;
    if(!(in>>magic>>version>>active>>s.largerText>>s.reducedMotion>>s.muted>>s.music>>s.ambience>>s.effects)||
       magic!="PAWS_AFTERNOON"||(version<1||version>5)||active<0||active>=3||
       s.music<0||s.music>100||s.ambience<0||s.ambience>100||s.effects<0||s.effects>100)
        throw std::invalid_argument("Unsupported or damaged afternoon save.");
    unsigned savedEpoch=1;int savedCoins=0,savedFriendship=0;std::array<bool,3> ledger{};
    if(version>=2&&(!(in>>savedEpoch>>savedCoins>>savedFriendship>>ledger[0]>>ledger[1]>>ledger[2])||
       savedEpoch==0||savedEpoch>=std::numeric_limits<AttemptId>::max()||savedCoins<0||savedCoins>60||savedFriendship<0||savedFriendship>3))
        throw std::invalid_argument("Invalid saved reward ledger.");
    if(version>=4&&(!(in>>candidate.saveRevision>>std::quoted(candidate.saveSession)>>candidate.userPaused)||
        candidate.saveRevision>=std::numeric_limits<unsigned>::max()-1||candidate.saveSession.empty()||candidate.saveSession.size()>64||
        !std::all_of(candidate.saveSession.begin(),candidate.saveSession.end(),[](unsigned char c){return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='-';})))
        throw std::invalid_argument("Invalid saved session identity.");
    if(version>=5&&(!(in>>s.quality)||s.quality<0||s.quality>2))
        throw std::invalid_argument("Invalid saved display quality.");
    std::array<bool,2> owned{};std::array<std::string,2> placements{};bool ending=false;
    if(version>=3&&(!(in>>owned[0]>>owned[1]>>std::quoted(placements[0])>>std::quoted(placements[1])>>ending)||
        placements[0].size()>128||placements[1].size()>128))throw std::invalid_argument("Invalid saved furnishings.");
    for(std::size_t i=0;i<3;++i) {
        std::string content;unsigned count=0;
        if(!(in>>std::quoted(content)>>count)||content!=candidate.quest(i).content().contentId||count>100000||
           (count&&!candidate.unlocked(i))) throw std::invalid_argument("Invalid saved quest.");
        unsigned savedAttempt=1,savedRevision=0;
        if(version>=2&&(!(in>>savedAttempt>>savedRevision)||savedAttempt==0||savedAttempt>=std::numeric_limits<AttemptId>::max()||
           savedRevision<count||savedRevision>=std::numeric_limits<Revision>::max())) throw std::invalid_argument("Invalid saved attempt identity.");
        candidate.active_=i;
        for(unsigned n=0;n<count;++n) {
            int action=0,direction=0;std::string object,target;
            if(!(in>>action>>std::quoted(object)>>direction>>std::quoted(target))||action<0||action>int(Action::ReturnBall)||
               direction<0||direction>3||object.size()>128||target.size()>128) throw std::invalid_argument("Invalid saved command.");
            const auto& state=candidate.session().state();
            if(!candidate.dispatch({state.attempt,state.revision,Action(action),object,Direction(direction),target}).accepted)
                throw std::invalid_argument("Save contains an illegal puzzle action.");
        }
        if(version>=2) {
            if(savedAttempt<candidate.session().state().attempt) throw std::invalid_argument("Saved attempt predates its restart history.");
            candidate.sessions_[i].state_.attempt=savedAttempt;
            candidate.sessions_[i].state_.revision=savedRevision;
        }
        if(version>=2&&savedEpoch<candidate.session().state().attempt) throw std::invalid_argument("Saved command epoch predates its attempts.");
        savedEpoch=std::max(savedEpoch,candidate.session().state().attempt);
    }
    in>>std::ws;if(!in.eof()||!candidate.unlocked(std::size_t(active))) throw std::invalid_argument("Invalid save ending.");
    candidate.owned_=owned;
    for(std::size_t i=0;i<placements.size();++i) {
        if(owned[i]&&!candidate.shopOpen())throw std::invalid_argument("Saved purchase predates shop unlock.");
        if(!placements[i].empty()) {
            auto reason=candidate.placementBlockage(shopItems[i].id,placements[i]);
            if(!reason.empty())throw std::invalid_argument("Invalid saved placement: "+reason);
            candidate.placements_[i]=placements[i];
        }
    }
    if(candidate.coins()<0||(ending&&!candidate.endingEligible()))throw std::invalid_argument("Invalid saved purchases or ending.");
    candidate.endingSeen_=ending;
    if(version>=2&&(savedCoins!=candidate.coins()||savedFriendship!=candidate.friendship()||ledger!=candidate.rewarded_))
        throw std::invalid_argument("Reward ledger does not match committed puzzle history.");
    candidate.active_=std::size_t(active);candidate.epoch_=savedEpoch;candidate.lastReward_=false;
    return candidate;
}
void Afternoon::invalidateImportedCommands(const Afternoon& previous) {
    AttemptId epoch=std::max(epoch_,previous.epoch_);
    for(const auto& s:previous.sessions_)epoch=std::max(epoch,s.state().attempt);
    for(const auto& s:sessions_)epoch=std::max(epoch,s.state().attempt);
    if(epoch>=std::numeric_limits<AttemptId>::max()-4)throw std::invalid_argument("Save identity limit reached.");
    for(auto& s:sessions_)s.invalidateCommands(++epoch);
    epoch_=epoch;saveRevision=std::max(saveRevision,previous.saveRevision);
}
}
