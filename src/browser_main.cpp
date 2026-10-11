#include "puzzle_scene.hpp"
#include "garden_scene.hpp"
#include "afternoon.hpp"
#include "engine/asset_browser.hpp"
#include "engine/glb_importer.hpp"
#include <optional>
#include "engine/application.hpp"
#include "engine/orbit_controller.hpp"
#include "engine/renderer.hpp"
#include "engine/gui_browser.hpp"
#include "engine/selection.hpp"
#include "paws_ui.hpp"
#include "game_audio.hpp"
#include "engine/audio_browser.hpp"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <memory>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <limits>
#include <cmath>

EM_JS(int, wants_gallery, (), { return new URLSearchParams(location.search).get('gallery')==='1'; });
EM_JS(void, gui_ready, (), { window.pawsGuiReady=true; window.pawsMaybeReady?.(); });
EM_JS(void, audio_status, (int active,int muted,char* buffer,int capacity), {
    const bytes=new TextEncoder().encode(window.PawsAudioHost.status(!!active,!!muted));
    const count=Math.min(capacity-1,bytes.length);HEAPU8.set(bytes.subarray(0,count),buffer);HEAPU8[buffer+count]=0;
});

namespace {
class BrowserGame {
public:
    BrowserGame() {
        auto backend=std::make_unique<engine::BrowserAudioBackend>();audioDevice_=backend->deviceId();
        audio_=std::make_unique<paws::GameAudio>(std::move(backend));
        audio_->request=[](std::size_t index,unsigned generation){EM_ASM({window.PawsAudioHost.request($0,$1);},index,generation);};
        audio_->event=[](std::string_view event){std::string name(event);EM_ASM({window.PawsAudioHost.event(UTF8ToString($0));},name.c_str());};
        EM_ASM({window.PawsAudioHost.init($0);window.PawsAudioHost.epoch($1);},audioDevice_,audio_->generation());
        for(const auto& spec:paws::gardenAssets()) application_.assets().registerAsset<engine::Mesh>(engine::AssetId{spec.id},
            engine::browserAssetLoader(spec.file,
                [spec](std::span<const std::byte> bytes,const engine::AssetLoadContext& context){
                    auto mesh=engine::GlbImporter::loadMemory(bytes);paws::validateGardenAsset(spec,*mesh);context.complete<engine::Mesh>(mesh);
                }));
        application_.assets().registerAsset<engine::Mesh>(engine::AssetId{std::string(paws::pipAssetId)},
            engine::browserAssetLoader(std::string(paws::pipAssetUrl),[](std::span<const std::byte> bytes,const engine::AssetLoadContext& context){
                auto mesh=engine::GlbImporter::loadMemory(bytes);paws::validatePip(*mesh);context.complete<engine::Mesh>(mesh);
            }));
        application_.assets().registerAsset<engine::Texture>(engine::AssetId{"pip.portrait"},
            engine::browserAssetLoader("assets/models/pip/portrait-dialogue.png",[](std::span<const std::byte> bytes,const engine::AssetLoadContext& context){
                context.complete<engine::Texture>(engine::Texture::loadMemory(bytes,"Pip portrait"));
            }));
        for(const auto& pair:std::array<std::pair<const char*,const char*>,3>{{
            {"shop.cushion","assets/models/garden/cushion-preview.png"},
            {"shop.bowl","assets/models/garden/bowl-preview.png"},
            {"pip.ending","assets/models/pip/portrait-ending.png"}}}) {
            application_.assets().registerAsset<engine::Texture>(engine::AssetId{pair.first},
                engine::browserAssetLoader(pair.second,[](std::span<const std::byte> bytes,const engine::AssetLoadContext& context){
                    context.complete<engine::Texture>(engine::Texture::loadMemory(bytes,"Garden portrait"));
                }));
        }
        if(auto error=paws::validateContent(afternoon_.session().content()); !error.empty()) throw std::runtime_error(error);
        auto puzzle=paws::makePuzzleScene(afternoon_.session());
        auto* scene=puzzle.scene.get(); auto* camera=puzzle.camera;cameraId_=camera->id();
        bindings_=std::move(puzzle.bindings);
        auto id=application_.loadScene(std::move(puzzle.scene));
        orbit_=std::make_unique<engine::OrbitController>(*camera);
        reset();
        renderer_=std::make_unique<engine::Renderer>(*scene,*camera);
        applyQuality();
        application_.start(id); renderer_->initialize(); publish("Slide the pots, connect the channels, then Roll.");
    }
    void frame(double timestamp) {
        const auto frameStarted=emscripten_get_now();
        if(pendingQuest_) prepareQuest();
        applyQuality();renderer_->serviceRecovery(); renderer_->resizeCanvas();
        application_.setSuspended(suspended_ || !renderer_->ready());
        if(ui_) {
            if(storageLoaded_&&!gallery_) {
                const bool paused=ui_->screen()==paws::UiScreen::Pause?true:
                    (ui_->screen()==paws::UiScreen::Garden||ui_->screen()==paws::UiScreen::Puzzle)?false:afternoon_.userPaused;
                if(paused!=afternoon_.userPaused){afternoon_.userPaused=paused;save();}
            }
            double width=0,height=0;emscripten_get_element_css_size("#viewport",&width,&height);
            resize(float(width/std::max(height,1.0)),float(height));
            if(ui_->willRebuild(float(width),float(height))) {
                renderer_->setGui(nullptr);engine::attachBrowserGui(nullptr);
                ui_->update(float(width),float(height));
                renderer_->setGui(&ui_->gui());engine::attachBrowserGui(&ui_->gui());
                engine::syncBrowserGui(true);
            } else ui_->update(float(width),float(height));
            const bool reaction=(ui_->screen()==paws::UiScreen::Completion&&happySeconds_>0&&!ui_->settings().reducedMotion)||
                (ui_->screen()==paws::UiScreen::Ending&&gardenUse_.phase>0);
            if((ui_->needsPause()&&!reaction)||(afternoon_.session().state().paused&&!bindings_.garden)) application_.pause();else application_.resume();
        }
        engine::ApplicationFrameCallbacks callbacks;
        callbacks.simulate=[this](engine::Scene& scene,const engine::ApplicationFrame& f){
            if(bindings_.garden) {
                const auto phase=gardenUse_.phase;paws::updateGardenUse(scene,gardenUse_,float(f.simulation.delta));
                if(phase&&!gardenUse_.phase)projectUi("Pip feels right at home.");
            }
        };
        callbacks.present=[this](engine::Scene& scene,const engine::ApplicationFrame& f){presentPuzzle(scene,f);};
        const auto& frame=application_.advance(timestamp,callbacks);
        EM_ASM({window.pawsLifecycle=({suspended:!!$0,paused:!!$1,simulationDelta:$2,simulationElapsed:$3});},
            suspended_||!renderer_->ready(),application_.state()==engine::ApplicationState::Paused,frame.simulation.delta,frame.simulation.seconds);
        if(auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(application_.activeScene()->findComponent(bindings_.cat))) {
            EM_ASM({window.pawsPip=({time:$0,staticPose:!!$1,usePhase:$2,item:UTF8ToString($3),spot:UTF8ToString($4)});},
                cat->animation().time(),ui_&&ui_->settings().reducedMotion,gardenUse_.phase,gardenUse_.item.c_str(),gardenUse_.spot.c_str());
        }
        happySeconds_=std::max(0.0,happySeconds_-frame.presentation.delta);
        updateAudio(frame.presentation.delta);
        if(renderer_->ready()) renderer_->render(emscripten_get_now()-frameStarted,frame.interval*1000);
        if(ui_) engine::syncBrowserGui();
        if(sceneFocusPending_&&!blocked()) {EM_ASM({document.getElementById('viewport').focus({preventScroll:true});});sceneFocusPending_=false;}
    }
    bool command(int action,int object,int direction,unsigned attempt,unsigned revision,const std::string& target={}) {
        if(suspended_ || !renderer_->ready() || !storageLoaded_ || pendingQuest_) return false;
        const auto& content=afternoon_.session().content();
        const std::string id=object>=0 && object<int(content.objects.size())?content.objects[object].id:"";
        const auto before=afternoon_.session().state();const auto coins=afternoon_.coins();
        if(attempt!=before.attempt||revision!=before.revision)return false;
        auto result=afternoon_.dispatch({attempt,revision,paws::Action(action),id,paws::Direction(direction),target});
        using A=paws::Action;
        if(result.accepted) {
            switch(A(action)) {
            case A::Select:audio_->cue("ui_select");break;
            case A::Move:audio_->cue(object>=0&&content.objects[object].kind==paws::Kind::Cushion?"cushion_move":"pot_slide");break;
            case A::Rotate:audio_->cue("channel_rotate");break;
            case A::Undo:audio_->loop("ball_roll",false);audio_->cue("ui_undo");break;
            case A::Hint:audio_->cue("ui_hint");break;
            case A::PickUp:audio_->cue("toy_pickup");break;
            case A::Place:audio_->cue("basket_place");break;
            case A::Restart:resetAudioScene();break;
            default:break;
            }
            const auto& after=afternoon_.session().state();
            if(before.puzzle.ballOnSwitch!=after.puzzle.ballOnSwitch){audio_->cue(after.puzzle.ballOnSwitch?"switch_down":"switch_up");audio_->cue(after.puzzle.ballOnSwitch?"gate_open":"gate_close");}
            if(!before.puzzle.completed&&after.puzzle.completed)audio_->cue("pip_happy");
            if(afternoon_.coins()>coins)audio_->cue("reward_coins");
        } else {audio_->cue("ui_blocked");if(A(action)==A::Roll)audio_->cue("ball_contact");}
        if(result.accepted) {
            if(afternoon_.session().state().paused) application_.pause(); else application_.resume();
            paws::projectPuzzle(*application_.activeScene(),bindings_,afternoon_.session(),ui_&&!ui_->settings().reducedMotion);
        }
        if(result.accepted&&!afternoon_.session().state().puzzle.completed)completionAfterRoll_=false;
        if(result.accepted) save(afternoon_.lastRewardGranted()||action==int(paws::Action::Restart)||action==int(paws::Action::Garden));
        publish(result.message);
        projectUi(result.message);
        if(result.accepted&&afternoon_.session().state().puzzle.completed&&(action==int(paws::Action::Roll)||action==int(paws::Action::Place)||action==int(paws::Action::PickUp))&&ui_) {
            if(!bindings_.ballRoute.empty()) {
                completionAfterRoll_=true;
                ui_->enter(paws::UiScreen::Puzzle);
            } else {happySeconds_=1.0;ui_->enter(paws::UiScreen::Completion);}
        }
        return result.accepted;
    }
    void publish(const std::string& message) {
        lastMessage_=message;
        const auto& s=afternoon_.session().state(); const auto& c=afternoon_.session().content();
        std::ostringstream out;
        out<<"{\"quest\":"<<std::quoted(s.questId)<<",\"coins\":"<<afternoon_.coins()<<",\"friendship\":"<<afternoon_.friendship()
           <<",\"endingSeen\":"<<afternoon_.endingSeen()<<",\"cushionOwned\":"<<afternoon_.owned("garden_cushion")
           <<",\"bowlOwned\":"<<afternoon_.owned("drinking_bowl")<<",\"cushionSpot\":"<<std::quoted(std::string(afternoon_.placement("garden_cushion")))
           <<",\"bowlSpot\":"<<std::quoted(std::string(afternoon_.placement("drinking_bowl")))
           <<",\"scene\":"<<std::quoted(application_.activeScene()->name)<<",\"production\":"<<bindings_.production
           <<",\"switchActive\":"<<s.puzzle.ballOnSwitch<<",\"save\":"<<int(saveState_)
           <<",\"attempt\":"<<s.attempt<<",\"revision\":"<<s.revision<<",\"completed\":"<<s.puzzle.completed
           <<",\"paused\":"<<s.paused<<",\"garden\":"<<s.inGarden<<",\"canUndo\":"<<afternoon_.session().canUndo()
           <<",\"message\":"<<std::quoted(message)<<",\"contentId\":"<<std::quoted(c.contentId)<<",\"objects\":[";
        for(std::size_t i=0;i<c.objects.size();++i) {
            if(i) out<<',';
            const auto& o=c.objects[i]; const auto& p=s.puzzle.objects[i];
            out<<"{\"name\":"<<std::quoted(o.name)<<",\"pot\":"<<(o.kind==paws::Kind::Pot)
               <<",\"id\":"<<std::quoted(o.id)<<",\"location\":"<<int(p.location)<<",\"socket\":"<<std::quoted(p.socket)
               <<",\"selected\":"<<(s.selected==o.id)<<",\"x\":"<<p.cell.x<<",\"y\":"<<p.cell.y<<",\"ports\":[";
            const auto ports=paws::ports(o.kind,p.orientation);
            for(std::size_t j=0;j<ports.size();++j) { if(j) out<<','; out<<int(ports[j]); }
            out<<"],\"moves\":[";
            for(int d=0;d<4;++d) { if(d) out<<','; out<<std::quoted(paws::moveBlockage(c,s.puzzle,i,paws::Direction(d))); }
            out<<"]}";
        }
        out<<"]}"; const auto json=out.str();
        EM_ASM({ window.pawsState=JSON.parse(UTF8ToString($0)); },json.c_str());
    }
    void projectUi(const std::string& message={}) {
        if(!ui_||gallery_) return;
        paws::UiModel model;const auto& s=afternoon_.session().state();const auto& c=afternoon_.session().content();
        model.audioStatus=audioStatus_;model.audioActive=audio_->system().backend().active();
        model.attempt=s.attempt;model.revision=s.revision;model.puzzleReady=storageLoaded_&&!pendingQuest_;model.canUndo=afternoon_.session().canUndo();
        model.canRestart=!s.paused;
        model.pipPortrait=ui_->screen()==paws::UiScreen::Ending?endingPortrait_:portrait_;
        model.cushionPreview=cushionPreview_;model.bowlPreview=bowlPreview_;
        model.inGarden=bindings_.garden;model.shopOpen=afternoon_.shopOpen();model.endingSeen=afternoon_.endingSeen();
        model.cushionOwned=afternoon_.owned("garden_cushion");model.bowlOwned=afternoon_.owned("drinking_bowl");
        const auto spotName=[](std::string_view id){auto* spot=paws::gardenSpot(id);return spot?std::string(spot->name):std::string{};};
        model.cushionSpot=spotName(afternoon_.placement("garden_cushion"));model.bowlSpot=spotName(afternoon_.placement("drinking_bowl"));
        model.itemId=placementItem_;model.placementSpot=placementSpot_;
        model.placementPreview=placementItem_=="garden_cushion"?cushionPreview_:bowlPreview_;
        model.placementReason=afternoon_.placementBlockage(placementItem_,placementSpot_);
        model.placementValid=model.placementReason.empty();
        if(model.placementValid)model.placementReason="Fits here. Clear space for Pip.";
        for(const auto& spot:paws::gardenSpots)model.spots.push_back({std::string(spot.id),std::string(spot.name),
            afternoon_.placementBlockage(placementItem_,spot.id),spot.id==placementSpot_});
        if(gardenUse_.phase)model.catUse=gardenUse_.cushion?"Pip is enjoying his cushion.":"Pip is taking a little drink.";
        model.revealedHints=s.hint;
        model.canContinue=storageLoaded_&&hasSave_;model.save=saveState_;
        model.canExport=storageLoaded_;model.storageAvailable=storageLoaded_;
        model.canExportOriginal=hasOriginal_;model.storageNotice=storageNotice_;
        model.coins=afternoon_.coins();model.friendship=afternoon_.friendship();
        model.rewardsReady=true;model.rewardGranted=afternoon_.lastRewardGranted();model.rewardCoins=20;model.rewardFriendship=1;
        model.questTitle=c.title;model.objective=c.objective;model.dialogue=c.dialogue;
        model.shortTitle=c.goal==paws::Goal::Tidy?"Pip's toy basket":c.goal==paws::Goal::Retrieve?"Pip's little gate":"Pip's ball";
        for(std::size_t q=0;q<3;++q) model.quests.push_back({std::to_string(q),afternoon_.quest(q).content().title,afternoon_.unlocked(q),afternoon_.rewarded(q)});
        model.selectedId=s.selected;model.feedback=message.empty()?lastMessage_:message;
        for(std::size_t i=0;i<c.hints.size()&&i<3;++i) model.hints[i]=c.hints[i];
        const char* directions[]={"north","east","south","west"};
        for(std::size_t i=0;i<c.objects.size();++i) {
            const auto& o=c.objects[i];const auto& p=s.puzzle.objects[i];
            std::string description="Column "+std::to_string(p.cell.x+1)+", row "+std::to_string(p.cell.y+1)+".";
            if(paws::channel(o.kind)) {description+=" Openings: ";for(auto d:paws::ports(o.kind,p.orientation)) description+=std::string(directions[int(d)])+" ";}
            if(o.kind==paws::Kind::Toy) description+=p.location==paws::Location::Held?" Holding this toy.":p.location==paws::Location::Socket?" In its recess.":p.location==paws::Location::Collected?" Retrieved.":" On the ground.";
            if(o.kind==paws::Kind::Gate) description+=paws::gateOpen(c,s.puzzle,o.id)?" Open. The mouse is reachable.":" Closed. Put the ball on the switch.";
            if(o.kind==paws::Kind::Switch) description+=paws::switchActive(c,s.puzzle,o.id)?" Pressed by the ball.":" Empty.";
            model.objects.push_back({o.id,o.name,description});
            if(o.id!=s.selected) continue;
            model.selectedName=o.name;
            if(paws::sliding(o.kind)) for(int d=0;d<4;++d) {
                auto reason=paws::moveBlockage(c,s.puzzle,i,paws::Direction(d));
                model.actions.push_back({paws::UiAction::Move,std::string("Move ")+directions[d],o.id+"/"+std::to_string(d),reason,reason.empty()&&!s.puzzle.completed});
            } else if(paws::channel(o.kind)) model.actions.push_back({paws::UiAction::Rotate,"Rotate",o.id,description,!s.puzzle.completed});
        }
            // Pick-up and placement use the same rule predicates as dispatch.
        for(std::size_t i=0;i<c.objects.size();++i) {
            const auto& o=c.objects[i];const auto& p=s.puzzle.objects[i];
            if(o.kind!=paws::Kind::Toy) continue;
            if(o.id==s.selected && p.location!=paws::Location::Held) {
                auto reason=paws::pickupBlockage(c,s.puzzle,i);
                model.actions.push_back({paws::UiAction::PickUp,"Pick up "+o.name,o.id,reason,reason.empty()&&!s.puzzle.completed});
            }
            if(p.location==paws::Location::Held) for(const auto& socket:c.objects) if(socket.kind==paws::Kind::Socket) {
                auto reason=paws::placeBlockage(c,s.puzzle,i,socket.id);
                model.actions.push_back({paws::UiAction::Place,"Place in "+socket.name,o.id+"/"+socket.id,reason.empty()?"Fits here. Clear and reachable.":reason,reason.empty()&&!s.puzzle.completed});
            }
        }
        if(c.goal!=paws::Goal::Tidy) model.actions.push_back({paws::UiAction::Roll,"Roll ball","","",!s.puzzle.completed&&!s.puzzle.ballOnSwitch});
        if(c.goal==paws::Goal::Retrieve) model.actions.push_back({paws::UiAction::ReturnBall,"Return ball to source","","Closes the gate when the switch is empty.",s.puzzle.ballOnSwitch&&!s.puzzle.completed});
        ui_->project(std::move(model));
    }
    void intent(const paws::UiIntent& i) {
        using U=paws::UiAction;using A=paws::Action;
        if(i.action==U::EnableSound){audio_->retry();EM_ASM({window.PawsAudioHost.retry();});audioStarted_=true;audio_->activate();return;}
        if(i.action==U::ConfirmSound){audio_->cue("ui_confirm");return;}
        if(!gallery_&&(i.attempt!=afternoon_.session().state().attempt||i.revision!=afternoon_.session().state().revision)) return;
        if(i.action==U::ResetView) {reset();return;}if(i.action==U::ZoomIn) {zoom(-80);return;}if(i.action==U::ZoomOut) {zoom(80);return;}
        if(i.action==U::Settings) {
            const auto& s=ui_->settings();const bool motionChanged=afternoon_.settings.reducedMotion!=s.reducedMotion;
            afternoon_.settings={s.largerText,s.reducedMotion,s.muted,s.music,s.ambience,s.effects,s.quality};
            applyQuality();
            audio_->settings(afternoon_.settings);
            if(motionChanged&&bindings_.garden&&gardenUse_.phase) {
                auto item=gardenUse_.item,spot=gardenUse_.spot;
                gardenUse_=paws::beginGardenUse(*application_.activeScene(),bindings_,item,spot,s.reducedMotion);
            }
            if(bindings_.garden&&gardenUse_.phase)bindings_.reducedMotion=s.reducedMotion;save();return;
        }
        if(i.action==U::OpenShop) {
            if(storageLoaded_&&bindings_.garden&&afternoon_.shopOpen()) {ui_->enter(paws::UiScreen::Shop);projectUi();}return;
        }
        if(i.action==U::Buy||i.action==U::BeginPlacement||i.action==U::SelectSpot||i.action==U::PlaceFurnishing||
            i.action==U::CancelPlacement||i.action==U::UseFurnishing||i.action==U::ReplayEnding) {
            if(!storageLoaded_||pendingQuest_||!bindings_.garden||suspended_||!renderer_->ready())return;
            if(i.action==U::Buy) {
                auto result=afternoon_.dispatchGarden({unsigned(i.attempt),unsigned(i.revision),paws::Afternoon::GardenAction::Buy,i.target,{}});
                audio_->cue(result.accepted?"shop_purchase":"ui_blocked");
                if(result.accepted)save();publish(result.message);projectUi(result.message);return;
            }
            if(i.action==U::BeginPlacement) {
                if(!afternoon_.owned(i.target))return;
                paws::cancelGardenUse(*application_.activeScene(),gardenUse_,ui_->settings().reducedMotion);
                placementItem_=i.target;placementSpot_=afternoon_.placement(i.target);
            } else if(i.action==U::SelectSpot) {
                if(placementItem_.empty()||!paws::gardenSpot(i.target))return;placementSpot_=i.target;
            } else if(i.action==U::CancelPlacement) {placementItem_.clear();placementSpot_.clear();}
            else if(i.action==U::PlaceFurnishing) {
                if(i.target!=placementItem_+"/"+placementSpot_)return;
                auto result=afternoon_.dispatchGarden({unsigned(i.attempt),unsigned(i.revision),paws::Afternoon::GardenAction::Place,placementItem_,placementSpot_});
                if(!result.accepted){audio_->cue("ui_blocked");publish(result.message);projectUi();return;}
                audio_->cue(placementItem_=="garden_cushion"?"furnishing_place":"bowl_place");audio_->cue("pip_happy");
                auto item=placementItem_,spot=placementSpot_;placementItem_.clear();placementSpot_.clear();
                paws::projectHomeGarden(*application_.activeScene(),bindings_,afternoon_);
                reset();
                gardenUse_=paws::beginGardenUse(*application_.activeScene(),bindings_,item,spot,ui_->settings().reducedMotion);
                ui_->enter(paws::UiScreen::Garden);checkEnding();save();publish(result.message);projectUi();return;
            } else if(i.action==U::UseFurnishing) {
                auto spot=afternoon_.placement(i.target);if(spot.empty())return;
                gardenUse_=paws::beginGardenUse(*application_.activeScene(),bindings_,i.target,spot,ui_->settings().reducedMotion);
                ui_->enter(paws::UiScreen::Garden);projectUi();return;
            } else if(i.action==U::ReplayEnding) {
                if(!afternoon_.endingSeen())return;
                showEnding();return;
            }
            paws::projectHomeGarden(*application_.activeScene(),bindings_,afternoon_,placementItem_,placementSpot_);
            framePlacement();projectUi();return;
        }
        if(i.action==U::RetrySave) { EM_ASM({window.pawsRetrySave?.();});return; }
        if(i.action==U::ExportSave) { if(storageLoaded_){auto bytes=afternoon_.encode();EM_ASM({window.pawsDownloadSave(UTF8ToString($0));},bytes.c_str());}return; }
        if(i.action==U::ExportOriginal) {EM_ASM({window.pawsExportOriginal?.();});return;}
        if(i.action==U::ImportSave) {EM_ASM({window.pawsChooseImport($0,$1);},unsigned(i.attempt),unsigned(i.revision));return;}
        if(i.action==U::CancelImport) {importCandidate_.reset();return;}
        if(i.action==U::ConfirmImport) {
            if(!importCandidate_)return;
            try {
                auto candidate=*importCandidate_;candidate.invalidateImportedCommands(afternoon_);
                install(candidate);cancelPreparation();afternoon_=std::move(candidate);importCandidate_.reset();
                storageLoaded_=hasSave_=true;restoreUiSettings();
                ui_->enter(paws::UiScreen::Title);save();publish("Imported afternoon ready. Continue when you're ready.");projectUi();
            } catch(...) {publish("The file could not be opened. Your current afternoon is unchanged.");projectUi();}
            return;
        }
        if(i.action==U::RetryLoad) {requestQuest(failedQuest_,enterAfterPrepare_,restartAfterPrepare_,gardenAfterPrepare_);return;}
        if(i.action==U::SelectQuest) {
            if(i.attempt!=afternoon_.session().state().attempt||i.revision!=afternoon_.session().state().revision||!storageLoaded_) return;
            auto q=i.target=="0"?0:i.target=="1"?1:i.target=="2"?2:3;
            if(!afternoon_.unlocked(q)) return;
            requestQuest(std::size_t(q),false);return;
        }
        if(i.action==U::Play||i.action==U::BackToGarden) {
            if(!storageLoaded_) return;
            if(i.action==U::Play){audioStarted_=true;audio_->activate();}
            if((pendingQuest_||ui_->screen()==paws::UiScreen::LoadFailure)&&meshes_.empty()) {
                cancelPreparation();ui_->enter(paws::UiScreen::Title);projectUi("Loading cancelled. Your afternoon is safe.");return;
            }
            requestQuest(afternoon_.active(),false,false,true);return;
        }
        if(i.action==U::AcceptQuest||i.action==U::Continue) {
            if(!storageLoaded_||i.attempt!=afternoon_.session().state().attempt||i.revision!=afternoon_.session().state().revision) return;
            audioStarted_=true;audio_->activate();if(i.action==U::AcceptQuest)audio_->cue("pip_request");
            const auto& s=afternoon_.session().state();
            const bool garden=i.action==U::Continue&&(s.inGarden||s.puzzle.completed);
            requestQuest(afternoon_.active(),!garden,i.action==U::AcceptQuest,garden,i.action==U::Continue&&ui_->screen()==paws::UiScreen::Title);return;
        }
        if(i.action==U::BackToGarden) cancelPreparation();
        A action;std::string object=i.target,target;int direction=0;
        switch(i.action) {
        case U::Play:case U::BackToGarden:action=A::Garden;break;
        case U::AcceptQuest:case U::Continue:action=A::Resume;break;
        case U::SelectObject:action=A::Select;break;
        case U::Move:action=A::Move;if(auto at=object.rfind('/');at!=std::string::npos){direction=std::stoi(object.substr(at+1));object.resize(at);}break;
        case U::Rotate:action=A::Rotate;break;case U::Roll:action=A::Roll;break;
        case U::ReturnBall:action=A::ReturnBall;break;
        case U::RevealHint:action=A::Hint;break;
        case U::PickUp:action=A::PickUp;break;
        case U::Place:action=A::Place;if(auto at=object.find('/');at!=std::string::npos){target=object.substr(at+1);object.resize(at);}break;
        case U::Undo:action=A::Undo;break;case U::Restart:action=A::Restart;break;
        default:return;
        }
        int index=-1;for(std::size_t n=0;n<afternoon_.session().content().objects.size();++n) if(afternoon_.session().content().objects[n].id==object) index=int(n);
        if(!command(int(action),index,direction,unsigned(i.attempt),unsigned(i.revision),target)) return;
        if(i.action==U::AcceptQuest||i.action==U::Continue||i.action==U::Restart) {
            // A completed request starts a practice attempt explicitly through Help Pip.
            if(i.action==U::AcceptQuest&&afternoon_.session().state().puzzle.completed) {
                auto st=afternoon_.session().state();command(int(A::Restart),-1,0,st.attempt,st.revision);
            }
            ui_->show(paws::UiScreen::Puzzle);
        }
        if(i.action==U::BackToGarden) ui_->show(paws::UiScreen::Garden);
    }
    ~BrowserGame() { audio_->sceneChanged();audio_.reset();renderer_->setGui(nullptr);engine::attachBrowserGui(nullptr); }
    void font(int index,const void* data,int bytes) {
        if(index<0||index>2||bytes<12||bytes>4*1024*1024||ui_) return;
        fonts_[index].resize(bytes);std::memcpy(fonts_[index].data(),data,bytes);
        if(fonts_[0].empty()||fonts_[1].empty()||fonts_[2].empty()) return;
        gallery_=PAWS_GUI_GALLERY&&wants_gallery();
        ui_=std::make_unique<paws::PawsUi>(std::move(fonts_),gallery_,[this](const paws::UiIntent& i){intent(i);});
        projectUi();gui_ready();
    }
    void escape() {
        if(pendingQuest_) {auto s=afternoon_.session().state();intent({paws::UiAction::BackToGarden,{},s.attempt,s.revision});}
        else if(ui_) ui_->escape();
    }
    void pointer(float x,float y,bool down) { if(ui_) ui_->pointer({x,y},down); }
    // CSS coordinates use a CSS-sized camera viewport, independent of render DPR.
    int objectPointer(int phase,float x,float y) {
        auto* scene=application_.activeScene();
        if(phase==3) {const bool active=gesture_.has_value();gesture_.reset();if(active&&scene&&!bindings_.garden)paws::projectPuzzle(*scene,bindings_,afternoon_.session(),!bindings_.ballRoute.empty());return 0;}
        const auto& session=afternoon_.session();const auto& state=session.state();const auto& content=session.content();
        if(blocked()||suspended_||pendingQuest_||bindings_.garden||state.paused||state.puzzle.completed) {gesture_.reset();return 0;}
        sceneFocusPending_=true;
        auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findComponent(cameraId_));if(!camera)return 0;
        double width=0,height=0;emscripten_get_element_css_size("#viewport",&width,&height);
        auto viewport=camera->pixelViewport(float(width),float(height));
        auto ray=camera->screenPointToRay({x,y},viewport);
        auto ground=[&]()->std::optional<engine::Vec3>{
            if(std::abs(ray.direction.y)<.0001f)return {};
            const float t=-ray.origin.y/ray.direction.y;if(t<0)return {};
            return ray.origin+ray.direction*t;
        };
        if(phase==0) {
            auto hit=scene->selection().resolveRay(ray);if(!hit)return 0;
            int index=-1;for(std::size_t i=0;i<bindings_.objects.size();++i)if(bindings_.objects[i].actor==hit->actor)index=int(i);
            if(index<0) {
                if(hit->actor!=bindings_.ball)return 0;
                auto point=ground();if(!point)return 1;
                auto* actor=scene->findActor(hit->actor);
                gesture_=Gesture{-1,hit->actor,state.attempt,state.revision,{x,y},*point,actor->transform.position,actor->transform.position,false,false};
                return 1;
            }
            const bool selected=state.selected==content.objects[index].id;
            if(!selected&&!command(int(paws::Action::Select),index,0,state.attempt,state.revision))return 0;
            paws::projectPuzzle(*scene,bindings_,session,!bindings_.ballRoute.empty());
            auto point=ground();if(!point)return 1;
            const auto& now=session.state();auto* actor=scene->findActor(hit->actor);
            gesture_=Gesture{index,hit->actor,now.attempt,now.revision,{x,y},*point,actor->transform.position,actor->transform.position,selected,false};
            return 1;
        }
        if(!gesture_)return 0;
        if(gesture_->attempt!=state.attempt||gesture_->revision!=state.revision||!scene->findActor(gesture_->actor)){gesture_.reset();return 1;}
        auto& g=*gesture_;const float dx=x-g.down.x,dy=y-g.down.y;
        g.dragged|=dx*dx+dy*dy>64;
        if(auto point=ground())g.preview=g.original+(*point-g.ground);
        if(phase==1)return 1;
        auto finished=g;gesture_.reset();paws::projectPuzzle(*scene,bindings_,session,!bindings_.ballRoute.empty());
        if(finished.index<0) {
            if(!finished.dragged)command(int(paws::Action::Roll),-1,0,state.attempt,state.revision);
            return 1;
        }
        const auto& object=content.objects[finished.index];
        if(!finished.dragged) {
            if(finished.selected&&paws::channel(object.kind))command(int(paws::Action::Rotate),finished.index,0,state.attempt,state.revision);
            return 1;
        }
        auto dispatch=[&](paws::Action action,int direction=0,const std::string& target={}){
            const auto st=session.state();return command(int(action),finished.index,direction,st.attempt,st.revision,target);
        };
        if(paws::sliding(object.kind)) {
            const float spacing=bindings_.production?1.f:1.45f;
            const paws::Cell target{int(std::round(finished.preview.x/spacing+2)),int(std::round(finished.preview.z/spacing+1))};
            // Validate the whole bounded slot path before committing any moves.
            struct Step {paws::Cell cell;std::vector<int> path;};
            std::vector<Step> queue{{state.puzzle.objects[finished.index].cell,{}}};
            std::optional<std::vector<int>> path;
            for(std::size_t n=0;n<queue.size()&&n<=object.slots.size();++n) {
                auto step=queue[n];if(step.cell==target){path=step.path;break;}
                auto candidate=state.puzzle;candidate.objects[finished.index].cell=step.cell;
                for(int d=0;d<4;++d)if(paws::moveBlockage(content,candidate,finished.index,paws::Direction(d)).empty()) {
                    auto next=paws::adjacent(step.cell,paws::Direction(d));
                    if(std::none_of(queue.begin(),queue.end(),[&](const Step& s){return s.cell==next;})) {auto moves=step.path;moves.push_back(d);queue.push_back({next,std::move(moves)});}
                }
            }
            if(path){for(int d:*path)if(!dispatch(paws::Action::Move,d))break;}
            else {publish("That slot is blocked or outside this object's sliding path.");projectUi();}
        } else if(object.kind==paws::Kind::Toy) {
            if(content.goal==paws::Goal::Retrieve)dispatch(paws::Action::PickUp);
            else {
                std::string socket;float nearest=.45f;
                for(std::size_t i=0;i<content.objects.size();++i)if(content.objects[i].kind==paws::Kind::Socket) {
                    auto* a=scene->findActor(bindings_.objects[i].actor);auto delta=a->transform.position-finished.preview;
                    float distance=std::sqrt(delta.x*delta.x+delta.z*delta.z);if(distance<nearest){nearest=distance;socket=content.objects[i].id;}
                }
                auto candidate=state.puzzle;candidate.objects[finished.index].location=paws::Location::Held;
                const auto reason=paws::pickupBlockage(content,state.puzzle,finished.index);
                if(!socket.empty()&&reason.empty()&&paws::placeBlockage(content,candidate,finished.index,socket).empty()) {
                    if(dispatch(paws::Action::PickUp))dispatch(paws::Action::Place,0,socket);
                } else {publish(reason.empty()?"Drop the toy into a clear matching recess.":reason);projectUi();}
            }
        } else {publish("This piece stays in place. Tap a selected channel to turn it.");projectUi();}
        return 1;
    }
    void interactionSnapshot() {
        if(bindings_.garden)return;
        auto* scene=application_.activeScene();
        auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findComponent(cameraId_));if(!camera)return;
        double width=0,height=0;emscripten_get_element_css_size("#viewport",&width,&height);
        const auto viewport=camera->pixelViewport(float(width),float(height));
        EM_ASM({window.pawsInteraction=({objects:{},slots:{}});});
        const auto point=[&](const std::string& id,engine::Vec3 position,bool slot){
            const auto p=camera->worldToScreen(position,viewport);
            EM_ASM({window.pawsInteraction[$3?'slots':'objects'][UTF8ToString($0)]=({x:$1,y:$2});},id.c_str(),p.pixel.x,p.pixel.y,slot);
        };
        for(const auto& binding:bindings_.objects)if(auto* actor=scene->findActor(binding.actor)) {
            auto position=actor->transform.position;
            if(auto* mesh=actor->getComponent<engine::MeshComponent>()) {
                const auto& b=mesh->mesh->localBounds();position=engine::transformPoint(actor->transform.matrix(),(b.minimum+b.maximum)*.5f);
            }
            point(binding.object,position,false);
        }
        if(auto* actor=scene->findActor(bindings_.ball))point("source_ball",actor->transform.position+engine::Vec3{0,.06f,0},false);
        const auto& content=afternoon_.session().content();
        for(int y=0;y<content.height;++y)for(int x=0;x<content.width;++x)point(std::to_string(x)+","+std::to_string(y),{float(x-2),0,float(y-1)},true);
    }
    void presentPuzzle(engine::Scene& scene,const engine::ApplicationFrame& frame) {
        if(gesture_&&(blocked()||suspended_))objectPointer(3,0,0);
        const bool routePaused=ui_&&(ui_->screen()==paws::UiScreen::Pause||ui_->screen()==paws::UiScreen::Settings);
        paws::animatePuzzle(scene,bindings_,routePaused?0.f:float(frame.presentation.delta),ui_&&ui_->settings().reducedMotion);
        if(completionAfterRoll_&&bindings_.ballRoute.empty()) {
            completionAfterRoll_=false;
            if(ui_&&afternoon_.session().state().puzzle.completed&&!bindings_.garden) {
                happySeconds_=1.0;ui_->enter(paws::UiScreen::Completion);projectUi();
            }
        }
        if(auto* ball=scene.findActor(bindings_.ball)) {
            EM_ASM({window.pawsBall=({rolling:!!$0,x:$1,y:$2,z:$3,completionPending:!!$4});},
                !bindings_.ballRoute.empty(),ball->transform.position.x,ball->transform.position.y,ball->transform.position.z,completionAfterRoll_);
        }
        if(!gesture_||!gesture_->dragged||gesture_->index<0)return;
        const auto& state=afternoon_.session().state();
        if(gesture_->attempt!=state.attempt||gesture_->revision!=state.revision){gesture_.reset();return;}
        const auto kind=afternoon_.session().content().objects[gesture_->index].kind;
        if(paws::sliding(kind)||kind==paws::Kind::Toy)
            if(auto* actor=scene.findActor(gesture_->actor))actor->transform.position=gesture_->preview;
    }
    int objectKey(int key) {
        if(blocked()||bindings_.garden)return 0;
        sceneFocusPending_=true;
        const auto& s=afternoon_.session().state();const auto& c=afternoon_.session().content();
        if(key==5){objectPointer(3,0,0);command(int(paws::Action::Roll),-1,0,s.attempt,s.revision);return 1;}
        for(std::size_t i=0;i<c.objects.size();++i)if(c.objects[i].id==s.selected) {
            if((key<4&&paws::sliding(c.objects[i].kind))||(key==4&&paws::channel(c.objects[i].kind))) {
                objectPointer(3,0,0);command(int(key==4?paws::Action::Rotate:paws::Action::Move),int(i),key==4?0:key,s.attempt,s.revision);return 1;
            }
        }
        return 0;
    }
    bool blocked() const { return !ui_||ui_->blocksScene(); }
    void motion(bool reduced) { if(ui_&&!hasSave_) {ui_->setReducedMotion(reduced);afternoon_.settings.reducedMotion=reduced;} }
    void save(bool immediate=true) {
        if(gallery_||!storageLoaded_) return;
        if(afternoon_.saveRevision>=std::numeric_limits<unsigned>::max()-2){saveState_=paws::SaveState::Failed;projectUi("Save revision limit reached. Export your afternoon.");return;}
        ++afternoon_.saveRevision;afternoon_.saveSession=saveSession_;
        saveState_=storageMode_==1?paws::SaveState::SecondTab:storageMode_==2?paws::SaveState::Temporary:paws::SaveState::Saving;
        auto bytes=afternoon_.encode();auto sequence=++saveSequence_;
        EM_ASM({window.pawsStoreSave(UTF8ToString($0),$1,!!$2,$3);},bytes.c_str(),sequence,immediate,afternoon_.saveRevision);
        projectUi();publish(lastMessage_);
    }
    void saved(unsigned sequence,bool success) {
        if(sequence!=saveSequence_) return; // Old completions cannot label a newer snapshot Saved.
        saveState_=success?paws::SaveState::Saved:paws::SaveState::Failed;
        hasSave_=hasSave_||success;projectUi(lastMessage_);publish(lastMessage_);
    }
    void storageMode(int mode,const char* session,const char* notice,bool original) {
        storageMode_=mode;saveSession_=session;storageNotice_=notice;hasOriginal_=original;
        if(mode==1||mode==2)saveState_=mode==1?paws::SaveState::SecondTab:paws::SaveState::Temporary;
        projectUi();publish(lastMessage_);
    }
    void retrySave(){save();}
    void restoreUiSettings() {
        const auto& s=afternoon_.settings;
        audio_->settings(s);
        if(ui_)ui_->restoreSettings({s.largerText,s.reducedMotion,s.muted,s.music,s.ambience,s.effects,s.quality});
        applyQuality();
    }
    bool stageImport(const char* data,int bytes,unsigned attempt,unsigned revision) {
        if(!storageLoaded_||bytes<0||bytes>16*1024*1024||(!data&&bytes)||attempt!=afternoon_.session().state().attempt||revision!=afternoon_.session().state().revision)return false;
        try {auto candidate=paws::Afternoon::decode(std::string_view(data,bytes));importCandidate_=std::move(candidate);ui_->show(paws::UiScreen::ImportConfirm);projectUi("File checked. Choose whether to use it.");return true;}
        catch(...) {notice("That file is damaged or uses unsupported content. Your afternoon is unchanged.");return false;}
    }
    void notice(const char* text){publish(text);projectUi();}
    bool restore(const char* data,int bytes,int status) {
        if(status<0||status>2||bytes<0||bytes>16*1024*1024||(!data&&bytes)) return false;
        if(storageLoaded_) return false; // A late load never replaces a played afternoon.
        try {
            if(status==2) throw std::runtime_error("Storage is unavailable.");
            if(status==1) {
                auto candidate=paws::Afternoon::decode(std::string_view(data,std::size_t(bytes)));
                install(candidate);afternoon_=std::move(candidate);hasSave_=true;
                restoreUiSettings();
            }
            storageLoaded_=true;saveState_=storageMode_==1?paws::SaveState::SecondTab:storageMode_==2?paws::SaveState::Temporary:hasSave_?paws::SaveState::Saved:paws::SaveState::Temporary;
            projectUi();publish("Your afternoon is ready.");return true;
        } catch(const std::exception&) {
            saveState_=paws::SaveState::Failed;projectUi("Your saved afternoon could not be opened. It has been kept unchanged. Retry from Save and recovery.");
            if(ui_) ui_->show(paws::UiScreen::Recovery);return false;
        }
    }
    void install(const paws::Afternoon& candidate) {
        const auto& state=candidate.session().state();
        auto puzzle=state.inGarden&&!meshes_.empty()?paws::makeHomeGarden(meshes_,candidate.rewarded(0),candidate.settings.reducedMotion,&candidate):
            paws::makePuzzleScene(candidate.session(),meshes_.empty()?nullptr:&meshes_);
        objectPointer(3,0,0);
        auto* scene=puzzle.scene.get();auto* camera=puzzle.camera;cameraId_=camera->id();
        auto controller=std::make_unique<engine::OrbitController>(*camera);
        const auto previous=application_.activeSceneId();
        auto id=application_.loadScene(std::move(puzzle.scene));
        try {
            const engine::RenderView views[]={{camera,std::array<float,4>{.29f,.38f,.30f,1}}};
            renderer_->setScene(*scene,views); // Validate/rebind before destroying borrowed references.
        } catch(...) {application_.unloadScene(id);throw;}
        resetAudioScene();
        orbit_=std::move(controller);bindings_=std::move(puzzle.bindings);happySeconds_=0;completionAfterRoll_=false;gardenUse_={};placementItem_.clear();placementSpot_.clear();
        application_.requestTransition(id);application_.commitTransition();application_.unloadScene(previous);reset();
    }
    void cancelPreparation() {
        pendingQuest_.reset();if(preparation_) preparation_->cancel();preparation_.reset();application_.assets().collectUnused();
    }
    void requestQuest(std::size_t q,bool enter,bool restartCompleted=false,bool garden=false,bool restorePause=false) {
        cancelPreparation();pendingQuest_=q;enterAfterPrepare_=enter;restartAfterPrepare_=restartCompleted;gardenAfterPrepare_=garden;
        restorePauseAfterPrepare_=restorePause;
        if(meshes_.empty()) {
            std::vector<engine::AssetId> ids;
            for(const auto& asset:paws::gardenAssets()) ids.emplace_back(asset.id);
            ids.emplace_back(std::string(paws::pipAssetId));
            ids.emplace_back("pip.portrait");
            ids.emplace_back("shop.cushion");ids.emplace_back("shop.bowl");ids.emplace_back("pip.ending");
            preparation_=std::make_shared<engine::AssetPreparation>(application_.assets(),std::move(ids));
        }
        publish("Loading the garden… Your progress is safe while the models arrive.");projectUi(lastMessage_);ui_->show(paws::UiScreen::Loading);
    }
    void prepareQuest() {
        auto index=*pendingQuest_;
        try {
            if(preparation_) {
                const auto progress=preparation_->progress();
                if(progress.status==engine::AssetStatus::Failed||progress.status==engine::AssetStatus::Cancelled) throw std::runtime_error(progress.error);
                if(progress.status!=engine::AssetStatus::Ready) return;
                paws::GardenMeshes ready;
                for(const auto& asset:paws::gardenAssets()) ready.emplace(asset.name,preparation_->get<engine::Mesh>(engine::AssetId{asset.id}));
                ready.emplace("pip",preparation_->get<engine::Mesh>(engine::AssetId{std::string(paws::pipAssetId)}));
                portrait_=preparation_->get<engine::Texture>(engine::AssetId{"pip.portrait"});
                cushionPreview_=preparation_->get<engine::Texture>(engine::AssetId{"shop.cushion"});
                bowlPreview_=preparation_->get<engine::Texture>(engine::AssetId{"shop.bowl"});
                endingPortrait_=preparation_->get<engine::Texture>(engine::AssetId{"pip.ending"});
                meshes_=std::move(ready);preparation_.reset();
            }
            auto candidate=afternoon_;
            if(!restorePauseAfterPrepare_)candidate.userPaused=false;
            if(!candidate.selectQuest(index)) throw std::runtime_error("Quest locked.");
            const auto& s=candidate.session().state();candidate.dispatch({s.attempt,s.revision,enterAfterPrepare_?paws::Action::Resume:paws::Action::Garden,{}});
            if(restartAfterPrepare_&&candidate.session().state().puzzle.completed) {
                const auto& st=candidate.session().state();candidate.dispatch({st.attempt,st.revision,paws::Action::Restart,{}});
            }
            install(candidate);afternoon_=std::move(candidate);pendingQuest_.reset();
            save();publish("Pip has a little request.");projectUi(lastMessage_);
            ui_->enter(enterAfterPrepare_?paws::UiScreen::Puzzle:gardenAfterPrepare_?paws::UiScreen::Garden:paws::UiScreen::Dialogue);
            if(afternoon_.userPaused&&(enterAfterPrepare_||gardenAfterPrepare_))ui_->show(paws::UiScreen::Pause);
            if(gardenAfterPrepare_)checkEnding();
        } catch(const std::exception& error) {
            EM_ASM({window.pawsLoadError=UTF8ToString($0);},error.what());
            failedQuest_=index;
            cancelPreparation();publish("The models could not load. Retry when you're ready.");projectUi(lastMessage_);ui_->enter(paws::UiScreen::LoadFailure);
        }
    }
    void showEnding() {
        for(const auto& item:paws::shopItems)if(!afternoon_.placement(item.id).empty()) {
            gardenUse_=paws::beginGardenUse(*application_.activeScene(),bindings_,item.id,afternoon_.placement(item.id),ui_->settings().reducedMotion);break;
        }
        ui_->enter(paws::UiScreen::Ending);projectUi("Pip gives you a slow blink. He's glad you're here.");
    }
    void framePlacement() {
        const auto* spot=paws::gardenSpot(placementSpot_);
        if(!spot||placementItem_.empty()){reset();return;}
        auto view=orbit_->snapshot();
        view.target={spot->position.x+(aspect_>=1?(shortViewport_?2.6f:.9f):0.f),.1f,spot->position.z+(aspect_<1?1.8f:0.f)};
        view.distance=aspect_<1?5.f:5.8f;view.yaw=0;view.elevation=.95f;orbit_->restore(view);orbit_->orbit(0,0);
    }
    void checkEnding() {
        if(!bindings_.garden||afternoon_.endingSeen()||!afternoon_.endingEligible())return;
        const auto s=afternoon_.session().state();
        if(afternoon_.dispatchGarden({s.attempt,s.revision,paws::Afternoon::GardenAction::RecordEnding,{},{}}).accepted){showEnding();save();publish(lastMessage_);}
    }
    void drag(float dx,float dy) { if(!suspended_ && (!afternoon_.session().state().paused||afternoon_.session().state().inGarden)) orbit_->orbit(dx,dy); }
    void zoom(float delta) { if(!suspended_) orbit_->zoom(delta); }
    void reset() {
        auto view=orbit_->snapshot(); view.target=view.resetTarget=bindings_.garden?engine::Vec3{0,.1f,.1f}:engine::Vec3{0,0,0};
        view.yaw=0; view.elevation=0.95f;
        view.distance=bindings_.production?std::max(shortViewport_?6.8f:6.3f,5.5f/aspect_):std::max(shortViewport_?8.5f:6.3f,9.0f/aspect_);
        orbit_->restore(view); orbit_->orbit(0,0);
    }
    void resize(float aspect,float height=900) { if(aspect>0 && (aspect!=aspect_||(height<500)!=shortViewport_)) { aspect_=aspect;shortViewport_=height<500;if(placementItem_.empty())reset();else framePlacement(); } }
    void suspend(bool value) { suspended_=value; application_.setSuspended(value);audio_->suspend(value); }
    void audioGesture() {if(audioStarted_&&!suspended_&&!audio_->system().backend().active())audio_->activate();}
    bool audioPublish(int index,unsigned generation,unsigned rate,unsigned channels,const float* samples,int count) {
        if(index<0||index>=47||count<=0||std::size_t(count)>paws::GameAudio::clipBudget/sizeof(float)||generation!=audio_->generation())return false;
        try {return audio_->publish(index,generation,engine::SoundAsset::create(rate,channels,{samples,std::size_t(count)}));}
        catch(...){audio_->failed(index,generation);return false;}
    }
    void audioFailed(int index,unsigned generation) {if(index>=0)audio_->failed(index,generation);}
