#include "cues.hpp"
#include "engine/audio_browser.hpp"
#include <emscripten.h>
#include <memory>

namespace {
std::unique_ptr<paws::GameAudio> audio;
std::unique_ptr<paws::release1::audio::Cues> cues;
unsigned device=0;
std::uint64_t scene=1;
paws::Revision revision=1;
double previous=0;
bool enabled=false;
void scope() {
    const auto old=audio->generation();
    cues->observe({scene,1,revision,"audio-preview"});
    if(old!=audio->generation()) EM_ASM({PawsAudioHost.epoch($0);},audio->generation());
}
void frame() {
    const double now=emscripten_get_now()/1000;
    audio->update(previous ? now-previous : 0);previous=now;
    EM_ASM({window.audioPreview=({device:$0,voices:$1,pcmBytes:$2,peakVoices:$3,peakPcmBytes:$4});},
        device,audio->system().voiceCount(),audio->pcmBytes(),audio->peakVoices(),audio->peakPcmBytes());
}
}
extern "C" {
EMSCRIPTEN_KEEPALIVE void paws_audio_failed(int i,unsigned g) {audio->failed(i,g);}
EMSCRIPTEN_KEEPALIVE int paws_audio_publish(int i,unsigned g,unsigned rate,unsigned channels,const float* samples,unsigned count) {
    try{return audio->publish(i,g,engine::SoundAsset::create(rate,channels,std::span<const float>(samples,count)));}catch(...){audio->failed(i,g);return 0;}
}
EMSCRIPTEN_KEEPALIVE void paws_audio_gesture() {if(enabled)audio->activate();}
EMSCRIPTEN_KEEPALIVE void audio_enable() {enabled=true;audio->retry();audio->activate();}
EMSCRIPTEN_KEEPALIVE void audio_suspend(int on) {cues->suspend(on);}
EMSCRIPTEN_KEEPALIVE void audio_scene() {++scene;revision=1;scope();}
EMSCRIPTEN_KEEPALIVE void audio_settings(int music,int ambience,int effects,int muted) {
    paws::AfternoonSettings s;s.music=music;s.ambience=ambience;s.effects=effects;s.muted=muted;audio->settings(s);
}
EMSCRIPTEN_KEEPALIVE void audio_music(int mode) {cues->music(mode==1,mode==2);audio->loop("amb_garden_air",mode!=2);}
EMSCRIPTEN_KEEPALIVE int audio_cue(int index,int stale) {
    if(index<0 || index>=static_cast<int>(paws::audioClips.size())) return 0;
    ++revision;scope();
    return cues->committed({scene-(stale?1:0),1,revision,"audio-preview","object",std::string(paws::audioClips[index].family)});
}
}
int main() {
    auto backend=std::make_unique<engine::BrowserAudioBackend>();device=backend->deviceId();
    audio=std::make_unique<paws::GameAudio>(std::move(backend));
    cues=std::make_unique<paws::release1::audio::Cues>(*audio);
    audio->request=[](auto i,auto g){EM_ASM({PawsAudioHost.request($0,$1);},i,g);};
    audio->event=[](auto name){std::string text(name);EM_ASM({PawsAudioHost.event(UTF8ToString($0));},text.c_str());};
    EM_ASM({PawsAudioHost.init($0);},device);scope();
    emscripten_set_main_loop(frame,0,true);
}
