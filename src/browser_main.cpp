#include "runtime/game_session.hpp"
#include "engine/asset_browser.hpp"
#include "engine/audio_browser.hpp"
#include "engine/gui_browser.hpp"
#include "engine/renderer.hpp"
#include <emscripten.h>
#include <emscripten/html5.h>

namespace {
using namespace paws::runtime;
class BrowserHost final : public RuntimeHost {
public:
    BrowserHost() {
        beachPreview=EM_ASM_INT({const p=new URLSearchParams(location.search);return (['localhost','127.0.0.1','[::1]']).includes(location.hostname)&&p.get('devBeach')!=='0';});
        correctIdlePose=EM_ASM_INT({return new URLSearchParams(location.search).get('idlePose')!=='original';});
        gallery=EM_ASM_INT({return new URLSearchParams(location.search).get('gallery')==='1';});
        msaaOverride=EM_ASM_INT({const v=new URLSearchParams(location.search).get('msaa');return v==='off'?1:v==='4'?4:0;});
    }
    engine::Application& application() override {return app_;}
    engine::AssetLoader assetLoader(std::string url,std::function<void(std::span<const std::byte>,const engine::AssetLoadContext&)> decode) override {
        return engine::browserAssetLoader(std::move(url),std::move(decode));
    }
    std::unique_ptr<engine::AudioBackend> makeAudio(bool ui) override {
        auto b=std::make_unique<engine::BrowserAudioBackend>();devices_[ui]=b->deviceId();return b;
    }
    unsigned audioDevice(bool ui) const override {return devices_[ui];}
    std::string audioStatus(bool active,bool muted) override {
        char buffer[256];EM_ASM({const b=new TextEncoder().encode(window.PawsAudioHost.status(!!$0,!!$1));const n=Math.min(255,b.length);HEAPU8.set(b.subarray(0,n),$2);HEAPU8[$2+n]=0;},active,muted,buffer);return buffer;
    }
    void event(HostEvent event,std::initializer_list<HostValue> values) override {
        const auto* v=values.begin();switch(event) {
#include "runtime/browser_events.inc"
        }
    }
    void viewport(double* w,double* h) const override {emscripten_get_element_css_size("#viewport",w,h);}
    bool ready() const override {return renderer_&&renderer_->ready();}
    void bindScene(engine::SceneId id,engine::CameraComponent& camera) override {
        auto& scene=*app_.findScene(id);
        if(renderer_){const engine::RenderView views[]={{&camera,std::array<float,4>{.56f,.63f,.53f,1}}};renderer_->setScene(scene,views);}
        else {renderer_=std::make_unique<engine::Renderer>(scene,camera);renderer_->initialize();}
    }
    void attachGui(engine::GuiSystem* gui) override {if(renderer_)renderer_->setGui(gui);engine::attachBrowserGui(gui);}
    void syncGui(bool force) override {engine::syncBrowserGui(force);}
    void guiReady() override {EM_ASM({window.pawsGuiReady=true;window.pawsMaybeReady?.();});}
    void setQuality(const engine::RenderQuality& q) override {renderer_->setQuality(q);}
    void setWaterTime(double t) override {renderer_->setWaterTime(t);}
    double waterTime() const override {return renderer_->waterTime();}
    void releaseRendering() override {renderer_.reset();}
    void frame(GameSession& session) {
        const double started=emscripten_get_now();renderer_->serviceRecovery();renderer_->resizeCanvas();
        session.beforeFrame(started/1000);const auto& f=app_.advance(started/1000,session.callbacks());
        if(ready())renderer_->render(emscripten_get_now()-started,f.interval*1000);engine::syncBrowserGui();
    }
private:
    engine::Application app_;
    std::unique_ptr<engine::Renderer> renderer_;
    unsigned devices_[2]{};
};
BrowserHost host;
std::unique_ptr<GameSession> game;
void frame(void*) {host.frame(*game);}
}
extern "C" {
EMSCRIPTEN_KEEPALIVE void paws_audio_gesture() {if(game)game->audioGesture();}
EMSCRIPTEN_KEEPALIVE int paws_audio_publish(int index,unsigned generation,unsigned rate,unsigned channels,const float* samples,int count) {return game&&game->audioPublish(index,generation,rate,channels,samples,count);}
EMSCRIPTEN_KEEPALIVE void paws_audio_failed(int index,unsigned generation) {if(game)game->audioFailed(index,generation);}
EMSCRIPTEN_KEEPALIVE void paws_orbit(float dx,float dy) { if(game) game->drag(dx,dy); }
EMSCRIPTEN_KEEPALIVE void paws_zoom(float delta) { if(game) game->zoom(delta); }
EMSCRIPTEN_KEEPALIVE void paws_reset_view() { if(game) game->reset(); }
EMSCRIPTEN_KEEPALIVE void paws_suspend(int value) { if(game) game->suspend(value!=0); }
EMSCRIPTEN_KEEPALIVE void paws_resize(float aspect) { if(game) game->resize(aspect); }
EMSCRIPTEN_KEEPALIVE void paws_font(int index,const void* data,int bytes) { if(game) game->font(index,data,bytes); }
EMSCRIPTEN_KEEPALIVE void paws_escape() { if(game) game->escape(); }
EMSCRIPTEN_KEEPALIVE void paws_pointer(float x,float y,int down) { if(game) game->pointer(x,y,down!=0); }
EMSCRIPTEN_KEEPALIVE int paws_volume_pointer(int action,float x,float y) {return game&&game->volumePointer(action,x,y);}
EMSCRIPTEN_KEEPALIVE int paws_volume_key(int key) {return game&&game->volumeKey(key);}
EMSCRIPTEN_KEEPALIVE int paws_object_pointer(int phase,float x,float y) { return game?game->objectPointer(phase,x,y):0; }
EMSCRIPTEN_KEEPALIVE void paws_interaction_snapshot() { if(game) game->interactionSnapshot(); }
EMSCRIPTEN_KEEPALIVE void paws_water_snapshot() { if(game) game->waterSnapshot(); }
EMSCRIPTEN_KEEPALIVE void paws_terrain_snapshot() { if(game) game->terrainSnapshot(); }
EMSCRIPTEN_KEEPALIVE void paws_reward_snapshot() { if(game) game->rewardSnapshot(); }
EMSCRIPTEN_KEEPALIVE int paws_object_key(int key) { return game?game->objectKey(key):0; }
EMSCRIPTEN_KEEPALIVE int paws_blocks_scene() { return !game||game->blocked(); }
EMSCRIPTEN_KEEPALIVE void paws_motion(int reduced) { if(game) game->motion(reduced!=0); }
EMSCRIPTEN_KEEPALIVE int paws_restore(const char* data,int bytes,int status) { return game&&game->restore(data,bytes,status); }
EMSCRIPTEN_KEEPALIVE void paws_saved(unsigned sequence,int success) { if(game) game->saved(sequence,success!=0); }
EMSCRIPTEN_KEEPALIVE void paws_storage_mode(int mode,const char* session,const char* notice,int original) {if(game)game->storageMode(mode,session,notice,original!=0);}
EMSCRIPTEN_KEEPALIVE void paws_retry_save() {if(game)game->retrySave();}
EMSCRIPTEN_KEEPALIVE void paws_cloud_snapshot() {if(game)game->cloudSnapshot();}
EMSCRIPTEN_KEEPALIVE void paws_cloud_stamp() {if(game)game->cloudStamp();}
EMSCRIPTEN_KEEPALIVE void paws_cloud_beach_access(int value) {if(game)game->cloudBeachAccess(value!=0);}
EMSCRIPTEN_KEEPALIVE void paws_notice(const char* text) {if(game)game->notice(text);}
EMSCRIPTEN_KEEPALIVE double paws_validate_save(const char* data,int bytes) {
    if(bytes<0||bytes>16*1024*1024||(!data&&bytes))return -1;
    try {auto a=paws::Afternoon::decode(std::string_view(data,bytes));return double(a.saveRevision); }catch(...){return -1;}
}
EMSCRIPTEN_KEEPALIVE int paws_stage_import(const char* data,int bytes,unsigned attempt,unsigned revision) {return game&&game->stageImport(data,bytes,attempt,revision);}
EMSCRIPTEN_KEEPALIVE void paws_command(int action,int object,int direction,unsigned attempt,unsigned revision) {
    if(game) game->command(action,object,direction,attempt,revision);
}
}
int main() {
    game=std::make_unique<GameSession>(host); emscripten_set_main_loop_arg(frame,nullptr,0,true);
}
