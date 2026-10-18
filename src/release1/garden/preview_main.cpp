#include "presentation.hpp"
#include "contact_plans.hpp"
#include "release1/props/prop_assets.hpp"
#include "release1/cats/cat_assets.hpp"
#include "release1/progression/codec.hpp"
#include "engine/application.hpp"
#include "engine/asset_browser.hpp"
#include "engine/glb_importer.hpp"
#include "engine/orbit_controller.hpp"
#include "engine/renderer.hpp"
#include <emscripten.h>
#include <algorithm>
#include <cmath>
#include <sstream>

namespace g=paws::release1::garden;
namespace p=paws::release1::progression;
using paws::release1::Verb;
namespace {
struct CatInput {const char* cat;const char* id;const char* url;};
#include "inputs.inc"
using g::Assets;
#include "contacts.inc"
void status(const char* state,const std::string& message){EM_ASM({window.gardenStatus(UTF8ToString($0),UTF8ToString($1));},state,message.c_str());}
std::string quote(std::string_view v){std::string out="\"";for(char c:v){if(c=='\\'||c=='\"')out+='\\';if(c=='\n')out+="\\n";else out+=c;}return out+'"';}
class Preview {
public:
 Preview(){
  for(auto& spec:paws::gardenAssets())registerProp(spec);
  for(auto& spec:paws::release1::props::assets())registerProp(spec);
  for(std::size_t i=0;i<4;++i)if(*catInputs[i].url){auto spec=catInputs[i];app_.assets().registerAsset<engine::Mesh>(engine::AssetId{spec.id},engine::browserAssetLoader(spec.url,
   [i](std::span<const std::byte> bytes,const engine::AssetLoadContext& context){auto mesh=engine::GlbImporter::loadMemory(bytes);if(i==0&&std::string_view(catInputs[i].id)=="cats/pip@1")paws::validatePip(*mesh);else paws::release1::cats::validate(*mesh);context.complete<engine::Mesh>(std::move(mesh));}));}
  // Application owns binding rollback and releases renderer/camera borrows before
  // it destroys any outgoing scene. Build the replacement objects before swap.
  app_.setSceneBinding([this](engine::Scene* scene){
   if(!scene){renderer_.reset();orbit_.reset();return;}
   auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findSavedNode("garden.camera"));
   if(!camera)throw std::invalid_argument("Candidate has no garden camera");
   auto orbit=std::make_unique<engine::OrbitController>(*camera);auto renderer=std::make_unique<engine::Renderer>(*scene,*camera);
   const engine::RenderView view{camera,std::array<float,4>{.56f,.63f,.53f,1}};renderer->setViews(std::span{&view,1});
   engine::RenderQuality quality;quality.resolution={1,1,1920,921600};quality.maxShadowMaps=shadow_?1:0;renderer->setQuality(quality);
   renderer_=std::move(renderer);orbit_=std::move(orbit);renderer_->initialize();
  });
 }
 ~Preview(){app_.shutdown();}
 void registerProp(paws::GardenAsset spec){app_.assets().registerAsset<engine::Mesh>(engine::AssetId{spec.id},engine::browserAssetLoader(spec.file,
  [spec](std::span<const std::byte> bytes,const engine::AssetLoadContext& context){auto mesh=engine::GlbImporter::loadMemory(bytes);paws::validateGardenAsset(spec,*mesh);context.complete<engine::Mesh>(std::move(mesh));}));}
 void rebuild(){
  ++loadGeneration_;pendingGeneration_=loadGeneration_;requests_.clear();specs_.clear();app_.assets().collectUnused();
  for(auto& spec:paws::gardenAssets())load(spec.id,spec.name);
  for(auto& spec:paws::release1::props::assets())load(spec.id,spec.name);
  for(std::size_t i=0;i<4;++i)if(*catInputs[i].url)load(catInputs[i].id,std::string("cat.")+std::to_string(i));
  waiting_=true;status("loading",renderer_?"Preparing garden; the current scene remains visible…":"Preparing the garden…");
 }
 void load(std::string id,std::string key){specs_.push_back(std::move(key));requests_.push_back(app_.assets().load<engine::Mesh>(engine::AssetId{std::move(id)}));}
 void cancelLoading(){++loadGeneration_;waiting_=false;requests_.clear();status(renderer_?"ready":"idle","Garden preparation cancelled.");}
 void failCandidate(){failCandidate_=true;rebuild();}
 void frame(double seconds){
  engine::ApplicationFrameCallbacks callbacks;callbacks.simulate=[this](engine::Scene& scene,const engine::ApplicationFrame& f){presentation_.update(scene,f.simulation.delta);};
  const double start=emscripten_get_now();const auto& f=app_.advance(seconds,callbacks);
  if(waiting_){bool ready=true;for(auto& request:requests_){auto progress=request.progress();if(progress.status==engine::AssetStatus::Failed){waiting_=false;status("failed",progress.error);break;}ready&=progress.status==engine::AssetStatus::Ready;}
   if(waiting_&&ready&&pendingGeneration_==loadGeneration_){waiting_=false;engine::SceneId candidate;
    try{g::Assets assets;for(std::size_t i=0;i<specs_.size();++i){auto mesh=requests_[i].get<engine::Mesh>();auto& key=specs_[i];if(key.starts_with("cat.")){auto c=std::size_t(key.back()-'0');assets.cats[c].mesh=mesh;assets.cats[c].contacts=g::contactGuides(c);if(c==0&&std::string_view(catInputs[0].id)=="cats/pip@1")assets.cats[0]=g::legacyPip(mesh);}else assets.props.emplace(key,mesh);}
     measuredContacts(assets);if(failCandidate_){failCandidate_=false;assets.props.erase("hammock");}
     auto built=g::build(assets,owner_.progressView(),owner_.settings.reducedMotion);built.scene.camera->setSavedId("garden.camera");
     candidate=app_.loadScene(std::move(built.scene.scene));
     if(!app_.activeScene())app_.start(candidate);else{app_.requestTransition(candidate,true);if(!app_.commitTransition())throw std::runtime_error("Garden activation was not ready");}
     presentation_=std::move(built.presentation);
     for(auto& i:p::items)owner_.setItemAvailable(i.id,true); // All prop meshes actually imported.
     presentation_.project(*app_.activeScene(),owner_.progressView(),owner_.settings.reducedMotion);fit(focus_);applyPause();status("ready","Garden loaded. Cat/contact gates are listed below.");publish();
    }catch(const std::exception& e){if(candidate&&candidate!=app_.activeSceneId())app_.unloadScene(candidate);status("failed",e.what());}
   }
  }
  if(renderer_){renderer_->serviceRecovery();renderer_->resizeCanvas();if(renderer_->ready())renderer_->render(emscripten_get_now()-start,f.interval*1000);}
  if(++frames_%20==0)publish();
 }
 void project(){if(auto* scene=app_.activeScene())presentation_.project(*scene,owner_.progressView(),owner_.settings.reducedMotion);publish();}
 void save(){if(mode_==1||mode_==2){saveStatus_="Temporary session";return;}saveStatus_="Saving…";owner_.saveSession=session_;++owner_.saveRevision;const auto text=owner_.encode();EM_ASM({window.pawsStoreSave(UTF8ToString($0),$1,true,$1);},text.c_str(),owner_.saveRevision);}
 void action(int verb,int item,int spot){if(!activeInput())return;auto result=owner_.dispatchIntent(owner_.intent(verb==0?Verb::Buy:Verb::PlaceFurnishing,item>=0&&item<8?std::string(p::items[item].id):"",spot>=0&&spot<9?std::string(p::spots[spot].id):""));message_=result.message;if(result.accepted){project();save();}publish();}
 void placementPreview(int item,int spot){if(auto* scene=app_.activeScene()){if(item<0||item>=8||spot<0||spot>=9){presentation_.previewPlacement(*scene,{},{},false);return;}auto reason=owner_.placementBlockage(p::items[item].id,p::spots[spot].id);presentation_.previewPlacement(*scene,p::items[item].id,p::spots[spot].id,reason.empty());message_=reason.empty()?"Placement preview only. Confirm or cancel.":reason;publish();}}
 void use(int item,int cat){if(!activeInput()||item<0||item>=8||cat<0||cat>=4)return;std::string reason;auto ticket=presentation_.use(*app_.activeScene(),p::items[item].id,cat,presentation_.stamp(),reason);message_=ticket?"Authored furnishing use started.":reason;publish();}
 void cancelUse(){if(auto* s=app_.activeScene())presentation_.cancelAll(*s);message_="Furnishing use cancelled.";publish();}
 void gather(int item){if(!activeInput()||item<6||item>7)return;std::string why;auto tickets=presentation_.gather(*app_.activeScene(),p::items[item].id,presentation_.stamp(),why);message_=tickets.empty()?why:"Four distinct contact seats reserved.";publish();}
 void celebration(){if(!activeInput())return;auto result=presentation_.celebrationProps(*app_.activeScene(),presentation_.stamp(),true);message_=result?"Supplied picnic props displayed; ownership and rewards unchanged.":"The owner has not made the picnic eligible.";publish();}
 void meet(int cat){if(!activeInput()||cat<0||cat>=4)return;auto r=owner_.introduce(p::catIds[cat],owner_.intent(Verb::Select));message_=r.message;if(r.accepted){project();save();}publish();}
 void attention(int cat){if(activeInput()&&cat>=0&&cat<4)presentation_.attention(*app_.activeScene(),cat,presentation_.stamp());publish();}
 void quest(int q){if(!activeInput()||q<0||q>=10)return;auto r=owner_.dispatchIntent(owner_.intent(Verb::SelectQuest,{},std::string(p::quests[q].id)));message_=r.message;if(r.accepted){owner_.returnToGarden(owner_.intent(Verb::Select));project();save();}publish();}
 void nextTrace(){if(!activeInput())return;std::size_t q=0;while(q<10&&owner_.rewarded(q))++q;if(q==10){message_="All ten known traces already committed.";publish();return;}
  auto r=owner_.dispatchIntent(owner_.intent(Verb::SelectQuest,{},std::string(p::quests[q].id)));if(!r.accepted){message_=r.message;publish();return;}
  for(auto& step:paws::release1::rules::quest(p::quests[q].id).solution){auto intent=owner_.intent(step.verb,step.object,step.target);intent.direction=step.direction;intent.destination=step.destination;r=owner_.dispatchIntent(intent);if(!r.accepted){message_=r.message;publish();return;}}
  owner_.returnToGarden(owner_.intent(Verb::Select));message_="Known solution committed through the real owner: "+std::string(p::quests[q].id);project();save();
 }
 bool activeInput()const{return restored_&&app_.activeScene()&&!owner_.userPaused&&!suspended_&&!waiting_;}
 void paused(bool v){if(!restored_)return;auto r=owner_.setUserPaused(v,owner_.intent(Verb::Select));message_=r.message;applyPause();if(r.accepted)save();publish();}
 void applyPause(){if(!app_.activeScene())return;if(owner_.userPaused)app_.pause();else app_.resume();app_.setSuspended(suspended_);}
 void suspended(bool v){suspended_=v;owner_.setSuspended(v);app_.setSuspended(v);publish();}
 void reduced(bool v){owner_.settings.reducedMotion=v;project();save();}
 void shadow(bool v){shadow_=v;if(renderer_)renderer_->setShadowsEnabled(v);}
 void fit(int focus){focus_=focus;if(!orbit_)return;auto state=orbit_->snapshot();state.yaw=.5f;state.elevation=.8f;state.target=state.resetTarget={0,.2f,1};
  // Fit all eight corners with perspective depth, rather than treating rotated
  // world width as projected width (which clipped the garden's right edge).
  const engine::Vec3 back{std::sin(state.yaw)*std::cos(state.elevation),std::sin(state.elevation),std::cos(state.yaw)*std::cos(state.elevation)};
  const engine::Vec3 right{std::cos(state.yaw),0,-std::sin(state.yaw)},up{-std::sin(state.yaw)*std::sin(state.elevation),std::cos(state.elevation),-std::cos(state.yaw)*std::sin(state.elevation)};
  state.distance=0;const float lens=std::tan(35*engine::pi/360);
  for(float x:{-6.6f,6.6f})for(float z:{-4.8f,6.8f})for(float y:{0.f,1.8f}){auto d=engine::Vec3{x,y,z}-state.target;state.distance=std::max(state.distance,engine::dot(d,back)+1.08f*std::max(std::abs(engine::dot(d,right))/(lens*aspect_),std::abs(engine::dot(d,up))/lens));}
  if(focus>=1&&focus<=4){auto* s=app_.activeScene();auto* a=s->findActor(presentation_.cats()[focus-1].actor);state.target=state.resetTarget=a?a->transform.position+engine::Vec3{0,.45f,0}:g::home(focus-1);state.distance=3.4f;state.elevation=.38f;}
  else if(focus>=5&&focus<14){state.target=state.resetTarget=g::spots()[focus-5].position+engine::Vec3{0,.4f,0};state.distance=4.5f;state.elevation=.45f;}
  orbit_->restore(state);orbit_->orbit(0,0);
 }
 void resize(float aspect){aspect_=std::max(.25f,aspect);fit(focus_);}
 void orbit(float x,float y){if(orbit_)orbit_->orbit(x,y);}
 int restore(const char* bytes,int length,bool present){try{paws::Afternoon candidate=present?paws::Afternoon::decode(std::string_view(bytes,length)):paws::Afternoon{};if(restored_)candidate.invalidateImportedCommands(owner_);owner_=std::move(candidate);owner_.saveSession=session_;owner_.returnToGarden(owner_.intent(Verb::Select));restored_=true;rebuild();return 1;}catch(const std::exception& e){message_=e.what();publish();return 0;}}
 void storage(int mode,const char* session){mode_=mode;session_=session;if(mode)saveStatus_="Temporary session";}
 void saved(unsigned seq,bool ok){if(seq==owner_.saveRevision)saveStatus_=ok?"Saved":"Save failed. Retry.";publish();}
 void notice(const char* value){message_=value;publish();}
 void publish(){auto v=owner_.progressView();std::ostringstream out;out<<"{\"generation\":"<<v.generation<<",\"revision\":"<<v.revision<<",\"chapter\":"<<v.chapter<<",\"coins\":"<<v.coins<<",\"paused\":"<<(owner_.userPaused?"true":"false")<<",\"suspended\":"<<(suspended_?"true":"false")<<",\"sceneCount\":"<<app_.loadedSceneCount()<<",\"reduced\":"<<(owner_.settings.reducedMotion?"true":"false")<<",\"message\":"<<quote(message_)<<",\"save\":"<<quote(saveStatus_)<<",\"cats\":[";
  for(std::size_t i=0;i<4;++i){if(i)out<<',';const auto& c=presentation_.cats()[i];auto* s=app_.activeScene();auto* a=s?s->findActor(c.actor):nullptr;auto* mesh=s?dynamic_cast<engine::SkeletalMeshComponent*>(s->findComponent(c.mesh)):nullptr;
   out<<"{\"name\":"<<quote(p::catNames[i])<<",\"introduced\":"<<(v.cats[i].introduced?"true":"false")<<",\"available\":"<<(a?"true":"false")<<",\"phase\":"<<int(c.phase)<<",\"time\":"<<(mesh?mesh->animation().time():0)<<",\"x\":"<<(a?a->transform.position.x:0)<<",\"y\":"<<(a?a->transform.position.y:0)<<",\"z\":"<<(a?a->transform.position.z:0)<<",\"requests\":[";
   auto requests=presentation_.requests(i);for(std::size_t n=0;n<requests.size();++n){if(n)out<<',';out<<quote(requests[n].id);}out<<"],\"uses\":[";for(std::size_t n=0;n<8;++n){if(n)out<<',';out<<quote(presentation_.useBlockage(p::items[n].id,i));}out<<"]}";
  }out<<"],\"items\":[";for(std::size_t i=0;i<v.items.size();++i){if(i)out<<',';const auto& it=v.items[i];out<<"{\"name\":"<<quote(it.name)<<",\"owned\":"<<(it.owned?"true":"false")<<",\"spot\":"<<quote(it.spot)<<",\"reason\":"<<quote(it.reason)<<"}";}out<<"]}";
  const auto text=out.str();EM_ASM({window.gardenProjection(JSON.parse(UTF8ToString($0)));},text.c_str());
 }
private:
 // Destruction order and binding callback ensure borrows end before Application.
 engine::Application app_;paws::Afternoon owner_;g::Presentation presentation_;
 std::vector<engine::AssetRequest> requests_;std::vector<std::string> specs_;
 std::unique_ptr<engine::OrbitController> orbit_;std::unique_ptr<engine::Renderer> renderer_;
 std::uint64_t loadGeneration_=0,pendingGeneration_=0,frames_=0;
 bool waiting_=false,restored_=false,suspended_=false,shadow_=true,failCandidate_=false;int focus_=0,mode_=0;float aspect_=1.6f;
 std::string message_,session_="garden-harness",saveStatus_="Ready";
};
std::unique_ptr<Preview> preview;void frame(void*){preview->frame(emscripten_get_now()/1000);}
}
extern "C" {
EMSCRIPTEN_KEEPALIVE void garden_rebuild(){preview->rebuild();}
EMSCRIPTEN_KEEPALIVE void garden_cancel_loading(){preview->cancelLoading();}
EMSCRIPTEN_KEEPALIVE void garden_fail_candidate(){preview->failCandidate();}
EMSCRIPTEN_KEEPALIVE void garden_action(int verb,int item,int spot){preview->action(verb,item,spot);}
EMSCRIPTEN_KEEPALIVE void garden_preview(int item,int spot){preview->placementPreview(item,spot);}
EMSCRIPTEN_KEEPALIVE void garden_use(int item,int cat){preview->use(item,cat);}
EMSCRIPTEN_KEEPALIVE void garden_cancel_use(){preview->cancelUse();}
EMSCRIPTEN_KEEPALIVE void garden_gather(int item){preview->gather(item);}
EMSCRIPTEN_KEEPALIVE void garden_celebration(){preview->celebration();}
EMSCRIPTEN_KEEPALIVE void garden_meet(int cat){preview->meet(cat);}
EMSCRIPTEN_KEEPALIVE void garden_attention(int cat){preview->attention(cat);}
EMSCRIPTEN_KEEPALIVE void garden_quest(int q){preview->quest(q);}
EMSCRIPTEN_KEEPALIVE void garden_next_trace(){preview->nextTrace();}
EMSCRIPTEN_KEEPALIVE void garden_pause(int v){preview->paused(v!=0);}
EMSCRIPTEN_KEEPALIVE void garden_suspend(int v){preview->suspended(v!=0);}
EMSCRIPTEN_KEEPALIVE void garden_reduced(int v){preview->reduced(v!=0);}
EMSCRIPTEN_KEEPALIVE void garden_shadows(int v){preview->shadow(v!=0);}
EMSCRIPTEN_KEEPALIVE void garden_view(int v){preview->fit(v);}
EMSCRIPTEN_KEEPALIVE void garden_resize(float v){preview->resize(v);}
EMSCRIPTEN_KEEPALIVE void garden_orbit(float x,float y){preview->orbit(x,y);}
EMSCRIPTEN_KEEPALIVE int paws_restore(const char* p,int n,int exists){return preview->restore(p,n,exists!=0);}
EMSCRIPTEN_KEEPALIVE int paws_validate_save(const char* p,int n){try{return int(paws::Afternoon::decode(std::string_view(p,n)).saveRevision);}catch(...){return -1;}}
EMSCRIPTEN_KEEPALIVE void paws_storage_mode(int mode,const char* session,const char*,int){preview->storage(mode,session);}
EMSCRIPTEN_KEEPALIVE void paws_saved(unsigned seq,int ok){preview->saved(seq,ok!=0);}
EMSCRIPTEN_KEEPALIVE void paws_notice(const char* p){preview->notice(p);}
EMSCRIPTEN_KEEPALIVE void paws_retry_save(){preview->save();}
}
int main(){preview=std::make_unique<Preview>();EM_ASM({window.pawsLoadSave();});emscripten_set_main_loop_arg(frame,nullptr,0,true);}
