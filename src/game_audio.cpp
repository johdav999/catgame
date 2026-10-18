#include "game_audio.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace paws {
GameAudio::GameAudio(std::unique_ptr<engine::AudioBackend> backend,std::unique_ptr<engine::AudioBackend> uiBackend):system_(std::move(backend),14) {
    buses_={system_.createBus("Music"),system_.createBus("Ambience"),system_.createBus("Effects")};
    if(uiBackend){uiSystem_=std::make_unique<engine::AudioSystem>(std::move(uiBackend),2);uiBus_=uiSystem_->createBus("UI effects");}
    settings({});
}
void GameAudio::settings(const AfternoonSettings& s) {
    muted_=s.muted;system_.setBusMuted(system_.masterBus(),muted_);
    system_.setBusVolume(buses_[0],std::clamp(s.music,0,100)/100.f);
    system_.setBusVolume(buses_[1],std::clamp(s.ambience,0,100)/100.f);
    system_.setBusVolume(buses_[2],std::clamp(s.effects,0,100)/100.f);
    if(uiSystem_){uiSystem_->setBusMuted(uiSystem_->masterBus(),muted_);uiSystem_->setBusVolume(uiBus_,std::clamp(s.effects,0,100)/100.f);}
    if(muted_) pending_.clear();
}
void GameAudio::activate() {
    enabled_=true;system_.backend().activate();if(uiSystem_&&!uiSystem_->backend().active())uiSystem_->backend().activate();
    // Small cues preload after consent; full-length music loads only on demand.
    for(std::size_t i=0;i<slots_.size();++i)if(audioClips[i].bus!=0)load(i);
}
void GameAudio::preload(std::string_view family) {
    for(std::size_t i=0;i<audioClips.size();++i)if(audioClips[i].family==family)load(i);
}
void GameAudio::suspend(bool value,bool allowUi) {
    if(uiSystem_&&uiSuspended_!=(value&&!allowUi)){uiSuspended_=value&&!allowUi;uiSystem_->backend().suspend(uiSuspended_);}
    // Repeating suspend(false) at frame cadence cancels the browser's pending gesture resume.
    if(suspended_==value)return;
    suspended_=value;system_.backend().suspend(value);if(value)pending_.clear();
}
std::size_t GameAudio::pcmBytes() const { std::size_t bytes=0;for(const auto& s:slots_)if(s.sound)bytes+=s.sound->samples().size_bytes();return bytes; }
void GameAudio::sceneChanged() {
    ++generation_;
    std::erase_if(playing_,[&](auto& p){if(!playlist_.empty()&&audioClips[p.clip].bus==0)return false;system_.stop(p.voice);return true;});
    pending_.clear();loops_.clear();exhausted_.clear();last_.clear();
    // UI voices own no scene references; a pause/revision must not cut off its click.
    // In-flight decodes carry the old scene generation. Background music owns no scene references.
    for(std::size_t i=0;i<slots_.size();++i){slots_[i].pending=false;if(audioClips[i].bus==0&&playlist_.empty())slots_[i].sound.reset();}
    music_.clear();nextMusic_.clear();musicFade_=0;
}
void GameAudio::retry() { for(auto& slot:slots_)slot.failed=false; }
void GameAudio::load(std::size_t i) {
    auto& s=slots_[i];if(s.sound||s.pending||s.failed||!request)return;
    s.pending=true;request(i,generation_);
}
bool GameAudio::publish(std::size_t i,unsigned generation,std::shared_ptr<const engine::SoundAsset> sound) {
    if(i>=slots_.size()||generation!=generation_||!slots_[i].pending||!sound)return false;
    if(audioClips[i].bus==0&&(!playlist_.empty()?i!=playlist_[playlistPosition_]:audioClips[i].family!=music_)){slots_[i].pending=false;return false;}
    const auto bytes=sound->samples().size_bytes();
    if(bytes>(audioClips[i].bus==0?musicClipBudget:clipBudget)||pcmBytes()+bytes>(playlist_.empty()?pcmBudget:musicPcmBudget)){failed(i,generation);return false;}
    slots_[i]={std::move(sound),false,false};peakPcm_=std::max(peakPcm_,pcmBytes());return true;
}
void GameAudio::failed(std::size_t i,unsigned generation) { if(i<slots_.size()&&generation==generation_)slots_[i].pending=false,slots_[i].failed=true; }
bool GameAudio::play(std::size_t i) {
    if(uiClip(i)){
        if(!enabled_||muted_||uiSuspended_||!uiSystem_->backend().active()||!slots_[i].sound)return false;
        std::erase_if(uiPlaying_,[&](auto& p){return !uiSystem_->isPlaying(p.voice);});
        // Preserve meaningful feedback over decorative panel tails/clicks.
        // At equal priority, the newest response replaces the oldest voice.
        if(uiPlaying_.size()>=2){
            auto priority=[](std::size_t clip){const auto family=audioClips[clip].family;return family=="ui_panel"?0:family=="ui_confirm"||family=="ui_select"?1:2;};
            const auto oldest=std::min_element(uiPlaying_.begin(),uiPlaying_.end(),[&](auto& a,auto& b){return priority(a.clip)<priority(b.clip);});
            if(priority(i)<priority(oldest->clip))return false;
            uiSystem_->stop(oldest->voice);uiPlaying_.erase(oldest);
        }
        if(voiceCount()>=14)return false;
        engine::AudioPlayOptions options;options.bus=uiBus_;options.volume=audioClips[i].gain;
        const auto voice=uiSystem_->play(slots_[i].sound,options);if(!voice)return false;
        uiPlaying_.push_back({voice,i});peakVoices_=std::max(peakVoices_,voiceCount());return true;
    }
    if(!enabled_||suspended_||muted_||!system_.backend().active()||!slots_[i].sound)return false;
    if(voiceCount()>=14)return false;
    const auto& spec=audioClips[i];
    if(spec.bus==2&&std::count_if(playing_.begin(),playing_.end(),[](const auto& p){return audioClips[p.clip].bus==2;})>=12)return false;
    engine::AudioPlayOptions options;options.bus=buses_[spec.bus];options.volume=spec.gain;options.loop=spec.loop&&(spec.bus!=0||playlist_.empty());
    auto voice=system_.play(slots_[i].sound,options);if(!voice)return false;
    if(spec.bus==0){system_.setVoiceVolume(voice,0);system_.fadeVoice(voice,spec.gain,.25);}
    playing_.push_back({voice,i});peakVoices_=std::max(peakVoices_,voiceCount());return true;
}
void GameAudio::cue(std::string_view family) {
    const std::string key(family);std::vector<std::size_t> clips;
    for(std::size_t i=0;i<audioClips.size();++i)if(audioClips[i].family==family)clips.push_back(i);
    if(clips.empty())return;
    const double now=uiClip(clips[0])?uiClock_:clock_;
    if(last_.contains(key)&&now-last_[key]<audioClips[clips[0]].cooldown)return;
    last_[key]=now;if(event)event(family);
    if(!enabled_||muted_||(uiClip(clips[0])?(uiSuspended_||!uiSystem_->backend().active()):(suspended_||!system_.backend().active())))return;
    const auto i=clips[variants_[key]++%clips.size()];load(i);
    if(!play(i)&&slots_[i].pending&&pending_.size()<12)pending_.push_back({i,now+.4});
}
void GameAudio::cancel(std::string_view family) {
    stopFamily(family);loops_[std::string(family)]=false;
}
void GameAudio::stopFamily(std::string_view family) {
    if(uiSystem_)std::erase_if(uiPlaying_,[&](auto& p){if(audioClips[p.clip].family!=family)return false;uiSystem_->stop(p.voice);return true;});
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
    if(!playlist_.empty())return;
    if(nextMusic_==family)return;
    last_.erase(std::string(family));
    nextMusic_=family;musicFade_=.25;
    for(auto& p:playing_)if(audioClips[p.clip].bus==0)system_.stop(p.voice,.25);
}
void GameAudio::playlist(std::span<const std::string_view> tracks) {
    std::vector<std::size_t> candidate;
    for(auto id:tracks){
        const auto clip=std::find_if(audioClips.begin(),audioClips.end(),[&](auto& c){return c.id==id&&c.bus==0;});
        if(clip==audioClips.end())throw std::invalid_argument("Unknown playlist music clip");
        candidate.push_back(std::size_t(clip-audioClips.begin()));
    }
    if(candidate==playlist_)return;
    clearMusic();playlist_=std::move(candidate);playlistPosition_=0;
    music_.clear();nextMusic_.clear();musicFade_=0;
}
void GameAudio::update(double delta) {
    if(!std::isfinite(delta)||delta<0)return;
    if(uiSystem_){
        uiSystem_->update(delta);
        if(!uiSuspended_&&uiSystem_->backend().active())uiClock_+=delta;
        std::erase_if(uiPlaying_,[&](auto& p){return !uiSystem_->isPlaying(p.voice);});
        std::erase_if(pending_,[&](auto& p){return uiClip(p.clip)&&(uiClock_>p.expires||slots_[p.clip].failed||play(p.clip));});
    }
    if(suspended_)return;
    system_.update(delta);
    if(!enabled_||!system_.backend().active())return;
    // Scene replacement cancels initial decodes. Warm every small cue again
    // so requests/UI/contact effects meet the 400 ms playback window.
    for(std::size_t i=0;i<slots_.size();++i)if(audioClips[i].bus!=0)load(i);
    clock_+=delta;
    std::erase_if(playing_,[&](auto& p){p.age+=delta;const auto& s=audioClips[p.clip];
        if((s.family=="pip_purr"&&p.age>=5)||((s.family=="ball_roll"||s.family=="water_flow")&&p.age>=4)) {system_.stop(p.voice);loops_[std::string(s.family)]=false;exhausted_[std::string(s.family)]=true;}
        const bool ended=!system_.isPlaying(p.voice);
        if(ended&&!playlist_.empty()&&p.clip==playlist_[playlistPosition_])playlistPosition_=(playlistPosition_+1)%playlist_.size();
        return ended;});
    if(!playlist_.empty()){
        // Retain/decode only the selected track. Failed tracks are skipped without spinning when all fail.
        for(std::size_t n=0;n<playlist_.size()&&slots_[playlist_[playlistPosition_]].failed;++n)
            playlistPosition_=(playlistPosition_+1)%playlist_.size();
        const auto index=playlist_[playlistPosition_];
        for(std::size_t i=0;i<slots_.size();++i)if(audioClips[i].bus==0&&i!=index)slots_[i].sound.reset();
        if(!muted_&&!slots_[index].failed){
            load(index);
            if(std::none_of(playing_.begin(),playing_.end(),[&](auto& p){return p.clip==index;})&&play(index)&&event)event(audioClips[index].id);
        }
    }
    if(musicFade_>0){musicFade_-=delta;if(musicFade_<=0){clearMusic();music_=nextMusic_;if(event&&!music_.empty())event(music_);}}
    // Ending is a one-shot per entry, even while its card remains open.
    for(std::size_t i=0;i<slots_.size();++i){const auto& spec=audioClips[i];
        const bool desired=(playlist_.empty()&&spec.bus==0&&spec.family==music_&&musicFade_<=0)||loops_[std::string(spec.family)];
        if(!desired||muted_)continue;
        if(spec.bus==0&&!spec.loop&&last_.contains(std::string(spec.family)))continue;
        load(i);
        if(std::none_of(playing_.begin(),playing_.end(),[&](auto& p){return p.clip==i;})&&play(i)&&!spec.loop)last_[std::string(spec.family)]=clock_;
    }
    std::erase_if(pending_,[&](auto& p){return !uiClip(p.clip)&&(clock_>p.expires||slots_[p.clip].failed||play(p.clip));});
}
}
