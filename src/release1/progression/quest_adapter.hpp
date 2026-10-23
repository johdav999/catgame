#pragma once
#include "release1/progression/policy.hpp"
#include "engine/quests/runtime.hpp"

namespace paws::release1::progression {
struct QuestFact {
    std::string id,content;
    AttemptId attempt=1;
    bool completed=false,earned=false,checkpoint=false;
};
using QuestFacts=std::array<QuestFact,questCount>;
struct QuestMigrationView {
    std::uint64_t comparisons=0;
    unsigned authoritative=questCount;
    bool matched=true;
    std::vector<std::string> newReceipts;
    std::map<std::string,std::string> stages;
};
// Immutable after publication; Afternoon copies share this projection until a
// committed command stages a replacement. No callbacks capture game/scene state.
class QuestAdapter {
public:
    static std::shared_ptr<const QuestAdapter> reconcile(const QuestFacts&,
        const std::shared_ptr<const QuestAdapter>& previous={},bool replay=false);
    bool earned(std::string_view id) const {return runtime_.view(id).earned;}
    bool completed(std::string_view id) const {return runtime_.view(id).status==engine::quests::Status::Completed;}
    bool available(std::string_view id) const {return runtime_.view(id).status!=engine::quests::Status::Locked;}
    const QuestMigrationView& diagnostics() const {return diagnostics_;}
    std::string saveSection(const QuestFacts&) const;
    // Compatibility projection for the released M5 reader only; never selects
    // a live authority. Its ID table lives beside the version-13 codec.
    std::string projectionRecord(const QuestFact&) const;
private:
    QuestAdapter();
    engine::quests::Runtime runtime_;
    QuestMigrationView diagnostics_;
};
}
