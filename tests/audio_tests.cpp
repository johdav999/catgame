#include "game_audio.hpp"
#include "garden_ambience.hpp"
#include "music_playlist.hpp"
#include "pip_sounds.hpp"
#include "release1/audio/effects.hpp"
#include <iostream>
#include <stdexcept>
#include <set>

using namespace paws;
void check(bool value,const char* text) {if(!value)throw std::runtime_error(text);}
struct Backend final:engine::AudioBackend {
    bool running=false;unsigned suspensionChanges=0;std::map<unsigned,float> voices;std::vector<std::pair<unsigned,float>> starts;
    bool start(unsigned id,const engine::SoundAsset&,const engine::AudioPlayOptions&,float gain) override {voices[id]=gain;starts.push_back({id,gain});return true;}
    void stop(unsigned id) noexcept override {voices.erase(id);}
    bool playing(unsigned id) const noexcept override{return voices.contains(id);}
    void gain(unsigned id,float gain) noexcept override {voices[id]=gain;}
    bool playbackRate(unsigned id,float) noexcept override {return voices.contains(id);}
    void position(unsigned,engine::Vec3) noexcept override{}
    void listener(const engine::AudioListenerPose&) noexcept override{}
    bool active() const noexcept override{return running;}
    void activate() noexcept override{running=true;}
    void suspend(bool on) noexcept override{++suspensionChanges;if(on)running=false;}
};
int main(){try{
    auto worldBackend=std::make_unique<Backend>();auto* worldDevice=worldBackend.get();
    auto menuBackend=std::make_unique<Backend>();auto* menuDevice=menuBackend.get();
    GameAudio menus(std::move(worldBackend),std::move(menuBackend));
    const auto menuSound=engine::SoundAsset::create(48000,1,std::vector<float>(480,.1f));
    std::vector<std::pair<std::size_t,unsigned>> menuLoads;
    menus.request=[&](auto i,auto g){menuLoads.emplace_back(i,g);};menus.activate();
    for(auto [i,g]:menuLoads)menus.publish(i,g,menuSound);
    menus.suspend(true,true);menus.cue("ui_confirm");menus.update(.2);menus.cue("ui_select");
    check(!worldDevice->active()&&menuDevice->active()&&menuDevice->voices.size()==2,"User pause freezes world but leaves two bounded menu feedback voices");
    menus.cue("ui_blocked");check(menuDevice->voices.size()==2,"Menu feedback cannot exceed two voices");
    check(menuDevice->starts.size()==3,"Meaningful feedback replaces an older click instead of dropping the response");
    menus.cue("ui_panel");check(menuDevice->starts.size()==3,"A decorative panel cannot displace meaningful feedback");
    menus.sceneChanged();check(menuDevice->voices.size()==2,"Menu clicks survive unrelated world/pause generations");
    AfternoonSettings menuSettings;menuSettings.muted=true;menus.settings(menuSettings);
    for(auto [id,gain]:menuDevice->voices)check(gain==0,"Mute reaches separate menu device");
    menus.suspend(true);check(!menuDevice->active(),"Host suspension freezes menu device too");
    menus.update(100);check(menus.peakVoices()<=14,"Shared world/menu voice cap remains bounded");
    release1::PuzzleView before,after;before.quest=after.quest="pip_stuck_gate";
    before.objects={{"switch","Switch","switch",{},0,{},"",false,true},{"gate","Gate","gate",{},0,{},"",false,true}};
    after=before;for(auto& o:after.objects)o.active=false;
    release1::Intent undo;undo.verb=release1::Verb::Undo;
    auto effects=release1::audio::effects(before,after,undo);
    check(effects.size()==3&&effects[1].cue=="switch_up"&&effects[2].cue=="gate_close","Undo projects both actual switch release and gate closure");
    check(release1::audio::effects(after,after,{}).size()==1,"Unchanged contact state emits no mechanical edges");
    release1::Intent move;move.verb=release1::Verb::Slide;move.object="cushion";after.objects={{"cushion","Cushion","blocking_cushion"}};
    check(release1::audio::effects(after,after,move)[0].cue=="cushion_move","Cushions use fabric movement rather than ceramic friction");
    release1::Intent tap;tap.verb=release1::Verb::TapSymbol;tap.object="paw";
    effects=release1::audio::effects(after,after,tap);
    check(effects.size()==2&&effects[0].cue=="bell_paw"&&effects[1].cue=="bell_reset","Wrong symbol retains its own bell followed by gentle reset");
    using Phase=release1::garden::Phase;
    PipSounds pip;
    check(!pip.advance(Phase::Approach,"garden_cushion",{0,0,0},0,.05).step,"First walking sample anchors without a phantom step");
    unsigned contacts=0;for(int n=1;n<=36;++n)contacts+=pip.advance(Phase::Approach,"garden_cushion",{float(n)*.01f,0,0},0,1./30).step;
    check(contacts>=3&&contacts<=4,"Canonical walk stride emits four bounded paw contacts");
    check(!pip.advance(Phase::Approach,"garden_cushion",{10,0,0},0,20).step,"Teleport and long frames cannot create catch-up footsteps");
    check(pip.advance(Phase::Enter,"garden_cushion",{10,0,0},0,.05).settle,"Cushion entry rustles exactly once");
    check(!pip.advance(Phase::Enter,"garden_cushion",{10,0,0},0,.05).settle,"Repeated presentation frames do not retrigger settling");
    auto resting=pip.advance(Phase::Use,"garden_cushion",{10,0,0},0,.05);
    check(resting.purr&&!resting.settle&&!resting.step,"Rest starts purr without a second cushion contact");
    pip.advance(Phase::Use,"garden_cushion",{10,0,0},0,0);
    check(!pip.advance(Phase::Use,"garden_cushion",{10,0,0},0,.05).settle,"Pause/resume does not duplicate contacts");
    check(!pip.advance(Phase::Leave,"garden_cushion",{10,0,0},0,.05).purr,"Leaving stops purr");
    pip.reset();resting=pip.advance(Phase::Use,"garden_cushion",{},0,.05);
    check(resting.settle&&resting.purr,"Reduced-motion direct use still has a single settling cue");
    check(pip.advance(Phase::Use,"drinking_bowl",{},0,.05).drink,"New bowl use plays lapping once");
    check(!pip.advance(Phase::Use,"drinking_bowl",{},0,.05).drink,"Drinking does not retrigger every frame");
    pip.reset();check(pip.advance(Phase::Use,"pip_lookout",{},0,.05).purr,"Shared lookout requests Pip purr");
    check(!pip.advance(Phase::Idle,"",{},0,.05).purr,"Leaving lookout stops its purr request");
    pip.reset();check(!pip.advance(Phase::Idle,"",{},0,.05).purr,"Scene reset releases Pip sound state");
    pip.reset();pip.advance(Phase::Approach,"",{},0,1./60,toonWalkStride);
    contacts=0;for(int n=1;n<=138;++n)contacts+=pip.advance(Phase::Approach,"",{float(n)*pipWalkSpeed/60,0,0},0,1./60,toonWalkStride).step;
    check(contacts>=3&&contacts<=4,"Replacement stride must not play legacy footsteps nearly three times too fast");
    GardenBirdSchedule birds;
    for(int n=0;n<1000;++n)check(!birds.advance(.1,false),"No birds while paused, muted or outside garden");
    for(int n=0;n<179;++n)check(!birds.advance(.1,true),"First bird waits eighteen audible seconds");
    check(birds.advance(.2,true),"Bird starts after audible countdown");
    check(!birds.advance(1000,true),"Long frame cannot cause a catch-up bird burst");
    birds.reset();check(!birds.advance(1000,true),"Scene reset starts a fresh bounded countdown");
    check(!birds.advance(std::nan(""),true),"Invalid presentation delta ignored");
    int birdCount=0;for(int n=0;n<6000;++n)birdCount+=birds.advance(.1,true);
    check(birdCount>=16&&birdCount<=34,"Garden birds stay sparse with varying intervals");
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
    loads.clear();audio.update(.1);
    check(loads.empty(),"Already decoded ambience remains cached across scene changes");
    GameAudio warm(std::make_unique<Backend>());std::vector<std::size_t> warmLoads;
    warm.request=[&](auto i,auto){warmLoads.push_back(i);};warm.activate();warm.sceneChanged();warmLoads.clear();warm.update(.1);
    check(warmLoads.size()==std::count_if(audioClips.begin(),audioClips.end(),[](const auto& c){return c.bus!=0;}),"Cancelled small-effect decodes preload again after scene replacement");
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
    auto playlistBackend=std::make_unique<Backend>();auto* playlistDevice=playlistBackend.get();
    GameAudio playlist(std::move(playlistBackend));playlist.playlist(gardenMusicPlaylist);
    std::vector<std::pair<std::size_t,unsigned>> musicLoads;std::vector<std::string> musicEvents;
    playlist.request=[&](auto i,auto g){if(audioClips[i].bus==0)musicLoads.emplace_back(i,g);};
    playlist.event=[&](auto e){musicEvents.emplace_back(e);};
    auto musicDeliver=[&]{auto l=std::move(musicLoads);musicLoads.clear();for(auto [i,g]:l)check(playlist.publish(i,g,sound),"Playlist decode accepted");};
    playlist.update(1);check(musicLoads.empty(),"Playlist waits for sound activation");
    playlist.activate();playlist.update(.1);musicDeliver();playlist.update(.1);
    check(musicEvents.back()==gardenMusicPlaylist[0]&&playlistDevice->voices.size()==1,"Playlist starts with track one");
    const auto firstVoice=playlistDevice->voices.begin()->first;
    playlist.music("music_pip_at_home");playlist.sceneChanged();playlist.update(.3);
    check(playlistDevice->voices.contains(firstVoice)&&musicEvents.size()==1,"Scene/ending changes preserve the live playlist voice");
    playlistDevice->voices.clear();playlist.update(.1);musicDeliver();playlist.update(.1);
    check(musicEvents.back()==gardenMusicPlaylist[1]&&playlistDevice->voices.size()==1,"Natural end advances to track two");
    check(playlist.pcmBytes()==sound->samples().size_bytes(),"Only current music PCM retained");
    playlist.suspend(true);playlist.update(500);check(musicEvents.size()==2,"Host pause freezes playlist");
    playlist.suspend(false);playlist.activate();playlist.update(.1);check(musicEvents.size()==2,"Resume keeps current track");
    const auto suspensionChanges=playlistDevice->suspensionChanges;
    for(int n=0;n<100;++n)playlist.suspend(false);
    check(playlistDevice->suspensionChanges==suspensionChanges,"Unchanged frames do not invalidate pending backend resume");
    playlistDevice->voices.clear();playlist.update(.1);musicDeliver();playlist.update(.1);
    check(musicEvents.back()==gardenMusicPlaylist[0]&&musicEvents.size()==3,"Track two wraps to track one");
    bool invalidPlaylist=false;try{playlist.playlist(std::array<std::string_view,1>{"missing"});}catch(const std::invalid_argument&){invalidPlaylist=true;}
    check(invalidPlaylist&&playlistDevice->voices.size()==1,"Invalid replacement preserves playlist");
    playlist.playlist({});check(playlistDevice->voices.empty()&&playlist.pcmBytes()==0,"Clearing playlist releases music");
    playlist.playlist(gardenMusicPlaylist);playlist.update(.1);
    auto staleMusic=musicLoads.back();musicLoads.clear();playlist.sceneChanged();
    check(!playlist.publish(staleMusic.first,staleMusic.second,sound),"Scene generation rejects pending stale playlist decode");
    playlist.update(.1);auto failedMusic=musicLoads.back();musicLoads.clear();playlist.failed(failedMusic.first,failedMusic.second);
    playlist.update(.1);musicDeliver();playlist.update(.1);
    check(musicEvents.back()==gardenMusicPlaylist[1],"Failed first track skips to playable second track");
    std::cout<<"Audio activation, buses, cooldown, 12-effect budget, loops, music, stale scene generation passed\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
