#include "afternoon.hpp"
#include "release1/contracts.hpp"
#include <iostream>
#include <stdexcept>

int main() {
    paws::Afternoon afternoon;
    auto check=[](bool value) { if(!value) throw std::runtime_error("Track baseline failed"); };
    for(std::size_t i=0;i<3;++i) {
        check(afternoon.selectQuest(i));
        const auto steps=afternoon.session().content().solution;
        for(const auto& step:steps) {
            const auto& state=afternoon.session().state();
            check(afternoon.dispatch({state.attempt,state.revision,step.action,
                step.object,step.direction,step.target}).accepted);
        }
        check(afternoon.rewarded(i));
    }
    check(afternoon.coins()==60 && afternoon.friendship()==3);
    const auto bytes=afternoon.encode();
    auto restored=paws::Afternoon::decode(bytes);
    check(restored.encode()==bytes);
    const auto& state=restored.session().state();
    check(!restored.dispatch({state.attempt+1,state.revision,paws::Action::Restart,{}}).accepted);
    check(restored.encode()==bytes);
    paws::release1::Intent intent;
    intent.attempt=state.attempt;intent.revision=state.revision;
    paws::release1::PuzzleView view;
    view.attempt=intent.attempt;view.revision=intent.revision;
    check(view.attempt==state.attempt);
    std::cout<<"Release-one track baseline: solutions, rewards, codec, stale identity passed\n";
}
