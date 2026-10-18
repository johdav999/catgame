#pragma once
#include "garden_kit.hpp"
#include "engine/scene.hpp"
#include "engine/skeletal_mesh_component.hpp"

namespace paws {
std::span<const GardenAsset> questRewardAssets();
// Presentation only. The caller supplies an already granted reward receipt.
class QuestRewards {
public:
    enum class Phase { Idle, Coin, Heart, Finished };
    enum Cue : unsigned { None=0, CoinSound=1, PurrSound=2 };
    static constexpr double coinSeconds=1.2,heartSeconds=2.2;
    void install(engine::Scene&,const GardenMeshes&,const std::vector<engine::ActorId>&);
    void start(engine::Scene&,bool reducedMotion);
    unsigned advance(engine::Scene&,double delta,engine::Vec3 eye,bool reducedMotion);
    void cancel(engine::Scene&);
    bool active() const {return phase_==Phase::Coin||phase_==Phase::Heart;}
    Phase phase() const {return phase_;}
    double elapsed() const {return elapsed_;}
    std::size_t recipients() const {return targets_.size();}
    std::vector<engine::Vec3> origins(const engine::Scene&) const;
private:
    struct Target {engine::ActorId cat,coin,heart;};
    std::vector<Target> targets_;
    Phase phase_=Phase::Idle;
    double elapsed_=0;
    unsigned emitted_=0;
    void pose(engine::Scene&,engine::Vec3 eye,bool reducedMotion);
};
} // namespace paws
