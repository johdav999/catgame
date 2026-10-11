#include "garden_kit.hpp"
#include "engine/application.hpp"
#include "engine/asset_browser.hpp"
#include "engine/glb_importer.hpp"
#include "engine/orbit_controller.hpp"
#include "engine/renderer.hpp"
#include <emscripten.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>
namespace {
void status(const char* value,const char* message){EM_ASM({window.gardenStatus(UTF8ToString($0),UTF8ToString($1));},value,message);}
class Preview {
public:
    Preview(){
        for(const auto& spec:paws::gardenAssets())app_.assets().registerAsset<engine::Mesh>(engine::AssetId{spec.id},engine::browserAssetLoader(spec.file,
            [spec](std::span<const std::byte> bytes,const engine::AssetLoadContext& context){auto mesh=engine::GlbImporter::loadMemory(bytes);paws::validateGardenAsset(spec,*mesh);context.complete<engine::Mesh>(std::move(mesh));}));
        app_.assets().registerAsset<engine::Mesh>(engine::AssetId{std::string(paws::pipAssetId)},engine::browserAssetLoader(std::string(paws::pipAssetUrl),
            [](std::span<const std::byte> bytes,const engine::AssetLoadContext& context){auto mesh=engine::GlbImporter::loadMemory(bytes);paws::validatePip(*mesh);context.complete<engine::Mesh>(std::move(mesh));}));
        retry();
    }
    void retry(){
        if(renderer_)return;
        requests_.clear();app_.assets().collectUnused();
        for(const auto& s:paws::gardenAssets())requests_.push_back(app_.assets().load<engine::Mesh>(engine::AssetId{s.id}));
        requests_.push_back(app_.assets().load<engine::Mesh>(engine::AssetId{std::string(paws::pipAssetId)}));
        waiting_=true;status("loading","Loading garden models…");
    }
    void frame(double seconds){
        const auto start=emscripten_get_now();const auto& frame=app_.advance(seconds);
        if(waiting_){
            bool ready=true;
            for(const auto& request:requests_){const auto p=request.progress();if(p.status==engine::AssetStatus::Failed){waiting_=false;status("failed",p.error.c_str());break;}ready&=p.status==engine::AssetStatus::Ready;}
            if(waiting_&&ready){
                waiting_=false;
                try{
                    paws::GardenMeshes meshes;std::size_t i=0;
                    for(const auto& spec:paws::gardenAssets())meshes.emplace(spec.name,requests_[i++].get<engine::Mesh>());
                    auto candidate=paws::makeGardenPreview(meshes,requests_.back().get<engine::Mesh>());
                    auto* scene=candidate.scene.scene.get();auto* camera=candidate.scene.camera;
                    auto orbit=std::make_unique<engine::OrbitController>(*camera);auto renderer=std::make_unique<engine::Renderer>(*scene,*camera);
                    const engine::RenderView view{camera,std::array<float,4>{.56f,.63f,.53f,1}};renderer->setViews(std::span{&view,1});
                    engine::RenderQuality quality;quality.resolution={1,1,1920,921600};quality.maxShadowMaps=1;renderer->setQuality(quality);
                    const auto id=app_.loadScene(std::move(candidate.scene.scene));bindings_=std::move(candidate);orbit_=std::move(orbit);renderer_=std::move(renderer);
                    app_.start(id);fit(0,aspect_);renderer_->initialize();status("ready","21 garden models loaded · inspect the assemblies");
                }catch(const std::exception& e){status("failed",e.what());}
            }
        }
        if(renderer_){renderer_->serviceRecovery();renderer_->resizeCanvas();if(renderer_->ready())renderer_->render(emscripten_get_now()-start,frame.interval*1000);}
    }
    void pose(int mode){if(auto* scene=app_.activeScene())if(mode>=0&&mode<=3){paws::poseGardenPreview(*scene,bindings_,mode);mode_=mode;}}
    int mode()const{return mode_;}
    void fit(int focus,float aspect){
        aspect_=std::max(.25f,aspect);focus_=focus;if(!orbit_)return;
        auto state=orbit_->snapshot();state.yaw=.5f;state.elevation=.78f;
        engine::Vec3 target{0,.15f,0};float width=6.8f,height=5.3f;
        if(focus==1){target={0,.12f,.0f};width=2.8f;height=1.8f;}
        if(focus==2){target={0,.16f,1.35f};width=2.25f;height=1.25f;}
        if(focus==3){target={1.6f,.4f,-1.1f};width=2.5f;height=2.3f;}
        if(focus==4){target={1.9f,.25f,.95f};width=2.4f;height=2.2f;}
        state.target=state.resetTarget=target;state.distance=std::max(height,width/aspect_)/(2*std::tan(35*engine::pi/360));
        orbit_->restore(state);orbit_->orbit(0,0);
    }
    void resize(float aspect){fit(focus_,aspect);}
    void orbit(float x,float y){if(orbit_)orbit_->orbit(x,y);}
    void zoom(float d){if(!orbit_)return;auto s=orbit_->snapshot();s.distance=std::clamp(s.distance*std::exp(d*.001f),1.5f,45.f);orbit_->restore(s);orbit_->orbit(0,0);}
    void shadows(bool enabled){if(renderer_)renderer_->setShadowsEnabled(enabled);}
    void suspend(bool value){app_.setSuspended(value);}
private:
    // Borrowed renderer/controller references die before Application and its scene.
    engine::Application app_;std::vector<engine::AssetRequest> requests_;paws::GardenPreview bindings_;
    std::unique_ptr<engine::OrbitController> orbit_;std::unique_ptr<engine::Renderer> renderer_;
    bool waiting_=false;float aspect_=1.6f;int focus_=0,mode_=0;
};
std::unique_ptr<Preview> preview;void frame(void*){preview->frame(emscripten_get_now()/1000);}
}
extern "C" {
EMSCRIPTEN_KEEPALIVE void garden_pose(int mode){if(preview)preview->pose(mode);}
EMSCRIPTEN_KEEPALIVE int garden_mode(){return preview?preview->mode():-1;}
EMSCRIPTEN_KEEPALIVE void garden_view(int focus,float aspect){if(preview)preview->fit(focus,aspect);}
EMSCRIPTEN_KEEPALIVE void garden_resize(float aspect){if(preview)preview->resize(aspect);}
EMSCRIPTEN_KEEPALIVE void garden_orbit(float x,float y){if(preview)preview->orbit(x,y);}
EMSCRIPTEN_KEEPALIVE void garden_zoom(float d){if(preview)preview->zoom(d);}
EMSCRIPTEN_KEEPALIVE void garden_shadows(int value){if(preview)preview->shadows(value!=0);}
EMSCRIPTEN_KEEPALIVE void garden_suspend(int value){if(preview)preview->suspend(value!=0);}
EMSCRIPTEN_KEEPALIVE void garden_retry(){if(preview)preview->retry();}
}
int main(){preview=std::make_unique<Preview>();emscripten_set_main_loop_arg(frame,nullptr,0,true);}
