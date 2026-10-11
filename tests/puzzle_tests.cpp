#include "puzzle.hpp"
#include "afternoon.hpp"
#include <iostream>
#include <stdexcept>
using namespace paws;
void check(bool condition,const char* message){if(!condition) throw std::runtime_error(message);}
CommandResult act(PawsSession& s,Action a,std::string id="",Direction d=Direction::North){return s.dispatch({s.state().attempt,s.state().revision,a,std::move(id),d});}
int main(){
 try {
  auto content=missingBall(); check(validateContent(content).empty(),"known solution validates");
  PawsSession s(content); const auto initial=s.state().puzzle;
  auto revision=s.state().revision;
  check(!act(s,Action::Move,"pot_a",Direction::East).accepted,"off-slot move rejected");
  check(s.state().puzzle==initial && s.state().revision==revision,"illegal move atomic");
  check(!act(s,Action::Roll).accepted && s.state().puzzle==initial,"failed roll keeps source and state");
  check(act(s,Action::Move,"pot_a").accepted,"first pot north");
  const auto occupied=s.state().puzzle;
  check(!act(s,Action::Move,"pot_b",Direction::West).accepted && s.state().puzzle==occupied,"occupied slot rejected");
  check(act(s,Action::Undo).accepted && s.state().puzzle==initial,"undo restores occupancy");
  check(!act(s,Action::Undo).accepted,"empty undo rejected");
  check(!act(s,Action::Rotate,"pot_a").accepted,"pot rotation rejected");
  check(!act(s,Action::Move,"channel_a").accepted,"channel slide rejected");
  check(!act(s,Action::Select,"missing").accepted,"unknown identity rejected");
  auto stale=Command{s.state().attempt,s.state().revision,Action::Rotate,"channel_a"};
  check(act(s,Action::Select,"pot_a").accepted,"select");
  check(!s.dispatch(stale).accepted,"stale revision rejected");
  stale={s.state().attempt,s.state().revision,Action::Roll,""};
  check(act(s,Action::Restart).accepted,"restart");
  stale.revision=s.state().revision;
  check(!s.dispatch(stale).accepted,"stale attempt rejected even with current revision");
  check(!s.canUndo() && s.state().puzzle==initial,"restart resets history and board");
  check(act(s,Action::Pause).accepted,"pause");
  check(!act(s,Action::Move,"pot_a").accepted,"pause rejects puzzle mutation");
  check(act(s,Action::Resume).accepted,"resume");
  check(act(s,Action::Garden).accepted && s.state().inGarden,"garden suspends attempt");
  check(act(s,Action::Resume).accepted && s.state().puzzle==initial,"continue retains attempt");
  for(const auto& step:content.solution) check(act(s,step.action,step.object,step.direction).accepted,"solution command");
  check(s.state().puzzle.completed,"solution completes");
  auto route=evaluateRoute(content,s.state().puzzle);
  check(route.reached && route.path.front()==content.source && route.path.back()==content.destination,"finite successful route");
  check(!act(s,Action::Roll).accepted,"duplicate complete roll rejected");
  check(act(s,Action::Undo).accepted && !s.state().puzzle.completed,"undo success restores source");
  auto before=s.state().puzzle;
  check(act(s,Action::Rotate,"channel_b").accepted,"rotate");
  check(act(s,Action::Undo).accepted && s.state().puzzle==before,"undo rotation");
  auto bad=content; bad.schemaVersion=3; check(!validateContent(bad).empty(),"future schema rejected");
  bad=content; bad.objects[1].id=bad.objects[0].id; check(!validateContent(bad).empty(),"duplicate IDs");
  bad=content; bad.solution.clear(); check(!validateContent(bad).empty(),"missing solution");
  bad=content; bad.solution.pop_back(); check(!validateContent(bad).empty(),"unsolved solution");
  bad=content; bad.objects[0].slots.push_back({40,40}); check(!validateContent(bad).empty(),"out-of-bounds content");
  // Four curves form a closed cycle. Traversal may begin inside the cycle.
  auto cycle=content;
  cycle.objects={{"a","A",Kind::Curve,{1,0},1,{}},{"b","B",Kind::Curve,{2,0},2,{}},
                 {"c","C",Kind::Curve,{2,1},3,{}},{"d","D",Kind::Curve,{1,1},0,{}}};
  auto loop=traceRoute(cycle,initialState(cycle),{1,0},Direction::South);
  check(!loop.reached && loop.message.find("loops")!=std::string::npos && loop.path.size()==4,"cycle terminates");
  auto empty=content; empty.objects.clear();
  check(!evaluateRoute(empty,{}).reached,"empty route terminates");
  auto edge=content; edge.sourceExit=Direction::West;
  check(evaluateRoute(edge,initialState(edge)).message.find("off the board")!=std::string::npos,"off-board route terminates");
  for(auto c:{tidyToys(),stuckGate()}) check(validateContent(c).empty(),"new known solution");
  PawsSession tidy(tidyToys());
  check(!act(tidy,Action::PickUp,"ball").accepted,"cushion blocks pickup");
  check(act(tidy,Action::Move,"cushion").accepted,"move cushion");
  check(act(tidy,Action::PickUp,"ball").accepted,"pickup reachable ball");
  check(!act(tidy,Action::PickUp,"mouse").accepted,"one held object");
  auto place=[&](std::string toy,std::string target) {return tidy.dispatch({tidy.state().attempt,tidy.state().revision,Action::Place,toy,Direction::North,target});};
  auto held=tidy.state().puzzle;auto heldRevision=tidy.state().revision;
  check(!place("ball","mouse_recess").accepted&&tidy.state().puzzle==held&&tidy.state().revision==heldRevision,"wrong shape is atomic");
  check(act(tidy,Action::Move,"cushion",Direction::South).accepted,"reblock basket");
  check(!place("ball","ball_recess").accepted,"blocked recess");
  check(act(tidy,Action::Undo).accepted,"undo blocker");
  check(place("ball","ball_recess").accepted,"matching recess");
  check(act(tidy,Action::PickUp,"mouse").accepted,"pickup second toy");
  check(!place("mouse","ball_recess").accepted,"occupied recess");
  check(act(tidy,Action::Undo).accepted&&act(tidy,Action::Undo).accepted&&tidy.state().puzzle==held,"undo pickup and placement restores held toy");
  check(act(tidy,Action::Restart).accepted&&tidy.state().puzzle==initialState(tidy.content()),"restart toys and sockets");
  PawsSession gate(stuckGate());
  check(!act(gate,Action::PickUp,"mouse").accepted,"closed gate retrieval blocked");
  for(auto step:gate.content().solution) if(step.action!=Action::PickUp) check(act(gate,step.action,step.object,step.direction).accepted,"gate route solution");
  check(switchActive(gate.content(),gate.state().puzzle,"switch")&&gateOpen(gate.content(),gate.state().puzzle,"gate"),"ball opens linked gate");
  check(act(gate,Action::ReturnBall).accepted&&!gateOpen(gate.content(),gate.state().puzzle,"gate"),"empty switch closes gate");
  check(!act(gate,Action::PickUp,"mouse").accepted,"retrieval blocked again");
  check(act(gate,Action::Undo).accepted&&gateOpen(gate.content(),gate.state().puzzle,"gate"),"undo restores switch and gate");
  check(act(gate,Action::Rotate,"channel_a").accepted&&!gateOpen(gate.content(),gate.state().puzzle,"gate"),"route edit recalls ball coherently");
  check(act(gate,Action::Undo).accepted,"undo route edit");
  check(act(gate,Action::PickUp,"mouse").accepted&&gate.state().puzzle.completed,"mouse retrieved through open gate");
  check(act(gate,Action::Undo).accepted&&!gate.state().puzzle.completed&&gateOpen(gate.content(),gate.state().puzzle,"gate"),"undo restores collectible while switch stays held");
  check(act(gate,Action::Undo).accepted&&!gateOpen(gate.content(),gate.state().puzzle,"gate"),"undo roll closes gate");
  check(act(gate,Action::Restart).accepted&&gate.state().puzzle==initialState(gate.content()),"restart gate");
  Afternoon afternoon;
  check(!afternoon.selectQuest(1)&&!afternoon.selectQuest(2),"ordered unlocks");
  auto send=[&](SolutionStep step){const auto& st=afternoon.session().state();return afternoon.dispatch({st.attempt,st.revision,step.action,step.object,step.direction,step.target});};
  for(std::size_t q=0;q<3;++q) {
      check(afternoon.selectQuest(q),"unlock next quest");
      auto solution=afternoon.session().content().solution;
      for(std::size_t n=0;n<solution.size();++n) {
          check(send(solution[n]).accepted,"campaign solution");
          auto saved=afternoon.encode();auto restored=Afternoon::decode(saved);
          check(restored.encode()==saved&&restored.session().state().puzzle==afternoon.session().state().puzzle,"mid-action save roundtrip including held/placed/switch state");
          afternoon=std::move(restored);
      }
      check(afternoon.coins()==int(q+1)*20&&afternoon.friendship()==int(q+1),"atomic once-only reward");
      check(send({Action::Undo,{}}).accepted,"undo completed quest");
      check(send(solution.back()).accepted&&afternoon.coins()==int(q+1)*20,"undo completion cannot duplicate reward");
  }
  auto done=afternoon.quest(2).state().puzzle;
  auto old=afternoon.session().state();
  check(afternoon.selectQuest(0),"practice earlier quest");
  check(!afternoon.dispatch({old.attempt,old.revision,Action::Restart,{}}).accepted,"cross-quest stale action rejected");
  check(send({Action::Restart,{}}).accepted,"practice restart");
  check(afternoon.quest(2).state().puzzle==done,"attempts are independent");
  for(auto step:afternoon.session().content().solution) check(send(step).accepted,"practice solve");
  check(afternoon.coins()==60&&afternoon.friendship()==3&&!afternoon.lastRewardGranted(),"practice no extra rewards");
  check(send({Action::Hint,{}}).accepted&&send({Action::Hint,{}}).accepted,"progressive hints");
  afternoon.settings={true,true,true,30,40,50};
  auto save=afternoon.encode();
  auto settingsRestore=Afternoon::decode(save);
  check(settingsRestore.session().state().hint==2&&settingsRestore.settings.largerText&&settingsRestore.settings.reducedMotion&&
      settingsRestore.settings.muted&&settingsRestore.settings.music==30&&settingsRestore.settings.ambience==40&&settingsRestore.settings.effects==50,"hint stages and settings persist");
  for(auto damaged:{save+"trailing",std::string("PAWS_AFTERNOON 9"),save.substr(0,save.size()/2)}) {
      bool rejected=false;try{Afternoon::decode(damaged);}catch(...){rejected=true;}check(rejected,"damaged save rejected");
  }
  std::cout<<"Rules: three known solutions, occupancy, shape/reachability, switch/gate, undo/restart, stale commands, independent attempts, rewards and saves passed\n";
 } catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
