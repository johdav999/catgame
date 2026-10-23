#pragma once
#include "puzzle.hpp"
#include "furnishings.hpp"
#include "release1/rules/content.hpp"
#include "release1/progression/policy.hpp"
#include "release1/progression/quest_adapter.hpp"
#include <array>
#include <string_view>

namespace paws {
struct AfternoonSettings {
    bool largerText=false,reducedMotion=false,muted=false;
    int music=70,ambience=60,effects=80;
    int quality=0; // 0 automatic, 1 mobile/shadow-free, 2 desktop
};
// Owns separate attempts and an append-only command record. Replay validates saves
// with exactly the same rules as play, including undo and reward idempotency.
class Afternoon {
public:
    Afternoon();
    Afternoon(const Afternoon&);
    Afternoon& operator=(const Afternoon&);
    Afternoon(Afternoon&&) noexcept=default;
    Afternoon& operator=(Afternoon&&) noexcept=default;
    // Release-one owner seam. No new wallet or browser persistence coordinator.
    release1::PuzzleView puzzleView() const;
    // Preflight complete drag transactions on copies, including pickup and every
    // intermediate slide. Destinations never mutate progress or saved selection.
    std::vector<release1::Intent> movementOptions(std::string_view object) const;
    release1::ProgressView progressView() const;
    std::vector<release1::SpotView> placementView(std::string_view item) const;
    release1::Intent intent(release1::Verb,std::string object={},std::string target={}) const;
    CommandResult dispatchIntent(const release1::Intent&);
    CommandResult returnToGarden(const release1::Intent&);
    CommandResult introduce(std::string_view cat,const release1::Intent&);
    std::vector<std::string> pendingIntroductions() const;
    void setItemAvailable(std::string_view,bool); // Verified presentation readiness; not persisted.
    void setSuspended(bool); // Host-only transient input cancellation, not user pause.
    CommandResult setUserPaused(bool,const release1::Intent&);
    bool inGarden() const { return garden_; }
    bool inBeach() const { return !garden_&&release1::progression::quests[activeQuest_].pack==release1::progression::beachPackId; }
    // Transient development access, never a purchase receipt or a saved entitlement.
    void setBeachDevelopmentAccess(bool value) { beachDevelopmentAccess_=value; }
    bool beachDevelopmentAccess() const { return beachDevelopmentAccess_; }
    bool beachComplete() const { return std::all_of(rewarded_.begin()+release1::progression::baseQuestCount,rewarded_.end(),[](bool b){return b;}); }
    bool beachPicnicComplete() const { return std::all_of(rewarded_.begin()+release1::progression::baseQuestCount,rewarded_.begin()+release1::progression::baseQuestCount+release1::progression::originalBeachQuestCount,[](bool b){return b;}); }
    bool beachEndingSeen() const { return beachEndingSeen_; }
    int chapter() const;
    int friendship(std::string_view cat) const;
    bool picnicEligible() const;
    bool picnicSeen() const { return picnicSeen_; }
    Revision ownerRevision() const { return ownerRevision_; }
    std::uint64_t generation() const { return generation_; }
    std::size_t questCount() const { return release1::progression::quests.size(); }
    const PawsSession& session() const { return sessions_.at(activeQuest_); } // Checked legacy-only projection; new quests use puzzleView().
    const PawsSession& quest(std::size_t i) const { return sessions_.at(i); }
    std::size_t active() const { return activeQuest_; }
    bool unlocked(std::size_t i) const;
    bool rewarded(std::size_t i) const { return rewarded_.at(i); }
    int friendship() const;
    int coins() const;
    bool shopOpen() const { return rewarded_[0]; }
    bool owned(std::string_view item) const;
    std::string_view placement(std::string_view item) const;
    std::string purchaseBlockage(std::string_view item) const;
    std::string placementBlockage(std::string_view item,std::string_view spot) const;
    enum class GardenAction { Buy, Place, RecordEnding };
    struct GardenCommand { AttemptId attempt; Revision revision; GardenAction action; std::string item,spot; };
    CommandResult dispatchGarden(const GardenCommand&);
    bool endingEligible() const;
    bool endingSeen() const { return endingSeen_; }
    bool lastRewardGranted() const { return lastReward_; }
    bool selectQuest(std::size_t);
    CommandResult dispatch(const Command&);
    std::string encode() const;
    static Afternoon decode(std::string_view);
    const release1::progression::QuestMigrationView& questMigrationView() const {return questAdapter_->diagnostics();}
    AfternoonSettings settings;
    // Snapshot metadata is portable; browser locks and runtime handles are not.
    unsigned saveRevision=0;
    std::string saveSession="legacy";
    bool userPaused=false;
    void invalidateImportedCommands(const Afternoon& previous);
    Afternoon restartedAdventure() const;
private:
    std::array<PawsSession,3> sessions_;
    std::array<std::vector<SolutionStep>,3> records_;
    std::array<bool,release1::progression::questCount> rewarded_{};
    std::size_t active_=0;
    AttemptId epoch_=1;
    bool lastReward_=false;
    std::array<bool,8> owned_{};
    std::array<std::string,8> placements_{};
    bool endingSeen_=false;
    std::array<std::unique_ptr<release1::rules::Attempt>,release1::progression::questCount-3> extra_;
    std::array<std::vector<release1::rules::Step>,release1::progression::questCount-3> extraRecords_;
    std::array<AttemptId,release1::progression::questCount-3> extraIds_=[] { std::array<AttemptId,release1::progression::questCount-3> ids; ids.fill(1); return ids; }();
    std::array<bool,4> introduced_{true,false,false,false};
    std::array<bool,8> available_{true,true,false,false,false,false,false,false};
    std::size_t activeQuest_=0;
    std::size_t homeQuest_=0; // Base request selected before travelling to an adventure.
    Revision ownerRevision_=0;
    std::uint64_t generation_=1;
    bool garden_=false,picnicSeen_=false,suspended_=false;
    bool beachDevelopmentAccess_=false;
    bool beachEndingSeen_=false;
    std::shared_ptr<const release1::progression::QuestAdapter> questAdapter_;
    bool replayingSave_=false;
    release1::progression::QuestFacts questFacts() const;
    void reconcileQuests(bool replay=false);
    void publishQuestCandidate(Afternoon&&) noexcept;
    CommandResult dispatchLegacyImpl(const Command&);
    CommandResult dispatchIntentImpl(const release1::Intent&);
    void initializeRelease();
    bool current(const release1::Intent&) const;
    bool canAdvance() const;
    bool itemUnlocked(std::size_t) const;
    AttemptId currentAttempt() const;
    int legacyFriendship() const;
    int legacyCoins() const;
    std::string encodeLegacy() const;
    static Afternoon decodeLegacy(std::string_view);
};
}
