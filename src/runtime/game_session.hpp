#pragma once
#include "puzzle_scene.hpp"
#include "quest_rewards.hpp"
#include "garden_scene.hpp"
#include "release1/integration/puzzle_scene.hpp"
#include "release1/rules/content.hpp"
#include "release1/props/prop_assets.hpp"
#include "release1/garden/presentation.hpp"
#include "release1/gui/views.hpp"
#include "release1/audio/cues.hpp"
#include "release1/audio/effects.hpp"
#include "release1/cats/cat_assets.hpp"
#include "afternoon.hpp"
#include "beach_assets.hpp"
#include "beach_environment.hpp"
#include "runtime/runtime_host.hpp"
#include "engine/glb_importer.hpp"
#include <optional>
#include "engine/application.hpp"
#include "engine/orbit_controller.hpp"

#if PAWS_TERRAIN
#include "engine/terrain.hpp"
#endif
#if PAWS_DAYTIME_SKY
#include "engine/sky.hpp"
#endif

#include "engine/selection.hpp"
#include "paws_ui.hpp"
#include "game_audio.hpp"
#include "music_playlist.hpp"
#include "garden_ambience.hpp"
#include "pip_sounds.hpp"



#include <memory>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <limits>
#include <cmath>

namespace paws::runtime {
// Keep the shared pinned build compatible until the engine's MSAA API is pinned.
// Updated engine checkouts own allocation, capability fallback and resolve passes.
template<class Quality>
unsigned configureMsaa(Quality& quality,bool high,int override) {
    if constexpr(requires { quality.msaaSamples; }) {
        quality.msaaSamples=high?4u:1u;
        // Explicit diagnostic override for matched-resolution browser comparisons.
        if(override)quality.msaaSamples=unsigned(override);
        return quality.msaaSamples;
    }
    return 1u;
}
inline void applyDaytimeSky(engine::Scene& scene,bool garden) {
#if PAWS_DAYTIME_SKY
    engine::SkyBackground sky;
    sky.geometry=engine::SkyGeometry::Sphere;
    sky.mode=engine::SkyMode::Atmosphere;
    // Match the existing directional key. Sky never takes ownership of lighting.
    sky.atmosphere.sunDirection=garden?engine::Vec3{.5f,1,.6f}:engine::Vec3{.5f,1,.3f};
    sky.atmosphere.mie=.6f;
    // The elevated puzzle camera sees below the horizon around the miniature garden.
    sky.nadir={.85f,1.7f,2.8f};
    sky.clouds.enabled=true;
    sky.clouds.coverage=.35f;
    sky.clouds.density=.65f;
    sky.clouds.wind={0,0};
    sky.dayNight.enabled=false;
    sky.dayNight.running=false;
    scene.setSky(sky);
#else
    (void)scene;(void)garden;
#endif
}
struct CatInput {const char* cat;const char* id;const char* url;};
struct PortraitInput {const char* cat;const char* url;};
#include "release1/integration/accepted.inc"
class GameSession {
public:
    const paws::Afternoon& model() const { return afternoon_; }
    explicit GameSession(RuntimeHost& host) : host_(host), application_(host.application()) {
        beachPreview_=host_.beachPreview;
        afternoon_.setBeachDevelopmentAccess(beachPreview_);
        paws::setForwardIdleCorrection(host_.correctIdlePose);
        auto backend=host_.makeAudio(false);audioDevice_=host_.audioDevice(false);
        auto uiBackend=host_.makeAudio(true);uiAudioDevice_=host_.audioDevice(true);
        audio_=std::make_unique<paws::GameAudio>(std::move(backend),std::move(uiBackend));
        audio_->playlist(paws::gardenMusicPlaylist);
        cues_=std::make_unique<paws::release1::audio::Cues>(*audio_);
        audio_->request=[this](std::size_t i,unsigned g){host_.event(HostEvent::AudioRequest, {i,g});};
        audio_->event=[this](std::string_view e){
            if(e.starts_with("beach_wave_"))if(auto* scene=application_.activeScene())paws::startBeachSurf(*scene);
            std::string n(e);host_.event(HostEvent::AudioEvent, {n.c_str()});
        };
        host_.event(HostEvent::AudioInit, {audioDevice_});
        const auto registerProp=[this](paws::GardenAsset spec){application_.assets().registerAsset<engine::Mesh>(engine::AssetId{spec.id},
            host_.assetLoader(spec.file,[spec](std::span<const std::byte> bytes,const engine::AssetLoadContext& c){
                auto mesh=engine::GlbImporter::loadMemory(bytes);paws::validateGardenAsset(spec,*mesh);c.complete<engine::Mesh>(mesh);}));};
        for(auto spec:paws::gardenAssets())registerProp(spec);
        for(auto spec:paws::beachAssets())registerProp(spec);
        for(auto spec:paws::release1::props::assets())registerProp(spec);
        for(auto spec:paws::questRewardAssets())registerProp(spec);
        application_.assets().registerAsset<engine::Mesh>(engine::AssetId{"garden.quest_marker"},host_.assetLoader("assets/models/garden/quest_marker.glb",
            [](std::span<const std::byte> bytes,const engine::AssetLoadContext& c){c.complete<engine::Mesh>(engine::GlbImporter::loadMemory(bytes));}));
        application_.assets().registerAsset<engine::Mesh>(engine::AssetId{std::string(paws::pipAssetId)},host_.assetLoader(std::string(paws::pipAssetUrl),
            [](std::span<const std::byte> bytes,const engine::AssetLoadContext& c){auto mesh=engine::GlbImporter::loadMemory(bytes);paws::validatePip(*mesh);c.complete<engine::Mesh>(mesh);}));
        for(auto spec:catInputs)if(*spec.url&&std::string_view(spec.id)!=paws::pipAssetId)application_.assets().registerAsset<engine::Mesh>(engine::AssetId{spec.id},host_.assetLoader(spec.url,
            [](std::span<const std::byte> bytes,const engine::AssetLoadContext& c){auto mesh=engine::GlbImporter::loadMemory(bytes);paws::release1::cats::validate(*mesh);c.complete<engine::Mesh>(mesh);}));
        const auto texture=[this](std::string id,std::string url){application_.assets().registerAsset<engine::Texture>(engine::AssetId{id},host_.assetLoader(url,
            [](std::span<const std::byte> bytes,const engine::AssetLoadContext& c){c.complete<engine::Texture>(engine::Texture::loadMemory(bytes,"Paws artwork"));}));};
        texture("pip.portrait","assets/models/pip/portrait-dialogue.png");texture("pip.ending","assets/models/pip/portrait-ending.png");
#if PAWS_TERRAIN
        texture("garden.grass","assets/textures/garden/terrain/meadow-v1.png");
        texture("garden.earth","assets/textures/garden/terrain/earth-v1.png");
        texture("beach.sand","assets/textures/beach/sand-v1.png");
#else
        texture("garden.grass","assets/textures/garden/grass-lawn-v1.png");
#endif
        for(auto spec:catPortraitInputs)if(spec.cat)texture("portrait."+std::string(spec.cat),spec.url);
        for(auto& item:paws::release1::progression::items)texture("shop."+std::string(item.id),"assets/models/release1/props/thumbnails/"+std::string(item.id)+".png");
        // Empty startup view is replaced only after accepted asset preparation.
        auto scene=std::make_unique<engine::Scene>("Preparing Paws");auto& camera=scene->spawn("View").addComponent<engine::CameraComponent>();
        applyDaytimeSky(*scene,true);
        camera.transform.position={0,9,6};camera.lookAt({0,0,0});cameraId_=camera.id();
        orbit_=std::make_unique<engine::OrbitController>(camera);
        auto id=application_.loadScene(std::move(scene));host_.bindScene(id,camera);application_.start(id);applyQuality();observeAudio();publish("Your afternoon is being prepared.");
    }
    void beforeFrame(double timestamp) {
        timestamp_=timestamp; applyQuality();
        const bool hostSuspended=suspended_||!host_.ready()||host_.platformSuspended();
        if(ownerSuspended_!=hostSuspended){ownerSuspended_=hostSuspended;afternoon_.setSuspended(hostSuspended);objectPointer(3,0,0);projectUi();}
        host_.setSuspended(hostSuspended);
        if(!host_.ready())application_.resetClocks();
        if(pendingQuest_&&!hostSuspended)prepareQuest();
        if(ui_){
            const auto screen=ui_->screen();
            if(storageLoaded_&&!pendingQuest_){
                bool paused=afternoon_.userPaused;
                if(screen==paws::UiScreen::Pause)paused=true;
                else if(screen==paws::UiScreen::Garden||screen==paws::UiScreen::Puzzle||screen==paws::UiScreen::Objects||screen==paws::UiScreen::Dialogue||screen==paws::UiScreen::Shop||screen==paws::UiScreen::Ending)paused=false;
                if(paused!=afternoon_.userPaused){afternoon_.setUserPaused(paused,afternoon_.intent(paws::release1::Verb::Select));projectUi();save();}
            }
            double width=0,height=0;host_.viewport(&width,&height);resize(float(width/std::max(height,1.0)),float(height));
            if(ui_->willRebuild(float(width),float(height))){host_.attachGui(nullptr);ui_->update(float(width),float(height));host_.attachGui(&ui_->gui());host_.syncGui(true);}else ui_->update(float(width),float(height));
            const auto b=views_->boardBounds();host_.event(HostEvent::Ui, {int(ui_->screen()),b.x,b.y,b.width,b.height});
            if(auto* camera=dynamic_cast<engine::CameraComponent*>(application_.activeScene()->findComponent(cameraId_))){
                const auto puzzle=afternoon_.puzzleView();
                const bool bridge=!bindings_.garden&&(paws::release1::progression::isBeachQuest(puzzle.quest)||puzzle.quest=="mabel_sunny_spot"||(puzzle.quest.starts_with("pip_friend_")||puzzle.quest.starts_with("mabel_friend_"))||puzzle.pictureSequence||puzzle.content=="pip-pepper-playhouse-v2"||puzzle.quest=="pepper_scent_trail"||puzzle.quest=="pip_wobbly_perch");
                if(bridge&&b.width>0&&b.height>0){
                    camera->setViewport({0,0,1,1});
                    const engine::GuiRect frame{b.x/float(width),b.y/float(height),b.width/float(width),b.height/float(height)};
                    if(frame.x!=bridgeFrame_.x||frame.y!=bridgeFrame_.y||frame.width!=bridgeFrame_.width||frame.height!=bridgeFrame_.height){bridgeFrame_=frame;reset();}
                }else if(ui_->screen()==paws::UiScreen::Ending&&b.width>0&&b.height>0){
                    camera->setViewport({b.x/float(width),b.y/float(height),b.width/float(width),b.height/float(height)});
                    const float fitted=b.width/b.height;if(fitted!=boardAspect_){boardAspect_=fitted;reset();}
                }else camera->setViewport({0,0,1,1});
            }
            const bool reaction=ui_->screen()==paws::UiScreen::Ending||(ui_->screen()==paws::UiScreen::Completion&&(questRewards_.active()||happySeconds_>0));
            if(afternoon_.userPaused||(ui_->needsPause()&&!reaction))application_.pause();else application_.resume();
        }
    }
    engine::ApplicationFrameCallbacks callbacks() {
        engine::ApplicationFrameCallbacks cb;
        cb.simulate=[this](engine::Scene& scene,const engine::ApplicationFrame& f){if(gardenPresentation_)gardenPresentation_->update(scene,f.simulation.delta);};
        cb.present=[this](engine::Scene& scene,const engine::ApplicationFrame& f){if(!bindings_.garden){presentPuzzle(scene,f);} afterFrame(f);};
        return cb;
    }
    void afterFrame(const engine::ApplicationFrame& f) {
        const bool hostSuspended=ownerSuspended_;const double timestamp=timestamp_;
        happySeconds_=std::max(0.0,happySeconds_-f.presentation.delta);updateAudio(f.presentation.delta,f.simulation.delta);
        if(gardenPresentation_&&f.simulation.delta>0){
            bool arrived=false;
            for(std::size_t n=0;n<4;++n){const auto& cat=gardenPresentation_->cats()[n];
                if(cat.eligible&&cat.arrivalComplete&&!cat.introduced)
                    arrived|=afternoon_.introduce(paws::release1::progression::catIds[n],afternoon_.intent(paws::release1::Verb::Select)).accepted;
            }
            if(arrived){gardenPresentation_->project(*application_.activeScene(),afternoon_.progressView(),afternoon_.settings.reducedMotion);save();publish(lastMessage_);projectUi();}
        }
        if(gardenPresentation_){std::ostringstream out;out<<'[';for(std::size_t n=0;n<4;++n){if(n)out<<',';const auto& c=gardenPresentation_->cats()[n];auto* a=application_.activeScene()->findActor(c.actor);out<<"{\"phase\":"<<int(c.phase)<<",\"item\":"<<std::quoted(c.item)<<",\"visible\":"<<(a&&a->enabled)<<",\"x\":"<<(a?a->transform.position.x:0)<<",\"y\":"<<(a?a->transform.position.y:0)<<",\"z\":"<<(a?a->transform.position.z:0)<<",\"yaw\":"<<(a?a->transform.rotation.y:0)<<",\"turn\":"<<((a&&a->getComponent<paws::CatTurn>())?a->getComponent<paws::CatTurn>()->weight:0)<<'}';}out<<']';auto json=out.str();host_.event(HostEvent::Garden, {json.c_str()});}
        host_.event(HostEvent::Lifecycle, {hostSuspended,application_.state()==engine::ApplicationState::Paused,afternoon_.userPaused,f.simulation.delta,f.simulation.seconds});
        if(ui_)ui_->recordFrame(timestamp, !hostSuspended);
#if PAWS_WATER_SHADER
        // Simulation clock freezes under pause/suspension; reduced motion is still.
        // Bound the shader phase for arbitrarily long afternoons (engine limit: 1e6 s).
        host_.setWaterTime(ui_&&ui_->settings().reducedMotion?0:std::fmod(f.simulation.seconds,1000000.));
#endif

        if(sceneFocusPending_&&!blocked()){host_.event(HostEvent::FocusViewport, {});sceneFocusPending_=false;}
    }
    bool command(int action,int object,int direction,unsigned attempt,unsigned revision,const std::string& target={}) {
        if(afternoon_.active()>=3||bindings_.garden)return false;
        const auto& legacy=afternoon_.session();if(attempt!=legacy.state().attempt||revision!=legacy.state().revision)return false;
        using A=paws::Action;using V=paws::release1::Verb;V verb;
        switch(A(action)){case A::Select:verb=V::Select;break;case A::Move:verb=V::Slide;break;case A::Rotate:verb=V::Rotate;break;case A::Roll:verb=V::Roll;break;case A::PickUp:verb=V::PickUp;break;case A::Place:verb=V::Fit;break;case A::Undo:verb=V::Undo;break;case A::Restart:verb=V::Restart;break;case A::Hint:verb=V::Hint;break;default:return false;}
        auto i=afternoon_.intent(verb,object>=0&&object<int(legacy.content().objects.size())?legacy.content().objects[object].id:"",target);i.direction=paws::Direction(direction);return dispatchTyped(i);
    }
    void observeAudio(){const auto p=afternoon_.puzzleView();const auto previous=audio_->generation();cues_->observe({p.generation,p.attempt,p.revision,p.quest});if(previous!=audio_->generation()){birds_.reset();host_.event(HostEvent::AudioEpoch, {audio_->generation()});if(audioStarted_){audio_->preload("reward_coins");audio_->preload("reward_friendship");}}}
    void sequenceTrace(const char* event,const std::string& object={},const std::string& reason={},int after=-1,float x=-1,float y=-1){
        const auto view=afternoon_.puzzleView();if(!view.pictureSequence)return;
        const auto d=paws::release1::rules::quest(view.quest,view.content);
        const auto response=std::find_if(d.objects.begin(),d.objects.end(),[&](const auto& o){return o.id==object;});
        const std::string symbol=response==d.objects.end()?"":response->symbol;
        const std::string expected=view.enteredSymbols.size()<view.symbols.size()?view.symbols[view.enteredSymbols.size()]:"";
        host_.event(HostEvent::SequenceDebug, {event,object.c_str(),symbol.c_str(),expected.c_str(),view.enteredSymbols.size(),after,bindings_.sequencePlayback.watching(),reason.c_str(),x,y,view.attempt,view.revision});
    }
    bool dispatchTyped(paws::release1::Intent i){
        using V=paws::release1::Verb;
        const auto before=afternoon_.puzzleView();if(i.generation!=before.generation||i.attempt!=before.attempt||i.revision!=before.revision||i.quest!=before.quest||suspended_||!host_.ready()||!storageLoaded_||pendingQuest_)return false;
        if(before.quest=="beach_pepper_basket"&&bindings_.pepper.active&&bindings_.pepper.reaction>=3&&(i.verb==V::Inspect||i.verb==V::FollowTrail)){
            publish("Pepper is checking the trail. Let her lead you to the next stop.");projectUi();return false;
        }
        if(i.verb==V::RecordPicnic||i.verb==V::ReplayPicnic){
            if(!finaleBlockage().empty()){publish(finaleBlockage());projectUi();return false;}
            auto candidate=afternoon_;candidate.setUserPaused(false,candidate.intent(V::Select));
            if(i.verb==V::ReplayPicnic&&!candidate.inGarden()){
                auto returned=candidate.returnToGarden(candidate.intent(V::Select));
                if(!returned.accepted){publish(returned.message);projectUi();return false;}
            }
            auto r=candidate.dispatchIntent(candidate.intent(i.verb));if(!r.accepted){publish(r.message);projectUi();return false;}
            // Validate the complete, independently reserved gathering before
            // publishing either its scene or its once-only persistent record.
            try{install(candidate,true);}catch(const std::exception& e){publish(e.what());projectUi();return false;}
            afternoon_=std::move(candidate);reset();observeAudio();showEnding();
            const auto after=afternoon_.puzzleView();cues_->committed({after.generation,after.attempt,after.revision,after.quest,"","picnic_complete"});
            save();publish(r.message);projectUi();return true;
        }
        if(i.verb==V::RecordBeachEnding&&afternoon_.userPaused){afternoon_.setUserPaused(false,afternoon_.intent(V::Select));i=afternoon_.intent(V::RecordBeachEnding);}
        if(i.verb==V::SelectQuest){const auto q=paws::release1::progression::questIndex(i.target);if(q>=paws::release1::progression::quests.size()||!afternoon_.unlocked(q))return false;requestQuest(q,false,false,false);return true;}
        if(before.quest=="pip_friend_ball"&&!bindings_.ballRoute.empty()&&i.verb!=V::Undo&&i.verb!=V::Restart&&i.verb!=V::Hint){publish("Let the ball reach its tray first.");projectUi();return false;}
        if(before.quest=="pip_stuck_gate"&&!bindings_.ballRoute.empty()&&(i.verb==V::PickUp||i.verb==V::Slide||i.verb==V::Rotate)){publish("Wait for the ball to reach the trigger.");projectUi();return false;}
        if(i.verb==V::TapSymbol&&before.pictureSequence&&bindings_.sequencePlayback.watching()){
            sequenceTrace("blocked",i.object,"clue playback");
            publish("Watch all the pictures first. Your turn starts when the blackboard goes blank.");projectUi();return false;
        }
        const bool keepPaused=afternoon_.userPaused&&(i.verb==V::Hint||i.verb==V::Undo);
        if(i.verb==V::TapSymbol)sequenceTrace("tap",i.object,"dispatch");
        const bool pepperMenuAction=before.quest=="beach_pepper_basket"&&(i.verb==V::Inspect||i.verb==V::FollowTrail);
        if(afternoon_.userPaused&&(i.verb==V::Buy||i.verb==V::Restart||keepPaused||pepperMenuAction)){afternoon_.setUserPaused(false,afternoon_.intent(V::Select));auto current=afternoon_.intent(i.verb,i.object,i.target);i=current;}
        auto r=afternoon_.dispatchIntent(i);
        if(i.verb==V::TapSymbol||i.verb==V::ReplaySequence)sequenceTrace("result",i.object,r.message,int(afternoon_.puzzleView().enteredSymbols.size()));
        if(keepPaused)afternoon_.setUserPaused(true,afternoon_.intent(V::Select));
        if(!r.accepted){
            if(before.quest=="beach_pepper_basket")paws::release1::presentation::reactPepper(*application_.activeScene(),bindings_,before,before,i,false,afternoon_.settings.reducedMotion);
            audio_->cue("ui_blocked");publish(r.message);projectUi();return false;}
        observeAudio();const auto after=afternoon_.puzzleView();
        if(before.content!=after.content)install(afternoon_);
        cues_->suspend(suspended_||!host_.ready()||afternoon_.userPaused,!suspended_&&host_.ready()&&afternoon_.userPaused);
        if(audioStarted_&&!suspended_&&!afternoon_.userPaused&&!audio_->system().backend().active())audio_->activate();
        auto cue=[&](std::string name){cues_->committed({after.generation,after.attempt,after.revision,after.quest,i.object,std::move(name)});};
        for(const auto& effect:paws::release1::audio::effects(before,after,i)) {
            if(after.quest=="pip_stuck_gate"&&i.verb==V::Roll&&!afternoon_.settings.reducedMotion&&
               (effect.cue=="switch_down"||effect.cue=="gate_open")) gateArrivalCues_.push_back(effect.cue);
            else cues_->committed(effect);
        }
        if(!before.completed&&after.completed){
            if((after.quest=="mabel_drinking_route"||after.quest=="mabel_friend_water")){cue("water_flow");waterSeconds_=1.5;}
            auto q=paws::release1::progression::questIndex(after.quest);if(q<paws::release1::progression::quests.size())for(unsigned n=0;n<4;++n)if(paws::release1::progression::quests[q].requesters&(1u<<n))cue(std::string(paws::release1::progression::catIds[n])+"_happy");
        }
        if(gardenPresentation_)gardenPresentation_->project(*application_.activeScene(),afternoon_.progressView(),afternoon_.settings.reducedMotion);
        else if(afternoon_.active()<3)paws::projectPuzzle(*application_.activeScene(),bindings_,afternoon_.session(),!afternoon_.settings.reducedMotion&&i.verb!=V::Undo&&i.verb!=V::Restart);
        else {
            if(after.quest=="beach_pepper_basket")paws::release1::presentation::reactPepper(*application_.activeScene(),bindings_,before,after,i,true,afternoon_.settings.reducedMotion);
            paws::release1::presentation::project(*application_.activeScene(),bindings_,after,afternoon_.settings.reducedMotion);
        }
        if(after.quest=="pip_friend_ball"||after.quest==paws::release1::progression::beachQuestId){
            bindings_.ballRoute.clear();bindings_.ballElapsed=0;
            if(i.verb==V::Roll&&!afternoon_.settings.reducedMotion){for(auto cell:before.route)bindings_.ballRoute.push_back(paws::release1::presentation::ballPosition(before.quest,cell,before.content));}
        }
        if(after.quest=="pip_stuck_gate"&&!afternoon_.session().state().puzzle.ballOnSwitch)gateArrivalCues_.clear();
        if(i.verb==V::Roll&&after.quest!="mabel_drinking_route"&&after.quest!="mabel_friend_water"){
            if(bindings_.ballRoute.empty())cue("ball_contact");else ballContactPending_=true;
        }
        if(after.quest=="pip_stuck_gate"&&(i.verb==V::Roll||i.verb==V::Slide))gateFinishPending_=true;
        if(i.verb==V::Undo||i.verb==V::Restart||after.completed)gateFinishPending_=false;
        if(i.verb==V::Undo||i.verb==V::Restart)ballContactPending_=false;
        if(i.verb==V::PlaceFurnishing){placementItem_.clear();ui_->enter(paws::UiScreen::Garden);reset();startUse(i.object);checkEnding();}
        if(i.verb==V::RecordPicnic||i.verb==V::ReplayPicnic){showEnding();cue("picnic_complete");}
        else if(after.completed&&!before.completed){rewardGranted_=afternoon_.lastRewardGranted();if(!bindings_.ballRoute.empty()||bindings_.ending.active){completionAfterRoll_=true;ui_->enter(paws::UiScreen::Puzzle);}else beginQuestRewards();}
        if(i.verb==V::RecordBeachEnding){ui_->enter(paws::UiScreen::BeachEnding);reset();}
        if(!after.completed)completionAfterRoll_=false;
        if(i.verb==V::Undo||i.verb==V::Restart){questRewards_.cancel(*application_.activeScene());audio_->cancel("reward_coins");audio_->cancel("reward_friendship");}
        if(before.completed&&(i.verb==V::Undo||i.verb==V::Restart))reset();
        save(afternoon_.lastRewardGranted()||i.verb==V::Restart||i.verb==V::Buy||i.verb==V::PlaceFurnishing);publish(r.message);projectUi();return true;
    }
    void publish(const std::string& message) {
        lastMessage_=message;const auto p=afternoon_.puzzleView();const auto g=afternoon_.progressView();std::ostringstream o;
        o<<"{\"quest\":"<<std::quoted(p.quest)<<",\"coins\":"<<g.coins<<",\"chapter\":"<<g.chapter<<",\"generation\":"<<g.generation<<",\"attempt\":"<<p.attempt<<",\"revision\":"<<g.revision<<",\"completed\":"<<p.completed<<",\"beachProgress\":"<<p.beachProgress<<",\"beachEndingSeen\":"<<g.beachEndingSeen<<",\"garden\":"<<afternoon_.inGarden()<<",\"paused\":"<<afternoon_.userPaused<<",\"save\":"<<int(saveState_)<<",\"canUndo\":"<<p.canUndo<<",\"picnicSeen\":"<<g.picnicEndingSeen<<",\"picnicEligible\":"<<g.picnicEligible<<",\"production\":"<<bindings_.production<<",\"sceneCount\":"<<application_.loadedSceneCount()<<",\"scene\":"<<std::quoted(application_.activeScene()->name)<<",\"message\":"<<std::quoted(message)<<",\"cats\":[";
        for(std::size_t n=0;n<g.cats.size();++n){if(n)o<<',';auto& c=g.cats[n];o<<"{\"id\":"<<std::quoted(c.id)<<",\"friendship\":"<<c.friendship<<",\"introduced\":"<<c.introduced<<",\"available\":"<<bool(gardenAssets_.cats[n].mesh)<<"}";}
        o<<"],\"items\":[";for(std::size_t n=0;n<g.items.size();++n){if(n)o<<',';auto& i=g.items[n];o<<"{\"id\":"<<std::quoted(i.id)<<",\"owned\":"<<i.owned<<",\"spot\":"<<std::quoted(i.spot)<<",\"available\":"<<i.available<<"}";}
        o<<"],\"actions\":[";for(std::size_t n=0;n<p.actions.size();++n){if(n)o<<',';auto& a=p.actions[n];o<<"{\"label\":"<<std::quoted(a.label)<<",\"verb\":"<<int(a.intent.verb)<<",\"object\":"<<std::quoted(a.intent.object)<<",\"target\":"<<std::quoted(a.intent.target)<<",\"direction\":"<<int(a.intent.direction)<<",\"x\":"<<a.intent.destination.x<<",\"y\":"<<a.intent.destination.y<<",\"enabled\":"<<a.enabled<<"}";}
        o<<"],\"objects\":[";for(std::size_t n=0;n<p.objects.size();++n){if(n)o<<',';auto& a=p.objects[n];o<<"{\"id\":"<<std::quoted(a.id)<<",\"name\":"<<std::quoted(a.name)<<",\"x\":"<<a.cell.x<<",\"y\":"<<a.cell.y<<",\"orientation\":"<<a.orientation<<",\"active\":"<<a.active<<"}";}
        o<<"],\"solution\":[";auto d=paws::release1::rules::quest(p.quest,p.content);for(std::size_t n=0;n<d.solution.size();++n){if(n)o<<',';auto& a=d.solution[n];o<<"{\"verb\":"<<int(a.verb)<<",\"object\":"<<std::quoted(a.object)<<",\"target\":"<<std::quoted(a.target)<<",\"direction\":"<<int(a.direction)<<",\"x\":"<<a.destination.x<<",\"y\":"<<a.destination.y<<"}";}
        const auto& migration=afternoon_.questMigrationView();
        o<<"],\"questKernel\":{\"authoritative\":"<<migration.authoritative<<",\"matched\":"<<migration.matched<<",\"comparisons\":"<<migration.comparisons<<",\"stages\":{";
        bool firstStage=true;for(const auto& [id,stage]:migration.stages){if(!firstStage)o<<',';firstStage=false;o<<std::quoted(id)<<':'<<std::quoted(stage);}
        o<<"}}}";auto json=o.str();host_.event(HostEvent::State, {json.c_str()});
    }
    std::string workshopDropTarget(std::string_view panel,engine::Vec3 landing) const {
        const auto view=afternoon_.puzzleView();const auto d=paws::release1::rules::quest(view.quest,view.content);
        std::string target;float nearest=.65f;
        for(const auto& option:afternoon_.movementOptions(panel)){
            const auto socket=std::find_if(d.sockets.begin(),d.sockets.end(),[&](const auto& t){return t.id==option.target;});
            if(socket==d.sockets.end())continue;
            const auto centre=(paws::release1::presentation::world(view.quest,socket->cell,view.content)+paws::release1::presentation::world(view.quest,socket->end,view.content))*.5f;
            const auto delta=landing-centre;const float distance=std::sqrt(delta.x*delta.x+delta.z*delta.z);
            if(distance<nearest){nearest=distance;target=socket->id;}
        }
        return target;
    }
    void refreshMovementGuidance(std::string_view selected={}) {
        auto* scene=application_.activeScene();if(!scene||bindings_.garden)return;
        const auto view=afternoon_.puzzleView();std::vector<paws::release1::Intent> options;
        // Selection is presentation state, not a delayed command. Pause changes
        // command generations but should retain the player's chosen basket.
        if(selectedBasket_&&(selectedBasket_->attempt!=view.attempt||selectedBasket_->quest!=view.quest))selectedBasket_.reset();
        if(blocked()||suspended_||pendingQuest_)workshopTap_.reset();
        if(selectedPlank_&&(selectedPlank_->attempt!=view.attempt||selectedPlank_->quest!=view.quest))selectedPlank_.reset();
        if(selected.empty()&&questGesture_)selected=questGesture_->stamp.object;
        if(selected.empty()&&selectedBasket_)selected=selectedBasket_->object;
        if(selected.empty()&&selectedPlank_)selected=selectedPlank_->object;
        if(selected.empty()&&gesture_&&gesture_->index>=0&&afternoon_.active()<3)selected=afternoon_.session().content().objects[gesture_->index].id;
        if(!blocked()&&!suspended_&&!pendingQuest_&&!view.completed){
            for(const auto& object:view.objects)if((selected.empty()&&view.quest!="beach_windbreak")||object.id==selected){
                // Legacy sliding rectangles are already projected from the
                // session. Skip the search instead of discarding its result.
                if(afternoon_.active()<3&&std::any_of(view.actions.begin(),view.actions.end(),[&](const auto& a){return a.intent.object==object.id&&a.intent.verb==paws::release1::Verb::Slide;}))continue;
                auto moves=afternoon_.movementOptions(object.id);
                options.insert(options.end(),moves.begin(),moves.end());
            }
        }
        const bool visible=!blocked()&&!suspended_&&!pendingQuest_&&!view.completed;
        std::vector<paws::release1::Intent> turned;
        if(view.quest!="beach_windbreak"&&visible&&!selected.empty()&&std::any_of(view.objects.begin(),view.objects.end(),[&](const auto& o){return o.id==selected&&(o.role=="plank"||o.role=="shade_panel");})){
            auto probe=afternoon_;
            if(probe.dispatchIntent(probe.intent(paws::release1::Verb::Rotate,std::string(selected))).accepted)turned=probe.movementOptions(selected);
        }
        const bool workshopDrag=view.quest=="beach_windbreak"&&questGesture_&&questGesture_->dragged&&visible;
        const auto target=workshopDrag?questGesture_->dropTarget:std::string{};
        if(workshopDrag)std::erase_if(options,[&](const auto& i){return i.target!=target;});
        paws::release1::presentation::previewWorkshop(*scene,bindings_,view,workshopDrag?selected:std::string_view{},target);
        paws::release1::presentation::guidePlank(*scene,bindings_,view,{});
        paws::release1::presentation::guideMovement(*scene,bindings_,view,options,visible?selected:std::string_view{},turned);
    }
    void explainPlank(std::string_view id){
        const auto view=afternoon_.puzzleView();const auto o=std::find_if(view.objects.begin(),view.objects.end(),[&](const auto& o){return o.id==id;});if(o==view.objects.end())return;
        const auto fits=afternoon_.movementOptions(id);auto probe=afternoon_;
        const bool rotated=probe.dispatchIntent(probe.intent(paws::release1::Verb::Rotate,std::string(id))).accepted;
        const bool canTurn=rotated&&!probe.movementOptions(id).empty();
        if(o->role=="shade_panel"){
            publish(view.quest=="beach_windbreak"?(fits.empty()?"No fit this way. Use Rotate to turn the shade.":"Drag to a white fit, or use Rotate."):o->name+" selected. "+(fits.empty()?"No free matching supports in this direction. Tap this panel again to turn.":"White pairs fit now; broken amber pairs need a turn. Tap this panel again to rotate. Every marked cell needs shade."));projectUi();return;
        }
        publish(o->name+" selected. "+(fits.empty()?(canTurn?"Needs a quarter-turn: tap this plank again. Broken amber outlines show where it will fit.":"No free matching gap. Clear the baskets or move another plank first."):(canTurn?"White gaps fit now; broken amber outlines need a turn. Tap this plank again to rotate.":"Drag to a white gap to fit. Tap this plank again to rotate.")));
        projectUi();
    }
    void projectUi(const std::string& message={}) {
        if(!views_||gallery_)return;
        paws::release1::gui::Model m;m.puzzle=afternoon_.puzzleView();m.progress=afternoon_.progressView();m.introductions=afternoon_.pendingIntroductions();m.rewardGranted=rewardGranted_;
        paws::release1::presentation::presentPepperEvidence(m.puzzle,bindings_);
        if(m.puzzle.quest=="beach_windbreak"&&selectedPlank_&&selectedPlank_->attempt==m.puzzle.attempt&&selectedPlank_->quest==m.puzzle.quest)m.workshopSelection=selectedPlank_->object;
        m.puzzle.sequenceWatching=bindings_.sequencePlayback.watching();m.puzzle.sequenceIndex=unsigned(bindings_.sequencePlayback.index());
        auto& v=m.services;v.inGarden=afternoon_.inGarden();v.puzzleReady=storageLoaded_&&!pendingQuest_;v.canContinue=storageLoaded_&&hasSave_;v.canExport=storageLoaded_;v.storageAvailable=storageLoaded_;
        v.save=saveState_;v.storageNotice=storageNotice_;v.canExportOriginal=hasOriginal_;v.feedback=message.empty()?lastMessage_:message;v.audioStatus=audioStatus_;v.audioActive=audio_->system().backend().active();
        v.pipPortrait=portrait_;v.attempt=m.progress.attempt;v.revision=m.progress.revision;
        m.nookQuiet=gardenPresentation_&&gardenPresentation_->cats()[0].item=="mabel_nook";
        m.lookoutQuiet=gardenPresentation_&&gardenPresentation_->cats()[0].item=="pip_lookout";
        m.portraits=portraits_;m.thumbnails=thumbnails_;m.finaleReason=finaleBlockage();
        if(gardenPresentation_)for(auto& item:m.progress.items){auto why=gardenPresentation_->useBlockage(item.id,useCat(item.id));if(item.id=="garden_cushion"||item.id=="drinking_bowl"){m.useReasons["pip:"+item.id]=gardenPresentation_->useBlockage(item.id,0);if(!why.empty())why=m.useReasons["pip:"+item.id];}m.useReasons[item.id]=why;}
        if(!placementItem_.empty())m.placement=afternoon_.placementView(placementItem_);
        views_->project(std::move(m));
        refreshMovementGuidance();
    }
    void startUse(std::string_view item,std::optional<std::size_t> chosenCat={}){
        if(!gardenPresentation_)return;std::string why;std::size_t cat=chosenCat.value_or(useCat(item));bool started=false;
        auto& scene=*application_.activeScene();
        if(item=="leave_pip_lookout"||item=="leave_mabel_nook"){gardenPresentation_->cancelAll(scene);reset();publish("Your seat will be here whenever you return.");return;}
        if(item=="pip_lookout"||item=="mabel_nook")gardenPresentation_->cancelAll(scene);
        if(item=="low_picnic_table"||item=="picnic_mat"){
            gardenPresentation_->cancelAll(scene);started=gardenPresentation_->gather(scene,item,gardenPresentation_->stamp(),why).size()==4;
        }else if(item=="mabel_nook")started=gardenPresentation_->sunnyVisit(scene,gardenPresentation_->stamp(),why);
        else started=bool(gardenPresentation_->use(scene,item,cat,gardenPresentation_->stamp(),why));
        if(!started){
            // Preserve the accepted legacy Pip rest/drink where the new requester is absent.
            if(item=="garden_cushion"||item=="drinking_bowl")started=bool(gardenPresentation_->use(scene,item,0,gardenPresentation_->stamp(),why));
            if(!why.empty())publish(why);
        }
        if(started){auto p=afternoon_.progressView();auto row=std::find_if(p.items.begin(),p.items.end(),[&](auto& i){return i.id==item;});if(row!=p.items.end())for(auto& spot:paws::release1::garden::spots())if(spot.id==row->spot){
            gardenLookUp_=0;
            auto v=orbit_->snapshot();v.target=spot.position+engine::Vec3{0,.55f,0};v.yaw=.25f;v.elevation=.60f;v.distance=std::max(3.5f,3.5f/std::max(.4f,aspect_));orbit_->restore(v);orbit_->orbit(0,0);
        }}
        if(started&&item=="pip_lookout"){
            gardenLookUp_=0;auto v=orbit_->snapshot();v.target=paws::release1::garden::Presentation::lookoutPosition()+engine::Vec3{-.45f,.65f,0};v.yaw=.15f;v.elevation=.42f;v.distance=std::max(3.6f,3.6f/std::max(.4f,aspect_));orbit_->restore(v);orbit_->orbit(0,0);publish("Pip saved you a seat. Stay as long as you like.");
        }
        if(started&&item=="mabel_nook"){gardenLookUp_=0;auto v=orbit_->snapshot();v.target=paws::release1::garden::Presentation::nookPosition()+engine::Vec3{0,.45f,0};v.yaw=.15f;v.elevation=.60f;v.distance=std::max(4.f,4.f/std::max(.4f,aspect_));orbit_->restore(v);orbit_->orbit(0,0);publish("Mabel and Pip settle into their patch of sunshine.");}
        if(started&&item=="playhouse"&&!afternoon_.settings.reducedMotion)playhouseNext_=gardenPresentation_->stamp();
    }
    std::size_t useCat(std::string_view item) const {return item=="drinking_bowl"||item=="sunny_perch"?1:item=="garden_cushion"||item=="hammock"?2:0;}
    std::string finaleBlockage() const {
        for(const auto& cat:gardenAssets_.cats)if(!cat.mesh)return "The four-cat gathering is waiting for accepted cat models. Your picnic progress is kept.";
        for(const auto& cat:gardenAssets_.cats){auto c=cat.contacts.find("picnic_mat");if(c==cat.contacts.end()||!c->second.verified)return "The four-cat gathering is waiting for measured picnic animations. Your picnic progress is kept.";}
        return {};
    }
    void intent(const paws::UiIntent& i) {
        using U=paws::UiAction;using V=paws::release1::Verb;
        if(i.action==U::EnableSound){audio_->retry();host_.event(HostEvent::AudioRetry, {});audioStarted_=true;audio_->activate();return;}
        if(i.action==U::ConfirmSound){audio_->cue(i.target=="ui_select"?"ui_select":i.target=="ui_panel"?"ui_panel":"ui_confirm");return;}
        const auto p=afternoon_.progressView();if(i.attempt!=p.attempt||i.revision!=p.revision)return;
        switch(i.action){
        case U::Settings:{const auto& v=ui_->settings();afternoon_.settings={v.largerText,v.reducedMotion,v.muted,v.music,v.ambience,v.effects,v.quality};audio_->settings(afternoon_.settings);if(gardenPresentation_)gardenPresentation_->project(*application_.activeScene(),afternoon_.progressView(),v.reducedMotion);save();break;}
        case U::Play:case U::Continue:if(storageLoaded_){audioStarted_=true;audio_->activate();requestQuest(afternoon_.active(),i.action==U::Continue&&!afternoon_.inGarden()&&!afternoon_.puzzleView().completed,false,i.action==U::Play||afternoon_.inGarden()||afternoon_.puzzleView().completed,i.action==U::Continue&&ui_->screen()==paws::UiScreen::Title);}break;
        case U::BackToGarden:if(pendingQuest_||ui_->screen()==paws::UiScreen::LoadFailure){cancelPreparation();ui_->enter(meshes_.empty()?paws::UiScreen::Title:afternoon_.inGarden()?paws::UiScreen::Garden:paws::UiScreen::Puzzle);projectUi("Loading cancelled. Your afternoon is safe.");}else requestQuest(afternoon_.active(),false,false,true);break;
        case U::Restart:
            if(afternoon_.inGarden())requestQuest(afternoon_.active(),true,true);
            else if(dispatchTyped(afternoon_.intent(V::Restart)))ui_->enter(paws::UiScreen::Puzzle);
            break;
        case U::RestartAdventure:
            try {
                auto fresh=afternoon_.restartedAdventure();
                if(!meshes_.empty())install(fresh);
                cancelPreparation();afternoon_=std::move(fresh);applyAvailability();
                importCandidate_.reset();hasSave_=storageLoaded_=true;rewardGranted_=false;
                restoreUiSettings();ui_->enter(meshes_.empty()?paws::UiScreen::Title:paws::UiScreen::Garden);
                save();publish("A fresh adventure. Pip has your first request.");projectUi();
            } catch(...) {notice("The adventure could not restart. Your progress is unchanged.");}
            break;
        case U::RetryLoad:requestQuest(failedQuest_,enterAfterPrepare_,restartAfterPrepare_,gardenAfterPrepare_,restorePauseAfterPrepare_);break;
        case U::CancelPlacement:placementItem_.clear();if(gardenPresentation_)gardenPresentation_->previewPlacement(*application_.activeScene(),{},{},false);reset();projectUi();break;
        case U::UseFurnishing:if(i.target=="pip:garden_cushion"||i.target=="pip:drinking_bowl")startUse(std::string_view(i.target).substr(4),0);else startUse(i.target);projectUi();break;
        case U::RetrySave:host_.event(HostEvent::RetrySave, {});break;
        case U::ExportSave:{auto bytes=afternoon_.encode();host_.event(HostEvent::DownloadSave, {bytes.c_str()});break;}
        case U::ExportOriginal:host_.event(HostEvent::ExportOriginal, {});break;
        case U::ImportSave:host_.event(HostEvent::ChooseImport, {unsigned(i.attempt),unsigned(i.revision)});break;
        case U::CancelImport:importCandidate_.reset();break;
        case U::ConfirmImport:if(importCandidate_){try{auto c=*importCandidate_;c.invalidateImportedCommands(afternoon_);if(!meshes_.empty())install(c);cancelPreparation();afternoon_=std::move(c);applyAvailability();importCandidate_.reset();hasSave_=storageLoaded_=true;restoreUiSettings();ui_->enter(paws::UiScreen::Title);save();publish("Imported afternoon ready. Continue when you're ready.");projectUi();}catch(...){notice("The file could not be opened. Your afternoon is unchanged.");}}break;
        case U::ResetView:reset();break;case U::ZoomIn:zoom(-80);break;case U::ZoomOut:zoom(80);break;
        default:break;
        }
    }
    ~GameSession() {host_.attachGui(nullptr);views_.reset();[]<class Gui>(Gui* gui){if constexpr(requires(Gui& value){value.clear();}){if(gui)gui->clear();}}(host_.sharedGui());ui_.reset();cues_.reset();audio_.reset();host_.releaseRendering();orbit_.reset();application_.shutdown();}
    void font(int index,const void* data,int bytes) {
        if(index<0||index>2||bytes<12||bytes>4*1024*1024||ui_) return;
        fonts_[index].resize(bytes);std::memcpy(fonts_[index].data(),data,bytes);
        if(fonts_[0].empty()||fonts_[1].empty()||fonts_[2].empty()) return;
        gallery_=PAWS_GUI_GALLERY&&host_.gallery;
        ui_=std::make_unique<paws::PawsUi>(std::move(fonts_),gallery_,[this](const paws::UiIntent& i){intent(i);},host_.sharedGui());
        views_=std::make_unique<paws::release1::gui::Views>(*ui_,[this](const paws::release1::Intent& i){dispatchTyped(i);},
            [this](std::string_view cat,const paws::release1::Intent& i){auto r=afternoon_.introduce(cat,i);if(r.accepted){if(gardenPresentation_)gardenPresentation_->project(*application_.activeScene(),afternoon_.progressView(),afternoon_.settings.reducedMotion);save();}publish(r.message);projectUi();},
            [this](std::string_view item){placementItem_=item;projectUi();},
            [this](std::string_view item,std::string_view spot){if(!gardenPresentation_)return;auto choices=afternoon_.placementView(item);auto at=std::find_if(choices.begin(),choices.end(),[&](auto& s){return s.id==spot;});gardenPresentation_->previewPlacement(*application_.activeScene(),item,spot,at!=choices.end()&&at->compatible);});
        projectUi();host_.guiReady();
    }
    void escape() {
        if(pendingQuest_) {auto s=afternoon_.progressView();intent({paws::UiAction::BackToGarden,{},s.attempt,s.revision});}
        else if(ui_) ui_->escape();
    }
    void pointer(float x,float y,bool down) { if(ui_) ui_->pointer({x,y},down); }
    bool volumePointer(int action,float x,float y) {return ui_&&action>=0&&action<=3&&ui_->volumePointer(engine::GuiPointer(action),{x,y});}
    bool volumeKey(int key) {return ui_&&ui_->volumeKey(key);}
    // CSS coordinates use a CSS-sized camera viewport, independent of render DPR.
    int objectPointer(int phase,float x,float y) {
        auto* scene=application_.activeScene();
        if(afternoon_.active()>=3||bindings_.garden)return newPointer(phase,x,y);
        if(phase==3) {const bool active=gesture_.has_value();gesture_.reset();if(active&&scene&&!bindings_.garden)paws::projectPuzzle(*scene,bindings_,afternoon_.session(),!bindings_.ballRoute.empty());return 0;}
        const auto& session=afternoon_.session();const auto& state=session.state();const auto& content=session.content();
        if(blocked()||suspended_||pendingQuest_||bindings_.garden||state.paused||state.puzzle.completed) {gesture_.reset();return 0;}
        sceneFocusPending_=true;
        auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findComponent(cameraId_));if(!camera)return 0;
        double width=0,height=0;host_.viewport(&width,&height);
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
            refreshMovementGuidance(content.objects[index].id);
            auto point=ground();if(!point)return 1;
            const auto& now=session.state();auto* actor=scene->findActor(hit->actor);
            gesture_=Gesture{index,hit->actor,now.attempt,now.revision,{x,y},*point,actor->transform.position,actor->transform.position,selected,false};
            return 1;
        }
        if(!gesture_)return 0;
        if(gesture_->attempt!=state.attempt||gesture_->revision!=state.revision||!scene->findActor(gesture_->actor)){gesture_.reset();return 1;}
        auto& g=*gesture_;const float dx=x-g.down.x,dy=y-g.down.y;
        g.dragged|=dx*dx+dy*dy>64;
            if(g.dragged)workshopTap_.reset();
        if(auto point=ground())g.preview=g.original+(*point-g.ground);
        if(phase==1)return 1;
        auto finished=g;gesture_.reset();paws::projectPuzzle(*scene,bindings_,session,!bindings_.ballRoute.empty());
        if(finished.index<0) {
            if(!finished.dragged)command(int(paws::Action::Roll),-1,0,state.attempt,state.revision);
            return 1;
        }
        const auto& object=content.objects[finished.index];
        if(!finished.dragged) {
            if(paws::channel(object.kind))command(int(paws::Action::Rotate),finished.index,0,state.attempt,state.revision);
            else if(object.kind==paws::Kind::Toy&&content.goal==paws::Goal::Retrieve)command(int(paws::Action::PickUp),finished.index,0,state.attempt,state.revision);
            return 1;
        }
        auto dispatch=[&](paws::Action action,int direction=0,const std::string& target={}){
            const auto st=session.state();return command(int(action),finished.index,direction,st.attempt,st.revision,target);
        };
        if(paws::sliding(object.kind)) {
            const float spacing=bindings_.production?1.f:1.45f;
            // Snap the same actor origin shown by the drag preview. Projecting
            // the pointer onto the paving here discards its original grab
            // offset: a pot's rim can then select a different row from its base.
            const paws::Cell target{int(std::round(finished.preview.x/spacing+2)),int(std::round(finished.preview.z/spacing+1))};
            // Use the same complete-path preflight as the visible parking
            // outlines. Never commit a partial path across another obstacle.
            const auto routes=paws::slidingRoutes(content,state.puzzle,finished.index);
            const auto path=std::find_if(routes.begin(),routes.end(),[&](const auto& route){return route.cell==target;});
            if(path!=routes.end()){for(auto d:path->moves)if(!dispatch(paws::Action::Move,int(d)))break;}
            else {
                std::string reason;
                if(routes.size()>1){
                    for(std::size_t i=0;i<content.objects.size();++i)if(i!=std::size_t(finished.index)&&paws::sliding(content.objects[i].kind)&&state.puzzle.objects[i].cell==target)
                        reason=content.objects[i].name+" occupies that space. ";
                    if(reason.empty())reason="That space is outside "+object.name+"'s free parking track. ";
                }
                audio_->cue("ui_blocked");publish(reason+paws::slidingAdvice(content,state.puzzle,finished.index));projectUi();
            }
        } else if(object.kind==paws::Kind::Toy) {
            if(content.goal==paws::Goal::Retrieve)dispatch(paws::Action::PickUp);
            else {
                std::string socket;float nearest=.45f;
                const auto landing=ground();if(!landing)return 1;
                for(std::size_t i=0;i<content.objects.size();++i)if(content.objects[i].kind==paws::Kind::Socket) {
                    auto* a=scene->findActor(bindings_.objects[i].actor);auto delta=a->transform.position-*landing;
                    float distance=std::sqrt(delta.x*delta.x+delta.z*delta.z);if(distance<nearest){nearest=distance;socket=content.objects[i].id;}
                }
                if(socket.empty()&&content.packedToysBlockAccess){auto home=engine::Vec3{float(object.initial.x-2),0,float(object.initial.y-1)};auto delta=*landing-home;if(delta.x*delta.x+delta.z*delta.z<.45f*.45f)socket="home";}
                auto candidate=state.puzzle;candidate.objects[finished.index].location=paws::Location::Held;
                auto reason=paws::pickupBlockage(content,state.puzzle,finished.index);
                if(reason.empty()&&!socket.empty())reason=paws::placeBlockage(content,candidate,finished.index,socket);
                if(!socket.empty()&&reason.empty()) {
                    if(dispatch(paws::Action::PickUp))dispatch(paws::Action::Place,0,socket);
                } else {audio_->cue("ui_blocked");publish(reason.empty()?"Drop the toy into a clear matching recess.":reason);projectUi();}
            }
        } else {publish("This piece stays in place. Tap a channel to turn it.");projectUi();}
        return 1;
    }
    void rewardSnapshot() {
        host_.event(HostEvent::Rewards, {int(questRewards_.phase()),questRewards_.elapsed(),int(questRewards_.recipients()),rewardGranted_});
        auto* scene=application_.activeScene();if(!scene)return;
        auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findComponent(cameraId_));if(!camera)return;
        double width=0,height=0;host_.viewport(&width,&height);const auto viewport=camera->pixelViewport(float(width),float(height));
        for(auto head:questRewards_.origins(*scene))host_.event(HostEvent::RewardOrigin, {head.x,head.y,head.z});
        scene->visit([&](const engine::Node& node,const engine::Mat4& world){
            const auto* actor=dynamic_cast<const engine::Actor*>(&node);
            if(!actor||(node.name!="Coin reward"&&node.name!="Friendship heart reward"))return;
            const auto position=engine::transformPoint(world,{0,0,0});const auto p=camera->worldToScreen(position,viewport);
            const auto* mesh=actor->getComponent<engine::MeshComponent>();
            host_.event(HostEvent::RewardToken, {node.name.c_str(),position.x,position.y,position.z,p.pixel.x,p.pixel.y,mesh->opacity});
        });
    }
    void terrainSnapshot() {
        host_.event(HostEvent::Terrain, {PAWS_TERRAIN});
#if PAWS_TERRAIN
        auto* scene=application_.activeScene();if(!scene)return;
        const engine::TerrainComponent* first=nullptr;engine::Mat4 terrainWorld;
        unsigned chunks=0,slabs=0;
        scene->visit([&](const engine::Node& node,const engine::Mat4& world){
            if(node.name=="Shaded planting bed")++slabs;
            if(auto* terrain=dynamic_cast<const engine::TerrainComponent*>(&node)){
                ++chunks;if(!first){first=terrain;terrainWorld=world;}
            }
        });
        host_.event(HostEvent::TerrainCounts, {chunks,slabs});
        host_.event(HostEvent::TerrainCoastalBegin, {});
        for(const auto& spec:paws::beachAssets()){
            unsigned instances=0;
            auto asset=meshes_.find(spec.name);
            if(asset!=meshes_.end())scene->visit([&](const engine::Node& node,const engine::Mat4&){
                if(auto* m=dynamic_cast<const engine::MeshComponent*>(&node);m&&m->mesh==asset->second)++instances;
            });
            host_.event(HostEvent::TerrainCoastalAsset, {spec.name,instances});
        }
        if(!first)return;
        const auto& data=first->asset()->data();const auto origin=engine::transformPoint(terrainWorld,{0,0,0});
        const float width=(data.samplesX-1)*data.spacing.x,depth=(data.samplesZ-1)*data.spacing.y;
        const auto [low,high]=std::minmax_element(data.heights.begin(),data.heights.end());
        host_.event(HostEvent::TerrainLayersBegin, {});
        for(const auto& layer:data.material->layers.layers)if(layer.texture)
            host_.event(HostEvent::TerrainLayer, {layer.texture->width,layer.texture->height,layer.texture->wrapU==engine::TextureWrap::Repeat&&layer.texture->wrapV==engine::TextureWrap::Repeat});
        host_.event(HostEvent::TerrainBounds, {origin.x,origin.x+width,origin.z,origin.z+depth,*low,*high,data.material->layers.enabled});
        auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findComponent(cameraId_));if(!camera)return;
        double w=0,h=0;host_.viewport(&w,&h);
        const auto viewport=camera->pixelViewport(float(w),float(h));
        for(float x:{viewport.x,viewport.x+viewport.width})for(float y:{viewport.y,viewport.y+viewport.height})for(float d:{0.f,1.f}){
            const auto p=camera->screenToWorld({x,y},d,viewport);
            const bool inside=p.x>origin.x&&p.x<origin.x+width&&p.z>origin.z&&p.z<origin.z+depth;
            host_.event(HostEvent::TerrainFrustum, {inside});
        }
