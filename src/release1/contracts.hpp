#pragma once
#include "puzzle.hpp"
#include <array>
#include <string>
#include <string_view>
#include <vector>

// Frozen cross-track DTOs. No state owner, browser types or engine handles.
// Implementations live in task namespaces; production registration is Prompt 9.
namespace paws::release1 {
inline constexpr std::string_view contractId="paws-release1/1";
enum class Verb { Select, Slide, Rotate, Roll, PickUp, Fit, Connect, Retrieve,
    TapSymbol, AssignCat, Undo, Restart, Hint, SelectQuest, Buy, PlaceFurnishing,
    RecordPicnic, ReplayPicnic, ReplaySequence, Inspect, FollowTrail, TestBalance, RecordBeachEnding };
struct Intent {
    std::uint64_t generation=0;
    AttemptId attempt=0;
    Revision revision=0;
    std::string quest,object,target;
    Verb verb=Verb::Select;
    Direction direction=Direction::North;
    Cell destination;
    int orientation=0;
};
struct LegalAction { Intent intent; std::string label,reason; bool enabled=false; };
struct ObjectView {
    std::string id,name,role;
    Cell cell;
    int orientation=0;
    std::vector<Cell> footprint;
    std::string socket;
    bool held=false,active=false,collected=false;
};
struct PuzzleView {
    std::string quest,content,title,objective,feedback;
    std::uint64_t generation=0;
    AttemptId attempt=0;
    Revision revision=0;
    std::vector<ObjectView> objects;
    std::vector<LegalAction> actions;
    std::array<std::string,3> hints;
    unsigned revealedHints=0;
    std::vector<std::string> symbols,enteredSymbols;
    bool pictureSequence=false,sequenceWatching=false;
    unsigned sequenceReplay=0,sequenceIndex=0;
    unsigned balanceSetup=0,balanceSetups=0; // One-based active setup; zero for other families.
    std::vector<Cell> route;
    bool completed=false,canUndo=false;
    unsigned beachProgress=0;
};
struct CatView { std::string id,name; int friendship=0,milestone=0; bool introduced=false; };
struct QuestView { std::string id,title; int chapter=1; bool unlocked=false,completed=false,rewarded=false; };
struct ItemView {
    std::string id,name,asset,spot,reason;
    int price=0;
    bool unlocked=false,owned=false,available=false;
};
struct SpotView { std::string id,role,occupant,reason; bool compatible=false; };
struct ProgressView {
    std::uint64_t generation=0;
    AttemptId attempt=0;
    Revision revision=0;
    int coins=0,chapter=1;
    std::vector<CatView> cats;
    std::vector<QuestView> quests;
    std::vector<ItemView> items;
    std::vector<SpotView> spots;
    bool mvpEndingSeen=false,picnicEndingSeen=false,picnicEligible=false;
    bool beachDevelopmentAccess=false;
    bool beachComplete=false,beachEndingSeen=false;
};
// Transient event consumed after an accepted commit. It cannot grant progress.
struct CommittedEvent {
    std::uint64_t generation=0;
    AttemptId attempt=0;
    Revision revision=0;
    std::string quest,object,cue;
};
}
