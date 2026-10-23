#pragma once
#ifdef PAWS_RELEASE1_GUI_PRODUCTION
#include "paws_ui.hpp"
#else
#include "runtime/paws_ui.hpp"
#endif
#include "release1/contracts.hpp"
#include <functional>

namespace paws::release1::gui {
struct QuestCopy { std::string_view quest,speaker,request,completion; };
const std::array<QuestCopy,29>& dialogue();
std::string catIntroduction(std::string_view);
struct Model {
    PuzzleView puzzle;
    ProgressView progress;
    UiModel services; // Existing recovery/loading/audio projection; never a save owner.
    std::vector<SpotView> placement;
    std::vector<std::string> introductions;
    std::map<std::string,std::shared_ptr<const engine::Texture>> portraits,thumbnails;
    std::map<std::string,std::string> useReasons;
    std::string finaleReason,workshopSelection;
    bool developmentReferences=false,rewardGranted=false,lookoutQuiet=false,nookQuiet=false;
};
// Extends the existing skins, fonts, focus stack, settings and service screens.
// Views owns copied projections and presentation selection only. Owner outlives callbacks.
class Views {
public:
    Views(PawsUi&,std::function<void(const Intent&)>,
        std::function<void(std::string_view,const Intent&)> introduce={},
        std::function<void(std::string_view)> placementRequested={},
        std::function<void(std::string_view,std::string_view)> previewRequested={});
    ~Views();
    Views(const Views&)=delete;
    Views& operator=(const Views&)=delete;
    void project(Model);
    void selectObject(std::string_view id);
    std::string_view item() const { return item_; }
    engine::GuiRect boardBounds() const { return board_; }
private:
    PawsUi& ui_;
    Model model_;
    std::function<void(const Intent&)> dispatch_;
    std::function<void(std::string_view,const Intent&)> introduce_;
    std::function<void(std::string_view)> placementRequested_;
    std::function<void(std::string_view,std::string_view)> previewRequested_;
    std::string selected_,item_="garden_cushion",spot_;
    std::string balanceSupportChange_,balancePipBefore_,balancePipAfter_,balanceStorage_;
    unsigned completedSetup_=0;
    engine::GuiRect board_;
    bool compose();
    Intent command(Verb,std::string object={},std::string target={}) const;
    void send(const Intent&);
    void beginPlacement(std::string item);
    void clues(engine::Widget&);
    void board(float,float,float,float);
    void portrait(engine::Widget&,std::string_view);
};
}
