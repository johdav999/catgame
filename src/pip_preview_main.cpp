#include "pip_asset.hpp"
#include "engine/application.hpp"
#include "engine/asset_browser.hpp"
#include "engine/glb_importer.hpp"
#include "engine/orbit_controller.hpp"
#include "engine/renderer.hpp"
#include <emscripten.h>
#include <memory>
#include <algorithm>
#include <cmath>

namespace {
void status(const char* state,const char* message) {
    EM_ASM({window.pipStatus(UTF8ToString($0),UTF8ToString($1));},state,message);
}
class Preview {
public:
    Preview() {
        app_.assets().registerAsset<engine::Mesh>(engine::AssetId{std::string(paws::pipAssetId)},
            engine::browserAssetLoader(std::string(paws::pipAssetUrl),
                [](std::span<const std::byte> bytes,const engine::AssetLoadContext& context) {
                    auto mesh=engine::GlbImporter::loadMemory(bytes);
                    paws::validatePip(*mesh);context.complete<engine::Mesh>(std::move(mesh));
                }));
        retry();
    }
    void retry() {
        if(renderer_) return;
        request_.cancel();request_={};app_.assets().collectUnused();
        request_=app_.assets().load<engine::Mesh>(engine::AssetId{std::string(paws::pipAssetId)});
        waiting_=true;status("loading","Loading Pip…");
    }
    void frame(double seconds) {
        // Application drives asset publication and animation exactly once.
        const auto updateStart=emscripten_get_now();
        engine::ApplicationFrameCallbacks callbacks;
        callbacks.simulate=[this](engine::Scene&,const engine::ApplicationFrame& frame) {
            if(moving_) {
                distance_+=paws::pipWalkSpeed*frame.simulation.delta;
                placeWalk();
            }
        };
        const auto& frame=app_.advance(seconds,callbacks);
        const auto updateMs=emscripten_get_now()-updateStart;
        if(waiting_) {
            const auto progress=request_.progress();
            if(progress.status==engine::AssetStatus::Failed) {waiting_=false;status("failed",progress.error.c_str());}
            else if(progress.status==engine::AssetStatus::Ready) {
                waiting_=false;
                try {
                    auto preview=paws::makePipPreview(request_.get<engine::Mesh>());
                    auto* scene=preview.scene.scene.get();auto* camera=preview.scene.camera;
                    auto orbit=std::make_unique<engine::OrbitController>(*camera);
                    auto renderer=std::make_unique<engine::Renderer>(*scene,*camera);
                    const engine::RenderView review{camera,std::array<float,4>{.56f,.63f,.53f,1}};
                    renderer->setViews(std::span{&review,1});
                    const auto id=app_.loadScene(std::move(preview.scene.scene));
                    cat_=preview.cat;contact_=preview.contact;orbit_=std::move(orbit);renderer_=std::move(renderer);
                    app_.start(id);view(false);renderer_->initialize();
                    status("ready","Pip loaded · choose an action");
                } catch(const std::exception& error) {status("failed",error.what());}
            }
        }
        if(renderer_) {renderer_->serviceRecovery();renderer_->resizeCanvas();if(renderer_->ready())renderer_->render(updateMs,frame.interval*1000);}
    }
    engine::SkeletalMeshComponent* cat() {
        auto* scene=app_.activeScene();
        return scene ? dynamic_cast<engine::SkeletalMeshComponent*>(scene->findComponent(cat_)) : nullptr;
    }
    void action(int index,bool reduced) {
        if(index<0 || index>=int(paws::pipActions.size())) return;
        moving_=false;distance_=0;placeWalk();
        if(auto* c=cat())paws::presentPip(*c,index,reduced);
    }
    void placeWalk() {
        const float z=moving_?float(std::fmod(distance_,1.8)-.9):0;
        if(auto* c=cat())if(auto* actor=c->owner())actor->transform.position.z=z;
        if(auto* scene=app_.activeScene())if(auto* contact=scene->findComponent(contact_))
            if(auto* actor=contact->owner())actor->transform.position.z=z;
    }
    void walkReview(bool moving,double sample) {
        if(auto* c=cat()) {
            moving_=moving;distance_=std::max(0.0,sample)*paws::pipWalkSpeed;
            c->animation().play(paws::pipClip(*c->mesh,"garden_walk"));
            if(sample>=0){c->animation().seek(std::clamp(sample,0.0,double(paws::pipWalkDuration)));c->animation().pause();moving_=false;}
            placeWalk();
        }
    }
    void reviewView(int angle) {
        if(!orbit_)return;
        auto state=orbit_->snapshot();state.target=state.resetTarget={0,.48f,0};
        state.yaw=angle==0?engine::pi/2:angle==1?0:angle==2?engine::pi:.55f;
        state.elevation=angle==4?.8f:.13f;state.distance=angle==4?6.f:moving_?4.8f:2.8f;
        orbit_->restore(state);orbit_->orbit(0,0);
    }
    double time() {auto* c=cat();return c?c->animation().time():-1;}
    void blinkReview(double sample,int angle) {
        moving_=false;distance_=0;placeWalk();
        if(auto* c=cat()) {
            c->animation().play(paws::pipClip(*c->mesh,"relaxed_idle"));
            c->animation().seek(sample<0?2.25:std::clamp(sample,2.25,2.9));
            if(sample>=0)c->animation().pause();
        }
        if(!orbit_)return;
        auto state=orbit_->snapshot();state.target=state.resetTarget={.082f,.756f,.540f};
        state.yaw=.70f+(angle==1?-.85f:angle==2?.85f:0.f);
        state.elevation=.03f;state.distance=angle==3?2.4f:.60f;
        orbit_->restore(state);orbit_->orbit(0,0);
    }
    void orbit(float x,float y) {if(orbit_)orbit_->orbit(x,y);}
    void zoom(float d) {
        if(!orbit_)return;
        auto state=orbit_->snapshot();state.distance=std::clamp(state.distance*std::exp(d*.001f),1.7f,9.f);
        orbit_->restore(state);orbit_->orbit(0,0);
    }
    void suspend(bool value) {app_.setSuspended(value);}
    void shadows(bool value) {
        if(renderer_)renderer_->setShadowsEnabled(value);
        if(auto* scene=app_.activeScene())if(auto* contact=scene->findComponent(contact_))contact->setEnabled(!value);
    }
    void view(bool wide) {
        if(!orbit_)return;
        auto state=orbit_->snapshot();state.target=state.resetTarget={0,.48f,0};state.yaw=.55f;state.elevation=.35f;state.distance=wide?6.f:2.4f;
        orbit_->restore(state);orbit_->orbit(0,0);
    }
private:
    engine::Application app_;
    engine::AssetRequest request_;
    engine::ComponentId cat_,contact_;
    std::unique_ptr<engine::OrbitController> orbit_;
    std::unique_ptr<engine::Renderer> renderer_;
    bool waiting_=false;
    bool moving_=false;
    double distance_=0;
};
std::unique_ptr<Preview> preview;
void frame(void*) {preview->frame(emscripten_get_now()/1000);}
}
extern "C" {
EMSCRIPTEN_KEEPALIVE void pip_action(int index,int reduced) {if(preview)preview->action(index,reduced!=0);}
EMSCRIPTEN_KEEPALIVE void pip_retry() {if(preview)preview->retry();}
EMSCRIPTEN_KEEPALIVE void pip_view(int wide) {if(preview)preview->view(wide!=0);}
EMSCRIPTEN_KEEPALIVE void pip_orbit(float x,float y) {if(preview)preview->orbit(x,y);}
EMSCRIPTEN_KEEPALIVE void pip_zoom(float d) {if(preview)preview->zoom(d);}
EMSCRIPTEN_KEEPALIVE void pip_shadows(int value) {if(preview)preview->shadows(value!=0);}
EMSCRIPTEN_KEEPALIVE void pip_suspend(int value) {if(preview)preview->suspend(value!=0);}
EMSCRIPTEN_KEEPALIVE double pip_time() {return preview?preview->time():-1;}
EMSCRIPTEN_KEEPALIVE void pip_walk_review(int moving,double sample) {if(preview)preview->walkReview(moving!=0,sample);}
EMSCRIPTEN_KEEPALIVE void pip_review_view(int angle) {if(preview)preview->reviewView(angle);}
EMSCRIPTEN_KEEPALIVE void pip_blink_review(double sample,int angle) {if(preview)preview->blinkReview(sample,angle);}
}
int main() {preview=std::make_unique<Preview>();emscripten_set_main_loop_arg(frame,nullptr,0,true);}
