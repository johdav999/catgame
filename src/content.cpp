#include "puzzle.hpp"
namespace paws {
PuzzleDefinition missingBall() {
    using D=Direction; using A=Action; using K=Kind;
    return {1,"missing-ball-v1","pip_missing_ball",5,3,{0,2},{4,0},D::East,D::West,{
        {"pot_a","Left pot",K::Pot,{1,2},0,{{1,2},{1,1},{1,0}}},
        {"pot_b","Middle pot",K::Pot,{2,1},0,{{2,1},{1,1},{0,1}}},
        {"channel_a","Source straight",K::Straight,{1,2},0,{}},
        {"channel_b","Lower bend",K::Curve,{2,2},0,{}},
        {"channel_c","Middle straight",K::Straight,{2,1},1,{}},
        {"channel_d","Upper bend",K::Curve,{2,0},0,{}},
        {"channel_e","Finish straight",K::Straight,{3,0},0,{}}
    },{
        {A::Move,"pot_a",D::North},{A::Move,"pot_a",D::North},{A::Move,"pot_b",D::West},
        {A::Rotate,"channel_a"},{A::Rotate,"channel_b"},{A::Rotate,"channel_b"},{A::Rotate,"channel_b"},
        {A::Rotate,"channel_c"},{A::Rotate,"channel_d"},{A::Rotate,"channel_e"},{A::Roll,""}
    },{
        "First slide both pots off the channels. They share a sliding slot, so order matters.",
        "Move the left pot north twice, then the middle pot west. Connect the source east, then north, then east to Pip.",
        "From a fresh attempt: left pot north twice; middle pot west; rotate Source once, Lower bend three times, Middle once, Upper once and Finish once. Roll!"
    }};
}
PuzzleDefinition tidyToys() {
    using D=Direction;using A=Action;using K=Kind;using S=Shape;
    PuzzleDefinition c;
    c.schemaVersion=2;c.contentId="tidy-toys-v1";c.questId="pip_tidy_toys";
    c.width=5;c.height=3;c.source={0,2};c.destination={4,2};c.goal=Goal::Tidy;
    c.title="Tidy the toy basket";c.objective="Clear the way. Match each toy to its recess.";
    c.dialogue="I may have scattered my toys. Shall we tuck them in?";
    c.objects={
        {"cushion","Blocking cushion",K::Cushion,{1,1},0,{{1,1},{1,0},{1,2}}},
        {"ball","Coral ball",K::Toy,{2,0},0,{},S::Ball},
        {"mouse","Toy mouse",K::Toy,{2,1},0,{},S::Mouse},
        {"fish","Toy fish",K::Toy,{2,2},0,{},S::Fish},
        {"ball_recess","Round ball recess",K::Socket,{3,0},0,{},S::Ball},
        {"mouse_recess","Mouse-shaped recess",K::Socket,{3,1},0,{},S::Mouse},
        {"fish_recess","Fish-shaped recess",K::Socket,{3,2},0,{},S::Fish}};
    c.access={0,1};c.walkable={{0,1},{1,1},{2,0},{2,1},{2,2},{3,0},{3,1},{3,2}};
    c.solution={{A::Move,"cushion",D::North},{A::PickUp,"ball"},{A::Place,"ball",D::North,"ball_recess"},
        {A::PickUp,"mouse"},{A::Place,"mouse",D::North,"mouse_recess"},{A::PickUp,"fish"},{A::Place,"fish",D::North,"fish_recess"}};
    c.hints={"The cushion blocks the narrow path to the toys and basket. Slide it aside.",
        "Pick up one toy at a time. The round, mouse and fish recesses match their silhouettes.",
        "Move the cushion north. Pick up the ball and place it in the round recess; repeat with the mouse and fish in their matching recesses."};
    return c;
}
PuzzleDefinition stuckGate() {
    using D=Direction;using A=Action;using K=Kind;using S=Shape;
    PuzzleDefinition c;
    c.schemaVersion=2;c.contentId="stuck-gate-v1";c.questId="pip_stuck_gate";
    c.width=5;c.height=3;c.source={0,2};c.destination={3,1};c.goal=Goal::Retrieve;
    c.title="The stuck little gate";c.objective="Hold the switch with the ball. Retrieve the mouse.";
    c.dialogue="My mouse is behind that gate. Could my ball help us open it?";
    c.objects={
        {"pot","Sliding pot",K::Pot,{1,2},0,{{1,2},{1,1}}},
        {"channel_a","Source straight",K::Straight,{1,2},0,{}},
        {"channel_b","Lower bend",K::Curve,{2,2},0,{}},
        {"channel_c","Switch bend",K::Curve,{2,1},0,{}},
        {"switch","Pressure switch",K::Switch,{3,1},0,{}},
        {"gate","Linked gate",K::Gate,{3,0},0,{},S::None,"switch"},
        {"mouse","Mouse behind gate",K::Toy,{4,0},0,{},S::Mouse,"gate"}};
    c.access={0,0};c.walkable={{0,0},{1,0},{2,0},{3,0},{4,0}};
    c.requestedItem="mouse";c.switchRole="switch";c.gateRole="gate";
    c.solution={{A::Move,"pot",D::North},{A::Rotate,"channel_a"},
        {A::Rotate,"channel_b"},{A::Rotate,"channel_b"},{A::Rotate,"channel_b"},
        {A::Rotate,"channel_c"},{A::Roll,""},{A::PickUp,"mouse"}};
    c.hints={"The gate follows the pressure switch. A ball resting there keeps the way open.",
        "Slide the pot north. Connect the ball's route east, north, then east to the switch.",
        "From a fresh attempt: pot north; Source straight once; Lower bend three times; Switch bend once. Roll, leave the ball on the switch, then pick up the mouse."};
    return c;
}
}
