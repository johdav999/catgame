#include "afternoon.hpp"
#include "release1/rules/content.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace paws;
using namespace paws::release1;
using namespace paws::release1::rules;
namespace {
int checks=0;
void check(bool ok,std::string message){++checks;if(!ok)throw std::runtime_error(message);}
void step(Attempt& a,Step s){auto r=a.dispatch(a.command(s));check(r.accepted,r.message);}
void blocked(Attempt& a,Step s,std::string_view reason){
    const auto before=a.view();auto result=a.dispatch(a.command(s));
    check(!result.accepted&&result.message.find(reason)!=std::string::npos,"Expected rejection: "+result.message);
    check(a.view().revision==before.revision,"Rejected action changes revision");
}
void solve(Attempt& a){for(auto s:a.definition().solution)step(a,s);check(a.view().completed,"Known solution completes");}
}
int main(int argc,char** argv){try{
    for(auto id:{"pip_missing_ball","pip_tidy_toys","pip_stuck_gate","button_mouse_bench","mabel_sunny_spot"}){
        auto d=quest(id);check(d.content.ends_with("-v2"),"New content identity");check(validate(d).empty(),"Legal known solution");
        Attempt a(d);solve(a);step(a,{Verb::Undo});check(!a.view().completed,"Completion undo");
        step(a,d.solution.back());check(a.view().completed,"Recompletion");step(a,{Verb::Restart});solve(a);
        auto old=d.content;old.back()='1';Attempt original(quest(id,old));solve(original);
    }
    Attempt ball(quest("pip_missing_ball"));
    step(ball,{Verb::Slide,"pot_a","",Direction::North});blocked(ball,{Verb::Slide,"pot_b","",Direction::West},"occupies");
    step(ball,{Verb::Slide,"pot_a","",Direction::North});step(ball,{Verb::Slide,"pot_b","",Direction::West});
    blocked(ball,{Verb::Slide,"pot_b","",Direction::North},"occupies");
    step(ball,{Verb::Slide,"pot_a","",Direction::West});step(ball,{Verb::Slide,"pot_b","",Direction::North});
    step(ball,{Verb::Rotate,"channel_a"});step(ball,{Verb::Rotate,"channel_a"});
    blocked(ball,{Verb::Roll},"catch tray");check(!ball.view().completed,"Catch branch never completes");
    Attempt basket(quest("pip_tidy_toys"));
    step(basket,{Verb::Slide,"cushion","",Direction::North});blocked(basket,{Verb::PickUp,"ball"},"pickup position");
    step(basket,{Verb::PickUp,"fish"});step(basket,{Verb::Fit,"fish","fish_recess"});
    step(basket,{Verb::Slide,"cushion","",Direction::South});step(basket,{Verb::Slide,"cushion","",Direction::South});
    // Each visible recess has side access, even with the ball already packed.
    step(basket,{Verb::PickUp,"ball"});step(basket,{Verb::Fit,"ball","ball_recess"});step(basket,{Verb::PickUp,"mouse"});
    step(basket,{Verb::Fit,"mouse","mouse_recess"});check(basket.view().completed,"Packed ball cannot block the exposed mouse recess");
    step(basket,{Verb::Undo});step(basket,{Verb::Fit,"mouse","home"});
    step(basket,{Verb::PickUp,"ball"});step(basket,{Verb::Fit,"ball","home"});
    step(basket,{Verb::PickUp,"mouse"});step(basket,{Verb::Fit,"mouse","mouse_recess"});
    step(basket,{Verb::PickUp,"ball"});step(basket,{Verb::Fit,"ball","ball_recess"});check(basket.view().completed,"Basket recovery completes");
    std::vector<std::string> order{"ball","fish","mouse"};
    do {
        Attempt sideAccess(quest("pip_tidy_toys"));int cushion=1;
        blocked(sideAccess,{Verb::PickUp,order.front()},"pickup position");
        for(const auto& toy:order){
            const int parking=toy=="fish"?0:2;
            while(cushion!=parking){const auto direction=cushion<parking?Direction::South:Direction::North;step(sideAccess,{Verb::Slide,"cushion","",direction});cushion+=cushion<parking?1:-1;}
            step(sideAccess,{Verb::PickUp,toy});step(sideAccess,{Verb::Fit,toy,toy+"_recess"});
        }
        check(sideAccess.view().completed,"Every packing order works through the visible side aisle");
    }while(std::next_permutation(order.begin(),order.end()));
    Attempt gate(quest("pip_stuck_gate"));blocked(gate,{Verb::Slide,"pot","",Direction::West},"sliding slot");
    for(auto s:gate.definition().solution){if(s.verb==Verb::Roll){step(gate,s);break;}step(gate,s);}
    check(!gate.view().completed,"Gate opening is an intermediate stage");blocked(gate,{Verb::PickUp,"mouse"},"crate");
    step(gate,{Verb::Slide,"gate_crate","",Direction::West});blocked(gate,{Verb::Slide,"gate_crate","",Direction::South},"occupies");
    step(gate,{Verb::Slide,"pot","",Direction::West});step(gate,{Verb::Slide,"gate_crate","",Direction::South});
    const auto openGate=gate.view();check(std::any_of(openGate.actions.begin(),openGate.actions.end(),[](auto& a){return a.intent.verb==Verb::PickUp&&a.intent.object=="mouse"&&a.enabled;}),"Ball still holds switch after obstacle moves");
    step(gate,{Verb::PickUp,"mouse"});check(gate.view().completed,"Second gate stage completes");
    Attempt bench(quest("button_mouse_bench"));step(bench,bench.definition().solution[0]);step(bench,bench.definition().solution[1]);
    blocked(bench,{Verb::Retrieve,"mouse","near_reach"},"crates");blocked(bench,{Verb::Slide,"crate_c","",Direction::North},"Upper crate");
    for(std::size_t i=2;i<bench.definition().solution.size();++i)step(bench,bench.definition().solution[i]);check(bench.view().completed,"Three crate ordering completes");
    Attempt planks(quest("mabel_sunny_spot"));step(planks,{Verb::Rotate,"long_plank"});step(planks,{Verb::PickUp,"long_plank"});step(planks,{Verb::Fit,"long_plank","direct_first"});
    step(planks,{Verb::PickUp,"step_plank"});blocked(planks,{Verb::Fit,"step_plank","direct_second"},"length");
    step(planks,{Verb::Rotate,"step_plank"});step(planks,{Verb::Fit,"step_plank","step_socket"});
    blocked(planks,{Verb::Connect},"do not connect");step(planks,{Verb::Restart});
    step(planks,{Verb::Rotate,"approach_plank"});step(planks,{Verb::PickUp,"approach_plank"});blocked(planks,{Verb::Fit,"approach_plank","approach_socket"},"planter");
    step(planks,{Verb::Slide,"planter","",Direction::West});blocked(planks,{Verb::Fit,"approach_plank","approach_socket"},"planter");
    step(planks,{Verb::Slide,"planter","",Direction::North});step(planks,{Verb::Fit,"approach_plank","approach_socket"});step(planks,{Verb::Restart});solve(planks);
    Afternoon campaign;
    for(std::size_t q=0;q<5;++q){check(campaign.selectQuest(q),"Quest unlock");
        for(auto s:quest(progression::quests[q].id).solution){auto i=campaign.intent(s.verb,s.object,s.target);i.direction=s.direction;i.destination=s.destination;
            auto r=campaign.dispatchIntent(i);check(r.accepted,r.message);auto saved=campaign.encode();
            check(Afternoon::decode(saved).encode()==saved,"Every new move roundtrips its journal");}
        check(campaign.coins()==20*int(q+1),"Once-only reward");
        if(q==0&&argc==2){
            auto fixture=campaign;check(fixture.selectQuest(1),"Tidy fixture unlock");
            std::filesystem::create_directories(argv[1]);
            std::ofstream(std::filesystem::path(argv[1])/"tidy-start.save")<<fixture.encode();
            auto move=fixture.intent(Verb::Slide,"cushion");move.direction=Direction::South;check(fixture.dispatchIntent(move).accepted,"Expose ball fixture");
            check(fixture.dispatchIntent(fixture.intent(Verb::PickUp,"ball")).accepted,"Pick ball fixture");
            check(fixture.dispatchIntent(fixture.intent(Verb::Fit,"ball","ball_recess")).accepted,"Pack ball fixture");
            std::ofstream(std::filesystem::path(argv[1])/"ball-first.save")<<fixture.encode();
        }
        if(q==2){
            auto historical=campaign;check(historical.selectQuest(3),"Historical extra quest unlock");
            auto first=historical.intent(Verb::Slide,"crate_b");first.direction=Direction::East;
            check(historical.dispatchIntent(first).accepted,"Shared original first move");
            auto bytes=historical.encode();const std::string identity="\"button-mouse-bench-v2\"";auto at=bytes.find(identity);
            check(at!=std::string::npos,"Saved typed content ID");bytes.replace(at,identity.size(),"\"button-mouse-bench-v1\"");
            auto restored=Afternoon::decode(bytes);check(restored.puzzleView().content=="button-mouse-bench-v1","Old typed save keeps original layout");
            for(std::size_t n=1;n<quest("button_mouse_bench","button-mouse-bench-v1").solution.size();++n){
                auto s=quest("button_mouse_bench","button-mouse-bench-v1").solution[n];auto i=restored.intent(s.verb,s.object,s.target);i.direction=s.direction;
                check(restored.dispatchIntent(i).accepted,"Original typed journal remains solvable");}
            check(restored.coins()==80&&Afternoon::decode(restored.encode()).encode()==restored.encode(),"Mixed old/new journal roundtrip");
        }
    }
    auto damaged=campaign.encode();const std::string identity="\"button-mouse-bench-v2\"";auto at=damaged.find(identity);
    check(at!=std::string::npos,"Typed identity found");damaged.replace(at,identity.size(),"\"\"");
    bool rejected=false;try{Afternoon::decode(damaged);}catch(const std::invalid_argument&){rejected=true;}check(rejected,"Missing content identity cannot select a default version");
    std::cout<<checks<<" quest extension checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
