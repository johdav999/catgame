#define main legacyAudioTests
#include "legacy_tests.inc"
#undef main
#include "release1/audio/cues.hpp"
#include <limits>

int main() { try {
    check(legacyAudioTests()==0,"Existing runtime regressions");
    auto backend=std::make_unique<Backend>();auto* device=backend.get();
    GameAudio audio(std::move(backend));
    paws::release1::audio::Cues cues(audio);
    auto sound=engine::SoundAsset::create(48000,1,std::vector<float>(480,.1f));
    std::vector<std::pair<std::size_t,unsigned>> loads;
    audio.request=[&](auto i,auto g){loads.emplace_back(i,g);};
    auto deliver=[&]{auto l=std::move(loads);loads.clear();for(auto [i,g]:l)audio.publish(i,g,sound);};
    cues.observe({5,9,1,"water"});audio.activate();deliver();
    paws::release1::CommittedEvent e{5,9,1,"water","route","water_flow"};
    check(cues.committed(e),"Current commit accepted");
    check(device->voices.size()==1,"Water plays on the existing system");
    check(!cues.committed(e),"Duplicate commit suppressed");
    auto stale=e;stale.generation=4;check(!cues.committed(stale),"Wrong scene rejected");
    stale=e;stale.attempt=8;check(!cues.committed(stale),"Wrong attempt rejected");
    stale=e;stale.revision=2;check(!cues.committed(stale),"Future revision rejected");
    stale=e;stale.quest="other";check(!cues.committed(stale),"Other quest rejected");
    stale=e;stale.object="other";check(!cues.committed(stale,.401),"Expired event rejected");
    check(!cues.committed(stale,std::numeric_limits<double>::quiet_NaN()),"Invalid clock rejected");
    audio.update(4.1);check(device->voices.empty(),"Water bounded to four seconds");
    audio.update(1);check(device->voices.empty(),"Water never restarts as a persistent loop");
    cues.observe({5,9,2,"water"});e.revision=2;check(cues.committed(e),"Next committed route");
    cues.observe({5,9,3,"water"});check(device->voices.empty(),"New arrangement cancels flow");
    audio.update(4.1);e.revision=3;check(cues.committed(e),"Route can restart after cooldown");
    cues.suspend(true);check(device->voices.empty(),"Pause cancels transient flow");
    e.cue="bowl_fill";check(!cues.committed(e),"Suspended events rejected");
    cues.suspend(false);check(!device->active(),"Return requires activation gesture");audio.activate();
    cues.observe({6,10,1,"bells"});e={6,10,1,"bells","leaf","bell_leaf"};
    check(cues.committed(e),"Visual leaf commit routes");
    auto count=device->voices.size();e.object="leaf2";check(cues.committed(e),"Distinct receipt accepted");
    check(device->voices.size()==count,"Runtime cooldown retained");
    AfternoonSettings settings;settings.muted=true;audio.settings(settings);
    e.object="star";e.cue="bell_star";check(cues.committed(e),"Muted bell still consumes committed receipt");
    check(device->voices.size()==count,"Muted bells do not queue");
    cues.observe({7,11,1,"picnic"});check(device->voices.empty(),"Transition stops all voices");
    e={7,11,1,"picnic","ending","picnic_complete"};check(cues.committed(e),"Picnic aliases existing Suno ending");
    check(!cues.committed(e),"Picnic receipt does not repeat");
    std::cout<<"Release-one scope, receipts, stale/expiry, water cancellation, muted bells and picnic passed; 66 slots, synthetic fixtures only\n";
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;} }
