#ifdef PAWS_RELEASE1_AUDIO_PRODUCTION
#include "game_audio.hpp"
#else
#include "release1/audio/runtime/game_audio.hpp"
#endif
#include <iostream>
#include <stdexcept>
#include <set>

using namespace paws;
void check(bool value,const char* text) {if(!value)throw std::runtime_error(text);}
struct Backend final:engine::AudioBackend {
    bool running=false;std::map<unsigned,float> voices;std::vector<std::pair<unsigned,float>> starts;
    bool start(unsigned id,const engine::SoundAsset&,const engine::AudioPlayOptions&,float gain) override {voices[id]=gain;starts.push_back({id,gain});return true;}
    void stop(unsigned id) noexcept override {voices.erase(id);}
    bool playing(unsigned id) const noexcept override{return voices.contains(id);}
    void gain(unsigned id,float gain) noexcept override {voices[id]=gain;}
    bool playbackRate(unsigned id,float) noexcept override {return voices.contains(id);}
    void position(unsigned,engine::Vec3) noexcept override{}
    void listener(const engine::AudioListenerPose&) noexcept override{}
    bool active() const noexcept override{return running;}
    void activate() noexcept override{running=true;}
    void suspend(bool on) noexcept override{if(on)running=false;}
};
int main(){try{
    auto backend=std::make_unique<Backend>();auto* device=backend.get();GameAudio audio(std::move(backend));
    const std::vector<float> pcm(480,.1f);auto sound=engine::SoundAsset::create(48000,1,pcm);
    std::vector<std::pair<std::size_t,unsigned>> loads;
    audio.request=[&](auto i,auto generation){loads.push_back({i,generation});};
    auto deliver=[&]{auto pending=std::move(loads);loads.clear();for(auto [i,g]:pending)audio.publish(i,g,sound);};
    audio.cue("pot_slide");check(device->voices.empty(),"No sound before activation");
    audio.activate();deliver();audio.update(1);audio.cue("pot_slide");check(device->voices.size()==1,"Accepted cue plays");
    audio.cue("pot_slide");check(device->voices.size()==1,"Cooldown prevents overlap spam");
    audio.update(.2);audio.cue("pot_slide");check(device->voices.size()==2,"Independent repeated voice");
    AfternoonSettings settings;settings.muted=true;audio.settings(settings);
    for(auto [id,gain]:device->voices)check(gain==0,"Master mute reaches live voices");
    audio.cue("channel_rotate");check(device->voices.size()==2,"Muted effects do not queue");
    settings.muted=false;settings.effects=0;audio.settings(settings);for(auto [id,gain]:device->voices)check(gain==0,"Effects bus zero independent of mute");
    settings.effects=80;audio.settings(settings);
    audio.sceneChanged();check(device->voices.empty(),"Scene change immediately releases voices");
    audio.loop("pip_purr",true);audio.update(.1);check(device->voices.size()==1,"Purr starts once");
    for(int i=0;i<60;++i){audio.loop("pip_purr",true);audio.update(.1);}check(device->voices.empty(),"Purr bounded even if caller retains request");
    audio.loop("pip_purr",false);audio.loop("pip_purr",true);audio.update(.1);check(device->voices.size()==1,"New use can purr");
    audio.suspend(true);audio.update(100);check(!device->active(),"Suspension freezes device");
    audio.suspend(false);check(!device->active(),"Visibility alone does not autoplay");audio.activate();
    audio.sceneChanged();
    for(int i=0;i<30;++i){audio.update(.2);audio.cue("pot_slide");}check(device->voices.size()==12,"Twelve effects maximum");
    audio.sceneChanged();audio.music("music_garden_afternoon");audio.update(.3);auto old=loads;loads.clear();
    audio.sceneChanged();for(auto [i,g]:old)check(!audio.publish(i,g,sound),"Stale decoded publication rejected");
    audio.music("music_garden_afternoon");audio.update(.3);deliver();audio.update(.1);check(device->voices.size()==1,"Garden music plays");
    audio.music("music_garden_puzzles");audio.update(.1);check(device->voices.size()==1,"Fade does not overlap music");
    audio.update(.2);deliver();audio.update(.1);check(device->voices.size()==1,"Music replacement releases old voice");
    audio.music("music_pip_at_home");audio.update(.3);deliver();audio.update(.1);device->voices.clear();audio.update(.1);check(device->voices.empty(),"Ending does not loop on open card");
    audio.music("music_garden_afternoon");audio.update(.3);deliver();audio.update(.1);
    audio.music("music_pip_at_home");audio.update(.3);deliver();audio.update(.1);check(device->voices.size()==1,"Explicit ending replay works");
    auto lateBackend=std::make_unique<Backend>();auto* lateDevice=lateBackend.get();GameAudio late(std::move(lateBackend));
    std::size_t pendingIndex=0;unsigned pendingGeneration=0;
    late.request=[&](auto i,auto g){if(audioClips[i].id=="pot_slide_01"){pendingIndex=i;pendingGeneration=g;}};
    late.activate();late.update(1);late.cue("pot_slide");late.update(1);
    check(late.publish(pendingIndex,pendingGeneration,sound),"Valid current decode accepted");late.update(.1);
    check(lateDevice->voices.empty(),"An expired effect never starts after slow decoding");
    const std::vector<float> oversized(GameAudio::clipBudget/sizeof(float)+1,.1f);
    check(!late.publish(pendingIndex+1,pendingGeneration,engine::SoundAsset::create(48000,1,oversized)),"Oversized pending decode rejected");
    check(audio.peakVoices()<=14&&audio.peakPcmBytes()<=GameAudio::pcmBudget,"Budget metrics bounded");
    std::cout<<"Audio activation, buses, cooldown, 12-effect budget, loops, music, stale scene generation passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