private:
    void applyQuality() {
        const bool mobile=afternoon_.settings.quality==1||(afternoon_.settings.quality==0&&
            EM_ASM_INT({return innerWidth<700||innerHeight<500||matchMedia('(pointer: coarse)').matches;}));
        const int profile=mobile?1:2;
        if(profile==appliedQuality_)return;
        engine::RenderQuality quality;
        quality.resolution={1.f,mobile?1.f:1.5f,4096,mobile?921600u:2073600u};
        quality.maxShadowMaps=mobile?0:1;
        quality.shadowResolution=1024;
        renderer_->setQuality(quality);appliedQuality_=profile;
        EM_ASM({window.pawsQuality=({profile:$0===1?'mobile':'desktop',maxPixels:$1,maxDpr:$2,maxShadowMaps:$3});},
            profile,quality.resolution.maxPixels,quality.resolution.maxPixelRatio,quality.maxShadowMaps);
    }
    int appliedQuality_=0;
    void resetAudioScene() {
        audio_->sceneChanged();EM_ASM({window.PawsAudioHost.epoch($0);},audio_->generation());
        previousRoll_=false;previousUse_=0;previousCatTime_=0;
    }
    void updateAudio(double delta) {
        if(!ui_)return;
        const auto screen=ui_->screen();
        if(screen!=audioScreen_){audio_->cue("ui_panel");if(screen==paws::UiScreen::Dialogue)audio_->cue("pip_request");audioScreen_=screen;}
        const bool inPuzzle=!bindings_.garden&&screen!=paws::UiScreen::Title;
        audio_->music(screen==paws::UiScreen::Ending?"music_pip_at_home":inPuzzle?"music_garden_puzzles":"music_garden_afternoon");
        audio_->loop("amb_garden_air",true);
        const bool moving=!ui_->needsPause()||screen==paws::UiScreen::Ending;
        const bool roll=!bindings_.ballRoute.empty();audio_->loop("ball_roll",roll);
        if(previousRoll_&&!roll)audio_->cue("ball_contact");previousRoll_=roll;
        if(gardenUse_.phase!=previousUse_&&gardenUse_.phase==3)audio_->cue(gardenUse_.cushion?"pip_settle":"pip_drink");
        audio_->loop("pip_purr",moving&&gardenUse_.phase==3&&gardenUse_.cushion);
        if(auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(application_.activeScene()->findComponent(bindings_.cat))){
            const auto time=cat->animation().time();
            // Canonical walk contact markers at quarter-cycle intervals. No hidden-time catch-up.
            if(moving&&!ui_->settings().reducedMotion&&(gardenUse_.phase==1||gardenUse_.phase==5)&&previousUse_==gardenUse_.phase&&
                (time<previousCatTime_||int(time/.3)!=int(previousCatTime_/.3)))audio_->cue("pip_paws_stone");
            if(happySeconds_>0&&time>=.6&&previousCatTime_<.6&&!ui_->settings().reducedMotion)audio_->cue("pip_paws_stone");
            previousCatTime_=time;
        }
        previousUse_=gardenUse_.phase;
        if(!suspended_&&bindings_.garden&&moving){birdSeconds_-=delta;if(birdSeconds_<=0){audio_->cue("amb_garden_bird");birdSeconds_=18+double((birdSequence_++*17)%19);}}
        audio_->update(delta);
        char text[256];audio_status(audio_->system().backend().active(),afternoon_.settings.muted,text,sizeof(text));
        if(audioStatus_!=text){audioStatus_=text;projectUi();}
        EM_ASM({window.pawsAudio=({device:$0,voices:$1,pcmBytes:$2,peakPcmBytes:$3,peakVoices:$4,generation:$5});},
            audioDevice_,audio_->system().voiceCount(),audio_->pcmBytes(),audio_->peakPcmBytes(),audio_->peakVoices(),audio_->generation());
    }
    std::unique_ptr<paws::GameAudio> audio_;
    unsigned audioDevice_=0,birdSequence_=0;
    bool audioStarted_=false,previousRoll_=false;
    int previousUse_=0;
    double previousCatTime_=0,birdSeconds_=18;
    paws::UiScreen audioScreen_=paws::UiScreen::Title;
    std::string audioStatus_="Final sound files are not installed. You can play quietly.";
    paws::Afternoon afternoon_;
    engine::Application application_;
    paws::SceneBindings bindings_;
    engine::ComponentId cameraId_;
    struct Gesture {
        int index;
        engine::ActorId actor;
        paws::AttemptId attempt;
        paws::Revision revision;
        engine::Vec2 down;
        engine::Vec3 ground,original,preview;
        bool selected=false,dragged=false;
    };
    std::optional<Gesture> gesture_;
    bool sceneFocusPending_=false;
    std::array<std::vector<std::byte>,3> fonts_;
    std::unique_ptr<paws::PawsUi> ui_;
    std::unique_ptr<engine::OrbitController> orbit_;
    std::unique_ptr<engine::Renderer> renderer_;
    bool suspended_=false;
    float aspect_=2.5f;
    bool shortViewport_=false;
    bool gallery_=false;
    std::optional<std::size_t> pendingQuest_;
    std::size_t failedQuest_=0;
    std::shared_ptr<engine::AssetPreparation> preparation_;
    paws::GardenMeshes meshes_;
    std::shared_ptr<const engine::Texture> portrait_;
    std::shared_ptr<const engine::Texture> cushionPreview_,bowlPreview_,endingPortrait_;
    paws::GardenUse gardenUse_;
    std::string placementItem_,placementSpot_;
    double happySeconds_=0;
    bool completionAfterRoll_=false;
    bool enterAfterPrepare_=false,restartAfterPrepare_=false,gardenAfterPrepare_=false;
    bool restorePauseAfterPrepare_=false;
    std::string lastMessage_;
    bool storageLoaded_=false,hasSave_=false;
    unsigned saveSequence_=0;
    int storageMode_=0;
    bool hasOriginal_=false;
    std::string saveSession_="legacy",storageNotice_;
    std::optional<paws::Afternoon> importCandidate_;
    paws::SaveState saveState_=paws::SaveState::Loading;
};
std::unique_ptr<BrowserGame> game;
void frame(void*) { game->frame(emscripten_get_now()/1000); }
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
EMSCRIPTEN_KEEPALIVE int paws_object_pointer(int phase,float x,float y) { return game?game->objectPointer(phase,x,y):0; }
EMSCRIPTEN_KEEPALIVE void paws_interaction_snapshot() { if(game) game->interactionSnapshot(); }
EMSCRIPTEN_KEEPALIVE int paws_object_key(int key) { return game?game->objectKey(key):0; }
EMSCRIPTEN_KEEPALIVE int paws_blocks_scene() { return !game||game->blocked(); }
EMSCRIPTEN_KEEPALIVE void paws_motion(int reduced) { if(game) game->motion(reduced!=0); }
EMSCRIPTEN_KEEPALIVE int paws_restore(const char* data,int bytes,int status) { return game&&game->restore(data,bytes,status); }
EMSCRIPTEN_KEEPALIVE void paws_saved(unsigned sequence,int success) { if(game) game->saved(sequence,success!=0); }
EMSCRIPTEN_KEEPALIVE void paws_storage_mode(int mode,const char* session,const char* notice,int original) {if(game)game->storageMode(mode,session,notice,original!=0);}
EMSCRIPTEN_KEEPALIVE void paws_retry_save() {if(game)game->retrySave();}
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
    game=std::make_unique<BrowserGame>(); emscripten_set_main_loop_arg(frame,nullptr,0,true);
}




