#include "game_audio.hpp"
#include <algorithm>
#include <cmath>

namespace paws {
GameAudio::GameAudio(std::unique_ptr<engine::AudioBackend> backend):system_(std::move(backend),14) {
    buses_={system_.createBus("Music"),system_.createBus("Ambience"),system_.createBus("Effects")};
    settings({});
}
void GameAudio::settings(const AfternoonSettings& s) {
    muted_=s.muted;system_.setBusMuted(system_.masterBus(),muted_);
    system_.setBusVolume(buses_[0],std::clamp(s.music,0,100)/100.f);
    system_.setBusVolume(buses_[1],std::clamp(s.ambience,0,100)/100.f);
    system_.setBusVolume(buses_[2],std::clamp(s.effects,0,100)/100.f);
    if(muted_) pending_.clear();
}
void GameAudio::activate() {
    enabled_=true;system_.backend().activate();
    // Small cues preload after consent; full-length music loads only on demand.
    for(std::size_t i=0;i<slots_.size();++i)if(audioClips[i].bus!=0)load(i);
}
void GameAudio::suspend(bool value) { suspended_=value;system_.backend().suspend(value);if(value)pending_.clear(); }
std::size_t GameAudio::pcmBytes() const { std::size_t bytes=0;for(const auto& s:slots_)if(s.sound)bytes+=s.sound->samples().size_bytes();return bytes; }
void GameAudio::sceneChanged() {
    ++generation_;system_.stopAll();playing_.clear();pending_.clear();loops_.clear();exhausted_.clear();last_.clear();
    // In-flight decodes carry the old scene generation. Retain immutable effects only.
    for(std::size_t i=0;i<slots_.size();++i){slots_[i].pending=false;if(audioClips[i].bus==0)slots_[i].sound.reset();}
    music_.clear();nextMusic_.clear();musicFade_=0;
}
void GameAudio::retry() { for(auto& slot:slots_)slot.failed=false; }
void GameAudio::load(std::size_t i) {
    auto& s=slots_[i];if(s.sound||s.pending||s.failed||!request)return;
    s.pending=true;request(i,generation_);
}
bool GameAudio::publish(std::size_t i,unsigned generation,std::shared_ptr<const engine::SoundAsset> sound) {
    if(i>=slots_.size()||generation!=generation_||!slots_[i].pending||!sound)return false;
    if(audioClips[i].bus==0&&audioClips[i].family!=music_){slots_[i].pending=false;return false;}
    const auto bytes=sound->samples().size_bytes();
    if(bytes>clipBudget||pcmBytes()+bytes>pcmBudget){failed(i,generation);return false;}
    slots_[i]={std::move(sound),false,false};peakPcm_=std::max(peakPcm_,pcmBytes());return true;
}
void GameAudio::failed(std::size_t i,unsigned generation) { if(i<slots_.size()&&generation==generation_)slots_[i].pending=false,slots_[i].failed=true; }
bool GameAudio::play(std::size_t i) {
    if(!enabled_||suspended_||muted_||!system_.backend().active()||!slots_[i].sound)return false;
    const auto& spec=audioClips[i];
    if(spec.bus==2&&std::count_if(playing_.begin(),playing_.end(),[](const auto& p){return audioClips[p.clip].bus==2;})>=12)return false;
    engine::AudioPlayOptions options;options.bus=buses_[spec.bus];options.volume=spec.gain;options.loop=spec.loop;
    auto voice=system_.play(slots_[i].sound,options);if(!voice)return false;
    if(spec.bus==0){system_.setVoiceVolume(voice,0);system_.fadeVoice(voice,spec.gain,.25);}
    playing_.push_back({voice,i});peakVoices_=std::max(peakVoices_,playing_.size());return true;
}
void GameAudio::cue(std::string_view family) {
    const std::string key(family);std::vector<std::size_t> clips;
    for(std::size_t i=0;i<audioClips.size();++i)if(audioClips[i].family==family)clips.push_back(i);
    if(clips.empty())return;
    if(last_.contains(key)&&clock_-last_[key]<audioClips[clips[0]].cooldown)return;
    last_[key]=clock_;if(event)event(family);
    if(!enabled_||suspended_||muted_||!system_.backend().active())return;
    const auto i=clips[variants_[key]++%clips.size()];load(i);
    if(!play(i)&&slots_[i].pending&&pending_.size()<12)pending_.push_back({i,clock_+.4});
}
void GameAudio::stopFamily(std::string_view family) {
    std::erase_if(playing_,[&](auto& p){if(audioClips[p.clip].family!=family)return false;system_.stop(p.voice);return true;});
    std::erase_if(pending_,[&](auto& p){return audioClips[p.clip].family==family;});
}
void GameAudio::loop(std::string_view family,bool on) {
    const std::string key(family);
    if(!on)exhausted_[key]=false;
    if(on&&exhausted_[key])return;
    if(loops_[key]==on)return;
    loops_[key]=on;if(on){if(event)event(family);}else stopFamily(family);
}
void GameAudio::clearMusic() {
    for(auto& p:playing_)if(audioClips[p.clip].bus==0)system_.stop(p.voice);
    std::erase_if(playing_,[](auto& p){return audioClips[p.clip].bus==0;});
    for(std::size_t i=0;i<slots_.size();++i)if(audioClips[i].bus==0)slots_[i].sound.reset();
}
void GameAudio::music(std::string_view family) {
    if(nextMusic_==family)return;
    last_.erase(std::string(family));
    nextMusic_=family;musicFade_=.25;
    for(auto& p:playing_)if(audioClips[p.clip].bus==0)system_.stop(p.voice,.25);
}
void GameAudio::update(double delta) {
    if(!std::isfinite(delta)||delta<0||suspended_)return;
    system_.update(delta);
    if(!enabled_||!system_.backend().active())return;
    clock_+=delta;
    std::erase_if(playing_,[&](auto& p){p.age+=delta;const auto& s=audioClips[p.clip];
        if((s.family=="pip_purr"&&p.age>=5)||(s.family=="ball_roll"&&p.age>=4)) {system_.stop(p.voice);loops_[std::string(s.family)]=false;exhausted_[std::string(s.family)]=true;}
        return !system_.isPlaying(p.voice);});
    if(musicFade_>0){musicFade_-=delta;if(musicFade_<=0){clearMusic();music_=nextMusic_;if(event&&!music_.empty())event(music_);}}
    // Ending is a one-shot per entry, even while its card remains open.
    for(std::size_t i=0;i<slots_.size();++i){const auto& spec=audioClips[i];
        const bool desired=(spec.bus==0&&spec.family==music_&&musicFade_<=0)||loops_[std::string(spec.family)];
        if(!desired||muted_)continue;
        if(spec.bus==0&&!spec.loop&&last_.contains(std::string(spec.family)))continue;
        load(i);
        if(std::none_of(playing_.begin(),playing_.end(),[&](auto& p){return p.clip==i;})&&play(i)&&!spec.loop)last_[std::string(spec.family)]=clock_;
    }
    std::erase_if(pending_,[&](auto& p){return clock_>p.expires||slots_[p.clip].failed||play(p.clip);});
}
}
