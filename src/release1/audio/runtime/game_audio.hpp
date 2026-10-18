#pragma once
#include "audio_catalog.hpp"
#include "afternoon.hpp"
#include "engine/audio.hpp"
#include <functional>
#include <map>

namespace paws {
// Paws policy over the engine backend. No scene pointers or browser types.
class GameAudio {
public:
    static constexpr std::size_t pcmBudget=48*1024*1024, clipBudget=24*1024*1024;
    explicit GameAudio(std::unique_ptr<engine::AudioBackend>);
    std::function<void(std::size_t,unsigned)> request;
    std::function<void(std::string_view)> event;
    engine::AudioSystem& system() { return system_; }
    unsigned generation() const { return generation_; }
    std::size_t pcmBytes() const;
    std::size_t peakPcmBytes() const { return peakPcm_; }
    std::size_t peakVoices() const { return peakVoices_; }
    void settings(const AfternoonSettings&);
    void activate();
    void suspend(bool);
    void sceneChanged();
    void retry();
    bool publish(std::size_t,unsigned,std::shared_ptr<const engine::SoundAsset>);
    void failed(std::size_t,unsigned);
    void cue(std::string_view); // Never grants game progress; late one-shots expire.
    void loop(std::string_view,bool); // Only bounded effect loops, or the ambience bed.
    void cancel(std::string_view family);
    void music(std::string_view);
    void update(double);
private:
    struct Slot { std::shared_ptr<const engine::SoundAsset> sound; bool pending=false,failed=false; };
    struct Playing { engine::AudioVoiceId voice; std::size_t clip; double age=0; };
    struct Pending { std::size_t clip; double expires; };
    engine::AudioSystem system_;
    std::array<engine::AudioBusId,3> buses_;
    std::array<Slot,audioClips.size()> slots_;
    std::vector<Playing> playing_;
    std::vector<Pending> pending_;
    std::map<std::string,double> last_;
    std::map<std::string,unsigned> variants_;
    std::map<std::string,bool> loops_;
    std::map<std::string,bool> exhausted_;
    std::string music_,nextMusic_;
    unsigned generation_=1;
    double clock_=0,musicFade_=0;
    bool enabled_=false,suspended_=false,muted_=false;
    std::size_t peakPcm_=0,peakVoices_=0;
    void load(std::size_t);
    bool play(std::size_t);
    void stopFamily(std::string_view);
    void clearMusic();
};
}
