#include "paws_ui.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace paws;
void check(bool ok,const char* message) {if(!ok) throw std::runtime_error(message);}
std::array<std::vector<std::byte>,3> fonts() {
    std::array<std::vector<std::byte>,3> result;
    const char* names[]={"NunitoSans-Regular.ttf","NunitoSans-Semibold.ttf","Fraunces-Semibold.ttf"};
    for(int i=0;i<3;++i) {
        std::ifstream f(std::string(PAWS_SOURCE_DIR)+"/assets/runtime/fonts/"+names[i],std::ios::binary|std::ios::ate);
        check(bool(f),"font absent");result[i].resize(std::size_t(f.tellg()));f.seekg(0);f.read(reinterpret_cast<char*>(result[i].data()),result[i].size());
    }
    return result;
}
std::uint64_t find(PawsUi& ui,const std::string& label) {
    for(auto& n:ui.gui().accessibilityNodes()) if(n.name==label&&n.role==engine::GuiRole::Button) return n.id;
    throw std::runtime_error("missing button: "+label);
}
int main() {try {
    std::vector<UiIntent> intents;PawsUi ui(fonts(),false,[&](auto i){intents.push_back(i);});ui.update(390,844);
    auto continueId=find(ui,"Continue");check(!ui.gui().activateWidget(continueId),"no fabricated continue");
    check(!ui.gui().activateWidget(find(ui,"Play")),"startup waits for storage preparation");
    UiModel ready;ready.puzzleReady=true;ui.project(ready);ui.update(390,844);
    ui.gui().activateWidget(find(ui,"Play"));check(intents.back().action==UiAction::Play,"play requests owner preparation");
    check(ui.screen()==UiScreen::Title,"owner publishes prepared scene before navigation");
    ui.enter(UiScreen::Garden);ui.update(390,844);check(!ui.blocksScene(),"garden is interactive");
    bool hasStatus=false;for(auto& n:ui.gui().accessibilityNodes())if(n.name.find("0 coins")!=std::string::npos&&n.name.find("Temporary")!=std::string::npos)hasStatus=true;
    check(hasStatus,"narrow HUD retains wallet and truthful save status");
    auto menu=find(ui,"Menu");ui.gui().focusWidget(menu);ui.gui().activateWidget(menu);ui.update(390,844);
    check(ui.needsPause(),"modal pauses scene");check(!ui.gui().activateWidget(menu),"stale modal underlying id rejected");
    ui.gui().activateWidget(find(ui,"Settings"));ui.update(390,844);
    ui.gui().activateWidget(find(ui,"Text size: Standard"));ui.update(390,844);
    check(ui.settings().largerText&&ui.gui().scale()==1.25f,"text scales layout and input");
    ui.gui().activateWidget(find(ui,"Quality: Automatic"));ui.update(390,844);
    check(ui.settings().quality==1&&intents.back().target=="quality","display quality is an owner command");
    ui.gui().activateWidget(find(ui,"Quality: Mobile"));ui.update(390,844);
    check(ui.settings().quality==2,"desktop display choice is accessible");
    for(auto& n:ui.gui().accessibilityNodes()) if(n.role==engine::GuiRole::Button) check(n.bounds.height>=44,"touch target");
    ui.escape();ui.update(390,844);check(ui.screen()==UiScreen::Pause,"nested modal returns to pause");
    check(ui.gui().focused()&&ui.gui().focused()->id()==find(ui,"Settings"),"nested focus returned");
    ui.escape();ui.update(390,844);check(ui.screen()==UiScreen::Garden,"modal returns garden");
    check(ui.gui().focused()&&ui.gui().focused()->id()==find(ui,"Menu"),"initiator focus restored");
    ui.show(UiScreen::Restart);ui.update(390,844);
    check(ui.gui().focused()&&ui.gui().focused()->id()==find(ui,"Keep playing"),"restart focuses the non-destructive action");
    ui.escape();ui.update(390,844);
    UiModel model;model.puzzleReady=true;model.attempt=47;model.revision=12;model.objects={{"curve","Curved channel","Rotates"}};
    model.actions={{UiAction::Rotate,"Rotate","curve","",true},{UiAction::Move,"Move left","pot","Occupied",false}};
    ui.project(model);ui.show(UiScreen::Objects);ui.update(1440,900);
    ui.gui().activateWidget(find(ui,"Curved channel"));check(intents.back().attempt==47&&intents.back().revision==12&&intents.back().target=="curve","command identity");
    for(const auto& node:ui.gui().accessibilityNodes())check(node.name!="Rotate"&&node.name!="Move left","movement buttons hidden for direct controls");
    for(auto size:{engine::Vec2{1440,900},engine::Vec2{390,844},engine::Vec2{844,390}}) {ui.update(size.x,size.y);ui.gui().draw(1);}
    model.inGarden=true;model.shopOpen=true;model.cushionOwned=true;model.placementValid=true;
    model.placementSpot="quiet_corner";model.placementReason="Fits here.";
    model.spots={{"sunny_corner","Sunny corner","",false},{"quiet_corner","Quiet nook","",true}};
    ui.project(model);ui.enter(UiScreen::Shop);ui.update(390,844);
    ui.gui().activateWidget(find(ui,"Garden cushion · Owned"));ui.update(390,844);
    ui.gui().activateWidget(find(ui,"Choose a spot"));check(intents.back().action==UiAction::BeginPlacement&&intents.back().target=="garden_cushion","preview begins with stable item identity");
    ui.update(390,844);ui.gui().activateWidget(find(ui,"Sunny corner"));
    check(intents.back().action==UiAction::SelectSpot&&intents.back().target=="sunny_corner","display name maps to stable spot identity");
    ui.gui().activateWidget(find(ui,"Place"));check(intents.back().action==UiAction::PlaceFurnishing&&intents.back().target=="garden_cushion/quiet_corner"&&intents.back().revision==12,"place retains displayed target and command revision");
    for(auto size:{engine::Vec2{390,844},engine::Vec2{844,390}}) {
        ui.update(size.x,size.y);
        for(auto& n:ui.gui().accessibilityNodes())if(n.name=="Place"||n.name=="Cancel")
            check(n.bounds.height>=44&&n.bounds.y>=0&&n.bounds.y+n.bounds.height<=size.y,"placement confirmation stays within narrow viewport");
    }
    ui.escape();check(intents.back().action==UiAction::CancelPlacement&&ui.screen()==UiScreen::Details,"escape explicitly discards placement preview");
    PawsUi gallery(fonts(),true,[&](auto i){intents.push_back(i);});gallery.update(390,844);
    auto count=intents.size();gallery.gui().activateWidget(find(gallery,"Shop / details"));gallery.update(390,844);
    gallery.gui().activateWidget(find(gallery,"Garden cushion · 30 coins"));gallery.update(390,844);
    gallery.gui().activateWidget(find(gallery,"Buy"));check(intents.size()==count,"gallery must never dispatch purchases");
    std::cout<<"Paws UI: semantic commands, modal ownership/focus, scale, disabled actions, gallery isolation passed\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}}

