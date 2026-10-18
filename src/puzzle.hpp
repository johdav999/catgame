#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace paws {
using ObjectId = std::string;
using AttemptId = std::uint32_t;
using Revision = std::uint32_t;
struct Cell { int x=0, y=0; bool operator==(const Cell&) const = default; };
enum class Direction { North, East, South, West };
enum class Kind { Pot, Straight, Curve, Cushion, Toy, Socket, Switch, Gate, Crate, Catch };
enum class Shape { None, Ball, Mouse, Fish };
enum class Goal { Route, Tidy, Retrieve, Relay };
enum class Location { Ground, Held, Socket, Collected };
bool sliding(Kind kind);
bool channel(Kind kind);
struct ObjectDefinition {
    ObjectId id;
    std::string name;
    Kind kind;
    Cell initial;
    int orientation=0;
    std::vector<Cell> slots; // Pots only; channel positions are fixed.
    Shape shape=Shape::None;
    ObjectId link; // Gate -> switch; collectible -> required gate.
    std::vector<Cell> gatedSlots; // Sliding destinations available only while link's gate is open.
    std::vector<Cell> approaches; // A toy also needs access to one of these pickup positions.
};
enum class Action { Select, Move, Rotate, Roll, Undo, Restart, Hint, Pause, Garden, Resume, PickUp, Place, ReturnBall };
struct SolutionStep { Action action; ObjectId object; Direction direction=Direction::North; ObjectId target; };
struct PuzzleDefinition {
    int schemaVersion=1;
    std::string contentId, questId;
    int width=0, height=0;
    Cell source, destination;
    Direction sourceExit=Direction::East, destinationEntry=Direction::West;
    std::vector<ObjectDefinition> objects;
    std::vector<SolutionStep> solution;
    std::vector<std::string> hints;
    ObjectId sourceId="source_ball", destinationId="pip_destination";
    Goal goal=Goal::Route;
    std::string title="Pip's missing ball", objective="Bring the ball back to Pip.",
        dialogue="My ball rolled behind the pots. Can you help me bring it back?";
    std::vector<Cell> walkable;
    Cell access;
    ObjectId requestedItem, switchRole, gateRole;
    bool packedToysBlockAccess=false;
    bool keepBallOnObstacleMove=false;
    Cell checkpoint;
    Direction checkpointEntry=Direction::West,checkpointExit=Direction::East;
};
struct ObjectState { Cell cell; int orientation=0; Location location=Location::Ground; ObjectId socket; bool operator==(const ObjectState&) const = default; };
struct PuzzleState {
    std::vector<ObjectState> objects;
    bool completed=false;
    bool ballOnSwitch=false;
    bool checkpointReached=false;
    bool operator==(const PuzzleState&) const = default;
};
struct RouteResult { bool reached=false; std::string message; std::vector<Cell> path; };
struct GameState {
    std::string questId;
    AttemptId attempt=1;
    Revision revision=0;
    PuzzleState puzzle;
    ObjectId selected;
    unsigned hint=0;
    bool paused=false, inGarden=false;
};
struct Command {
    AttemptId attempt;
    Revision revision;
    Action action;
    ObjectId object;
    Direction direction=Direction::North;
    ObjectId target;
};
struct CommandResult { bool accepted=false; std::string message; };
Cell adjacent(Cell cell, Direction direction);
std::vector<Direction> ports(Kind kind, int orientation);
PuzzleState initialState(const PuzzleDefinition& content);
std::string validateContent(const PuzzleDefinition& content, bool replaySolution=true);
// Finite traversal tracks (cell, incoming port), including adversarial cyclic graphs.
RouteResult traceRoute(const PuzzleDefinition&, const PuzzleState&, Cell start, Direction entry);
RouteResult evaluateRoute(const PuzzleDefinition&, const PuzzleState&);
std::string moveBlockage(const PuzzleDefinition&, const PuzzleState&, std::size_t object, Direction);
struct SlideRoute { Cell cell; std::vector<Direction> moves; };
// All reachable parking spaces, including the current cell. Shared by the
// drag preflight and its presentation so an outline always means a legal drop.
std::vector<SlideRoute> slidingRoutes(const PuzzleDefinition&,const PuzzleState&,std::size_t object);
std::string slidingAdvice(const PuzzleDefinition&,const PuzzleState&,std::size_t object);
bool switchActive(const PuzzleDefinition&, const PuzzleState&, const ObjectId&);
bool gateOpen(const PuzzleDefinition&, const PuzzleState&, const ObjectId&);
bool reachable(const PuzzleDefinition&, const PuzzleState&, Cell);
std::string pickupBlockage(const PuzzleDefinition&, const PuzzleState&, std::size_t);
std::string placeBlockage(const PuzzleDefinition&, const PuzzleState&, std::size_t, const ObjectId&);
class PawsSession {
public:
    explicit PawsSession(PuzzleDefinition definition);
    const PuzzleDefinition& content() const { return content_; }
    const GameState& state() const { return state_; }
    bool canUndo() const { return !undo_.empty(); }
    CommandResult dispatch(const Command& command);
    void invalidateCommands(AttemptId attempt) { state_.attempt=attempt; ++state_.revision; }
private:
    friend class Afternoon; // Validated state codec restores portable command identities.
    PuzzleDefinition content_;
    GameState state_;
    std::vector<PuzzleState> undo_;
};
PuzzleDefinition missingBall(int version=2);
PuzzleDefinition tidyToys(int version=2);
PuzzleDefinition stuckGate(int version=2);
}
