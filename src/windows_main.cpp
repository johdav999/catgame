#include "runtime/game_session.hpp"
#include "engine/desktop.hpp"
#include "engine/audio_desktop.hpp"
#include <windows.h>
#include <SDL3/SDL.h>
#include <deque>
#include <fstream>
#include <iostream>
#include <thread>

namespace {
using namespace paws::runtime;
using namespace engine;
#include "paws_native_audio.inc"
static_assert(std::size(nativeAudioPaths)==paws::audioClips.size());
constexpr std::size_t saveLimit=16*1024*1024;
std::span<const std::byte> bytes(const std::string& s) {return std::as_bytes(std::span(s.data(),s.size()));}
std::string readSave(const std::filesystem::path& path) {
    const auto data=readDesktopFile(path,saveLimit);return {reinterpret_cast<const char*>(data.data()),data.size()};
}
class WindowsHost final : public RuntimeHost {
public:
    DesktopHost desktop;
    DesktopFiles files;
    GameSession* session=nullptr;
    std::string state,loadError;
    unsigned saves=0,decoded=0;
    bool writeAllowed=false;
    WindowsHost(const DesktopSettings& settings,const DesktopFileSettings& storage,bool preview)
        :desktop(settings),files(storage) {
        beachPreview=preview;
        const auto lock=files.userPath(DesktopStorage::Saves,"writer.lock");
        std::filesystem::create_directories(lock.parent_path());
        writer_=CreateFileW(lock.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        writeAllowed=writer_!=INVALID_HANDLE_VALUE;
    }
    ~WindowsHost() override {desktop.setEventHandler({});if(writer_!=INVALID_HANDLE_VALUE)CloseHandle(writer_);}
    Application& application() override {return desktop.application();}
    AssetLoader assetLoader(std::string url,std::function<void(std::span<const std::byte>,const AssetLoadContext&)> decode) override {
        if(!url.starts_with("assets/"))throw std::invalid_argument("Invalid Paws asset URL");
        return [files=files,path=url.substr(7),decode=std::move(decode)](AssetLoadContext context)->AssetCancel {
            if(context.cancelled())return {};
            const auto data=files.readAsset(path);context.progress(data.size(),data.size());
            if(!context.cancelled())decode(data,context);return {};
        };
    }
    std::unique_ptr<AudioBackend> makeAudio(bool) override {return std::make_unique<DesktopAudioBackend>();}
    std::string audioStatus(bool active,bool muted) override {
        if(muted)return "Sound is muted.";
        return active?"Sound is on.":"Enable sound to hear the garden.";
    }
    void event(HostEvent event,std::initializer_list<HostValue> values) override {
        const auto* v=values.begin();
        switch(event) {
        case HostEvent::State:state=v[0].text();break;
        case HostEvent::LoadError:loadError=v[0].text();break;
        case HostEvent::AudioRequest:
            if(audioQueue_.size()<paws::audioClips.size())audioQueue_.emplace_back(unsigned(v[0].number()),unsigned(v[1].number()));
            break;
        case HostEvent::AudioEpoch:case HostEvent::AudioSceneEpoch:audioQueue_.clear();break;
        case HostEvent::StoreSave: {
            if(!writeAllowed)break; // Preserve the shared SecondTab status.
            bool ok=false;
            try {if(writeAllowed){files.writeUser(DesktopStorage::Saves,"afternoon.save",bytes(v[0].text()),saveLimit);ok=true;++saves;}}
            catch(const std::exception& e){std::cerr<<"Save failed: "<<e.what()<<'\n';}
            session->saved(unsigned(v[1].number()),ok);break;
        }
        case HostEvent::RetrySave:retry_=true;break;
        case HostEvent::DownloadSave:export_=v[0].text();break;
        case HostEvent::ExportOriginal:export_=original_;break;
        case HostEvent::ChooseImport:import_=std::pair(unsigned(v[0].number()),unsigned(v[1].number()));break;
        default:break; // Browser probes and cloud services are optional host diagnostics.
        }
    }
    void viewport(double* w,double* h) const override {const auto s=desktop.size();*w=s.width;*h=s.height;}
    bool ready() const override {return desktop.diagnostics().ready;}
    bool platformSuspended() const override {return desktop.application().suspended();}
    void setSuspended(bool) override {} // DesktopHost owns OS focus/minimize suspension.
    void bindScene(SceneId id,CameraComponent& camera) override {
        const DesktopView views[]={{camera.id(),std::array<float,4>{.56f,.63f,.53f,1}}};desktop.setViews(id,views);
    }
    GuiSystem* sharedGui() override {return &desktop.gui();}
    void attachGui(GuiSystem*) override {} // Stable host-owned GUI, rebuilt at idle.
    void setQuality(const RenderQuality& q) override {desktop.setQuality(q);}
    void setWaterTime(double t) override {water_=t;desktop.setWaterTime(t);}
    double waterTime() const override {return water_;}
    void releaseRendering() override {} // Application shutdown releases DesktopHost bindings.
    bool restore() {
        const auto file=files.userPath(DesktopStorage::Saves,"afternoon.save");
        original_=std::filesystem::exists(file)?readSave(file):"";
        session->storageMode(writeAllowed?0:1,"windows",writeAllowed?"":"Another Paws window owns this profile. Export changes to keep them.",!original_.empty());
        const bool ok=session->restore(original_.data(),int(original_.size()),std::filesystem::exists(file)?1:0);
        if(ok&&writeAllowed&&!original_.empty()&&!original_.starts_with("PAWS_AFTERNOON 14")) {
            const auto backup=files.userPath(DesktopStorage::Saves,"pre-quest-m6.save");
            if(!std::filesystem::exists(backup))files.writeUser(DesktopStorage::Saves,"pre-quest-m6.save",bytes(original_),saveLimit);
        }
        if(ok) {
            auto backup=files.userPath(DesktopStorage::Saves,"pre-quest-m6.save");
            if(!std::filesystem::exists(backup))backup=files.userPath(DesktopStorage::Saves,"pre-quest-m5.save");
            if(std::filesystem::exists(backup)) {
                original_=readSave(backup);
                session->storageMode(writeAllowed?0:1,"windows",writeAllowed?"":"Another Paws window owns this profile. Export changes to keep them.",true);
            }
        }
        if(!ok&&std::filesystem::exists(file)) {
            // Preserve the exact invalid bytes before enabling recovery/import.
            // Never replace an earlier backup or let a secondary writer touch it.
            if(writeAllowed){
                auto name="recovery-"+std::to_string(GetTickCount64());
                while(std::filesystem::exists(files.userPath(DesktopStorage::Saves,name+".save")))name+="_";
                files.writeUser(DesktopStorage::Saves,name+".save",bytes(original_),saveLimit);
            }
            session->storageMode(writeAllowed?0:1,"windows",
                "The original save is kept unchanged in a recovery copy. Import a valid afternoon or export the original.",true);
            session->restore(nullptr,0,0);
        }
        return ok;
    }
    void pump() {
        if(retry_){retry_=false;if(!session->model().saveRevision)restore();else session->retrySave();}
        if(export_){auto data=std::move(*export_);export_.reset();DesktopFileDialogOptions o;o.kind=DesktopDialogKind::Save;o.title="Export afternoon";o.defaultExtension="save";o.filters={{"Paws afternoon","save"}};
            const auto r=desktop.showFileDialog(o);if(r.status==DesktopDialogStatus::Selected)writeDesktopFileAtomic(r.path,bytes(data),saveLimit);}
        if(import_){const auto stamp=*import_;import_.reset();DesktopFileDialogOptions o;o.title="Import afternoon";o.filters={{"Paws afternoon","save;txt"}};
            const auto r=desktop.showFileDialog(o);if(r.status==DesktopDialogStatus::Selected){const auto data=readSave(r.path);session->stageImport(data.data(),int(data.size()),stamp.first,stamp.second);}}
        if(!audioQueue_.empty()) {
            const auto [index,generation]=audioQueue_.front();audioQueue_.pop_front();
            try {if(index>=std::size(nativeAudioPaths)||!*nativeAudioPaths[index])throw std::runtime_error("Clip unavailable");
                const auto data=files.readAsset(nativeAudioPaths[index],192*1024*1024);const auto sound=decodeDesktopWav(data);
                if(session->audioPublish(index,generation,sound->sampleRate(),sound->channels(),sound->samples().data(),int(sound->samples().size())))++decoded;
            }catch(...){session->audioFailed(index,generation);}
        }
    }
    void inputs() {
        desktop.setEventHandler([this](const DesktopEvent& e){
            if(e.type==DesktopEventType::InputCancelled){dragging_=false;pointerDown_=false;session->pointer(-1,-1,false);session->volumePointer(3,0,0);session->objectPointer(3,0,0);return false;}
            if(e.type==DesktopEventType::FocusChanged){session->suspend(!e.focused);return false;}
            if(e.type==DesktopEventType::FileDrop){const auto data=readSave(e.path);const auto& a=session->model();session->stageImport(data.data(),int(data.size()),unsigned(a.progressView().attempt),unsigned(a.ownerRevision()));return true;}
            if(e.type==DesktopEventType::PointerPreview&&e.primary)return session->volumePointer(int(e.pointer),e.position.x,e.position.y);
            if(e.type==DesktopEventType::Pointer){
                const auto p=e.position;if(e.pointer==GuiPointer::Down)pointerDown_=true;else if(e.pointer==GuiPointer::Up)pointerDown_=false;session->pointer(p.x,p.y,pointerDown_);
                if(e.pointer==GuiPointer::Down)session->audioGesture();
                if(e.guiConsumed){if(e.pointer==GuiPointer::Up)dragging_=false;return false;}
                if(e.primary){int phase=e.pointer==GuiPointer::Down?0:e.pointer==GuiPointer::Move?1:e.pointer==GuiPointer::Up?2:3;
                    if(session->objectPointer(phase,p.x,p.y)){dragging_=false;last_=p;return true;}}
                if(e.pointer==GuiPointer::Down){dragging_=!session->blocked();last_=p;}
                else if(e.pointer==GuiPointer::Up)dragging_=false;
                else if(dragging_){session->drag(p.x-last_.x,p.y-last_.y);last_=p;}return true;
            }
            if(e.type==DesktopEventType::Wheel&&!e.guiConsumed&&!session->blocked()){session->objectPointer(3,0,0);session->zoom(e.wheel.y);return true;}
            if(e.type==DesktopEventType::Control&&e.down&&!e.repeat&&!e.guiConsumed){
                if((e.control==DesktopControl::Up||e.control==DesktopControl::Right||e.control==DesktopControl::Down||e.control==DesktopControl::Left)&&session->volumeKey(e.control==DesktopControl::Up||e.control==DesktopControl::Right?0:1))return true;
                if(e.control==DesktopControl::Escape){session->escape();return true;}
                int key=-1;
                switch(e.control){case DesktopControl::Up:key=0;break;case DesktopControl::Right:key=1;break;case DesktopControl::Down:key=2;break;case DesktopControl::Left:key=3;break;case DesktopControl::R:key=4;break;case DesktopControl::Space:key=5;break;default:break;}
                if(key>=0&&session->objectKey(key))return true;
                if(key>=0&&key<4&&!session->blocked()){session->drag(key==1?12:key==3?-12:0,key==0?-12:key==2?12:0);return true;}
            }return false;
        });
    }
private:
    HANDLE writer_=INVALID_HANDLE_VALUE;
    double water_=0;
    bool retry_=false,dragging_=false,pointerDown_=false;
    Vec2 last_{};
    std::string original_;
    std::optional<std::string> export_;
    std::optional<std::pair<unsigned,unsigned>> import_;
    std::deque<std::pair<unsigned,unsigned>> audioQueue_;
};
void require(bool v,const char* why){if(!v)throw std::runtime_error(why);}
}

int main(int argc,char** argv) {
    try {
        DesktopSettings settings;settings.title="Paws";settings.width=1280;settings.height=800;
        DesktopFileSettings storage;storage.organization="Paws";storage.application="Paws";
        std::string smoke;std::filesystem::path output,importFile;bool preview=false;
        for(int i=1;i<argc;++i){std::string a=argv[i];
            if(a=="--user-directory"&&i+1<argc)storage.userDirectory=argv[++i];
            else if(a=="--assets"&&i+1<argc)storage.assetDirectory=argv[++i];
            else if(a=="--dev-beach")preview=true;
            else if(a=="--import"&&i+1<argc)importFile=argv[++i];
            else if(a=="--smoke"&&i+2<argc){smoke=argv[++i];output=argv[++i];settings.hidden=true;}
            else throw std::invalid_argument("Use --user-directory path, --assets path, --dev-beach, --import file or --smoke seed|resume|import|catalogue|hold|secondary|pilot-seed|pilot-resume output");
        }
        WindowsHost host(settings,storage,preview);GameSession game(host);host.session=&game;
        struct Clear {WindowsHost& h;~Clear(){h.desktop.setEventHandler({});h.session=nullptr;}} clear{host};
        const char* fonts[]={"NunitoSans-Regular.ttf","NunitoSans-Semibold.ttf","Fraunces-Semibold.ttf"};
        for(int i=0;i<3;++i){const auto data=host.files.readAsset(std::string("fonts/")+fonts[i],4*1024*1024);game.font(i,data.data(),int(data.size()));}
        host.restore();host.inputs();
        if(!importFile.empty()){const auto data=readSave(importFile);require(game.stageImport(data.data(),int(data.size()),unsigned(game.model().progressView().attempt),unsigned(game.model().ownerRevision())),"Import rejected");const auto stamp=game.model().progressView();game.intent({paws::UiAction::ConfirmImport,{},stamp.attempt,stamp.revision});}
        auto tick=[&]{host.pump();game.beforeFrame(host.desktop.nowSeconds());return host.desktop.step(game.callbacks());};
        if(smoke.empty()){while(tick()){}return 0;}
        std::filesystem::create_directories(output);
        auto wait=[&](auto done){for(int n=0;n<2400;++n){require(tick(),"Host closed");if(!host.loadError.empty())throw std::runtime_error(host.loadError);if(done())return;std::this_thread::sleep_for(std::chrono::milliseconds(5));}throw std::runtime_error("Paws smoke timed out");};
        auto click=[&](std::string_view label){for(const auto& node:host.desktop.gui().accessibilityNodes())if(node.name==label&&!node.disabled){
            int count=0;auto** windows=SDL_GetWindows(&count);require(count>0,"Missing SDL window");const auto id=SDL_GetWindowID(windows[0]);SDL_free(windows);
            SDL_Event e{};e.type=SDL_EVENT_MOUSE_BUTTON_DOWN;e.button.windowID=id;e.button.button=SDL_BUTTON_LEFT;e.button.down=true;e.button.x=node.bounds.x+node.bounds.width*.5f;e.button.y=node.bounds.y+node.bounds.height*.5f;require(SDL_PushEvent(&e),"Pointer down failed");
            e.type=SDL_EVENT_MOUSE_BUTTON_UP;e.button.down=false;require(SDL_PushEvent(&e),"Pointer up failed");tick();return;
        }throw std::runtime_error("Missing GUI control: "+std::string(label));};
        auto capture=[&](const char* name){host.desktop.captureFrame(output/name);wait([&]{return !host.desktop.capturePending();});require(host.desktop.captureError().empty(),"Capture failed");};
        wait([&]{return host.ready();});
        {int count=0;auto** windows=SDL_GetWindows(&count);require(count>0,"Missing lifecycle window");const auto id=SDL_GetWindowID(windows[0]);SDL_free(windows);
         SDL_Event e{};e.type=SDL_EVENT_WINDOW_MINIMIZED;e.window.windowID=id;SDL_PushEvent(&e);tick();const auto clock=host.application().clocks().simulation.seconds;
         for(int n=0;n<3;++n){std::this_thread::sleep_for(std::chrono::milliseconds(20));tick();}
         require(host.application().suspended()&&host.application().clocks().simulation.seconds==clock,"Minimize advanced simulation");
         e.type=SDL_EVENT_WINDOW_RESTORED;SDL_PushEvent(&e);tick();require(!host.application().suspended()&&host.application().clocks().simulation.delta==0,"Restore did not rebase clock");}
        click(game.model().saveRevision?"Continue":"Play");
        wait([&]{return host.state.find("\"production\":1")!=std::string::npos;});capture("garden.ppm");
        if(smoke=="hold") {
            require(host.writeAllowed,"Primary smoke did not acquire the profile");game.save();tick();
            std::ofstream(output/"ready")<<"primary";
            wait([&]{return std::filesystem::exists(output/"release");});
        } else if(smoke=="secondary") {
            require(!host.writeAllowed,"Second process acquired an owned profile");
            const auto path=host.files.userPath(DesktopStorage::Saves,"afternoon.save");
            const auto before=readSave(path);game.save();tick();
            require(host.saves==0&&readSave(path)==before,"Second process replaced the primary save");
        } else if(smoke=="seed") {
            game.requestQuest(0,true);wait([&]{return !game.model().inGarden()&&game.model().active()==0;});
            const auto d=paws::release1::rules::quest(game.model().puzzleView().quest,game.model().puzzleView().content);
            require(!d.solution.empty(),"Missing first quest solution");const auto& step=d.solution.front();auto intent=game.model().intent(step.verb,step.object,step.target);intent.direction=step.direction;intent.destination=step.destination;
            require(game.dispatchTyped(intent),"First move rejected");game.escape();tick();game.save();
        } else if(smoke=="catalogue") {
            namespace p=paws::release1::progression;using V=paws::release1::Verb;
            std::ofstream log(output/"catalogue.json");log<<'[';
            auto apply=[&](const paws::release1::rules::Step& step){
                bool accepted=false;wait([&]{
                    const auto nodes=host.desktop.gui().accessibilityNodes();
                    for(const auto& node:nodes)if(node.name.starts_with("Start setup ")&&!node.disabled){click(node.name);break;}
                    auto intent=game.model().intent(step.verb,step.object,step.target);intent.direction=step.direction;intent.destination=step.destination;
                    accepted=game.dispatchTyped(intent);return accepted;
                });
                require(game.model().questMigrationView().authoritative==p::quests.size(),"Incomplete kernel authority");tick();
            };
            for(std::size_t q=0;q<p::quests.size();++q) {
                game.requestQuest(q,true,true);wait([&]{return game.model().active()==q&&!game.model().inGarden()&&host.application().loadedSceneCount()==1;});tick();
                const auto d=paws::release1::rules::quest(p::quests[q].id,game.model().puzzleView().content);const int before=game.model().coins();
                for(const auto& step:d.solution){if(game.model().puzzleView().completed)break;apply(step);}
                require(game.model().puzzleView().completed&&game.model().rewarded(q),"Catalogue solution failed");
                const int coins=before+(p::quests[q].pack.empty()?20:0);require(game.model().coins()==coins,"Catalogue reward changed");
                const auto saved=game.model().encode();require(paws::Afternoon::decode(saved).encode()==saved,"Catalogue save roundtrip changed");
                const auto filename=std::string(p::quests[q].id)+".save";writeDesktopFileAtomic(output/filename,bytes(saved),saveLimit);
                apply({V::Undo});require(!game.model().puzzleView().completed&&game.model().rewarded(q),"Catalogue undo lost receipt");
                const auto stamp=game.model().progressView();game.intent({paws::UiAction::Restart,{},stamp.attempt,stamp.revision});tick();
                for(const auto& step:d.solution){if(game.model().puzzleView().completed)break;apply(step);require(game.model().questMigrationView().newReceipts.empty(),"Catalogue practice issued another receipt");}
                require(game.model().puzzleView().completed&&game.model().coins()==coins,"Catalogue practice changed reward");
                game.requestQuest(q,false,false,true);wait([&]{return game.model().inGarden();});
                require(host.application().loadedSceneCount()==1,"Catalogue travel leaked scene");
                if(q)log<<',';log<<"{\"quest\":"<<std::quoted(std::string(p::quests[q].id))<<",\"completed\":true,\"coins\":"<<coins<<",\"sceneCount\":1}";log.flush();
                std::cout<<"Catalogue passed "<<p::quests[q].id<<'\n';
            }
            log<<']';game.save();
        } else if(smoke=="pilot-seed"||smoke=="pilot-resume") {
            using V=paws::release1::Verb;
            require(game.model().puzzleView().quest=="beach_fresh_water","Missing modern pilot");
            if(game.model().userPaused){click("Continue");tick();}
            const auto definition=paws::release1::rules::quest("beach_fresh_water",game.model().puzzleView().content);
            auto apply=[&](const paws::release1::rules::Step& step){auto intent=game.model().intent(step.verb,step.object,step.target);intent.direction=step.direction;intent.destination=step.destination;require(game.dispatchTyped(intent),"Water pilot command rejected");tick();require(game.model().questMigrationView().matched,"Quest shadow mismatch");};
            auto basin=[&]{for(const auto& o:game.model().puzzleView().objects)if(o.role=="water_basin")return o.active;return false;};
            const auto firstRoll=std::find_if(definition.solution.begin(),definition.solution.end(),[](const auto& s){return s.verb==V::Roll;});require(firstRoll!=definition.solution.end(),"Missing basin release");
            if(smoke=="pilot-seed") {
                apply({V::Restart});for(auto it=definition.solution.begin();it!=firstRoll+1;++it)apply(*it);
                require(basin()&&!game.model().puzzleView().completed,"Basin checkpoint not reached");game.escape();tick();game.save();capture("basin.ppm");
            } else {
                require(basin()&&game.model().puzzleView().canUndo,"Basin checkpoint did not restore");
                const int coins=game.model().coins();apply({V::Undo});require(!basin(),"Basin undo did not regress");apply(*firstRoll);
                for(auto it=firstRoll+1;it!=definition.solution.end();++it)apply(*it);
                require(game.model().puzzleView().completed&&game.model().rewarded(game.model().active()),"Water pilot did not complete");
                require(game.model().coins()==coins&&!game.model().lastRewardGranted(),"Beach pilot leaked base reward");capture("water-complete.ppm");
                apply({V::Undo});require(!game.model().puzzleView().completed&&game.model().rewarded(game.model().active()),"Pilot undo revoked receipt");
                apply({V::Restart});for(const auto& step:definition.solution){apply(step);require(game.model().questMigrationView().newReceipts.empty(),"Practice reissued pilot receipt");}
                game.requestQuest(0,false,false,true);wait([&]{return game.model().inGarden();});require(game.model().coins()==coins,"Water travel changed wallet");
                auto before=game.model().encode(),bad=before;const auto at=bad.find("/first-completion",bad.find("QUEST_KERNEL "));require(at!=std::string::npos,"Missing pilot receipt");bad.replace(at,17,"/invalid-receipt");
                require(!game.stageImport(bad.data(),int(bad.size()),unsigned(game.model().progressView().attempt),unsigned(game.model().ownerRevision())),"Inconsistent pilot import accepted");require(game.model().encode()==before,"Rejected pilot import changed session");game.save();
            }
        } else if(smoke=="resume"||smoke=="import") {
            const auto round=game.model().encode();require(paws::Afternoon::decode(round).encode()==round,"Save round trip changed");
            if(smoke=="resume") {
                require(game.model().puzzleView().canUndo,"Partial quest did not restore");
                if(game.model().userPaused){click("Continue");tick();}
                // Restart retains durable rewards; replay the authored first quest
                // through the same command gate used by both hosts.
                auto restart=game.model().intent(paws::release1::Verb::Restart);require(game.dispatchTyped(restart),"Restart rejected");tick();
                const auto d=paws::release1::rules::quest(game.model().puzzleView().quest,game.model().puzzleView().content);
                for(const auto& step:d.solution){auto i=game.model().intent(step.verb,step.object,step.target);i.direction=step.direction;i.destination=step.destination;require(game.dispatchTyped(i),"Solution command rejected");tick();}
                require(game.model().puzzleView().completed,"Quest did not complete");
                const int coins=game.model().coins();require(coins>0,"Reward missing");
                game.requestQuest(0,false,false,true);wait([&]{return game.model().inGarden();});
                require(game.model().coins()==coins,"Travel changed reward");
                capture("completed-garden.ppm");
            }
            const auto before=game.model().encode();require(!game.stageImport("broken",6,unsigned(game.model().progressView().attempt),unsigned(game.model().ownerRevision())),"Bad import accepted");require(game.model().encode()==before,"Bad import changed state");
            game.save();
        }else throw std::invalid_argument("Unknown smoke mode");
        capture("after.ppm");const auto data=game.model().encode();writeDesktopFileAtomic(output/"afternoon.save",bytes(data),saveLimit);
        std::ofstream(output/"state.json")<<host.state;
        std::ofstream(output/"report.json")<<"{\"mode\":\""<<smoke<<"\",\"frames\":"<<host.desktop.diagnostics().presentedFrames<<",\"saves\":"<<host.saves<<",\"decodedAudio\":"<<host.decoded<<",\"sceneCount\":"<<host.application().loadedSceneCount()<<"}";
        require(host.application().loadedSceneCount()==1,"Scene leak");require(smoke=="secondary"?host.saves==0:host.saves>0,"Incorrect durable save ownership");
        std::cout<<"Paws Windows "<<smoke<<" passed\n";return 0;
    }catch(const std::exception& e){std::cerr<<"Paws: "<<e.what()<<'\n';return 1;}
}
