#include "puzzle.hpp"
namespace paws {
PuzzleDefinition missingBall(int version) {
    using D=Direction; using A=Action; using K=Kind;
    PuzzleDefinition c{1,"missing-ball-v1","pip_missing_ball",5,3,{0,2},{4,0},D::East,D::West,{
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
    if(version==1)return c;
    c.schemaVersion=2;c.contentId="missing-ball-v2";c.height=4;
    c.objective="Drag the pots through their shared parking spaces. Click channels to turn them, then click the ball.";
    c.objects={
        {"pot_a","Left pot",K::Pot,{1,2},0,{{1,2},{1,1},{1,0},{0,0}}},
        {"pot_b","Middle pot",K::Pot,{2,1},0,{{2,1},{1,1},{1,0}}},
        {"channel_a","Source bend",K::Curve,{1,2}},
        {"channel_b","Shared-space bend",K::Curve,{1,1}},
        {"channel_c","Middle straight",K::Straight,{2,1}},
        {"channel_d","Perch bend",K::Curve,{3,1}},
        {"channel_e","Finish bend",K::Curve,{3,0}},
        {"catch_tray","Wrong-way catch tray",K::Catch,{1,3}}
    };
    c.solution={{A::Move,"pot_a",D::North},{A::Move,"pot_a",D::North},
        {A::Move,"pot_b",D::West},{A::Move,"pot_a",D::West},{A::Move,"pot_b",D::North},
        {A::Rotate,"channel_a"},{A::Rotate,"channel_a"},{A::Rotate,"channel_a"},
        {A::Rotate,"channel_b"},{A::Rotate,"channel_c"},
        {A::Rotate,"channel_d"},{A::Rotate,"channel_d"},{A::Rotate,"channel_d"},
        {A::Rotate,"channel_e"},{A::Roll,""}};
    c.hints={"The pots share two spaces. The lower branch ends at the catch tray, not Pip.",
        "Park Left pot above the bends; move Middle into its vacated space, then move Left farther left so Middle can park above the route.",
        "Left north twice; Middle west; Left west; Middle north. Turn Source three times, Shared-space once, Middle once, Perch three times, Finish once. Click the ball."};
    return c;
}
PuzzleDefinition tidyToys(int version) {
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
    if(version==1)return c;
    c.contentId="tidy-toys-v2";c.packedToysBlockAccess=true;
    c.objective="Move the cushion to expose the toys. Drag each toy into its matching basket recess. Drag a packed toy home to remove it.";
    c.objects[1].approaches={{1,0}};c.objects[2].approaches={{1,0}};c.objects[3].approaches={{1,2}};
    c.objects[4].initial={4,0};c.objects[5].initial={4,1};c.objects[6].initial={4,2};
    // The production basket is open along its entire left side. Keep that
    // visible aisle connected beside all three recesses; routing only through
    // {4,0} made a packed ball an invisible wall in front of the mouse/fish.
    c.walkable={{0,1},{1,1},{1,0},{1,2},{2,0},{2,1},{2,2},{3,0},{3,1},{3,2},{4,0},{4,1},{4,2}};
    c.solution={{A::Move,"cushion",D::North},{A::PickUp,"fish"},{A::Place,"fish",D::North,"fish_recess"},
        {A::Move,"cushion",D::South},{A::Move,"cushion",D::South},
        {A::PickUp,"mouse"},{A::Place,"mouse",D::North,"mouse_recess"},
        {A::PickUp,"ball"},{A::Place,"ball",D::North,"ball_recess"}};
    c.hints={"The cushion's two parking places expose different toys. Each basket recess is reached from the open side.",
        "Park the cushion toward the house to reach the fish. Park it toward the front to reach the mouse and ball. The toys can be packed in any order.",
        "Cushion north; fish into its recess. Cushion south twice; mouse into its recess; ball into its recess. To recover an early placement, drag that toy to its original home."};
    return c;
}
PuzzleDefinition stuckGate(int version) {
    using D=Direction;using A=Action;using K=Kind;using S=Shape;
    PuzzleDefinition c;
    c.schemaVersion=2;c.contentId="stuck-gate-v1";c.questId="pip_stuck_gate";
    c.width=5;c.height=3;c.source={0,2};c.destination={3,1};c.goal=Goal::Retrieve;
    c.title="The stuck little gate";c.objective="Roll the ball onto the switch to open the gate.";
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
        "From a fresh attempt: pot north; Source straight once; Lower bend three times; Switch bend once. Roll the ball onto the switch. Pip gets his mouse when the gate opens."};
    if(version==1)return c;
    c.contentId="stuck-gate-v2";c.keepBallOnObstacleMove=true;
    c.objective="Roll the ball onto the switch. Keep it there, park the pot in the unlocked space, then drag the crate clear of Pip's path.";
    c.objects[0].slots.push_back({0,1});c.objects[0].link="gate";c.objects[0].gatedSlots={{0,1}};
    c.objects.push_back({"gate_crate","Gate-path crate",K::Crate,{2,0},0,{{2,0},{1,0},{1,1}},S::None,"gate",{{1,0},{1,1}}});
    c.solution.insert(c.solution.end()-1,{{A::Move,"pot",D::West},{A::Move,"gate_crate",D::West},{A::Move,"gate_crate",D::South}});
    c.hints={"Opening the gate is only the first stage. The crate still blocks Pip's approach; the ball must stay on the switch.",
        "The opened gate unlocks the pot's left parking space. Use it to free the crate's final parking space.",
        "Pot north; Source once, Lower bend three times, Switch bend once; click ball. After arrival: pot west; crate west, then south. Pip retrieves the mouse when the path is clear."};
    return c;
}
}
