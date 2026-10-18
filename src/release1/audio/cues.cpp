#include "cues.hpp"
#include <algorithm>
#include <cmath>

namespace paws::release1::audio {
void Cues::observe(const Scope& s) {
    if(!observed_ || s.generation!=current_.generation || s.attempt!=current_.attempt || s.quest!=current_.quest) {
        audio_.sceneChanged();seen_.clear();
    } else if(s.revision!=current_.revision) {
        seen_.clear();
        // A newer arrangement invalidates an in-flight water presentation.
        audio_.cancel("water_flow");
    }
    current_=s;observed_=true;
}
bool Cues::committed(const CommittedEvent& e, double age) {
    if(!observed_ || suspended_ || !std::isfinite(age) || age<0 || age>.4 ||
       e.generation!=current_.generation || e.attempt!=current_.attempt ||
       e.revision!=current_.revision || e.quest!=current_.quest) return false;
    const auto family=e.cue=="picnic_complete" ? std::string_view("music_pip_at_home") : std::string_view(e.cue);
    if(std::none_of(audioClips.begin(),audioClips.end(),[&](auto& c){return c.family==family;})) return false;
    // Allow multiple effects of one commit (switches, gate, reward) exactly once.
    if(seen_.size()>=64 || !seen_.emplace(e.object,e.cue).second) return false;
    if(e.cue=="bowl_fill") audio_.cancel("water_flow");
    if(e.cue=="picnic_complete") audio_.music(family);
    else if(family.starts_with("music_") || family=="amb_garden_air" || family=="pip_purr" || family=="ball_roll") {
        seen_.erase({e.object,e.cue});return false; // Existing environment/use owner controls those loops.
    } else audio_.cue(family);
    return true;
}
void Cues::suspend(bool on,bool allowUi) {
    suspended_=on;
    if(on) audio_.cancel("water_flow");
#ifdef PAWS_RELEASE1_AUDIO_PRODUCTION
    audio_.suspend(on,allowUi);
#else
    (void)allowUi;audio_.suspend(on);
#endif
}
void Cues::music(bool puzzle, bool picnic) {
    audio_.music(picnic ? "music_pip_at_home" : puzzle ? "music_garden_puzzles" : "music_garden_afternoon");
}
}
