#pragma once
#include "puzzle.hpp"
#include "furnishings.hpp"
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
    const PawsSession& session() const { return sessions_[active_]; }
    const PawsSession& quest(std::size_t i) const { return sessions_.at(i); }
    std::size_t active() const { return active_; }
    bool unlocked(std::size_t i) const { return i<3 && (i==0||rewarded_[i-1]); }
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
    AfternoonSettings settings;
    // Snapshot metadata is portable; browser locks and runtime handles are not.
    unsigned saveRevision=0;
    std::string saveSession="legacy";
    bool userPaused=false;
    void invalidateImportedCommands(const Afternoon& previous);
private:
    std::array<PawsSession,3> sessions_;
    std::array<std::vector<SolutionStep>,3> records_;
    std::array<bool,3> rewarded_{};
    std::size_t active_=0;
    AttemptId epoch_=1;
    bool lastReward_=false;
    std::array<bool,2> owned_{};
    std::array<std::string,2> placements_{};
    bool endingSeen_=false;
};
}
