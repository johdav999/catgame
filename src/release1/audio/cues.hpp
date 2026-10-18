#pragma once
#ifdef PAWS_RELEASE1_AUDIO_PRODUCTION
#include "game_audio.hpp"
#else
#include "runtime/game_audio.hpp" // Generated shared-owner patch fixture only.
#endif
#include "release1/contracts.hpp"
#include <set>
#include <tuple>

namespace paws::release1::audio {
struct Scope {
    std::uint64_t generation=0;
    AttemptId attempt=0;
    Revision revision=0;
    std::string quest;
};
// A projection of the authoritative owner, never a second command/rules owner.
class Cues {
public:
    explicit Cues(GameAudio& audio):audio_(audio) {}
    void observe(const Scope& current);
    bool committed(const CommittedEvent&, double ageSeconds=0);
    void suspend(bool,bool allowUi=false); // World pause; UI may remain active during user pause only.
    void music(bool puzzle, bool picnic=false);
private:
    GameAudio& audio_;
    Scope current_;
    bool observed_=false, suspended_=false;
    std::set<std::pair<std::string,std::string>> seen_;
};
}