#endif
    }
    void waterSnapshot() {
        host_.event(HostEvent::Water, {PAWS_WATER_SHADER});
#if PAWS_WATER_SHADER
        host_.event(HostEvent::WaterPhase, {host_.waterTime()});
        auto* scene=application_.activeScene();if(!scene)return;
        auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findComponent(cameraId_));if(!camera)return;
        double width=0,height=0;host_.viewport(&width,&height);
        const auto viewport=camera->pixelViewport(float(width),float(height));
        scene->visit([&](const engine::Node& node,const engine::Mat4& world){
            const auto* mesh=dynamic_cast<const engine::MeshComponent*>(&node);
            if(!mesh||mesh->resolvedMaterial(0).shader!=engine::MaterialShader::Water)return;
            const auto& b=mesh->mesh->localBounds();
            const auto p=camera->worldToScreen(engine::transformPoint(world,(b.minimum+b.maximum)*.5f),viewport);
            const auto material=mesh->resolvedMaterial(0);
            host_.event(HostEvent::WaterSurface, {node.name.c_str(),p.pixel.x,p.pixel.y,
                material.water.amplitude,material.water.foamStrength,material.water.refractionStrength,int(mesh->mesh->vertices.size())});
        });
#endif
    }
    void interactionSnapshot() {
        auto* scene=application_.activeScene();
        auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findComponent(cameraId_));if(!camera)return;
        double width=0,height=0;host_.viewport(&width,&height);
        const auto viewport=camera->pixelViewport(float(width),float(height));
        const bool inputBusy=(!bindings_.ballRoute.empty()&&(bindings_.questId=="pip_stuck_gate"||bindings_.questId=="pip_friend_ball"))||
            bindings_.sequencePlayback.watching()||(bindings_.questId=="beach_pepper_basket"&&bindings_.pepper.active&&bindings_.pepper.reaction>=3);
        host_.event(HostEvent::Interaction, {inputBusy});
        host_.event(HostEvent::InteractionGuidesBegin, {});
        const auto selected=selectedBasket_?selectedBasket_->object:std::string{};
        host_.event(HostEvent::InteractionBasket, {selected.c_str()});
        const auto* workshopGhost=scene->findActor(bindings_.workshopPreview);
        const auto workshopTarget=questGesture_?questGesture_->dropTarget:std::string{};
        const auto workshopCells=bindings_.workshopCells.size()-bindings_.workshopCovered.size();
        host_.event(HostEvent::InteractionWorkshop, {workshopGhost&&workshopGhost->enabled,workshopTarget.c_str(),workshopCells});
        const auto plank=selectedPlank_?selectedPlank_->object:std::string{};
        host_.event(HostEvent::InteractionPlank, {plank.c_str()});
        for(const auto id:bindings_.rotationGuides)if(auto* guide=scene->findActor(id);guide&&guide->enabled){
            const auto p=camera->worldToScreen(guide->transform.position,viewport);
            host_.event(HostEvent::InteractionRotationGuide, {p.pixel.x,p.pixel.y,guide->transform.position.x,guide->transform.position.z});
        }
        for(const auto id:bindings_.movementGuides)if(auto* guide=scene->findActor(id);guide&&guide->enabled){
            const auto p=camera->worldToScreen(guide->transform.position,viewport);
            host_.event(HostEvent::InteractionMovementGuide, {p.pixel.x,p.pixel.y,guide->transform.position.x,guide->transform.position.z});
        }
        for(const auto& binding:bindings_.slideGuides)if(auto* guide=scene->findActor(binding.actor);guide&&guide->enabled){
            const auto p=camera->worldToScreen(guide->transform.position,viewport);
            host_.event(HostEvent::InteractionBasketGuide, {p.pixel.x,p.pixel.y,guide->transform.position.x,guide->transform.position.z});
        }
        const auto point=[&](const std::string& id,engine::Vec3 position,bool slot){
            const auto p=camera->worldToScreen(position,viewport);
            host_.event(HostEvent::InteractionObject, {id.c_str(),p.pixel.x,p.pixel.y,slot,position.x,position.y,position.z});
        };
        if(bindings_.garden&&gardenPresentation_){
            if(gardenPresentation_->lookoutReady())if(auto* seat=scene->findActor(gardenPresentation_->lookoutSeat())){
                auto* mesh=seat->getComponent<engine::MeshComponent>();const auto& b=mesh->mesh->localBounds();point("lookout_seat",engine::transformPoint(mesh->worldMatrix(),(b.minimum+b.maximum)*.5f),false);
            }
            if(gardenPresentation_->nookReady())if(auto* seat=scene->findActor(gardenPresentation_->nookSeat())){auto* mesh=seat->getComponent<engine::MeshComponent>();const auto& b=mesh->mesh->localBounds();point("nook_seat",engine::transformPoint(mesh->worldMatrix(),(b.minimum+b.maximum)*.5f),false);}
            host_.event(HostEvent::QuestMarkers, {});
            for(std::size_t n=0;n<4;++n){const auto& binding=gardenPresentation_->cats()[n];
                if(auto* marker=scene->findActor(binding.questMarker)){
                    host_.event(HostEvent::QuestMarker, {n,marker->enabled,marker->transform.rotation.y,marker->transform.position.y});
                }
                if(auto* actor=scene->findActor(binding.actor);actor&&actor->enabled){
                    auto* mesh=actor->getComponent<engine::MeshComponent>();const auto& bounds=mesh->mesh->localBounds();
                    point(std::string(paws::release1::progression::catIds[n]),engine::transformPoint(actor->transform.matrix(),(bounds.minimum+bounds.maximum)*.5f),false);
                }
            }return;
        }
        for(const auto& binding:bindings_.objects)if(auto* actor=scene->findActor(binding.actor)) {
            auto position=actor->transform.position;
            if(auto* mesh=actor->getComponent<engine::MeshComponent>()) {
                const auto& b=mesh->mesh->localBounds();position=engine::transformPoint(actor->transform.matrix(),(b.minimum+b.maximum)*.5f);
                if(binding.object=="balance_beam"){
                    const float y=(b.minimum.y+b.maximum.y)*.5f,z=(b.minimum.z+b.maximum.z)*.5f;
                    point("balance_beam.left",engine::transformPoint(mesh->worldMatrix(),{b.minimum.x,y,z}),false);
                    point("balance_beam.right",engine::transformPoint(mesh->worldMatrix(),{b.maximum.x,y,z}),false);
                }
                if(binding.object.starts_with("panel_")){
                    unsigned corner=0;
                    for(float x:{b.minimum.x,b.maximum.x})for(float y:{b.minimum.y,b.maximum.y})for(float z:{b.minimum.z,b.maximum.z})
                        point(binding.object+".corner_"+std::to_string(corner++),engine::transformPoint(mesh->worldMatrix(),{x,y,z}),false);
                }
            }
            if(binding.object=="sequence_board")position=actor->transform.position+engine::Vec3{0,1.6f,.105f};
            point(binding.object,position,false);
            if(binding.object=="leaf"||binding.object=="star"||binding.object=="paw")point(binding.object+".picture",engine::transformPoint(actor->transform.matrix(),{0,-.20f,.24f}),false);
        }
        if(auto* actor=scene->findActor(bindings_.ball))point("source_ball",actor->transform.position+engine::Vec3{0,.06f,0},false);
        if(!bindings_.garden&&afternoon_.inBeach())for(std::size_t n=0;n<bindings_.requesters.size();++n){
            auto* actor=scene->findActor(bindings_.requesters[n].actor);if(!actor)continue;
            if(auto* skin=actor->getComponent<engine::SkeletalMeshComponent>()){
                const auto bounds=skin->localBounds();unsigned corner=0;
                for(float x:{bounds.minimum.x,bounds.maximum.x})for(float y:{bounds.minimum.y,bounds.maximum.y})for(float z:{bounds.minimum.z,bounds.maximum.z})
                    point("beach_cat_"+std::to_string(n)+"_"+std::to_string(corner++),engine::transformPoint(actor->transform.matrix(),{x,y,z}),false);
            }
        }
        if(!bindings_.garden&&afternoon_.active()>=3){
            const auto definition=paws::release1::rules::quest(afternoon_.puzzleView().quest,afternoon_.puzzleView().content);
            for(const auto& socket:definition.sockets){
                auto at=(socket.role=="plank_socket"||socket.role=="shade_socket")?(paws::release1::presentation::world(definition.quest,socket.cell,definition.content)+paws::release1::presentation::world(definition.quest,socket.end,definition.content))*.5f:paws::release1::presentation::world(definition.quest,socket.cell,definition.content);
                if(socket.role=="lookout_seat")at.y=.33f;
                point(socket.id,at,true);
            }
            for(int y=0;y<definition.height;++y)for(int x=0;x<definition.width;++x)point(std::to_string(x)+","+std::to_string(y),paws::release1::presentation::world(definition.quest,{x,y},definition.content),true);
            const auto view=afternoon_.puzzleView();
            for(const auto& o:view.objects)if(o.role.starts_with("cushion_"))for(const auto& binding:bindings_.objects)if(binding.object==o.id){
                if(auto* actor=scene->findActor(binding.actor))if(auto* mesh=actor->getComponent<engine::MeshComponent>()){
                    const auto& bounds=mesh->mesh->localBounds();const auto offset=engine::transformPoint(actor->transform.matrix(),(bounds.minimum+bounds.maximum)*.5f)-actor->transform.position;
                    for(auto cell:definition.basket)point(o.id+"@"+std::to_string(cell.x)+","+std::to_string(cell.y),paws::release1::presentation::shapePosition(view.quest,o.id,cell,o.orientation,view.content)+offset,true);
                }
            }
            return;
        }
        if(afternoon_.active()>=3)return;
        const auto& content=afternoon_.session().content();
        for(std::size_t i=0;i<content.objects.size();++i)if(content.objects[i].kind==paws::Kind::Socket)
            if(auto* actor=scene->findActor(bindings_.objects[i].actor))point(content.objects[i].id,actor->transform.position,true);
        if(content.packedToysBlockAccess)for(const auto& o:content.objects)if(o.kind==paws::Kind::Toy)point(o.id+"_home",{float(o.initial.x-2),0,float(o.initial.y-1)},true);
        for(int y=0;y<content.height;++y)for(int x=0;x<content.width;++x)point(std::to_string(x)+","+std::to_string(y),{float(x-2),0,float(y-1)},true);
    }
    void beginQuestRewards() {
        happySeconds_=0;
        ui_->enter(paws::UiScreen::Completion);
        if(rewardGranted_){
            auto& scene=*application_.activeScene();questRewards_.start(scene,ui_->settings().reducedMotion);
            const auto heads=questRewards_.origins(scene);
            if(orbit_&&!heads.empty()){
                auto low=heads.front(),high=low;for(auto p:heads){low={std::min(low.x,p.x),std::min(low.y,p.y),std::min(low.z,p.z)};high={std::max(high.x,p.x),std::max(high.y,p.y),std::max(high.z,p.z)};}
                auto view=orbit_->snapshot();view.target=(low+high)*.5f+engine::Vec3{0,.2f,0};view.yaw=0;view.elevation=.62f;
                const float span=std::max(high.x-low.x,high.z-low.z)+1.6f;
                view.distance=std::max(6.5f,span/(2*std::tan(engine::pi*17.5f/180))*std::max(1.f,1/std::max(.3f,aspect_)));
                orbit_->restore(view);orbit_->orbit(0,0);
            }
        }
        projectUi();
    }
    void presentPuzzle(engine::Scene& scene,const engine::ApplicationFrame& frame) {
        if(gesture_&&(blocked()||suspended_))objectPointer(3,0,0);
        const bool routePaused=ui_&&(ui_->needsPause()||afternoon_.userPaused||suspended_);
        const auto previousPicture=bindings_.sequencePlayback.index();
        const auto previousEvidence=bindings_.pepper.awaitingClues;
        if(afternoon_.active()<3)paws::animatePuzzle(scene,bindings_,routePaused?0.f:float(frame.presentation.delta),ui_&&ui_->settings().reducedMotion);
        else {bindings_.reducedMotion=ui_&&ui_->settings().reducedMotion;paws::release1::presentation::animate(scene,bindings_,routePaused?0:frame.presentation.delta);}
        if(bindings_.questId=="beach_pepper_basket"){
            if(previousEvidence!=bindings_.pepper.awaitingClues)projectUi();
            const auto& p=bindings_.pepper;engine::Vec3 position{};
            if(!bindings_.requesters.empty())if(auto* cat=scene.findActor(bindings_.requesters[0].actor))position=cat->transform.position;
            host_.event(HostEvent::Pepper, {p.active,p.reaction,p.stage,p.inspected,p.returning,p.arrived,p.elapsed,position.x,position.y,position.z});
        }else host_.event(HostEvent::PepperClear, {});
        if(bindings_.sequencePlayback.size()){
            const auto picture=bindings_.sequencePlayback.picture();
            host_.event(HostEvent::Sequence, {bindings_.sequencePlayback.watching(),bindings_.sequencePlayback.index(),bindings_.sequencePlayback.size(),picture.c_str(),bindings_.sequencePlayback.elapsed()});
            if(previousPicture!=bindings_.sequencePlayback.index()){
                if(!bindings_.sequencePlayback.watching())publish("Your turn. Repeat the pictures you watched.");
                projectUi();
            }
        }else host_.event(HostEvent::SequenceClear, {});
        if(auto* cat=scene.findActor(bindings_.ending.actor)){
            const auto& e=bindings_.ending;
            if(e.active&&e.elapsed>0&&!routePaused&&!ui_->settings().reducedMotion&&orbit_){
                auto view=orbit_->snapshot();const float blend=1-std::exp(-float(frame.presentation.delta)*1.8f);
                const auto focus=(e.kind>=4?cat->transform.position:e.target)+engine::Vec3{-.15f,.65f,-.3f};
                view.target=view.target+(focus-view.target)*blend;
                view.distance+=(std::max(6.5f,3.f/std::max(.3f,aspect_))-view.distance)*blend;
                view.yaw+=std::remainder((e.kind==3?0.f:(e.kind==4||e.kind==5||e.kind==8)?.9f:-.35f)-view.yaw,2*engine::pi)*blend;
                view.elevation+=((e.kind==3?.85f:.62f)-view.elevation)*blend;orbit_->restore(view);orbit_->orbit(0,0);
            }
            host_.event(HostEvent::Ending, {e.active,e.finished,e.kind,e.elapsed,e.walkSeconds+e.leadSeconds,cat->transform.position.x,cat->transform.position.y,cat->transform.position.z});
        }
        if(bindings_.ballRoute.empty()&&!gateArrivalCues_.empty()) {
            const auto p=afternoon_.puzzleView();
            for(const auto& cue:gateArrivalCues_)cues_->committed({p.generation,p.attempt,p.revision,p.quest,"",cue});
            gateArrivalCues_.clear();
        }
        // Finish through the existing authoritative retrieval/reward path once
        // the ball has arrived and the gate has finished opening.
        if(afternoon_.active()<3&&gateFinishPending_&&!routePaused&&!afternoon_.userPaused&&!suspended_&&
           bindings_.ballRoute.empty()&&bindings_.motion.empty()&&bindings_.arrivalMotion.empty()) {
            gateFinishPending_=false;
            const auto& session=afternoon_.session();
            auto item=std::find_if(session.content().objects.begin(),session.content().objects.end(),[&](const auto& o){return o.id==session.content().requestedItem;});
            if(session.content().questId=="pip_stuck_gate"&&session.state().puzzle.ballOnSwitch&&!session.state().puzzle.completed&&
               item!=session.content().objects.end()&&paws::pickupBlockage(session.content(),session.state().puzzle,std::size_t(item-session.content().objects.begin())).empty())
                dispatchTyped(afternoon_.intent(paws::release1::Verb::PickUp,session.content().requestedItem));
        }
        if(completionAfterRoll_&&bindings_.ballRoute.empty()&&!bindings_.ending.active) {
            completionAfterRoll_=false;
            if(ui_&&afternoon_.puzzleView().completed&&!bindings_.garden) {
                beginQuestRewards();
            }
        }
        if(ui_&&ui_->screen()==paws::UiScreen::Completion&&!afternoon_.userPaused&&!suspended_&&host_.ready()){
            auto* camera=dynamic_cast<engine::CameraComponent*>(scene.findComponent(cameraId_));
            const auto cues=questRewards_.advance(scene,frame.presentation.delta,camera?camera->worldPosition():engine::Vec3{0,2,6},ui_->settings().reducedMotion);
            const auto p=afternoon_.puzzleView();
            if(cues&paws::QuestRewards::CoinSound)cues_->committed({p.generation,p.attempt,p.revision,p.quest,"reward_sequence","reward_coins"});
            if(cues&paws::QuestRewards::PurrSound)cues_->committed({p.generation,p.attempt,p.revision,p.quest,"reward_sequence","reward_friendship"});
        }
        host_.event(HostEvent::RewardPhase, {int(questRewards_.phase()),questRewards_.elapsed(),int(questRewards_.recipients()),rewardGranted_});
        if(auto* ball=scene.findActor(bindings_.ball)) {
            const auto centreOffset=bindings_.production?paws::gardenAnchor("ball","centre"):engine::Vec3{};
            const auto centre=engine::transformPoint(ball->transform.matrix(),centreOffset);
            const auto routePose=centre-centreOffset;
            host_.event(HostEvent::Ball, {!bindings_.ballRoute.empty(),routePose.x,routePose.y,routePose.z,completionAfterRoll_,centre.y});
            if(bindings_.questId=="pip_stuck_gate") for(const auto& binding:bindings_.objects) {
                if(auto* actor=scene.findActor(binding.actor)) {
                    if(binding.object=="gate")host_.event(HostEvent::BallGate, {actor->transform.rotation.y});
                    if(binding.object=="switch")host_.event(HostEvent::BallTrigger, {actor->transform.position.y});
                }
            }
        }
        if(!gesture_||!gesture_->dragged||gesture_->index<0)return;
        const auto& state=afternoon_.session().state();
        if(gesture_->attempt!=state.attempt||gesture_->revision!=state.revision){gesture_.reset();return;}
        const auto kind=afternoon_.session().content().objects[gesture_->index].kind;
        if(paws::sliding(kind)||kind==paws::Kind::Toy)
            if(auto* actor=scene.findActor(gesture_->actor))actor->transform.position=gesture_->preview;
    }
    int objectKey(int key) {
        if(blocked()||bindings_.garden||afternoon_.active()>=3)return 0;
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
    bool blocked() const { return !ui_||ui_->blocksScene()||bindings_.ending.active||(bindings_.questId=="beach_pepper_basket"&&bindings_.pepper.active&&bindings_.pepper.reaction>=3); }
    void motion(bool reduced) { if(ui_&&!hasSave_) {ui_->setReducedMotion(reduced);afternoon_.settings.reducedMotion=reduced;} }
    void save(bool immediate=true) {
        if(gallery_||!storageLoaded_) return;
        if(afternoon_.saveRevision>=std::numeric_limits<unsigned>::max()-2){saveState_=paws::SaveState::Failed;projectUi("Save revision limit reached. Export your afternoon.");return;}
        ++afternoon_.saveRevision;afternoon_.saveSession=saveSession_;
        saveState_=storageMode_==1?paws::SaveState::SecondTab:storageMode_==2?paws::SaveState::Temporary:paws::SaveState::Saving;
        auto bytes=afternoon_.encode();auto sequence=++saveSequence_;
        host_.event(HostEvent::StoreSave, {bytes.c_str(),sequence,immediate,afternoon_.saveRevision});
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
    void cloudSnapshot(){const auto bytes=afternoon_.encode();host_.event(HostEvent::CloudSnapshot, {bytes.c_str()});}
    void cloudStamp(){host_.event(HostEvent::CloudStamp, {afternoon_.progressView().attempt,afternoon_.ownerRevision()});}
    void cloudBeachAccess(bool value){
        cloudBeachAccess_=value;
        afternoon_.setBeachDevelopmentAccess(beachPreview_||cloudBeachAccess_);
        if(importCandidate_)configureAdventureAccess(*importCandidate_);
        if(!beachPreview_&&!cloudBeachAccess_&&afternoon_.inBeach()){
            cancelPreparation();auto candidate=afternoon_;configureAdventureAccess(candidate);
            if(!meshes_.empty())install(candidate);
            afternoon_=std::move(candidate);save();ui_->enter(paws::UiScreen::Garden);
        }
        projectUi();publish(lastMessage_);
    }
    void restoreUiSettings() {
        const auto& s=afternoon_.settings;
        audio_->settings(s);
        if(ui_)ui_->restoreSettings({s.largerText,s.reducedMotion,s.muted,s.music,s.ambience,s.effects,s.quality});
        applyQuality();
    }
    bool stageImport(const char* data,int bytes,unsigned attempt,unsigned revision) {
        if(!storageLoaded_||bytes<0||bytes>16*1024*1024||(!data&&bytes)||attempt!=afternoon_.progressView().attempt||revision!=afternoon_.ownerRevision())return false;
        try {auto candidate=paws::Afternoon::decode(std::string_view(data,bytes));configureAdventureAccess(candidate);importCandidate_=std::move(candidate);ui_->show(paws::UiScreen::ImportConfirm);projectUi("File checked. Choose whether to use it.");return true;}
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
                configureAdventureAccess(candidate);
                afternoon_=std::move(candidate);hasSave_=true;
                restoreUiSettings();
            }
            storageLoaded_=true;saveState_=storageMode_==1?paws::SaveState::SecondTab:storageMode_==2?paws::SaveState::Temporary:hasSave_?paws::SaveState::Saved:paws::SaveState::Temporary;
            projectUi();publish("Your afternoon is ready.");return true;
        } catch(const std::exception&) {
            saveState_=paws::SaveState::Failed;projectUi("Your saved afternoon could not be opened. It has been kept unchanged. Retry from Save and recovery.");
            if(ui_) ui_->show(paws::UiScreen::Recovery);return false;
        }
    }
    void configureAdventureAccess(paws::Afternoon& candidate) {
        candidate.setBeachDevelopmentAccess(beachPreview_||cloudBeachAccess_);
        if(candidate.inBeach()&&!beachPreview_&&!cloudBeachAccess_) {
            const auto returned=candidate.returnToGarden(candidate.intent(paws::release1::Verb::Select));
            if(!returned.accepted)throw std::runtime_error(returned.message);
        }
    }
    void install(const paws::Afternoon& candidate,bool picnic=false) {
        paws::PuzzleScene puzzle;std::optional<paws::release1::garden::Presentation> garden;
        if(candidate.inGarden()) {auto b=paws::release1::garden::build(gardenAssets_,candidate.progressView(),candidate.settings.reducedMotion);puzzle.scene=std::move(b.scene.scene);puzzle.camera=b.scene.camera;puzzle.bindings.garden=puzzle.bindings.production=true;garden=std::move(b.presentation);}
        else puzzle=candidate.active()<3?paws::makePuzzleScene(candidate.session(),&meshes_):paws::release1::presentation::build(candidate.puzzleView(),meshes_,candidate.settings.reducedMotion);
        applyDaytimeSky(*puzzle.scene,candidate.inGarden());
#if PAWS_TERRAIN
        // Keep distant slopes beyond portrait framing visible. Terrain extends
        // beyond all eight frustum corners for the supported viewport range.
        puzzle.camera->setPerspective(puzzle.camera->verticalFov(),puzzle.camera->nearPlane,candidate.inBeach()?80.f:180.f);
#endif
        paws::QuestRewards rewards;
        if(!candidate.inGarden()){
            std::vector<engine::ActorId> cats;
            if(candidate.active()<3)cats.push_back(puzzle.bindings.ending.actor);
            else {
                for(const auto& cat:puzzle.bindings.requesters)cats.push_back(cat.actor);
                if(candidate.active()==8)for(const auto& object:puzzle.bindings.objects)
                    if(object.object=="pip"||object.object=="pepper")cats.push_back(object.actor);
            }
            rewards.install(*puzzle.scene,meshes_,cats);
        }
        if(picnic){
            if(!garden)throw std::runtime_error("Return to the garden for our picnic.");
            garden->celebrationProps(*puzzle.scene,garden->stamp(),true);std::string why;
            if(garden->gather(*puzzle.scene,"picnic_mat",garden->stamp(),why,true).size()!=4)
                throw std::runtime_error("The gathering could not prepare: "+why+" Your picnic remains ahead.");
        }
        objectPointer(3,0,0);auto* scene=puzzle.scene.get();auto* camera=puzzle.camera;auto controller=std::make_unique<engine::OrbitController>(*camera);
        const auto previous=application_.activeSceneId();auto id=application_.loadScene(std::move(puzzle.scene));
        try{host_.bindScene(id,*camera);}catch(...){application_.unloadScene(id);throw;}
        selectedBasket_.reset();
        selectedPlank_.reset();
        resetAudioScene();questRewards_=std::move(rewards);orbit_=std::move(controller);cameraId_=camera->id();bindings_=std::move(puzzle.bindings);gardenPresentation_=std::move(garden);happySeconds_=0;completionAfterRoll_=false;placementItem_.clear();placementSpot_.clear();
        gateFinishPending_=!candidate.inGarden()&&candidate.active()==2&&candidate.session().state().puzzle.ballOnSwitch&&!candidate.session().state().puzzle.completed;
        application_.requestTransition(id);application_.commitTransition();application_.unloadScene(previous);reset();
    }
    void applyAvailability(){for(auto& item:paws::release1::progression::items)afternoon_.setItemAvailable(item.id,!meshes_.empty());}
    void cancelPreparation() {
        pendingQuest_.reset();if(preparation_) preparation_->cancel();preparation_.reset();application_.assets().collectUnused();
    }
    void requestQuest(std::size_t q,bool enter,bool restartAttempt=false,bool garden=false,bool restorePause=false) {
        if(!storageLoaded_||q>=paws::release1::progression::quests.size()||!afternoon_.unlocked(q))return;
        if(auto* scene=application_.activeScene())questRewards_.cancel(*scene);
        audio_->cancel("reward_coins");audio_->cancel("reward_friendship");
        cancelPreparation();pendingQuest_=q;enterAfterPrepare_=enter;restartAfterPrepare_=restartAttempt;gardenAfterPrepare_=garden;restorePauseAfterPrepare_=restorePause;
        if(meshes_.empty()){
            std::vector<engine::AssetId> ids;for(auto spec:paws::gardenAssets())ids.emplace_back(spec.id);for(auto spec:paws::release1::props::assets())ids.emplace_back(spec.id);
            for(auto spec:paws::beachAssets())ids.emplace_back(spec.id);
            for(auto spec:paws::questRewardAssets())ids.emplace_back(spec.id);
            ids.emplace_back(std::string(paws::pipAssetId));ids.emplace_back("pip.portrait");ids.emplace_back("pip.ending");
            ids.emplace_back("garden.grass");
#if PAWS_TERRAIN
            ids.emplace_back("garden.earth");
            ids.emplace_back("beach.sand");
#endif
            ids.emplace_back("garden.quest_marker");
            for(auto spec:catInputs)if(*spec.url&&std::string_view(spec.id)!=paws::pipAssetId)ids.emplace_back(spec.id);
            for(auto spec:catPortraitInputs)if(spec.cat)ids.emplace_back("portrait."+std::string(spec.cat));
            for(auto& item:paws::release1::progression::items)ids.emplace_back("shop."+std::string(item.id));
            preparation_=std::make_shared<engine::AssetPreparation>(application_.assets(),std::move(ids));
        }
        publish("Loading the garden… Your progress is safe while the models arrive.");projectUi();ui_->enter(paws::UiScreen::Loading);
    }
    void prepareQuest(){
        const auto index=*pendingQuest_;
        try{
            if(preparation_){const auto p=preparation_->progress();if(p.status==engine::AssetStatus::Failed||p.status==engine::AssetStatus::Cancelled)throw std::runtime_error(p.error);if(p.status!=engine::AssetStatus::Ready)return;
                paws::GardenMeshes ready;for(auto spec:paws::gardenAssets())ready.emplace(spec.name,preparation_->get<engine::Mesh>(engine::AssetId{spec.id}));for(auto spec:paws::release1::props::assets())ready.emplace(spec.name,preparation_->get<engine::Mesh>(engine::AssetId{spec.id}));
                for(auto spec:paws::beachAssets())ready.emplace(spec.name,preparation_->get<engine::Mesh>(engine::AssetId{spec.id}));
                for(auto spec:paws::questRewardAssets())ready.emplace(spec.name,preparation_->get<engine::Mesh>(engine::AssetId{spec.id}));
                ready.emplace("pip",preparation_->get<engine::Mesh>(engine::AssetId{std::string(paws::pipAssetId)}));
                auto lawn=std::make_shared<engine::Mesh>(*engine::Mesh::cube());
                auto grass=std::make_shared<engine::Material>();grass->name="Garden lawn";
                grass->baseColorTexture=preparation_->get<engine::Texture>(engine::AssetId{"garden.grass"});
                grass->roughness=1;
                lawn->materials={grass};ready.emplace("grass_ground",std::move(lawn));
#if PAWS_TERRAIN
                auto earth=std::make_shared<engine::Material>();earth->name="ImageGen limestone earth";
                earth->baseColorTexture=preparation_->get<engine::Texture>(engine::AssetId{"garden.earth"});earth->roughness=1;
                auto carrier=std::make_shared<engine::Mesh>(*engine::Mesh::plane(1));carrier->materials={earth};
                ready.emplace("terrain_earth",std::move(carrier));
                auto sand=std::make_shared<engine::Material>();sand->name="ImageGen coastal sand";
                sand->baseColorTexture=preparation_->get<engine::Texture>(engine::AssetId{"beach.sand"});sand->roughness=.92f;
                auto beachCarrier=std::make_shared<engine::Mesh>(*engine::Mesh::plane(1));beachCarrier->materials={sand};
                ready.emplace("beach_sand",std::move(beachCarrier));
#endif
                for(auto spec:catInputs)if(*spec.url&&std::string_view(spec.id)!=paws::pipAssetId)ready[spec.cat]=preparation_->get<engine::Mesh>(engine::AssetId{spec.id});
                portrait_=preparation_->get<engine::Texture>(engine::AssetId{"pip.portrait"});portraits_["pip"]=portrait_;endingPortrait_=preparation_->get<engine::Texture>(engine::AssetId{"pip.ending"});
                for(auto& item:paws::release1::progression::items)thumbnails_[std::string(item.id)]=preparation_->get<engine::Texture>(engine::AssetId{"shop."+std::string(item.id)});
                for(auto spec:catPortraitInputs)if(spec.cat)portraits_[spec.cat]=preparation_->get<engine::Texture>(engine::AssetId{"portrait."+std::string(spec.cat)});
                portrait_=portraits_.at("pip");
                ready.emplace("quest_marker",preparation_->get<engine::Mesh>(engine::AssetId{"garden.quest_marker"}));
                paws::prepareGardenPresentationMeshes(ready);
                meshes_=std::move(ready);gardenAssets_.props=meshes_;gardenAssets_.cats[0].mesh=meshes_.at("pip");
                if(std::string_view(catInputs[0].id)==paws::pipAssetId)gardenAssets_.cats[0]=paws::release1::garden::legacyPip(meshes_.at("pip"));
                for(std::size_t n=1;n<4;++n)if(meshes_.contains(catInputs[n].cat))gardenAssets_.cats[n].mesh=meshes_.at(catInputs[n].cat);
                acceptedContacts(gardenAssets_);preparation_.reset();
            }
            auto candidate=afternoon_;const bool pause=candidate.userPaused&&restorePauseAfterPrepare_;
            candidate.setUserPaused(false,candidate.intent(paws::release1::Verb::Select));
            auto r=candidate.dispatchIntent(candidate.intent(paws::release1::Verb::SelectQuest,{},std::string(paws::release1::progression::quests[index].id)));if(!r.accepted)throw std::runtime_error(r.message);
            if(gardenAfterPrepare_)candidate.returnToGarden(candidate.intent(paws::release1::Verb::Select));
            else if(restartAfterPrepare_||(candidate.puzzleView().completed&&!candidate.inBeach())){auto restarted=candidate.dispatchIntent(candidate.intent(paws::release1::Verb::Restart));if(!restarted.accepted)throw std::runtime_error(restarted.message);}
            for(auto& item:paws::release1::progression::items)candidate.setItemAvailable(item.id,true);
            candidate.setUserPaused(pause,candidate.intent(paws::release1::Verb::Select));install(candidate);afternoon_=std::move(candidate);pendingQuest_.reset();reset();observeAudio();
            ui_->enter(enterAfterPrepare_?paws::UiScreen::Puzzle:gardenAfterPrepare_?paws::UiScreen::Garden:paws::UiScreen::Dialogue);if(pause)ui_->show(paws::UiScreen::Pause);
            if(afternoon_.inBeach()&&afternoon_.puzzleView().completed)ui_->enter(paws::UiScreen::Completion);
            if(!gardenAfterPrepare_){requestSounds_=paws::release1::progression::quests[index].requesters;requestSoundGeneration_=audio_->generation();}
            save();publish("A little request is ready.");projectUi();if(gardenAfterPrepare_&&!pause)checkEnding();
        }catch(const std::exception& e){host_.event(HostEvent::LoadError, {e.what()});failedQuest_=index;cancelPreparation();publish("The models could not load. Retry when you're ready.");projectUi();ui_->enter(paws::UiScreen::LoadFailure);}
    }
    void showEnding(){
        ui_->enter(paws::UiScreen::Ending);reset();projectUi("Our garden has room for everyone.");
    }
    void framePlacement(){reset();}
    void checkEnding(){if(bindings_.garden&&afternoon_.picnicEligible()&&!afternoon_.picnicSeen())dispatchTyped(afternoon_.intent(paws::release1::Verb::RecordPicnic));}
    void applyGardenLookUp() {
        if(auto* scene=application_.activeScene())
            if(auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findComponent(cameraId_)))
                camera->transform.rotation.x=-orbit_->snapshot().elevation+gardenLookUp_;
    }
    void drag(float dx,float dy) {
        if(suspended_||afternoon_.userPaused)return;
        // Below the orbit's horizon limit, tilt in place instead of moving the
        // camera underground. One continuous drag also returns to normal orbit.
        if(bindings_.garden) {
            const float elevation=orbit_->snapshot().elevation;
            const float desired=std::clamp(elevation-gardenLookUp_+dy*.006f,-1.15f,1.45f);
            gardenLookUp_=std::max(0.f,.05f-desired);
            orbit_->orbit(dx,(std::max(.05f,desired)-elevation)/.006f);
            applyGardenLookUp();
        } else orbit_->orbit(dx,dy);
    }
    void zoom(float delta) { if(!suspended_){orbit_->zoom(delta);applyGardenLookUp();} }
    void reset(){
        gardenLookUp_=0;
        if(!orbit_)return;const bool ending=ui_&&(ui_->screen()==paws::UiScreen::Ending||ui_->screen()==paws::UiScreen::BeachEnding);
        const int chapter=afternoon_.progressView().chapter;
        auto v=orbit_->snapshot();v.target=v.resetTarget={bindings_.garden&&chapter==1&&!ending?-1.f:0.f,.3f,ending?4.f:bindings_.garden?(chapter<3?-1.f:1.f):0.f};v.yaw=0;v.elevation=.85f;
        const auto d=paws::release1::rules::quest(afternoon_.puzzleView().quest,afternoon_.puzzleView().content);const float step=d.family==paws::release1::rules::Family::Shapes?.25f:1.f;
        const bool legacy=afternoon_.active()<3;
        const float apron=d.family==paws::release1::rules::Family::Switches?0.f:1.5f;
        const float width=ending?5.f:bindings_.garden?(chapter==1?9.1f:13.1f):legacy?8.1f:(d.width+2)*step+apron+1.1f;
        const float height=ending?4.f:bindings_.garden?(chapter<3?7.1f:11.1f):legacy?6.1f:std::max((d.height+2)*step,4.f)+1.1f;
        if(!bindings_.garden&&!legacy)v.target.x=v.resetTarget.x=apron*.5f;
        if(!ending){
            // Shared close courtyard view, matched to the October 12 camera
            // reference. Anchor it to the rear paving edge in every scene.
            const float back=bindings_.garden?-4.f:legacy?-2.5f:-std::max((d.height+2)*step*.5f,2.f);
            v.target.z=back+1.5f;v.target.y=.8f;v.resetTarget=v.target;
            v.yaw=-.012f;v.elevation=.478f;
        }
        const auto* camera=dynamic_cast<engine::CameraComponent*>(application_.activeScene()->findComponent(cameraId_));
        const float tangent=std::tan((camera?camera->verticalFov():engine::pi*35/180)*.5f);
        const float aspect=std::max(.25f,ending&&!paws::release1::progression::isBeachQuest(d.quest)?boardAspect_:aspect_);
        if(!bindings_.garden&&d.quest=="beach_windbreak"){
            // Fit the workshop's entire play area, including the loose panels in
            // front. The old ground-height estimate ignored their mesh overhang
            // and perspective enlargement near the camera.
            v.yaw=0;v.elevation=.88f;
            engine::Vec3 lo{1000,1000,1000},hi{-1000,-1000,-1000};
            const auto include=[&](engine::Vec3 p){
                lo={std::min(lo.x,p.x),std::min(lo.y,p.y),std::min(lo.z,p.z)};
                hi={std::max(hi.x,p.x),std::max(hi.y,p.y),std::max(hi.z,p.z)};
            };
            for(const auto& socket:d.sockets)for(auto cell:{socket.cell,socket.end}){
                const auto p=paws::release1::presentation::world(d.quest,cell,d.content);
                include(p+engine::Vec3{-.55f,0,-.55f});include(p+engine::Vec3{.55f,1.4f,.55f});
            }
            for(const auto& binding:bindings_.objects){
                const auto def=std::find_if(d.objects.begin(),d.objects.end(),[&](const auto& o){return o.id==binding.object&&o.role=="shade_panel";});
                if(def==d.objects.end())continue;
                auto* actor=application_.activeScene()->findActor(binding.actor);if(!actor)continue;
                auto* mesh=actor->getComponent<engine::MeshComponent>();if(!mesh||!mesh->mesh)continue;
                const auto& b=mesh->mesh->localBounds();
                for(float x:{b.minimum.x,b.maximum.x})for(float y:{b.minimum.y,b.maximum.y})for(float z:{b.minimum.z,b.maximum.z}){
                    include(engine::transformPoint(mesh->worldMatrix(),{x,y,z}));
                    // Keep the tray in frame after placements and when a loose
                    // panel is turned; fitting must not zoom in as it empties.
                    auto initial=actor->transform;
                    initial.position=paws::release1::presentation::world(d.quest,def->initial,d.content)+engine::Vec3{0,.58f,0};
                    for(int turn=0;turn<4;++turn){initial.rotation.y=turn*engine::pi/2;include(engine::transformPoint(initial.matrix(),{x,y,z}));}
                }
            }
            const auto centre=(lo+hi)*.5f;
            const float cx=2*(bridgeFrame_.x+bridgeFrame_.width*.5f)-1;
            const float cy=2*(bridgeFrame_.y+bridgeFrame_.height*.5f)-1;
            const float halfWidth=std::max(.1f,bridgeFrame_.width*.94f),halfHeight=std::max(.1f,bridgeFrame_.height*.94f);
            v.distance=1;
            for(float x:{lo.x,hi.x})for(float y:{lo.y,hi.y})for(float z:{lo.z,hi.z}){
                const auto p=engine::Vec3{x,y,z}-centre;
                const float up=p.y*std::cos(v.elevation)-p.z*std::sin(v.elevation);
                const float depth=p.y*std::sin(v.elevation)+p.z*std::cos(v.elevation);
                // Solve the shifted perspective frustum, not an orthographic
                // approximation: near corners need extra room below the HUD.
                v.distance=std::max(v.distance,depth+std::max(
                    std::abs(p.x+cx*tangent*aspect*depth)/(tangent*aspect*halfWidth),
                    std::abs(up-cy*tangent*depth)/(tangent*halfHeight)));
            }
            const float offsetX=cx*v.distance*tangent*aspect,offsetY=cy*v.distance*tangent;
            v.target=v.resetTarget=centre+engine::Vec3{-offsetX,offsetY*std::cos(v.elevation),-offsetY*std::sin(v.elevation)};
            orbit_->restore(v);orbit_->orbit(0,0);return;
        }
        if(!bindings_.garden&&(d.quest=="pip_friend_ball"||(paws::release1::progression::isBeachQuest(d.quest)&&d.family!=paws::release1::rules::Family::Planks&&d.family!=paws::release1::rules::Family::PictureSequence)||d.quest=="mabel_friend_water"||d.sunnyNook||paws::release1::rules::cooperativePlayhouse(d)||d.family==paws::release1::rules::Family::Balance||d.family==paws::release1::rules::Family::Scent)){
            v.yaw=0;v.elevation=.88f;
            const bool scent=d.family==paws::release1::rules::Family::Scent;
            const bool beach=paws::release1::progression::isBeachQuest(d.quest);
            // Beach reference: fit the projected ground depth, with space for
            // cat height and perspective, rather than an unprojected square.
            const bool pepper=d.quest=="beach_pepper_basket";
            // Pepper's feature comparisons need the whole vignette, including
            // the paths around its rear edge, at a useful size on narrow screens.
            const float beachWidth=float(d.width)+(pepper?1.2f:1.3f);
            // Include the named waiting cats in the picnic framing, below the mats.
            const float beachHeight=pepper?7.3f:d.beachPicnic?5.7f:float(d.height-1)*std::sin(v.elevation)+1.4f;
            v.distance=std::max((beach?beachWidth:scent?8.5f:10.f)/(2*tangent*aspect*std::max(.15f,bridgeFrame_.width)),(beach?beachHeight:scent?5.5f:6.8f)/(2*tangent*std::max(.15f,bridgeFrame_.height)))*(pepper?1.08f:beach?1.18f:1.05f);
            const float offsetX=(bridgeFrame_.x+bridgeFrame_.width*.5f-.5f)*2*v.distance*tangent*aspect;
            const float offsetY=(bridgeFrame_.y+bridgeFrame_.height*.5f-.5f)*2*v.distance*tangent;
            const float coastLook=beach&&aspect>1.2f&&!shortViewport_?.65f:0.f;
            v.target=v.resetTarget={(beach?.35f:0.f)-offsetX,.3f+offsetY*std::cos(v.elevation),(pepper?1.7f:d.beachPicnic?.85f:-coastLook)-offsetY*std::sin(v.elevation)};
            orbit_->restore(v);orbit_->orbit(0,0);return;
        }
        if(!bindings_.garden&&d.family==paws::release1::rules::Family::PictureSequence){
            // Fit the board, responses and cat into the actual free HUD area.
            v.yaw=0;v.elevation=.50f;
            v.distance=std::max(4.5f/(2*tangent*aspect*std::max(.15f,bridgeFrame_.width)),3.3f/(2*tangent*std::max(.15f,bridgeFrame_.height)))*1.10f;
            const float offsetX=(bridgeFrame_.x+bridgeFrame_.width*.5f-.5f)*2*v.distance*tangent*aspect;
            const float offsetY=(bridgeFrame_.y+bridgeFrame_.height*.5f-.5f)*2*v.distance*tangent;
            v.target=v.resetTarget={.6f-offsetX,.85f+offsetY*std::cos(v.elevation),-.65f-offsetY*std::sin(v.elevation)};
            orbit_->restore(v);orbit_->orbit(0,0);return;
        }
        if(!bindings_.garden&&d.family==paws::release1::rules::Family::Planks){
            // Present both ends of Mabel's bridge in one readable composition.
            v.yaw=0;v.elevation=.82f;
            v.distance=std::max(6.8f/(2*tangent*aspect*std::max(.15f,bridgeFrame_.width)),4.5f/(2*tangent*std::max(.15f,bridgeFrame_.height)))*1.12f;
            const float offsetX=(bridgeFrame_.x+bridgeFrame_.width*.5f-.5f)*2*v.distance*tangent*aspect;
            const float offsetY=(bridgeFrame_.y+bridgeFrame_.height*.5f-.5f)*2*v.distance*tangent;
            v.target=v.resetTarget={-.65f-offsetX,.15f+offsetY*std::cos(v.elevation),-.45f-offsetY*std::sin(v.elevation)};
            orbit_->restore(v);orbit_->orbit(0,0);return;
        }
        if(!ending){
            // Preserve the reference pose at its 1485 x 1117 viewport. Pull
            // back only for a larger courtyard or a narrower viewport.
            v.distance=15.3f*std::max({1.f,width/9.1f,height/7.1f,
                (width/9.1f)*(1485.f/1117.f)/aspect});
            orbit_->restore(v);orbit_->orbit(0,0);return;
        }
        // The ending reserves a separate viewport beside its closing card.
        // Fit its gathering bounds with perspective depth accounted for.
        v.distance=0;
        for(float x:{-width*.5f,width*.5f})for(float z:{-height*.5f,height*.5f})for(float y:{-.3f,1.2f}){
            const float depth=z*std::cos(v.elevation)+y*std::sin(v.elevation);
            v.distance=std::max(v.distance,depth+std::max(std::abs(x)/(tangent*aspect),std::abs(y*std::cos(v.elevation)-z*std::sin(v.elevation))/tangent));
        }
        v.distance*=1.035f;orbit_->restore(v);orbit_->orbit(0,0);
    }
    void resize(float aspect,float height=900) { if(aspect>0 && (aspect!=aspect_||(height<500)!=shortViewport_)) { aspect_=aspect;shortViewport_=height<500;if(placementItem_.empty())reset();else framePlacement(); } }
    void suspend(bool value) {suspended_=value;host_.setSuspended(value);cues_->suspend(value);afternoon_.setSuspended(value);ownerSuspended_=value;projectUi();publish(lastMessage_);}
    void audioGesture() {if(audioStarted_&&!suspended_&&(!audio_->system().backend().active()||!audio_->uiActive()))audio_->activate();}
    bool audioPublish(int index,unsigned generation,unsigned rate,unsigned channels,const float* samples,int count) {
        if(index<0||std::size_t(index)>=paws::audioClips.size()||count<=0||generation!=audio_->generation())return false;
        const auto budget=paws::audioClips[index].bus==0?paws::GameAudio::musicClipBudget:paws::GameAudio::clipBudget;
        if(std::size_t(count)>budget/sizeof(float))return false;
        try {return audio_->publish(index,generation,engine::SoundAsset::create(rate,channels,{samples,std::size_t(count)}));}
        catch(...){audio_->failed(index,generation);return false;}
    }
    void audioFailed(int index,unsigned generation) {if(index>=0)audio_->failed(index,generation);}
