#include "release1/gui/views.hpp"
#include "release1/progression/owner.hpp"
#include "release1/rules/content.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace paws;using namespace paws::release1;
int assertions=0;
void check(bool v,const std::string& s){++assertions;if(!v)throw std::runtime_error(s);}
std::array<std::vector<std::byte>,3> fonts(){auto root=std::filesystem::path(__FILE__).parent_path().parent_path().parent_path().parent_path();std::array<std::vector<std::byte>,3> result;const char* names[]={"NunitoSans-Regular.ttf","NunitoSans-Semibold.ttf","Fraunces-Semibold.ttf"};for(int i=0;i<3;++i){std::ifstream f(root/"assets/runtime/fonts"/names[i],std::ios::binary|std::ios::ate);check(bool(f),"Font present");result[i].resize(std::size_t(f.tellg()));f.seekg(0);f.read(reinterpret_cast<char*>(result[i].data()),result[i].size());}return result;}
struct Driver {
    Afternoon owner;float width=390,height=844;std::string placing;bool reward=false;int cancellations=0;
    PawsUi ui;gui::Views views;
    Driver():ui(fonts(),false,[this](const UiIntent& i){if(i.action==UiAction::BackToGarden){check(owner.returnToGarden(owner.intent(Verb::Select)).accepted,"Garden command");ui.enter(UiScreen::Garden);project();}else if(i.action==UiAction::Restart){send(owner.intent(Verb::Restart));ui.enter(UiScreen::Puzzle);}else if(i.action==UiAction::CancelPlacement){++cancellations;placing.clear();project();}}),views(ui,[this](const Intent& i){send(i);},[this](std::string_view cat,const Intent& i){check(owner.introduce(cat,i).accepted,"Introduction commit");project();},[this](std::string_view id){placing=id;project();}){for(auto& item:progression::items)owner.setItemAvailable(item.id,true);check(owner.returnToGarden(owner.intent(Verb::Select)).accepted,"Initial garden");project();ui.enter(UiScreen::Garden);}
    void project(bool development=true){gui::Model m;m.puzzle=owner.puzzleView();m.progress=owner.progressView();m.services.inGarden=owner.inGarden();m.rewardGranted=reward;m.developmentReferences=development;m.services.canExport=true;m.introductions=owner.pendingIntroductions();if(!placing.empty())m.placement=owner.placementView(placing);views.project(std::move(m));}
    void send(const Intent& i){check(owner.dispatchIntent(i).accepted,"Real intent accepted: "+std::to_string(int(i.verb))+" "+i.object+" "+i.target);reward=owner.lastRewardGranted();if(i.verb==Verb::SelectQuest)ui.enter(UiScreen::Dialogue);else if(i.verb==Verb::RecordBeachEnding)ui.enter(UiScreen::BeachEnding);else if(i.verb==Verb::PlaceFurnishing)ui.enter(UiScreen::Garden);else if(i.verb==Verb::RecordPicnic||i.verb==Verb::ReplayPicnic)ui.enter(UiScreen::Ending);else if(owner.puzzleView().completed&&i.verb!=Verb::Buy)ui.enter(UiScreen::Completion);project();}
    void render(){ui.update(width,height);}
    engine::GuiAccessibleNode node(const std::string& name){render();for(auto n:ui.gui().accessibilityNodes())if(n.name==name&&n.role==engine::GuiRole::Button)return n;throw std::runtime_error("Missing control: "+name);}
    void click(const std::string& name){auto n=node(name);check(ui.gui().focusWidget(n.id),"Control focusable: "+name);check(ui.gui().key(engine::GuiKey::Enter),"Control activated: "+name);render();}
    bool text(const std::string& value){render();for(auto n:ui.gui().accessibilityNodes())if(n.name.find(value)!=std::string::npos)return true;return false;}
    void solve(const rules::Definition& d){
        for(auto& step:d.solution){
            ui.enter(UiScreen::Puzzle);
            auto v=owner.puzzleView();auto at=std::find_if(v.actions.begin(),v.actions.end(),[&](auto& a){auto& i=a.intent;return i.verb==step.verb&&i.object==step.object&&i.target==step.target&&i.direction==step.direction&&(i.verb!=Verb::Fit||i.destination==step.destination);});
            check(at!=v.actions.end(),"Authored command projected");check(at->enabled,"Authored command legal");send(at->intent);
        }check(owner.puzzleView().completed,"Owner request completed");check(ui.screen()==UiScreen::Completion,"Completion surfaced");
    }
};
int main(){try{
    {
        PawsUi ui(fonts(),false,{});gui::Views views(ui,{});gui::Model model;
        rules::Attempt attempt(rules::quest("pepper_garden_bells"));model.puzzle=attempt.view();
        model.progress=Afternoon{}.progressView();model.developmentReferences=false;
        for(const auto size:std::array<engine::Vec2,3>{{{2187,964},{390,844},{844,390}}}){
            auto render=[&]{views.project(model);ui.enter(UiScreen::Puzzle);ui.update(size.x,size.y);return views.boardBounds();};
            model.puzzle.sequenceWatching=true;model.services.feedback="Watch the blackboard again. Your answer starts fresh.";const auto watching=render();
            model.puzzle.sequenceWatching=false;model.services.feedback="Your turn. Repeat the pictures you watched.";const auto answering=render();
            model.services.feedback="Different picture. Your answer is cleared. Try again or replay the blackboard.";const auto error=render();
            model.puzzle.enteredSymbols={"leaf"};model.services.feedback="Picture matched. Keep playing the remembered order.";const auto matched=render();
            model.services.save=SaveState::Saving;const auto saving=render();model.services.save=SaveState::Saved;const auto saved=render();
            auto same=[&](const engine::GuiRect& r){return r.x==watching.x&&r.y==watching.y&&r.width==watching.width&&r.height==watching.height;};
            check(same(answering)&&same(error)&&same(matched)&&same(saving)&&same(saved),"Picture sequence camera frame stays fixed across playback, error, correct taps and saving");
            model.puzzle.enteredSymbols.clear();
        }
    }
    // The garden retains a current request. Its menu must allow the host to
    // prepare a fresh puzzle scene after confirmation, rather than disabling reset.
    {
        Afternoon owner;
        check(owner.returnToGarden(owner.intent(Verb::Select)).accepted,"Restart fixture enters garden");
        check(owner.setUserPaused(true,owner.intent(Verb::Select)).accepted,"Garden menu pauses owner");
        const auto saved=owner.encode();
        std::vector<UiIntent> intents;
        PawsUi ui(fonts(),false,[&](const UiIntent& i){if(i.action==UiAction::Restart)intents.push_back(i);});
        gui::Views views(ui,{});
        gui::Model model;model.puzzle=owner.puzzleView();model.progress=owner.progressView();model.services.inGarden=true;
        views.project(model);ui.enter(UiScreen::Pause);ui.update(390,844);
        auto activate=[&](const char* name){for(auto n:ui.gui().accessibilityNodes())if(n.name==name&&n.role==engine::GuiRole::Button){check(!n.disabled,"Garden restart control enabled");check(ui.gui().activateWidget(n.id),"Garden restart control activated");ui.update(390,844);return;}throw std::runtime_error("Missing garden restart control");};
        activate("Restart puzzle");check(ui.screen()==UiScreen::Restart,"Garden restart opens confirmation");
        activate("Keep playing");check(intents.empty()&&owner.encode()==saved,"Cancel retains the current request and garden");
        activate("Restart puzzle");activate("Restart puzzle");
        check(intents.size()==1&&intents.front().attempt==model.progress.attempt&&intents.front().revision==model.progress.revision,"Confirmed garden restart reaches the host with current identity");
    }
    Driver d;
    d.project(false);d.ui.enter(UiScreen::Puzzle);check(!d.text("Puzzle board."),"Production composition excludes the numbered development board");d.project();
    for(auto& definition:rules::catalogue()){
        if(!progression::quests[progression::questIndex(definition.quest)].pack.empty())continue;
        if(!progression::quests[progression::questIndex(definition.quest)].pack.empty())continue;
        d.ui.enter(UiScreen::Dialogue);d.click(definition.title+" · Available");d.click("Help "+std::string(gui::dialogue()[progression::questIndex(definition.quest)].speaker));
        auto stale=d.owner.intent(Verb::Hint);
        d.click("Menu");check(!d.text("Puzzle controls"),"Quest action menu removed");d.click("Hint");d.click("Next hint");d.click("Next hint");check(d.owner.puzzleView().revealedHints==3,"Three real free hints");check(!d.owner.dispatchIntent(stale).accepted,"Stale GUI intent rejected");d.click("Back");
        if(definition.family==rules::Family::PictureSequence){d.ui.enter(UiScreen::Puzzle);check(!d.text("leaf, star, leaf, paw, star")&&d.text("blackboard"),"Blackboard memory clue must not expose the complete answer");}
        d.solve(definition);check(d.text("20 coins")&&!d.text("Thank you!"),"Compact reward receipt keeps the scene visible");d.click("Back to garden");
        if(!d.owner.pendingIntroductions().empty()){
            const auto id=d.owner.pendingIntroductions().front();
            const auto cats=d.owner.progressView().cats;
            const auto cat=std::find_if(cats.begin(),cats.end(),[&](const auto& c){return c.id==id;});
            check(!d.text("New friends have arrived")&&!d.text("Meet "+cat->name),"Chapter transition has no arrival panel or Meet button");
            check(d.text("Tap a cat with !"),"Garden directs the player to world quest markers");
            const auto saved=d.owner.encode();d.owner=Afternoon::decode(saved);d.project();
            check(d.ui.screen()==UiScreen::Garden&&!d.text("Meet "+cat->name),"Restored pending arrivals keep the garden visible");
            // The browser commits this same owner intent after physical arrival.
            while(!d.owner.pendingIntroductions().empty()){auto next=d.owner.pendingIntroductions().front();check(d.owner.introduce(next,d.owner.intent(Verb::Select)).accepted,"Arrival recorded");}d.project();
            check(d.text("Tap a cat with !"),"After introductions garden directs the player to visible request cats");
        }
    }
    check(d.owner.coins()==360,"Eighteen once-only rewards");
    {
        Driver beach;beach.owner.setBeachDevelopmentAccess(true);beach.project();beach.ui.enter(UiScreen::Garden);
        beach.click("Adventures");check(beach.text("Development preview"),"Adventure entry distinguishes preview from purchased ownership");
        beach.click("Start / Continue beach");beach.click("Help Pip");
        beach.solve(rules::quest(progression::beachQuestId));
        check(beach.text("Beach quest complete")&&!beach.text("20 coins"),"Beach has a separate completion receipt");
        beach.click("Back to garden");check(beach.owner.inGarden()&&beach.owner.active()==0,"Beach GUI returns to base request");
        beach.click("Adventures");check(beach.text("Revisit"),"Adventure preview retains completion");beach.click("Pip's runaway beach ball · Revisit");
        check(beach.owner.puzzleView().completed&&beach.owner.coins()==0,"Revisit cannot reset completed board or grant coins");
        beach.ui.enter(UiScreen::Completion);
        for(std::size_t stage=1;stage<6;++stage){
            beach.click("Continue beach story");beach.click("Help "+std::string(gui::dialogue()[progression::baseQuestCount+stage].speaker));
            beach.solve(rules::quest(progression::beachQuestIds[stage]));
        }
        beach.click("Watch beach ending");check(beach.owner.beachEndingSeen()&&beach.text("postcard"),"Beach ending commits through the retained GUI");
        beach.click("Back to garden");beach.click("Adventures");check(beach.text("Postcard saved"),"Saved ending appears in Adventures");

    }
    d.ui.enter(UiScreen::Shop);d.click("Garden cushion · 30 coins");d.click("Buy");check(d.owner.coins()==330,"Actual purchase");
    d.click("Choose a spot");d.click("bench corner · Fits");d.click("Cancel");check(d.owner.placement("garden_cushion").empty(),"Cancelled placement unchanged");
    check(d.cancellations==1,"Cancel notifies the existing host to release its prepared placement preview");
    d.click("Choose a spot");d.click("bench corner · Fits");d.click("Place");check(d.owner.placement("garden_cushion")=="bench_corner","Actual placement");
    d.ui.enter(UiScreen::Pause);d.click("Picnic celebration");check(d.owner.picnicSeen(),"Distinct release-one ending");check(d.text("Pepper: And tomorrow"),"Four-cat closing");
    bool firstLineVisible=false;for(const auto& n:d.ui.gui().accessibilityNodes())if(n.name.starts_with("Pip: A little"))firstLineVisible=n.bounds.y>=0&&n.bounds.y+n.bounds.height<d.height;
    check(firstLineVisible,"Closing opens on the first cat line; fixed footer focus must not scroll to the last line");
    d.click("Continue in garden");d.ui.enter(UiScreen::Pause);d.click("Replay picnic celebration");check(d.owner.coins()==330,"Replay no reward");
    {
        Driver replayMenu;replayMenu.owner=Afternoon::decode(d.owner.encode());replayMenu.project();replayMenu.ui.enter(UiScreen::Dialogue);
        replayMenu.click("The garden bells · Practice");replayMenu.click("Help Pepper");replayMenu.ui.enter(UiScreen::Pause);
        check(!replayMenu.node("Replay picnic celebration").disabled,"Recorded picnic celebration can be replayed from a puzzle menu");
        check(!replayMenu.node("New Game").disabled,"New Game is available independently of picnic completion");
    }
    d.click("Continue in garden");d.ui.enter(UiScreen::Dialogue);d.click("The garden bells · Practice");d.click("Help Pepper");d.ui.enter(UiScreen::Restart);d.click("Restart puzzle");d.solve(rules::quest("pepper_garden_bells"));check(d.text("practice round"),"Truthful practice reward");check(d.owner.coins()==330,"Practice no reward");
    const auto bytes=d.owner.encode();auto restored=Afternoon::decode(bytes);check(restored.coins()==330&&restored.picnicSeen(),"Real save codec retains GUI progress");
    // Reproduce the shop placement flow for every furnishing, including the
    // scratching post's single spot and the bowl beside an occupied cushion.
    Driver placement;placement.owner=Afternoon::decode(bytes);
    check(placement.owner.returnToGarden(placement.owner.intent(Verb::Select)).accepted,"Placement regression returns to the garden");
    for(const auto& item:progression::items)placement.owner.setItemAvailable(item.id,true);
    placement.project();
    for(const auto& item:progression::items){
        placement.ui.enter(UiScreen::Shop);
        const bool owned=placement.owner.owned(item.id);
        placement.click(std::string(item.name)+" · "+(owned?"Owned":std::to_string(item.price)+" coins"));
        if(!owned)placement.click("Buy");
        const auto coins=placement.owner.coins();const auto previous=placement.owner.placement(item.id);
        placement.click("Choose a spot");
        check(!placement.node("Place").disabled,"A clear suitable spot is selected on opening: "+std::string(item.id));
        check(placement.owner.placement(item.id)==previous&&placement.owner.coins()==coins,"Opening a preview preserves committed placement and coins");
        for(const auto& spot:progression::spots){
            const bool suitable=progression::compatible(item,spot);
            std::string name(spot.id);std::replace(name.begin(),name.end(),'_',' ');
            check(placement.text(name+" · ")==suitable,"Only destinations suited to this furnishing are listed");
        }
        if(item.id=="drinking_bowl")check(placement.text("Another furnishing occupies this spot."),"Suitable occupied spots retain the actual blockage");
        if(item.id=="scratching_post"){
            check(placement.text("Selected spot: scratch corner"),"The scratching post's sole legal spot is visibly selected");
            for(auto [w,h]:{std::pair{585.f,791.f},std::pair{390.f,844.f},std::pair{844.f,390.f}}){
                placement.width=w;placement.height=h;placement.render();
                const auto spot=placement.node("scratch corner · Fits");
                check(spot.bounds.y>=0&&spot.bounds.y+spot.bounds.height<=placement.node("Place").bounds.y,"Scratch corner stays above the fixed footer");
                auto centre=engine::Vec2{spot.bounds.x+spot.bounds.width/2,spot.bounds.y+spot.bounds.height/2};
                placement.ui.gui().pointer(engine::GuiPointer::Down,centre);placement.ui.gui().pointer(engine::GuiPointer::Up,centre);placement.render();
                check(!placement.node("Place").disabled,"Pointer selection enables placement");
            }
        }
        placement.click("Cancel");check(placement.owner.placement(item.id)==previous&&placement.owner.coins()==coins,"Cancelling the initial preview preserves placement and coins");
        placement.click("Choose a spot");placement.click("Place");
        check(!placement.owner.placement(item.id).empty()&&placement.owner.coins()==coins,"Explicit Place commits a free valid placement");
    }
    check(placement.owner.placement("scratching_post")=="scratch_corner","Purchased scratching post is placed at its authored destination");
    const auto placementSave=Afternoon::decode(placement.owner.encode());check(placementSave.placement("scratching_post")=="scratch_corner","Scratching post placement survives save restoration");
    d.ui.enter(UiScreen::Puzzle);d.click("Menu");d.click("Settings");d.ui.escape();d.render();check(d.ui.gui().focused()&&d.ui.gui().focused()->id()==d.node("Settings").id,"Nested focus return to Settings");d.ui.escape();d.render();check(d.ui.gui().focused()&&d.ui.gui().focused()->id()==d.node("Menu").id,"Nested focus return to Menu");
    for(auto [w,h]:{std::pair{1440.f,900.f},std::pair{390.f,844.f},std::pair{844.f,390.f}})for(bool large:{false,true})for(auto s:{UiScreen::Garden,UiScreen::Puzzle,UiScreen::Dialogue,UiScreen::Objects,UiScreen::Hints,UiScreen::Pause,UiScreen::Settings,UiScreen::Recovery,UiScreen::Shop,UiScreen::Details,UiScreen::Placement,UiScreen::Completion,UiScreen::Ending}){
        d.width=w;d.height=h;auto settings=d.ui.settings();settings.largerText=large;d.ui.restoreSettings(settings);d.ui.enter(s);d.render();
        std::vector<std::string> buttons;for(auto n:d.ui.gui().accessibilityNodes())if(n.role==engine::GuiRole::Button&&!n.disabled)buttons.push_back(n.name);
        for(auto& name:buttons){auto n=d.node(name);check(n.bounds.width>=44&&n.bounds.height>=44,"Touch target "+name);check(d.ui.gui().focusWidget(n.id),"Focus target "+name);d.render();n=d.node(name);check(n.bounds.x>=-1&&n.bounds.y>=-1&&n.bounds.x+n.bounds.width<=w+1&&n.bounds.y+n.bounds.height<=h+1,"Focused target on display "+name);}
    }
    std::cout<<"Release-one GUI: "<<assertions<<" assertions passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
