#include "release1/rules/content.hpp"
#include "release1/rules/picture_sequence.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace paws;using namespace paws::release1;using namespace paws::release1::rules;
namespace {
int checks=0;
void check(bool ok,const char* why) {++checks;if(!ok)throw std::runtime_error(why);}
void go(Attempt& a,Step s) {auto r=a.dispatch(a.command(s));if(!r.accepted)throw std::runtime_error(r.message);}
void blocked(Attempt& a,Step s,std::string_view reason) {
    auto state=a.logicalState();auto rev=a.view().revision;auto result=a.dispatch(a.command(s));
    check(!result.accepted,"illegal command accepted");check(result.message.find(reason)!=std::string::npos,"missing useful blocked reason");
    check(a.logicalState()==state&&a.view().revision==rev,"rejected command changed state");
}
void solve(Attempt& a) {
    for(auto& s:a.definition().solution) {
        const auto c=a.command(s);const auto view=a.view();
        check(std::any_of(view.actions.begin(),view.actions.end(),[&](auto& action){
            const auto& intent=action.intent;
            return action.enabled&&intent.verb==c.verb&&intent.object==c.object&&intent.target==c.target&&intent.direction==c.direction&&intent.destination==c.destination;
        }),"known solution is absent from enabled read-model actions");go(a,s);
    }
    check(a.view().completed,"not completed");
}
void bad(Definition d) {check(!validate(d).empty(),"malformed content accepted");}
}
int main() {try {
    auto all=catalogue();check(all.size()==24,"Base layouts and first beach adventure required");
    for(auto d:all) {
        auto e=validate(d);if(!e.empty())throw std::runtime_error(d.quest+": "+e);
        Attempt a(d),b(d);auto fresh=a.command({Verb::Hint});
        go(a,{Verb::Hint});check(!a.dispatch(fresh).accepted,"stale revision accepted");
        auto h=a.command({Verb::Hint});++h.generation;check(!a.dispatch(h).accepted,"stale generation accepted");
        h=a.command({Verb::Hint});++h.attempt;check(!a.dispatch(h).accepted,"stale attempt accepted");
        h=a.command({Verb::Hint});h.quest="other";check(!a.dispatch(h).accepted,"foreign quest accepted");
        go(a,{Verb::Hint});go(a,{Verb::Hint});go(a,{Verb::Hint});check(a.view().revealedHints==3,"hint bound");
        check(b.view().revision==0&&!b.view().completed,"attempts share mutable state");
        auto before=a.command({Verb::Hint});a.activate(9);check(!a.dispatch(before).accepted,"activation did not invalidate command");
        a.setPaused(true);check(!a.dispatch(a.command({Verb::Hint})).accepted,"pause accepted action");a.setPaused(false);
        solve(a);go(a,{Verb::Undo});check(!a.view().completed,"undo failed to reopen completion");
        go(a,d.solution.back());check(a.view().completed,"redo final operation failed");
        auto previous=a.command({Verb::Hint});go(a,{Verb::Restart});check(a.view().attempt!=previous.attempt,"restart kept attempt identity");
        check(!a.dispatch(previous).accepted,"restart stale command accepted");check(!a.view().canUndo&&!a.view().completed&&a.view().revealedHints==0,"restart not fresh");solve(a);
        auto invalid=d;invalid.solution[0].object="missing";bad(invalid);
        invalid=d;invalid.hints[1].clear();bad(invalid);
        invalid=d;invalid.width=17;bad(invalid);
        std::cout<<d.quest<<": validate, complete, undo, restart, stale commands passed\n";
    }
    Attempt bench(quest("button_mouse_bench"));
    blocked(bench,{Verb::Retrieve,"mouse","near_reach"},"crates");
    blocked(bench,{Verb::Slide,"crate_a","",Direction::North},"Upper crate");
    blocked(bench,{Verb::Retrieve,"mouse","far_reach"},"cannot reach");
    go(bench,{Verb::Slide,"crate_b","",Direction::East});go(bench,{Verb::Slide,"crate_a","",Direction::North});
    go(bench,{Verb::Undo});blocked(bench,{Verb::Retrieve,"mouse","near_reach"},"crates");
    auto bd=bench.definition();bd.requested="bench";bad(bd);bd=bench.definition();bd.reach={4,2};bad(bd);
    Attempt planks(quest("mabel_sunny_spot"));blocked(planks,{Verb::Connect},"do not connect");
    Attempt directPlanks(quest("mabel_sunny_spot"));
    go(directPlanks,{Verb::Rotate,"long_plank"});go(directPlanks,{Verb::Rotate,"step_plank"});
    check(directPlanks.logicalState().objects[0].location==Location::Ground&&directPlanks.logicalState().objects[1].location==Location::Ground,"direct rotations claim a held piece");
    go(directPlanks,{Verb::PickUp,"long_plank"});go(directPlanks,{Verb::Fit,"long_plank","long_socket"});
    go(directPlanks,{Verb::Rotate,"long_plank"});
    check(directPlanks.logicalState().objects[0].socket.empty()&&directPlanks.logicalState().objects[0].location==Location::Ground,"rotated fitted plank retains a supported edge");
    go(directPlanks,{Verb::Undo});check(directPlanks.logicalState().objects[0].socket=="long_socket","undo did not restore fitted plank");
    go(planks,{Verb::PickUp,"long_plank"});blocked(planks,{Verb::Fit,"long_plank","step_socket"},"length");
    blocked(planks,{Verb::Fit,"long_plank","long_socket"},"Rotate");
    go(planks,{Verb::Rotate,"long_plank"});go(planks,{Verb::Fit,"long_plank","long_socket"});
    go(planks,{Verb::PickUp,"step_plank"});blocked(planks,{Verb::Fit,"step_plank","unsupported_socket"},"supported");
    auto pd=planks.definition();pd.sockets[0].end={2,1};bad(pd);
    // A cycle connected to the start terminates and does not imply a destination.
    auto cycle=planks.definition();cycle.sockets[0]={"a","a","plank_socket",{0,2},{1,2},1,0};
    cycle.sockets[1]={"b","b","plank_socket",{1,2},{1,1},1,1};
    cycle.sockets[2]={"c","c","plank_socket",{1,1},{0,2},2,0};
    State cs;for(auto& o:cycle.objects)cs.objects.push_back({o.initial,0});
    for(int i=0;i<3;++i){cycle.objects[i].length=cycle.sockets[i].length;cs.objects[i].orientation=cycle.sockets[i].orientation;cs.objects[i].location=Location::Socket;cs.objects[i].socket=cycle.sockets[i].id;}
    check(!plankRoute(cycle,cs).reached,"cyclic route fabricated success");check(!plankRoute(cycle,{}).reached,"malformed route state accepted");
    Attempt shapes(quest("button_cushion_bed"));go(shapes,{Verb::PickUp,"square"});
    Attempt directShapes(quest("button_cushion_bed"));go(directShapes,{Verb::Rotate,"bar"});go(directShapes,{Verb::Rotate,"elbow"});
    check(directShapes.logicalState().objects[1].orientation==1&&directShapes.logicalState().objects[2].orientation==1,"independent cushion clicks failed");
    blocked(shapes,{Verb::PickUp,"bar"},"holding");blocked(shapes,{Verb::Fit,"square","basket",Direction::North,{4,2}},"outside");
    go(shapes,{Verb::Fit,"square","basket",Direction::North,{0,0}});go(shapes,{Verb::PickUp,"bar"});
    blocked(shapes,{Verb::Fit,"bar","basket",Direction::North,{0,0}},"overlaps");
    go(shapes,{Verb::Rotate,"bar"});go(shapes,{Verb::Fit,"bar","basket",Direction::North,{2,0}});
    go(shapes,{Verb::PickUp,"elbow"});auto origin=shapes.logicalState().objects[2];
    for(int n=0;n<4;++n){go(shapes,{Verb::Rotate,"elbow"});check(footprint(shapes.definition().objects[2],shapes.logicalState().objects[2]).size()==3,"rotation changed area");}
    check(shapes.logicalState().objects[2].orientation==origin.orientation,"four turns do not restore shape");
    auto sd=shapes.definition();sd.objects[0].shape.push_back({0,0});bad(sd);sd=shapes.definition();sd.basket[0]=sd.basket[1];bad(sd);
    // History is bounded independently of hints and restart; rotations preserve
    // cell area and a long interaction session cannot accumulate unbounded undo.
    Attempt history(quest("button_cushion_bed"));go(history,{Verb::PickUp,"bar"});
    for(int n=0;n<140;++n)go(history,{Verb::Rotate,"bar"});
    for(int n=0;n<128;++n)go(history,{Verb::Undo});blocked(history,{Verb::Undo},"Nothing");
    check(history.logicalState().objects[1].location==Location::Held,"history overflow lost current held piece");
    Attempt water(quest("mabel_drinking_route"));blocked(water,{Verb::Roll},"water");solve(water);
    auto waterDef=*water.definition().legacy;auto ws=initialState(waterDef);ws.objects.clear();
    check(!evaluateRoute(waterDef,ws).reached,"bad water state accepted");
    Attempt bells(quest("pepper_garden_bells"));go(bells,{Verb::TapSymbol,"leaf"});go(bells,{Verb::TapSymbol,"paw"});
    check(bells.view().enteredSymbols.empty(),"sequence error did not clear row");
    go(bells,{Verb::Undo});check(bells.view().enteredSymbols==std::vector<std::string>{"leaf"},"undo did not restore prior sequence");
    go(bells,{Verb::ReplaySequence});check(bells.view().enteredSymbols.empty()&&bells.view().sequenceReplay==1,"replay did not clear answer and publish cue identity");
    go(bells,{Verb::Undo});check(bells.view().enteredSymbols==std::vector<std::string>{"leaf"},"undo replay lost previous prefix");
    PictureSequencePlayback playback;playback.start({"leaf","star","leaf","paw","star"},1);
    check(playback.picture()=="leaf","first picture not immediate");playback.advance(.999);check(playback.index()==0,"picture ended early");
    playback.advance(.001);check(playback.index()==1&&playback.picture()=="star","one-second boundary failed");
    playback.advance(5,true);check(playback.index()==1,"paused clue advanced");playback.advance(4);check(!playback.watching()&&playback.picture().empty(),"answer phase did not blank the board");
    auto reusable=pictureSequence("other_sequence","other-v1","Other pictures",
        {{"response_a","First response","bell_leaf",Piece::Bell,{0,0},0,1,{}, {},"flower"},{"response_b","Second response","bell_star",Piece::Bell,{1,0},0,1,{}, {},"moon"}},
        {"moon","flower","moon"},.5);
    check(validate(reusable).empty(),"reusable sequence still depends on Pepper or three fixed symbols");Attempt other(reusable);solve(other);
    reusable.pictureSeconds=0;bad(reusable);
    go(bells,{Verb::Restart});solve(bells);auto dd=bells.definition();dd.sequence.push_back("unknown");bad(dd);dd=bells.definition();dd.sequence.resize(9,"leaf");bad(dd);
    Attempt cats(quest("pip_pepper_playhouse","pip-pepper-playhouse-v1"));go(cats,{Verb::AssignCat,"pip","pip_switch"});go(cats,{Verb::AssignCat,"pip","waiting"});
    go(cats,{Verb::AssignCat,"pepper","pepper_switch"});blocked(cats,{Verb::Connect},"together now");
    blocked(cats,{Verb::AssignCat,"pip","pepper_switch"},"own marked");
    go(cats,{Verb::AssignCat,"pip","pip_switch"});check(heldSwitches(cats.definition(),cats.logicalState()),"simultaneous switches false");
    go(cats,{Verb::Connect});go(cats,{Verb::Undo});go(cats,{Verb::AssignCat,"pepper","waiting"});
    check(!heldSwitches(cats.definition(),cats.logicalState()),"switch latched after release");
    Attempt picnic(quest("garden_picnic"));go(picnic,{Verb::Fit,"bed","spot_1_2"});go(picnic,{Verb::Fit,"bowl","spot_2_1"});blocked(picnic,{Verb::Connect},"SHADE");
    go(picnic,{Verb::Fit,"bed","spot_1_0"});blocked(picnic,{Verb::Connect},"QUIET");
    go(picnic,{Verb::Fit,"bed","spot_1_1"});go(picnic,{Verb::Fit,"bowl","spot_2_0"});blocked(picnic,{Verb::Connect},"edge");
    blocked(picnic,{Verb::Fit,"bowl","spot_1_1"},"occupies");go(picnic,{Verb::Fit,"bowl","spot_1_0"});go(picnic,{Verb::Connect});check(picnic.view().completed,"alternate picnic solution failed");
    const auto pv=picnic.view();
    check(std::any_of(pv.objects.begin(),pv.objects.end(),[&](auto& o){return o.role=="shade_clue"&&o.footprint==picnic.definition().shade;}),"portable view omitted shade cells");
    check(std::any_of(pv.objects.begin(),pv.objects.end(),[&](auto& o){return o.role=="quiet_clue"&&o.footprint==picnic.definition().quiet;}),"portable view omitted quiet cells");
    // Read-model legality must predict the same command path and remain read-only.
    for(auto d:all) {
        Attempt a(d);auto before=a.logicalState();auto v=a.view();check(a.logicalState()==before&&a.view().revision==v.revision,"projection mutates state");
        for(auto action:v.actions) {Attempt fresh(d);check(fresh.dispatch(action.intent).accepted==action.enabled,"preflight diverges from commit");}
    }
    std::cout<<checks<<" checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
