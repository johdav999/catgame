#pragma once
#include "release1/garden/presentation.hpp"
#include "pip_asset.hpp"
#include <cmath>

namespace paws {
struct PipSoundCues { bool step=false,settle=false,drink=false,purr=false; };
// Observes the already-simulated Pip presentation. Owns no scene references and
// cannot change puzzle rules, reservations, animation or rewards.
class PipSounds {
public:
    void reset(){*this={};}
    PipSoundCues advance(release1::garden::Phase phase,std::string_view item,
                         engine::Vec3 position,std::size_t segment,double simulationDelta,double stride=pipWalkStride) {
        using P=release1::garden::Phase;
        PipSoundCues out;
        const bool rest=item=="garden_cushion"||item=="hammock"||item=="pip_lookout"||item=="mabel_nook"||phase==P::RestEnter||phase==P::Resting||phase==P::RestLeave;
        out.purr=(phase==P::Use||phase==P::Resting)&&rest;
        // Pause preserves the last phase; resuming cannot replay contact sounds.
        if(!std::isfinite(simulationDelta)||simulationDelta<=0)return out;
        const bool entered=phase!=phase_||item!=item_;
        out.settle=rest&&entered&&(phase==P::Enter||phase==P::RestEnter||(phase==P::Use&&phase_!=P::Enter));
        out.drink=entered&&phase==P::Use&&item=="drinking_bowl";
        const bool moving=phase==P::Approach||phase==P::Return||phase==P::Wander||phase==P::WanderHome||phase==P::WelcomeWalk||phase==P::WelcomeHome||(phase==P::Attention&&segment!=1);
        const auto d=position-position_;
        const double travelled=std::sqrt(double(d.x)*d.x+double(d.z)*d.z);
        if(moving&&moving_&&simulationDelta<=.25&&std::isfinite(travelled)&&travelled<.25){
            distance_+=travelled;
            // Four contacts per cycle of the selected rig, not the legacy stride.
            const double contactDistance=std::isfinite(stride)&&stride>0?stride/4:pipWalkStride/4;
            if(distance_>=contactDistance){out.step=true;distance_=std::fmod(distance_,contactDistance);}
        }else distance_=0; // Scene teleports/long frames never cause a catch-up burst.
        moving_=moving;position_=position;phase_=phase;item_=item;
        return out;
    }
private:
    release1::garden::Phase phase_=release1::garden::Phase::Idle;
    std::string item_;
    engine::Vec3 position_{};
    double distance_=0;
    bool moving_=false;
};
}
