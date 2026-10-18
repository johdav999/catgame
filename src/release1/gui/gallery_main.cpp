#include "views.hpp"
#include "release1/progression/owner.hpp"
#include "starter_scene.hpp"
#include "engine/application.hpp"
#include "engine/gui_browser.hpp"
#include "engine/orbit_controller.hpp"
#include "engine/renderer.hpp"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <iomanip>
#include <sstream>
#include <optional>
namespace {
using namespace paws;using namespace paws::release1;
std::string quote(std::string_view s){std::ostringstream o;o<<std::quoted(std::string(s));return o.str();}
class Harness {
public:
    ~Harness(){if(renderer)renderer->setGui(nullptr);engine::attachBrowserGui(nullptr);views.reset();ui.reset();renderer.reset();orbit.reset();}
    Harness(){auto scene=makeStarterScene();auto* borrowed=scene.scene.get();orbit=std::make_unique<engine::OrbitController>(*scene.camera);
        renderer=std::make_unique<engine::Renderer>(*borrowed,*scene.camera);auto id=app.loadScene(std::move(scene.scene));app.start(id);renderer->initialize();
        for(auto& item:progression::items)owner.setItemAvailable(item.id,true); // Explicit development readiness; no production art acceptance.
        owner.returnToGarden(owner.intent(Verb::Select));
    }
    void font(int index,const std::byte* bytes,std::size_t size){if(index<0||index>2||!size||size>4*1024*1024)return;fonts[index].assign(bytes,bytes+size);
        if(!fonts[0].empty()&&!fonts[1].empty()&&!fonts[2].empty()&&!ui){
            ui=std::make_unique<PawsUi>(fonts,false,[this](const UiIntent& i){legacy(i);});
            views=std::make_unique<gui::Views>(*ui,[this](const Intent& i){dispatch(i);},[this](std::string_view cat,const Intent& i){auto r=owner.introduce(cat,i);feedback=r.message;project();},[this](std::string_view item){placing=item;project();});
            project();ui->enter(UiScreen::Garden);EM_ASM({window.pawsGuiReady=true;window.maybeReady();});
        }
    }
    void project(){if(!views)return;gui::Model m;m.puzzle=owner.puzzleView();m.progress=owner.progressView();m.introductions=owner.pendingIntroductions();m.developmentReferences=true;m.rewardGranted=reward;
        m.services.inGarden=owner.inGarden();m.services.puzzleReady=true;m.services.canContinue=true;m.services.canExport=true;m.services.storageAvailable=true;m.services.save=saveState;
        m.services.storageNotice="Development harness: temporary memory. Export/import uses the real version-6 codec; production IndexedDB remains integration-owned.";
        m.services.feedback=feedback;m.services.audioStatus="Development harness plays quietly. Every clue is visible.";m.portraits=portraits;m.thumbnails=thumbnails;if(!placing.empty())m.placement=owner.placementView(placing);
        views->project(std::move(m));publish();
    }
    void dispatch(const Intent& i){
        const auto r=owner.dispatchIntent(i);feedback=r.message;
        if(r.accepted){if(i.verb==Verb::SelectQuest)ui->enter(UiScreen::Dialogue);
            else if(i.verb==Verb::RecordPicnic||i.verb==Verb::ReplayPicnic)ui->enter(UiScreen::Ending);
            else if(i.verb==Verb::PlaceFurnishing)ui->enter(UiScreen::Garden);
            else if(owner.puzzleView().completed&&i.verb!=Verb::Hint&&i.verb!=Verb::Select&&i.verb!=Verb::Buy){reward=owner.lastRewardGranted();ui->enter(UiScreen::Completion);}}
        project();
    }
    void legacy(const UiIntent& i){
        const auto current=owner.progressView();
        if(i.attempt!=current.attempt||i.revision!=current.revision){feedback="That control belongs to an earlier view.";project();return;}
        switch(i.action){
        case UiAction::ConfirmSound:case UiAction::EnableSound:break;
        case UiAction::BackToGarden:{auto r=owner.returnToGarden(owner.intent(Verb::Select));feedback=r.message;if(r.accepted)ui->enter(UiScreen::Garden);project();break;}
        case UiAction::Play:case UiAction::Continue:ui->enter(UiScreen::Garden);break;
        case UiAction::Restart:dispatch(owner.intent(Verb::Restart));ui->enter(UiScreen::Puzzle);break;
        case UiAction::Settings:{auto s=ui->settings();owner.settings={s.largerText,s.reducedMotion,s.muted,s.music,s.ambience,s.effects,s.quality};project();break;}
        case UiAction::ExportSave:{const auto bytes=owner.encode();EM_ASM({window.downloadSave(UTF8ToString($0));},bytes.c_str());break;}
        case UiAction::ImportSave:EM_ASM({window.chooseImport();});break;
        case UiAction::CancelImport:candidate.reset();break;
        case UiAction::CancelPlacement:placing.clear();project();break;
        case UiAction::ConfirmImport:if(candidate){candidate->invalidateImportedCommands(owner);owner=std::move(*candidate);candidate.reset();for(auto& item:progression::items)owner.setItemAvailable(item.id,true);const auto s=owner.settings;ui->restoreSettings({s.largerText,s.reducedMotion,s.muted,s.music,s.ambience,s.effects,s.quality});reward=false;ui->enter(owner.inGarden()?UiScreen::Garden:UiScreen::Puzzle);feedback="Imported afternoon ready.";project();}break;
        case UiAction::RetrySave:feedback="This isolated harness keeps temporary memory; export a copy. Production Save retry uses the existing host coordinator.";project();break;
        default:break;
        }
    }
    void import(std::string bytes){try{candidate=Afternoon::decode(bytes);ui->show(UiScreen::ImportConfirm);feedback="The save is valid. Confirm before replacing the afternoon.";}catch(const std::exception& e){feedback=std::string("Save could not be opened: ")+e.what();}project();}
    void importFailure(int reason){feedback=reason==0?"The save file is too large (maximum 16 MiB).":reason==1?"There was not enough memory to open the save. Your afternoon is unchanged.":"The save file could not be read. Your afternoon is unchanged.";ui->show(UiScreen::Recovery);project();}
    void publish(){auto p=owner.puzzleView();auto g=owner.progressView();std::ostringstream out;
        out<<"{\"quest\":"<<quote(p.quest)<<",\"completed\":"<<(p.completed?"true":"false")<<",\"coins\":"<<g.coins<<",\"chapter\":"<<g.chapter<<",\"generation\":"<<g.generation<<",\"revision\":"<<g.revision<<",\"picnicSeen\":"<<(g.picnicEndingSeen?"true":"false")<<",\"feedback\":"<<quote(feedback)<<",\"actions\":[";
        for(std::size_t n=0;n<p.actions.size();++n){auto& a=p.actions[n];if(n)out<<',';out<<"{\"label\":"<<quote(a.label)<<",\"verb\":"<<int(a.intent.verb)<<",\"object\":"<<quote(a.intent.object)<<",\"target\":"<<quote(a.intent.target)<<",\"direction\":"<<int(a.intent.direction)<<",\"x\":"<<a.intent.destination.x<<",\"y\":"<<a.intent.destination.y<<",\"enabled\":"<<(a.enabled?"true":"false")<<"}";}
        out<<"],\"objects\":[";for(std::size_t n=0;n<p.objects.size();++n){if(n)out<<',';out<<"{\"id\":"<<quote(p.objects[n].id)<<",\"name\":"<<quote(p.objects[n].name)<<"}";}
        out<<"],\"solution\":[";auto definition=rules::quest(p.quest);for(std::size_t n=0;n<definition.solution.size();++n){auto& s=definition.solution[n];if(n)out<<',';out<<"{\"verb\":"<<int(s.verb)<<",\"object\":"<<quote(s.object)<<",\"target\":"<<quote(s.target)<<",\"direction\":"<<int(s.direction)<<",\"x\":"<<s.destination.x<<",\"y\":"<<s.destination.y<<"}";}
        out<<"]}";auto json=out.str();EM_ASM({window.release1State=JSON.parse(UTF8ToString($0));},json.c_str());
    }
    void frame(double seconds){
        renderer->serviceRecovery();renderer->resizeCanvas();
        if(ui){double w,h;emscripten_get_element_css_size("#viewport",&w,&h);
            if(ui->willRebuild(float(w),float(h))){renderer->setGui(nullptr);engine::attachBrowserGui(nullptr);ui->update(float(w),float(h));renderer->setGui(&ui->gui());engine::attachBrowserGui(&ui->gui());engine::syncBrowserGui(true);}else ui->update(float(w),float(h));
            if(ui->needsPause())app.pause();else app.resume();
            engine::RenderQuality q;q.resolution={1,1,1920,921600};q.maxShadowMaps=ui->settings().quality==1?0:1;renderer->setQuality(q);
            auto b=views->boardBounds();EM_ASM({window.guiHarness=({screen:$0,board:{x:$1,y:$2,width:$3,height:$4},paused:!!$5});},int(ui->screen()),b.x,b.y,b.width,b.height,ui->needsPause());
        }
        app.setSuspended(suspended||!renderer->ready());const auto& f=app.advance(seconds);if(renderer->ready())renderer->render(0,f.interval*1000);if(ui)engine::syncBrowserGui();
    }
    void suspend(bool v){suspended=v;owner.setSuspended(v);project();}
    void pointer(float x,float y,bool d){if(ui)ui->pointer({x,y},d);}
    void escape(){if(ui)ui->escape();}
    void orbitBy(float x,float y){if(ui&&!ui->blocksScene())orbit->orbit(x,y);}
    void serviceState(int s){saveState=SaveState(std::clamp(s,0,5));project();}
    void motion(bool v){if(ui){ui->setReducedMotion(v);owner.settings.reducedMotion=v;}}
    void image(int index,const std::byte* p,unsigned size){if(size>4*1024*1024)return;try{auto texture=engine::Texture::loadMemory(std::span(p,size),"Accepted GUI reference");if(index==0)portraits["pip"]=texture;else if(index>=1&&index<=8)thumbnails[std::string(progression::items[index-1].id)]=texture;project();}catch(const std::exception& e){feedback=std::string("Preview unavailable: ")+e.what();project();}}
private:
    engine::Application app;Afternoon owner;std::optional<Afternoon> candidate;
    std::unique_ptr<engine::OrbitController> orbit;std::unique_ptr<engine::Renderer> renderer;
    std::array<std::vector<std::byte>,3> fonts;std::unique_ptr<PawsUi> ui;std::unique_ptr<gui::Views> views;
    std::map<std::string,std::shared_ptr<const engine::Texture>> portraits,thumbnails;
    std::string feedback,placing;bool reward=false,suspended=false;SaveState saveState=SaveState::Temporary;
};
std::unique_ptr<Harness> harness;void frame(void*){harness->frame(emscripten_get_now()/1000);}
}
extern "C" {
EMSCRIPTEN_KEEPALIVE void gui_font(int i,const std::byte* p,unsigned size){harness->font(i,p,size);}
EMSCRIPTEN_KEEPALIVE void gui_pointer(float x,float y,int down){harness->pointer(x,y,down);}
EMSCRIPTEN_KEEPALIVE void gui_escape(){harness->escape();}
EMSCRIPTEN_KEEPALIVE void gui_suspend(int v){harness->suspend(v!=0);}
EMSCRIPTEN_KEEPALIVE void gui_orbit(float x,float y){harness->orbitBy(x,y);}
EMSCRIPTEN_KEEPALIVE void gui_import(const char* p,unsigned size){harness->import(std::string(p,size));}
EMSCRIPTEN_KEEPALIVE void gui_import_failure(int reason){harness->importFailure(reason);}
EMSCRIPTEN_KEEPALIVE void gui_save_state(int v){harness->serviceState(v);}
EMSCRIPTEN_KEEPALIVE void gui_motion(int v){harness->motion(v!=0);}
EMSCRIPTEN_KEEPALIVE void gui_image(int i,const std::byte* p,unsigned size){harness->image(i,p,size);}
}
int main(){harness=std::make_unique<Harness>();emscripten_set_main_loop_arg(frame,nullptr,0,true);}