private:
    void applyQuality() {
        const int profile=afternoon_.settings.quality;
        const bool mobile=profile==1, high=profile==2;
        const bool beach=afternoon_.inBeach();
        if(profile==appliedQuality_&&beach==appliedBeachQuality_)return;
        engine::RenderQuality quality;
        // Automatic starts at one backing pixel per CSS pixel, including HiDPI.
        // Supersampling is an explicit choice; keep Automatic's shadows enabled.
        quality.resolution=mobile?engine::ResolutionBudget{1.f,1.f,4096,921600u}
                                 :engine::ResolutionBudget{high?2.f:1.f,1.f,8192,8294400u};
        quality.maxShadowMaps=mobile?0:1;
        // The beach's broader sun volume and 3x3 PCF produce gentle penumbras
        // at 512px, without adding samples or supersampling the whole scene.
        quality.shadowResolution=beach?512:1024;
        quality.shadowFilterTaps=9;
        const auto msaaSamples=configureMsaa(quality,high,host_.msaaOverride);
        host_.setQuality(quality);appliedQuality_=profile;appliedBeachQuality_=beach;
        host_.event(HostEvent::Quality, {profile,quality.resolution.maxPixels,quality.resolution.maxPixelRatio,quality.maxShadowMaps,quality.resolution.scale,msaaSamples});
    }
    int appliedQuality_=-1;
    bool appliedBeachQuality_=false;
    bool beachMusic_=false;
    void resetAudioScene(){waterSeconds_=0;ballContactPending_=false;gateArrivalCues_.clear();playhouseNext_.reset();useAudioPhases_.fill(0);pipSounds_.reset();requestSounds_=0;audio_->sceneChanged();birds_.reset();host_.event(HostEvent::AudioSceneEpoch, {audio_->generation()});}
    void updateAudio(double delta,double simulationDelta){
        if(!ui_)return;observeAudio();const auto screen=ui_->screen();cues_->suspend(suspended_||!host_.ready()||afternoon_.userPaused,!suspended_&&host_.ready()&&afternoon_.userPaused);cues_->music(!bindings_.garden&&screen!=paws::UiScreen::Title,screen==paws::UiScreen::Ending);
        const bool beach=afternoon_.inBeach()&&!bindings_.garden&&screen!=paws::UiScreen::Title&&!pendingQuest_;
        const bool beachMusic=afternoon_.inBeach()&&!bindings_.garden&&screen!=paws::UiScreen::Title;
        if(beachMusic!=beachMusic_){audio_->playlist(beachMusic?std::span<const std::string_view>(paws::beachMusicPlaylist):std::span<const std::string_view>(paws::gardenMusicPlaylist));beachMusic_=beachMusic;}
        audio_->beachWaves(beach);audio_->loop("amb_garden_air",!beach);audio_->loop("ball_roll",!bindings_.ballRoute.empty());audio_->update(delta);
        if(auto* scene=application_.activeScene()){
            const bool active=!suspended_&&host_.ready()&&!afternoon_.userPaused;
            const bool followsAudio=audio_->system().backend().active()&&!afternoon_.settings.muted&&afternoon_.settings.effects>0;
            const auto surf=paws::updateBeachSurf(*scene,active?delta:0,ui_->settings().reducedMotion,followsAudio);
            host_.event(HostEvent::Surf, {surf.age,surf.starts,surf.travel,surf.opacity,followsAudio});
        }
        if(requestSounds_&&(requestSoundGeneration_!=audio_->generation()||afternoon_.settings.muted))requestSounds_=0;
        if(requestSounds_&&!pendingQuest_&&!suspended_&&!afternoon_.userPaused&&audio_->system().backend().active()&&(screen==paws::UiScreen::Dialogue||screen==paws::UiScreen::Puzzle)){
            for(unsigned n=0;n<4;++n)if(requestSounds_&(1u<<n))audio_->cue(std::string(paws::release1::progression::catIds[n])+"_request");requestSounds_=0;
        }
        if(ballContactPending_&&bindings_.ballRoute.empty()&&!suspended_&&!afternoon_.userPaused){audio_->cue("ball_contact");ballContactPending_=false;}
        if(waterSeconds_>0&&delta>0&&!suspended_&&!afternoon_.userPaused){waterSeconds_=std::max(0.,waterSeconds_-delta);if(waterSeconds_==0){const auto p=afternoon_.puzzleView();cues_->committed({p.generation,p.attempt,p.revision,p.quest,"","bowl_fill"});}}
        bool purring=false,changed=false;if(gardenPresentation_)for(std::size_t n=0;n<4;++n){
            const auto& c=gardenPresentation_->cats()[n];const int phase=int(c.phase);changed|=phase!=useAudioPhases_[n];
            if(phase!=useAudioPhases_[n]&&c.phase==paws::release1::garden::Phase::Use){if(c.item=="scratching_post")audio_->cue("cat_scratch");if(c.item=="playhouse")audio_->cue("playhouse_use");}
            if(phase!=useAudioPhases_[n]&&c.phase==paws::release1::garden::Phase::WelcomeHello)
                audio_->cue(std::string(paws::release1::progression::catIds[n])+"_request");
            useAudioPhases_[n]=phase;
            if(n==0){
                auto* actor=application_.activeScene()->findActor(c.actor);
                if(actor&&actor->enabled){
                    const auto* skin=actor->getComponent<engine::SkeletalMeshComponent>();
                    const auto sound=pipSounds_.advance(c.phase,c.item,actor->transform.position,c.segment,simulationDelta,
                        skin&&paws::isToonCat(*skin->mesh)?paws::toonWalkStride:paws::pipWalkStride);
                    if(sound.step)audio_->cue("pip_paws_stone");if(sound.settle)audio_->cue("pip_settle");if(sound.drink)audio_->cue("pip_drink");purring=sound.purr;
                    if(c.phase!=paws::release1::garden::Phase::Use||c.item!="drinking_bowl")audio_->cancel("pip_drink");
                }else pipSounds_.reset();
            }
        }audio_->loop("pip_purr",purring);
        if(playhouseNext_&&gardenPresentation_){
            if(*playhouseNext_!=gardenPresentation_->stamp())playhouseNext_.reset();
            else if(gardenPresentation_->cats()[0].phase==paws::release1::garden::Phase::Idle){std::string why;gardenPresentation_->use(*application_.activeScene(),"playhouse",3,*playhouseNext_,why);playhouseNext_.reset();changed=true;}
        }
        if(changed)projectUi();
        const bool audibleGarden=bindings_.garden&&!pendingQuest_&&!suspended_&&host_.ready()&&!afternoon_.userPaused&&
            audioStarted_&&audio_->system().backend().active()&&!afternoon_.settings.muted&&afternoon_.settings.ambience>0&&
            screen!=paws::UiScreen::Ending&&screen!=paws::UiScreen::Title;
        if(birds_.advance(delta,audibleGarden))audio_->cue("amb_garden_bird");
        const auto text=host_.audioStatus(audio_->system().backend().active(),afternoon_.settings.muted);if(audioStatus_!=text){audioStatus_=text;projectUi();}
        host_.event(HostEvent::Audio, {audioDevice_,audio_->voiceCount(),audio_->pcmBytes(),audio_->peakPcmBytes(),audio_->peakVoices(),audio_->generation(),uiAudioDevice_});
    }
    bool directActions(const std::vector<paws::release1::rules::Step>& steps){
        auto candidate=afternoon_;
        for(const auto& step:steps){auto i=candidate.intent(step.verb,step.object,step.target);i.direction=step.direction;i.destination=step.destination;
            const auto r=candidate.dispatchIntent(i);if(!r.accepted){audio_->cue("ui_blocked");publish(r.message);projectUi();return false;}}
        for(const auto& step:steps){auto i=afternoon_.intent(step.verb,step.object,step.target);i.direction=step.direction;i.destination=step.destination;if(!dispatchTyped(i))return false;}
        // A valid arrangement completes without a separate GUI check.
        if(!steps.empty()&&(steps.back().verb==paws::release1::Verb::Fit||steps.back().verb==paws::release1::Verb::AssignCat)){
            const auto view=afternoon_.puzzleView();
            for(const auto& action:view.actions)if(action.enabled&&action.intent.verb==paws::release1::Verb::Connect){dispatchTyped(action.intent);break;}
        }
        return true;
    }
    int newPointer(int phase,float x,float y){
        if(phase==3||blocked()||suspended_||pendingQuest_)workshopTap_.reset();
        // Scene previews never change the attempt. Commit only after release
        // through the authoritative puzzle commands.
        if(questGesture_){
            auto* scene=application_.activeScene();const auto view=afternoon_.puzzleView();
            auto cancel=[&]{questGesture_.reset();if(scene&&!bindings_.garden&&afternoon_.active()>=3){paws::release1::presentation::project(*scene,bindings_,view,afternoon_.settings.reducedMotion);refreshMovementGuidance();}};
            const auto& stamp=questGesture_->stamp;
            if(phase==3||blocked()||suspended_||pendingQuest_||stamp.generation!=view.generation||stamp.attempt!=view.attempt||stamp.revision!=view.revision||stamp.quest!=view.quest||!scene||!scene->findActor(questGesture_->actor)){sequenceTrace("cancel",stamp.object,phase==3?"pointer cancelled":"blocked or stale gesture",-1,x,y);cancel();return 0;}
            auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findComponent(cameraId_));if(!camera){cancel();return 0;}
            double w,h;host_.viewport(&w,&h);
            const auto ray=camera->screenPointToRay({x,y},camera->pixelViewport(float(w),float(h)));
            auto& g=*questGesture_;const float dx=x-g.down.x,dy=y-g.down.y;g.dragged|=dx*dx+dy*dy>64;
            if(std::abs(ray.direction.y)>.0001f){const float t=(g.original.y-ray.origin.y)/ray.direction.y;if(t>=0)g.preview=g.original+(ray.origin+ray.direction*t-g.ground);}
            const auto object=std::find_if(view.objects.begin(),view.objects.end(),[&](const auto& o){return o.id==g.stamp.object;});
            if(object==view.objects.end()){cancel();return 0;}
            const bool movable=(object->role=="plank"||object->role=="shade_panel")||object->role.starts_with("cushion_")||object->role=="crate"||object->role.starts_with("cat_")||object->role=="picnic_bed"||object->role=="picnic_bowl"||object->role=="balance_weight"||object->role=="pot"||object->role=="lookout_cushion";
            if(view.quest=="beach_windbreak"&&g.dragged){
                g.dropTarget=std::abs(ray.direction.y)>.0001f?workshopDropTarget(object->id,ray.origin+ray.direction*(-ray.origin.y/ray.direction.y)):std::string{};
            }
            if(phase==1){
                if(g.dragged&&movable)scene->findActor(g.actor)->transform.position=g.preview;
                if(view.quest=="beach_windbreak"&&g.dragged)refreshMovementGuidance();
                return 1;
            }
            if(phase!=2)return 1;
            const auto finished=g;sequenceTrace("release",g.stamp.object,g.dragged?"drag threshold exceeded":"tap",-1,x,y);cancel();using V=paws::release1::Verb;
            const auto definition=paws::release1::rules::quest(view.quest,view.content);using F=paws::release1::rules::Family;
            // Explain a blocked sliding piece before testing drop proximity. The
            // player may try any part of the track while the helper is missing.
            auto slideBlockage=[&]() -> std::string {
                if(object->role!="crate"||!afternoon_.movementOptions(object->id).empty())return {};
                const auto authored=std::find_if(definition.objects.begin(),definition.objects.end(),[&](const auto& o){return o.id==object->id;});
                if(authored==definition.objects.end())return {};
                for(const auto& action:view.actions)if(action.intent.object==object->id&&action.intent.verb==V::Slide&&!action.enabled){
                    const auto cell=paws::adjacent(object->cell,action.intent.direction);
                    if(std::find(authored->slots.begin(),authored->slots.end(),cell)!=authored->slots.end())return action.reason;
                }
                return {};
            };
            if(const auto reason=slideBlockage();!reason.empty()){publish(object->name+" cannot move yet. "+reason);projectUi();return 1;}
            if(!finished.dragged){
                if(object->role=="crate"){publish(object->name+" selected. Drag it to a white rectangle.");projectUi();}
                else if(object->role=="plank"||object->role=="shade_panel"){
                    if(view.quest=="beach_windbreak"){
                        const double now=application_.clocks().input.seconds;
                        const auto dx=x-workshopTapPoint_.x,dy=y-workshopTapPoint_.y;
                        const bool doubleClick=workshopTap_&&workshopTap_->object==object->id&&workshopTap_->quest==view.quest
                            &&workshopTap_->generation==view.generation&&workshopTap_->attempt==view.attempt&&workshopTap_->revision==view.revision
                            &&now-workshopTapTime_<=.4&&now-finished.started<=.35&&dx*dx+dy*dy<=144;
                        if(doubleClick){workshopTap_.reset();directActions({{V::Rotate,object->id}});}
                        else if(now-finished.started<=.35){workshopTap_=finished.stamp;workshopTapTime_=now;workshopTapPoint_={x,y};}
                        else workshopTap_.reset();
                    }else if(finished.wasSelected)directActions({{V::Rotate,object->id}});
                    explainPlank(object->id);
                }
                else if(object->role=="trail_clue")dispatchTyped(afternoon_.intent(V::Inspect,object->id));
                else if(object->role=="trail_choice")dispatchTyped(afternoon_.intent(V::FollowTrail,object->id));
                else if(object->role=="balance_test")directActions({{V::TestBalance}});
                else if((object->role=="plank"||object->role=="shade_panel")||object->role.starts_with("cushion_")||object->role=="gutter"||object->role=="channel")directActions({{V::Rotate,object->id}});
                else if(definition.family==F::PictureSequence&&object->role!="sequence_blackboard")directActions({{V::TapSymbol,object->id}});
                else if(object->role=="sequence_blackboard")dispatchTyped(afternoon_.intent(V::ReplaySequence));
                else if(definition.family==F::Bench&&object->id=="mouse")directActions({{V::Retrieve,object->id,"near_reach"}});
                else if(object->role=="water_basin"||object->role=="rain_barrel"||object->role=="drinking_bowl"||object->role=="ball")directActions({{V::Roll}});
                return 1;
            }
            if(!movable)return 1;
            if(std::abs(ray.direction.y)<.0001f)return 1;
            const float t=-ray.origin.y/ray.direction.y;if(t<0)return 1;
            const auto landing=ray.origin+ray.direction*t;
            if(object->role=="crate"||object->role=="pot"){
                const auto authored=std::find_if(definition.objects.begin(),definition.objects.end(),[&](const auto& o){return o.id==object->id;});
                paws::Cell destination=object->cell;float nearest=.55f;bool found=false;
                // Match the dragged actor, retaining the pointer's grab offset.
                // A ray from a crate's top/edge to the paving is not its centre.
                std::vector<paws::Cell> slots;
                if(definition.legacy){for(const auto& o:definition.legacy->objects)if(o.id==object->id)slots=o.slots;}else if(authored!=definition.objects.end())slots=authored->slots;
                for(auto cell:slots){const auto delta=finished.preview-paws::release1::presentation::world(view.quest,cell,view.content);const float distance=std::sqrt(delta.x*delta.x+delta.z*delta.z);if(distance<nearest){nearest=distance;destination=cell;found=true;}}
                if(!found){publish(view.quest.starts_with("beach_")?"Keep "+object->name+" on its track. Drag it to a highlighted space.":definition.family==F::Switches?"Drag the crate along its marked track toward the paw switch.":object->id=="crate_b"?"The rear crate slides sideways along its marked track.":"The front crate slides toward the house along its marked track.");projectUi();return 1;}
                struct Node {paws::Afternoon candidate;std::vector<paws::release1::rules::Step> steps;paws::Cell cell;};
                std::vector<Node> queue{{afternoon_,{},object->cell}};std::vector<paws::Cell> seen{object->cell};
                for(std::size_t n=0;n<queue.size();++n){const auto current=queue[n];if(current.cell==destination){directActions(current.steps);return 1;}
                    for(int dir=0;dir<4;++dir){auto next=current;auto i=next.candidate.intent(V::Slide,object->id);i.direction=paws::Direction(dir);if(!next.candidate.dispatchIntent(i).accepted)continue;
                        const auto v=next.candidate.puzzleView();for(const auto& o:v.objects)if(o.id==object->id)next.cell=o.cell;
                        if(std::find(seen.begin(),seen.end(),next.cell)!=seen.end())continue;seen.push_back(next.cell);next.steps.push_back({V::Slide,object->id,"",paws::Direction(dir)});queue.push_back(std::move(next));}}
                if(definition.family==F::Switches){
                    auto probe=afternoon_;auto i=probe.intent(V::Slide,object->id);i.direction=object->cell==authored->initial?paws::Direction::South:paws::Direction::West;
                    const auto r=probe.dispatchIntent(i);publish(r.accepted?"Follow the crate track. Move Pip to his waiting mat to clear the paw switch.":r.message);
                }else publish(view.quest.starts_with("beach_")?"That space is blocked. Tap the basket to see where it can move now.":"Move the rear crate sideways first to free the shared space.");projectUi();return 1;
            }
            if(object->role.starts_with("cushion_")){
                paws::Cell destination;float nearest=.2f;bool found=false;
                for(auto cell:definition.basket){const auto delta=finished.preview-paws::release1::presentation::shapePosition(view.quest,object->id,cell,object->orientation,view.content);const float distance=std::sqrt(delta.x*delta.x+delta.z*delta.z);if(distance<nearest){nearest=distance;destination=cell;found=true;}}
                if(!found){publish("Drop the cushion into the basket.");projectUi();return 1;}
                std::vector<paws::release1::rules::Step> steps;if(!object->held)steps.push_back({V::PickUp,object->id});steps.push_back({V::Fit,object->id,"basket",paws::Direction::North,destination});directActions(steps);return 1;
            }
            std::string target;
            if(view.quest=="beach_windbreak"){
                target=finished.dropTarget;
                if(target.empty()){publish("No placement here. Drag to a white fit or use Rotate.");projectUi();return 1;}
            }else if(finished.dragged){
                if(std::abs(ray.direction.y)<.0001f)return 1;
                const float t=-ray.origin.y/ray.direction.y;if(t<0)return 1;
                float nearest=.65f;
                for(const auto& socket:definition.sockets){
                    if(object->role=="lookout_cushion"&&socket.role!="lookout_seat"&&socket.role!="lookout_approach")continue;
                    if(object->role=="balance_weight"&&socket.role!="balance_socket"&&socket.role!="balance_home")continue;
                    if((object->role=="plank"||object->role=="shade_panel")&&socket.role!="plank_socket"&&socket.role!="shade_socket")continue;
                    if(object->role.starts_with("cat_")&&socket.role!="waiting"&&socket.role!="switch_pip"&&socket.role!="switch_pepper"&&socket.role!="storage_spot"&&socket.role!="inside_entry"&&socket.role!="inside_latch"&&socket.role!="inside_spot")continue;
                    const auto centre=(object->role=="plank"||object->role=="shade_panel")?(paws::release1::presentation::world(view.quest,socket.cell,view.content)+paws::release1::presentation::world(view.quest,socket.end,view.content))*.5f:paws::release1::presentation::world(view.quest,socket.cell,view.content);
                    // Raised lookout seats must be picked on their visible surface,
                    // not where that same pointer ray intersects the paving below.
                    const auto drop=object->role=="lookout_cushion"&&socket.role=="lookout_seat"
                        ?ray.origin+ray.direction*((.33f-ray.origin.y)/ray.direction.y):landing;
                    const auto delta=drop-centre;const float distance=std::sqrt(delta.x*delta.x+delta.z*delta.z);
                    if(distance<nearest){nearest=distance;target=socket.id;}
                }
                if(target.empty()){audio_->cue("ui_blocked");publish("Drop the object onto a marked destination.");projectUi();return 1;}
            }
            // Preflight both commands so a rejected drop leaves state intact.
            std::vector<paws::release1::rules::Step> steps;
            if((object->role=="plank"||object->role=="shade_panel")&&!object->held)steps.push_back({V::PickUp,object->id});
            steps.push_back({object->role.starts_with("cat_")?V::AssignCat:V::Fit,object->id,target});
            if(view.quest=="beach_windbreak")selectedPlank_.reset();
            directActions(steps);return 1;
        }
        if(phase==3||blocked()||suspended_||pendingQuest_){pickedCat_=4;return 0;}
        if(bindings_.garden){
            if(!gardenPresentation_||ui_->screen()!=paws::UiScreen::Garden)return 0;
            auto* scene=application_.activeScene();auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findComponent(cameraId_));if(!camera)return 0;
            double w,h;host_.viewport(&w,&h);
            if(phase==0){pickedCat_=4;catTapMoved_=false;catTapStart_={x,y};catTapStamp_=gardenPresentation_->stamp();
                auto hit=scene->selection().resolveRay(camera->screenPointToRay({x,y},camera->pixelViewport(float(w),float(h))));if(!hit)return 0;
                if(gardenPresentation_->lookoutReady()&&hit->actor==gardenPresentation_->lookoutSeat()){pickedCat_=5;return 1;}
                if(gardenPresentation_->nookReady()&&hit->actor==gardenPresentation_->nookSeat()){pickedCat_=6;return 1;}
                for(std::size_t n=0;n<4;++n)if(gardenPresentation_->cats()[n].actor==hit->actor&&!gardenPresentation_->requests(n).empty()){pickedCat_=n;return 1;}return 0;
            }
            if(pickedCat_==5||pickedCat_==6){const bool nook=pickedCat_==6;const float dx=x-catTapStart_.x,dy=y-catTapStart_.y;catTapMoved_|=dx*dx+dy*dy>144;
                if(phase==2){pickedCat_=4;auto hit=scene->selection().resolveRay(camera->screenPointToRay({x,y},camera->pixelViewport(float(w),float(h))));if(!catTapMoved_&&catTapStamp_==gardenPresentation_->stamp()&&hit&&hit->actor==(nook?gardenPresentation_->nookSeat():gardenPresentation_->lookoutSeat())){startUse(nook?"mabel_nook":"pip_lookout",0);projectUi();}}return 1;}
            if(pickedCat_>=4)return 0;
            const float dx=x-catTapStart_.x,dy=y-catTapStart_.y;catTapMoved_|=dx*dx+dy*dy>144;
            if(phase==2){const auto cat=pickedCat_;pickedCat_=4;
                auto hit=scene->selection().resolveRay(camera->screenPointToRay({x,y},camera->pixelViewport(float(w),float(h))));
                if(!catTapMoved_&&catTapStamp_==gardenPresentation_->stamp()&&hit&&hit->actor==gardenPresentation_->cats()[cat].actor){
                    const auto requests=gardenPresentation_->requests(cat);
                    if(!requests.empty()){
                        // A cat can offer its request while still walking in.
                        if(!gardenPresentation_->cats()[cat].introduced){
                            if(!afternoon_.introduce(paws::release1::progression::catIds[cat],afternoon_.intent(paws::release1::Verb::Select)).accepted)return 1;
                            save();
                        }
                        // A cat starts a conversation before handing over its puzzle.
                        requestQuest(paws::release1::progression::questIndex(requests.front().id),false,false,false);
                    }
                }return 1;
            }return 1;
        }
        if(phase==0){
            auto* scene=application_.activeScene();auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findComponent(cameraId_));if(!camera)return 0;double w,h;host_.viewport(&w,&h);auto hit=scene->selection().resolveRay(camera->screenPointToRay({x,y},camera->pixelViewport(float(w),float(h))));if(!hit){workshopTap_.reset();selectedBasket_.reset();if(afternoon_.puzzleView().quest=="beach_windbreak"){selectedPlank_.reset();publish("");projectUi();}refreshMovementGuidance();sequenceTrace("miss",{},"no interactive surface",-1,x,y);return 0;}
            for(auto& b:bindings_.objects)if(b.actor==hit->actor){
                const auto view=afternoon_.puzzleView();
                const auto object=std::find_if(view.objects.begin(),view.objects.end(),[&](const auto& o){return o.id==b.object;});
                if(object!=view.objects.end()&&!view.completed){
                    const auto ray=camera->screenPointToRay({x,y},camera->pixelViewport(float(w),float(h)));
                    if(std::abs(ray.direction.y)<.0001f)return 0;
                    const auto position=scene->findActor(b.actor)->transform.position;
                    const float t=(position.y-ray.origin.y)/ray.direction.y;if(t<0)return 0;
                    questGesture_=QuestGesture{afternoon_.intent(paws::release1::Verb::Select,b.object),b.actor,{x,y},ray.origin+ray.direction*t,position,position};
                    questGesture_->started=application_.clocks().input.seconds;
                    if(workshopTap_&&workshopTap_->object!=b.object)workshopTap_.reset();
                    questGesture_->wasSelected=selectedPlank_&&selectedPlank_->object==b.object&&selectedPlank_->attempt==view.attempt&&selectedPlank_->quest==view.quest;
                    if(object->role=="crate")selectedBasket_=questGesture_->stamp;else selectedBasket_.reset();
                    if(object->role=="plank"||object->role=="shade_panel")selectedPlank_=questGesture_->stamp;else selectedPlank_.reset();
                    if(view.quest=="beach_pepper_basket"){publish(object->name);projectUi();}
                    refreshMovementGuidance(b.object);
                    sequenceTrace("pick",b.object,"pointer down",-1,x,y);
                    return 1;
                }
                return 0;}return 0;}
        return 0;
    }
    struct QuestGesture {
        paws::release1::Intent stamp;
        engine::ActorId actor;
        engine::Vec2 down;
        engine::Vec3 ground,original,preview;
        bool dragged=false,wasSelected=false;
        std::string dropTarget;
        double started=0;
    };
    std::optional<QuestGesture> questGesture_;
    std::optional<paws::release1::Intent> selectedBasket_;
    std::optional<paws::release1::Intent> selectedPlank_,workshopTap_;
    engine::Vec2 workshopTapPoint_;
    double workshopTapTime_=0;
    std::size_t pickedCat_=4;
    engine::Vec2 catTapStart_;
    bool catTapMoved_=false;
    paws::release1::garden::Stamp catTapStamp_;
    std::unique_ptr<paws::release1::audio::Cues> cues_;
    std::unique_ptr<paws::release1::gui::Views> views_;
    std::optional<paws::release1::garden::Presentation> gardenPresentation_;
    paws::release1::garden::Assets gardenAssets_;
    std::map<std::string,std::shared_ptr<const engine::Texture>> portraits_,thumbnails_;
    bool ownerSuspended_=false,rewardGranted_=false;

    std::unique_ptr<paws::GameAudio> audio_;
    unsigned audioDevice_=0,uiAudioDevice_=0;
    paws::GardenBirdSchedule birds_;
    paws::PipSounds pipSounds_;
    unsigned requestSounds_=0,requestSoundGeneration_=0;
    bool ballContactPending_=false;
    bool gateFinishPending_=false;
    std::vector<std::string> gateArrivalCues_;
    bool audioStarted_=false,previousRoll_=false;
    int previousUse_=0;
    double previousCatTime_=0;
    paws::UiScreen audioScreen_=paws::UiScreen::Title;
    std::string audioStatus_="Final sound files are not installed. You can play quietly.";
    paws::Afternoon afternoon_;
    RuntimeHost& host_;
    engine::Application& application_;
    double timestamp_=0;
    paws::SceneBindings bindings_;
    paws::QuestRewards questRewards_;
    bool beachPreview_=false;
    bool cloudBeachAccess_=false;
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
    float gardenLookUp_=0;

    bool suspended_=false;
    float aspect_=2.5f;
    float boardAspect_=1.5f;
    engine::GuiRect bridgeFrame_{0,0,1,1};
    double waterSeconds_=0;
    std::array<int,4> useAudioPhases_{};
    std::optional<paws::release1::garden::Stamp> playhouseNext_;
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
} // namespace paws::runtime
