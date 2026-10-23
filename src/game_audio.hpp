#pragma once
#include "audio_catalog.hpp"
#include "afternoon.hpp"
#include "engine/audio.hpp"
#include <functional>
#include <map>
#include <random>

namespace paws {
// Paws policy over the engine backend. No scene pointers or browser types.
class GameAudio {
public:
    static constexpr std::size_t pcmBudget=48*1024*1024, clipBudget=24*1024*1024;
    static constexpr std::size_t musicClipBudget=96*1024*1024, musicPcmBudget=128*1024*1024;
    explicit GameAudio(std::unique_ptr<engine::AudioBackend>,std::unique_ptr<engine::AudioBackend> uiBackend={});
    std::function<void(std::size_t,unsigned)> request;
    std::function<void(std::string_view)> event;
    engine::AudioSystem& system() { return system_; }
    unsigned generation() const { return generation_; }
    std::size_t pcmBytes() const;
    std::size_t peakPcmBytes() const { return peakPcm_; }
    std::size_t peakVoices() const { return peakVoices_; }
    std::size_t voiceCount() const { return system_.voiceCount()+(uiSystem_?uiSystem_->voiceCount():0); }
    bool uiActive() { return uiSystem_?uiSystem_->backend().active():system_.backend().active(); }
    void settings(const AfternoonSettings&);
    void activate();
    void preload(std::string_view family);
    void suspend(bool,bool allowUi=false);
    void sceneChanged();
    void retry();
    bool publish(std::size_t,unsigned,std::shared_ptr<const engine::SoundAsset>);
    void failed(std::size_t,unsigned);
    void cue(std::string_view); // Never grants game progress; late one-shots expire.
    void loop(std::string_view,bool); // Only bounded effect loops, or the ambience bed.
    void cancel(std::string_view family);
    void music(std::string_view);
    void playlist(std::span<const std::string_view>); // Ordered, repeating background music; survives scene changes.
    void beachWaves(bool); // Four non-overlapping Effects accents, only while at the beach.
    void update(double);
private:
    struct Slot { std::shared_ptr<const engine::SoundAsset> sound; bool pending=false,failed=false; };
    struct Playing { engine::AudioVoiceId voice; std::size_t clip; double age=0; };
    struct Pending { std::size_t clip; double expires; };
    engine::AudioSystem system_;
    std::unique_ptr<engine::AudioSystem> uiSystem_;
    engine::AudioBusId uiBus_;
    std::array<engine::AudioBusId,3> buses_;
    std::array<Slot,audioClips.size()> slots_;
    std::vector<Playing> playing_;
    std::vector<Playing> uiPlaying_;
    std::vector<Pending> pending_;
    std::map<std::string,double> last_;
    std::map<std::string,unsigned> variants_;
    std::map<std::string,bool> loops_;
    std::map<std::string,bool> exhausted_;
    std::string music_,nextMusic_;
    std::vector<std::size_t> playlist_;
    std::size_t playlistPosition_=0;
    std::mt19937 waveRandom_{std::random_device{}()};
    std::size_t lastWave_=audioClips.size(),nextWave_=audioClips.size();
    double waveDelay_=0;
    bool waves_=false,effectsAudible_=true;
    unsigned generation_=1;
    double clock_=0,uiClock_=0,musicFade_=0;
    bool enabled_=false,suspended_=false,muted_=false;
    bool uiSuspended_=false;
    std::size_t peakPcm_=0,peakVoices_=0;
    void load(std::size_t);
    bool play(std::size_t);
    bool uiClip(std::size_t i) const { return uiSystem_&&(audioClips[i].family.starts_with("ui_")||audioClips[i].family=="shop_purchase"); }
    void stopFamily(std::string_view);
    void clearMusic();
};
}
