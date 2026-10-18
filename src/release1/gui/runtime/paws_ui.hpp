#pragma once
#include "engine/gui.hpp"
#include <array>
#include <functional>
#include <map>

namespace paws {
namespace release1::gui { class Views; }
enum class UiScreen { Title, Garden, Puzzle, Dialogue, Hints, Pause, Settings, Objects,
    Restart, Completion, Shop, Details, Placement, Ending, Recovery, Gallery, Loading, LoadFailure, ImportConfirm };
enum class UiAction { Play, Continue, AcceptQuest, SelectObject, Move, Rotate, Roll,
    PickUp, Place, Undo, Restart, BackToGarden, Buy, CancelPlacement,
    RetrySave, ExportSave, ImportSave, ReplayEnding, ResetView, ZoomIn, ZoomOut, Settings, SelectQuest, ReturnBall, RevealHint, RetryLoad,
    OpenShop, BeginPlacement, SelectSpot, PlaceFurnishing, UseFurnishing, ConfirmImport, CancelImport, ExportOriginal, EnableSound, ConfirmSound };
struct UiIntent {
    UiAction action;
    std::string target;
    std::uint64_t attempt=0, revision=0;
};
struct UiObject { std::string id, name, description; };
struct UiLegalAction { UiAction action; std::string label, target, reason; bool enabled=true; };
enum class SaveState { Temporary, Saving, Saved, Failed, SecondTab, Loading };
struct UiModel {
    struct Quest { std::string id,title; bool unlocked=false,completed=false; };
    std::vector<Quest> quests;
    std::string questTitle="Pip's missing ball", dialogue="My ball rolled behind the pots. Can you help me bring it back?";
    std::string shortTitle="Pip's ball";
    std::uint64_t attempt=0, revision=0;
    bool canContinue=false, puzzleReady=false, shopOpen=false, canUndo=false;
    bool canRestart=false, storageAvailable=false, canExport=false;
    bool canExportOriginal=false;
    std::string storageNotice;
    bool rewardGranted=false, rewardsReady=false, placementValid=false, cushionOwned=false, bowlOwned=false;
    bool inGarden=false,endingSeen=false;
    std::string audioStatus="Final sound files are not installed. You can play quietly.";
    bool audioActive=false;
    std::string cushionSpot,bowlSpot,placementSpot,catUse;
    struct Spot { std::string id,name,reason; bool selected=false; };
    std::vector<Spot> spots;
    int coins=0, friendship=0;
    int rewardCoins=0, rewardFriendship=0;
    unsigned revealedHints=0;
    std::string objective="Bring the ball back to Pip.", selectedId, selectedName="Choose an object";
    std::string feedback, placementReason="Choose a garden spot.", itemId="garden_cushion";
    SaveState save=SaveState::Temporary;
    std::vector<UiObject> objects;
    std::vector<UiLegalAction> actions;
    std::array<std::string,3> hints{"Look at the route from the ball to Pip.",
        "The pots need room to slide away from the channels.",
        "Clear both pots, then turn the curved channel toward Pip."};
    // Supplied by accepted phase 3/4 assets. Never substitutes a generated cat identity.
    std::shared_ptr<const engine::Texture> pipPortrait, cushionPreview, bowlPreview, placementPreview;
};
struct UiSettings { bool largerText=false, reducedMotion=false, muted=false; int music=70, ambience=60, effects=80; int quality=0; };

// Game composition over retained engine widgets. The model is a read-only projection;
// every domain action carries the model's attempt/revision back to its owner.
class PawsUi {
public:
    PawsUi(std::array<std::vector<std::byte>,3> fonts, bool gallery,
        std::function<void(const UiIntent&)> dispatch);
    engine::GuiSystem& gui() { return *gui_; }
    void project(UiModel model);
    void show(UiScreen screen);
    void enter(UiScreen screen); // Domain navigation discards obsolete modal return paths.
    void update(float width, float height);
    bool willRebuild(float width,float height) const { return dirty_||width!=width_||height!=height_; }
    void pointer(engine::Vec2 position, bool down);
    void escape();
    void setReducedMotion(bool value) { settings_.reducedMotion=value;dirty_=true; }
    void restoreSettings(UiSettings settings) { settings_=settings;dirty_=true; }
    bool blocksScene() const { return screen_!=UiScreen::Garden && screen_!=UiScreen::Puzzle; }
    bool needsPause() const { return blocksScene(); }
    const UiSettings& settings() const { return settings_; }
    UiScreen screen() const { return screen_; }
private:
    friend class release1::gui::Views;
    std::function<bool()> composition_;
    struct Skin { engine::Button* button; engine::Image* image; engine::Image* text;
        std::string label, icon; bool primary; int state=-1; float width=0, height=52; };
    std::unique_ptr<engine::GuiSystem> gui_;
    std::array<std::vector<std::byte>,3> fontBytes_;
    std::shared_ptr<engine::GuiFont> heading_, wordmark_, control_;
    std::function<void(const UiIntent&)> dispatch_;
    UiModel model_;
    UiSettings settings_;
    UiScreen screen_=UiScreen::Title;
    bool gallery_=false, preview_=false, dirty_=true, down_=false;
    float width_=0,height_=0,contentWidth_=0;
    int hintStep_=1, recoveryExample_=0;
    std::string pendingFocus_, item_="garden_cushion";
    std::vector<std::pair<UiScreen,std::string>> history_;
    engine::Vec2 pointer_{-1,-1};
    std::vector<Skin> skins_;
    std::map<std::string,std::shared_ptr<const engine::Texture>> art_;
    void build();
    void emit(UiAction action, std::string target={});
    void close();
    engine::Panel& surface(float x,float y,float w,float h,bool scroll=true);
    engine::TextLabel& copy(engine::Widget& parent,std::string text,bool error=false);
    void heading(engine::Widget& parent,const std::string& text,bool wordmark=false);
    engine::Button& button(engine::Widget& parent,const std::string& label,
        std::function<void()> action,bool primary=false,bool enabled=true,std::string icon={},std::string sound="ui_confirm");
    void previewImage(engine::Widget& parent,std::shared_ptr<const engine::Texture> image,const std::string& name);
    std::shared_ptr<const engine::Texture> textArt(const std::string&,engine::GuiFont&,bool light=false);
    std::shared_ptr<const engine::Texture> tile(int width,int height,int state,bool primary,const std::string& icon={});
    void setFonts();
};
}
