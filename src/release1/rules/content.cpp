#include "content.hpp"
#include <algorithm>
#include <stdexcept>
namespace paws::release1::rules {
namespace {
using V=Verb; using D=Direction; using P=Piece;
Definition base(Family f,std::string q,std::string c,std::string title,std::string objective,int w,int h) {
    Definition d;d.family=f;d.quest=q;d.content=c;d.title=title;d.objective=objective;d.width=w;d.height=h;return d;
}
Definition old(PuzzleDefinition p,Family f=Family::Legacy) {
    auto d=base(f,p.questId,p.contentId,p.title,p.objective,p.width,p.height);
    if(p.questId=="pip_missing_ball"&&p.contentId.ends_with("-v1"))d.objective="Drag the pots aside. Click channels to rotate. Click the ball to roll it to Pip.";
    if(p.questId=="pip_tidy_toys"&&p.contentId.ends_with("-v1"))d.objective="Drag the blocking cushion aside. Drag each toy into its matching basket recess.";
    if(p.questId=="pip_stuck_gate"&&p.contentId.ends_with("-v1"))d.objective="Drag the pot aside. Click channels to rotate, then click the ball to open the gate.";
    for(int i=0;i<3;++i)d.hints[i]=p.hints[i];
    for(auto s:p.solution) {
        const V v=s.action==Action::Move?V::Slide:s.action==Action::Rotate?V::Rotate:
            s.action==Action::Roll?V::Roll:s.action==Action::PickUp?V::PickUp:V::Fit;
        d.solution.push_back({v,s.object,s.target,s.direction});
    }
    d.source=p.source;d.destination=p.destination;d.legacy=std::move(p);return d;
}
Definition benchV1() {
    auto d=base(Family::Bench,"button_mouse_bench","button-mouse-bench-v1","Mouse under the bench",
        "Slide the crates along their marked tracks to clear Button's path. Then click his toy mouse.",5,3);
    d.objects={{"crate_a","Lane crate","crate",P::Crate,{1,1},0,1,{{1,1},{1,0}}},
        {"crate_b","Upper crate","crate",P::Crate,{1,0},0,1,{{1,0},{2,0}}},
        {"mouse","Button's toy mouse","mouse",P::Toy,{3,1}},
        {"bench","Low bench","bench",P::Marker,{3,0}}};
    d.source={0,1};d.reach={2,1};d.requested="mouse";
    d.walkable={{0,1},{1,1},{2,1}};
    d.sockets={{"near_reach","Near reach socket","reach_socket",{2,1}},
        {"far_reach","Far reach socket","reach_socket",{4,2}}};
    d.solution={{V::Slide,"crate_b","",D::East},{V::Slide,"crate_a","",D::North},{V::Retrieve,"mouse","near_reach"}};
    d.hints={"The near reach socket can reach the mouse. Two crates share a sliding slot.",
        "Move the upper crate out of the lane crate's way before clearing the lane.",
        "Upper crate east; lane crate north; retrieve Button's mouse using Near reach socket."};return d;
}
Definition planksV1() {
    auto d=base(Family::Planks,"mabel_sunny_spot","mabel-sunny-spot-v1","The sunny spot",
        "Tap a plank to see its fitting gaps; tap it again to rotate. Drag it to a white gap to connect Mabel's route to the sunny perch.",5,4);
    d.source={0,2};d.destination={3,1};
    d.objects={{"long_plank","Long plank (2 m)","plank",P::Plank,{0,0},1,2},
        {"step_plank","Step plank (1 m)","plank",P::Plank,{1,0},0,1},
        {"finish_plank","Finish plank (1 m)","plank",P::Plank,{2,0},1,1}};
    d.sockets={{"long_socket","Long crossing","plank_socket",{0,2},{2,2},2,0},
        {"step_socket","Turn toward perch","plank_socket",{2,2},{2,1},1,1},
        {"finish_socket","Perch crossing","plank_socket",{2,1},{3,1},1,0},
        {"unsupported_socket","Unsupported gap","plank_socket",{3,3},{4,3},1,0,false}};
    d.solution={{V::PickUp,"long_plank"},{V::Rotate,"long_plank"},{V::Fit,"long_plank","long_socket"},
        {V::PickUp,"step_plank"},{V::Rotate,"step_plank"},{V::Fit,"step_plank","step_socket"},
        {V::PickUp,"finish_plank"},{V::Rotate,"finish_plank"},{V::Fit,"finish_plank","finish_socket"},{V::Connect}};
    d.hints={"Length, alignment and support all matter. A gap without support cannot carry Mabel.",
        "The long plank crosses first. Turn upward on the step plank, then cross right to the perch.",
        "Select each plank, then tap it again to turn once. Drag Long to Long crossing, Step to Turn toward perch, and Finish to Perch crossing."};return d;
}
Definition bench() {
    auto d=benchV1();d.content="button-mouse-bench-v2";
    d.objective="Drag the three crates through their shared spaces. Clear both parts of Button's path, then click the mouse.";
    d.objects[0].slots={{1,1},{1,0},{0,0}};
    d.objects[1].slots={{1,0},{2,0}};
    d.objects.push_back({"crate_c","Reach crate","crate",P::Crate,{2,1},0,1,{{2,1},{2,0}}});
    d.source={0,2};d.walkable={{0,2},{0,1},{1,1},{2,1}};
    d.solution={{V::Slide,"crate_b","",D::East},{V::Slide,"crate_a","",D::North},
        {V::Slide,"crate_a","",D::West},{V::Slide,"crate_b","",D::West},
        {V::Slide,"crate_c","",D::North},{V::Retrieve,"mouse","near_reach"}};
    d.hints={"Both the lane and the reach position must be clear. The upper crate needs to move away and come back.",
        "Move Upper right; park Lane in the far-left space. Return Upper left to free Reach's parking space.",
        "Upper east; Lane north, then west; Upper west; Reach north. Click Button's mouse."};
    return d;
}
Definition planks() {
    auto d=planksV1();d.content="mabel-sunny-spot-v2";d.height=5;
    d.source={0,3};d.destination={4,3};
    d.objective="Choose a route using one long plank and four short planks. Drag the planter off the landing; click planks to turn and drag them across gaps.";
    d.objects={{"long_plank","Long plank (2 m)","plank",P::Plank,{0,0},1,2},
        {"step_plank","Step plank (1 m)","plank",P::Plank,{1,0},0,1},
        {"finish_plank","Finish plank (1 m)","plank",P::Plank,{2,0},1,1},
        {"approach_plank","Approach plank (1 m)","plank",P::Plank,{3,0},1,1},
        {"perch_plank","Perch plank (1 m)","plank",P::Plank,{4,0},0,1},
        {"planter","Landing planter","crate",P::Crate,{4,2},0,1,{{4,2},{3,2},{3,1}}}};
    d.sockets={{"long_socket","Side-platform long crossing","plank_socket",{0,2},{2,2},2,0},
        {"step_socket","Step onto side route","plank_socket",{0,3},{0,2},1,1},
        {"finish_socket","Middle short crossing","plank_socket",{2,2},{3,2},1,0},
        {"unsupported_socket","Unsupported gap","plank_socket",{0,4},{1,4},1,0,false},
        {"approach_socket","Approach crossing","plank_socket",{3,2},{4,2},1,0},
        {"perch_socket","Turn onto sunny perch","plank_socket",{4,2},{4,3},1,1},
        {"direct_first","Direct route first wide gap","plank_socket",{0,3},{2,3},2,0},
        {"direct_second","Direct route second wide gap","plank_socket",{2,3},{4,3},2,0}};
    d.solution={{V::Slide,"planter","",D::West},{V::Slide,"planter","",D::North},
        {V::Rotate,"long_plank"},{V::PickUp,"long_plank"},{V::Fit,"long_plank","long_socket"},
        {V::Rotate,"step_plank"},{V::PickUp,"step_plank"},{V::Fit,"step_plank","step_socket"},
        {V::Rotate,"finish_plank"},{V::PickUp,"finish_plank"},{V::Fit,"finish_plank","finish_socket"},
        {V::Rotate,"approach_plank"},{V::PickUp,"approach_plank"},{V::Fit,"approach_plank","approach_socket"},
        {V::Rotate,"perch_plank"},{V::PickUp,"perch_plank"},{V::Fit,"perch_plank","perch_socket"},{V::Connect}};
    d.hints={"The direct route has two wide gaps, but you have only one long plank. The planter blocks a landing on the side route.",
        "Use the side platforms. Slide the planter left, then toward the house, so it clears both landings.",
        "Planter west, north. Turn every plank once. Long to Side-platform long crossing; Step to Step onto side route; Finish to Middle; Approach to Approach; Perch to Turn onto sunny perch."};
    return d;
}
Definition shapes() {
    auto d=base(Family::Shapes,"button_cushion_bed","button-cushion-bed-v1","A bed for Button",
        "Click cushions to rotate. Drag them into the basket to cover every cell without overlap.",5,5);
    d.objects={{"square","Square cushion","cushion_square",P::Shape,{0,4},0,1,{},{{0,0},{1,0},{0,1},{1,1}}},
        {"bar","Bar cushion","cushion_bar",P::Shape,{2,4},0,1,{},{{0,0},{1,0},{2,0}}},
        {"elbow","Elbow cushion","cushion_elbow",P::Shape,{4,4},0,1,{},{{0,0},{1,0},{0,1}}}};
    d.basket={{0,0},{1,0},{0,1},{1,1},{2,0},{2,1},{2,2},{3,1},{4,1},{4,2}};
    d.sockets={{"basket","Button's ten-cell basket","cushion_basket",{0,0}}};
    d.solution={{V::PickUp,"square"},{V::Fit,"square","basket",D::North,{0,0}},
        {V::PickUp,"bar"},{V::Rotate,"bar"},{V::Fit,"bar","basket",D::North,{2,0}},
        {V::PickUp,"elbow"},{V::Rotate,"elbow"},{V::Fit,"elbow","basket",D::North,{3,1}}};
    d.hints={"Every basket cell needs one cushion cell. Click cushions to rotate and drag them into the basket.",
        "The square fills the left block. The bar stands vertically in the middle. Turn the elbow to fit the right corner.",
        "Square at (0,0) unturned; bar rotated once at (2,0); elbow rotated once at (3,1). Coordinates use column,row from the top left."};return d;
}
Definition water() {
    auto p=missingBall(1);p.questId="mabel_drinking_route";p.contentId="mabel-drinking-route-v1";
    p.title="A drink after playing";p.objective="Click gutters to rotate. Connect the barrel to Mabel's bowl, then click the barrel to release water.";
    p.objects.erase(p.objects.begin(),p.objects.begin()+2);p.solution.erase(p.solution.begin(),p.solution.begin()+3);
    p.sourceId="rain_barrel";p.destinationId="drinking_bowl";
    p.objects[0].name="Source gutter";p.objects[1].name="Lower bend";
    p.objects[2].name="Middle gutter";p.objects[3].name="Upper bend";p.objects[4].name="Finish gutter";
    p.hints={"Water needs a continuous route with openings facing each other. The bowl must be reached from its left side.",
        "Flow travels right, up, then right. The lower bend needs to face left and up.",
        "From reset: Source gutter once, Lower bend three times, Middle gutter once, Upper bend once, Finish gutter once; check flow."};
    return old(std::move(p),Family::Water);
}
Definition bells() {
    return pictureSequence("pepper_garden_bells","pepper-garden-bells-v1","The garden bells",
        {{"leaf","Leaf bell","bell_leaf",P::Bell,{0,0},0,1,{}, {},"leaf"},
        {"star","Star bell","bell_star",P::Bell,{1,0},0,1,{}, {},"star"},
        {"paw","Paw bell","bell_paw",P::Bell,{2,0},0,1,{}, {},"paw"}},
        {"leaf","star","leaf","paw","star"},1);
}
Definition switchesV1() {
    auto d=base(Family::Switches,"pip_pepper_playhouse","pip-pepper-playhouse-v1","Two kittens, one doorway",
        "Drag Pip onto the paw switch and Pepper onto the star switch. Hold both together to open the doorway.",5,3);
    d.objects={{"pip","Pip","cat_pip",P::Cat,{0,2}}, {"pepper","Pepper","cat_pepper",P::Cat,{4,2}}};
    d.sockets={{"pip_switch","Pip's paw switch","switch_pip",{1,1}},
        {"pepper_switch","Pepper's star switch","switch_pepper",{3,1}},
        {"waiting","Waiting spot (release switch)","waiting",{2,2}}};
    d.solution={{V::AssignCat,"pip","pip_switch"},{V::AssignCat,"pepper","pepper_switch"},{V::Connect}};
    d.hints={"Both switches must be held at the same time. Walking away releases a switch immediately.",
        "The paw switch belongs to Pip; the star switch belongs to Pepper. One kitten cannot hold both.",
        "Drag Pip to the paw switch and Pepper to the star switch. The doorway opens when both are held."};return d;
}
Definition switches() {
    auto d=base(Family::Switches,"pip_pepper_playhouse","pip-pepper-playhouse-v2","Two kittens, one doorway",
        "Help both kittens get inside. Drag Pip onto the paw switch to open the side gate for Pepper.",8,6);
    d.objects={{"pip","Pip","cat_pip",P::Cat,{1,5}}, {"pepper","Pepper","cat_pepper",P::Cat,{6,5}},
        {"crate","Switch-sized crate","crate",P::Crate,{5,1},0,1,{{5,1},{5,2},{4,2},{4,3},{3,3},{2,3},{1,3}}}};
    d.sockets={{"pip_switch","Paw switch (Pip or crate)","switch_pip",{1,3}},
        {"pepper_switch","Pepper's star switch","switch_pepper",{5,3}},
        {"waiting","Pip's waiting mat","waiting",{2,5}},
        {"pepper_waiting","Pepper's waiting mat","waiting",{6,5}},
        {"storage","Pepper's storage alcove","storage_spot",{6,1}},
        {"inside_entry","Pip's place inside the doorway","inside_entry",{2,1}},
        {"inside_latch","Inside latch tile (Pip)","inside_latch",{1,0}},
        {"pepper_inside","Pepper's place inside","inside_spot",{3,1}}};
    d.source={5,2};d.destination={2,2}; // Side gate crossing and main doorway.
    d.solution={{V::AssignCat,"pip","pip_switch"},{V::AssignCat,"pepper","storage"},
        {V::Slide,"crate","",D::South},{V::AssignCat,"pip","waiting"},
        {V::Slide,"crate","",D::West},{V::Slide,"crate","",D::South},
        {V::Slide,"crate","",D::West},{V::Slide,"crate","",D::West},{V::Slide,"crate","",D::West},
        {V::AssignCat,"pepper","pepper_switch"},{V::AssignCat,"pip","inside_entry"},
        {V::AssignCat,"pip","inside_latch"},{V::AssignCat,"pepper","pepper_inside"}};
    d.hints={"The paw switch opens the storage gate. Pepper can help slide the crate out while Pip holds it.",
        "Park the crate outside, then move Pip to his waiting mat. The crate fits the paw switch and frees Pip to walk inside.",
        "Pip to paw; Pepper to storage; crate out, Pip to waiting, crate to paw; Pepper to star; Pip inside, then onto the latch; Pepper inside."};
    return d;
}
Definition picnic() {
    auto d=base(Family::Picnic,"garden_picnic","garden-picnic-v1","The garden picnic",
        "Drag Button's bed into shade and quiet. Drag Mabel's bowl into shade beside the bed, sharing an edge.",5,4);
    d.objects={{"bed","Button's picnic bed","picnic_bed",P::Placement,{0,3}},
        {"bowl","Mabel's picnic bowl","picnic_bowl",P::Placement,{4,3}}};
    d.shade={{1,0},{2,0},{1,1},{2,1},{3,1}};d.quiet={{1,1},{1,2}};
    for(auto cell:std::vector<Cell>{{0,0},{1,0},{2,0},{1,1},{2,1},{3,1},{1,2},{3,2}})
        d.sockets.push_back({"spot_"+std::to_string(cell.x)+"_"+std::to_string(cell.y),
            "Garden spot ("+std::to_string(cell.x)+","+std::to_string(cell.y)+")","picnic_spot",cell});
    d.solution={{V::Fit,"bed","spot_1_1"},{V::Fit,"bowl","spot_2_1"},{V::Connect}};
    d.hints={"The SHADE and QUIET marks are two separate clues. Beside means an edge, never a diagonal.",
        "Only spot (1,1) is both shaded and quiet. The bowl needs a shaded edge-neighbour of it.",
        "Drag the bed to (1,1) and the bowl to (2,1) or (1,0). A correct arrangement completes the request."};return d;
}
}
Definition pictureSequence(std::string quest,std::string content,std::string title,
    std::vector<Object> responses,std::vector<std::string> pictures,double seconds){
    int width=1,height=2;for(const auto& o:responses){width=std::max(width,o.initial.x+1);height=std::max(height,o.initial.y+1);}
    auto d=base(Family::PictureSequence,std::move(quest),std::move(content),std::move(title),
        "Watch the blackboard pictures, then play the matching responses in the same order. Tap the board to replay.",width,height);
    d.objects=std::move(responses);d.sequence=std::move(pictures);d.pictureSeconds=seconds;
    d.objects.push_back({"sequence_board","Picture blackboard","sequence_blackboard",P::Marker,{width/2,1}});
    for(const auto& s:d.sequence){auto o=std::find_if(d.objects.begin(),d.objects.end(),[&](const auto& o){return o.kind==P::Bell&&o.symbol==s;});
        if(o==d.objects.end())throw std::invalid_argument("Picture has no response");d.solution.push_back({V::TapSymbol,o->id});}
    d.hints={"Watch one picture at a time. Each picture matches a response; sound is optional.",
        "Repeated pictures mean repeated taps. Tap the blackboard or Replay pictures whenever you want another look.",
        "After the last picture disappears, repeat the whole order. A wrong response clears your answer without a penalty."};
    if(auto e=validate(d,false);!e.empty())throw std::invalid_argument(e);return d;
}
Definition scentHunt() {
    auto d=base(Family::Scent,"pepper_scent_trail","pepper-scent-trail-v1","Follow the scent",
        "Help Pepper track her catnip pouch. Inspect all three clues, then choose the place that fits every clue.",7,5);
    d.objects={{"clue_0","Paw-print clue","trail_clue",P::Marker,{1,1}},
        {"clue_1","Snagged-fluff clue","trail_clue",P::Marker,{3,1}},
        {"clue_2","Scent clue","trail_clue",P::Marker,{5,1}},
        {"trail_0","First trail","trail_choice",P::Marker,{1,3}},
        {"trail_1","Second trail","trail_choice",P::Marker,{3,3}},
        {"trail_2","Third trail","trail_choice",P::Marker,{5,3}}};
    d.trail={
      {{{"Fresh prints lead past something round.","Cream fluff caught on a low clay rim.","Catnip smells strongest near leaves, away from water."}},{{"Round herb pot","Round drinking bowl","Tall square planter"}},0},
      {{{"The next prints pass UNDER something wooden.","Fluff snagged on a slat, with open space below.","The trail stays dry, beside the quiet wall."}},{{"Rain-barrel stand","Low garden bench","Solid wooden chest"}},1},
      {{{"Small prints turn toward a woven edge.","Cream fluff rests on soft fabric.","The pouch brushed something with NO toys inside."}},{{"Empty clay pot","Woven toy basket","Woven cushion basket"}},2},
      {{{"The next prints climb onto a LOW stone edge.","Fluff is caught beside yellow petals.","The scent leads away from the dripping barrel."}},{{"Yellow flowers by the barrel","Yellow flowers on the dry border","Blue flowers on the dry border"}},1},
      {{{"Fresh prints circle a narrow wooden post.","Cream fluff clings to rough rope.","The scent stops at something for scratching, without a roof."}},{{"Rope scratching post","Rope hammock stand","Wooden playhouse"}},0},
      {{{"The last prints lead into a sheltered nook.","Cream fluff caught on the LEFT cushion seam.","Catnip is strongest in the shaded nook with the LEAF cushion."}},{{"Left sunny nook, leaf cushion","Right shaded nook, leaf cushion","Left shaded nook, leaf cushion"}},2}};
    for(const auto& stage:d.trail){for(int n=0;n<3;++n)d.solution.push_back({V::Inspect,"clue_"+std::to_string(n)});d.solution.push_back({V::FollowTrail,"trail_"+std::to_string(stage.answer)});}
    d.hints={"Inspect the paw prints, fluff and scent at every stop. Each choice must satisfy all three clues; there is no time limit.",
        "Some places fit two clues. Check the material, shelter and position as well. A wrong trail keeps your current discoveries.",
        "Follow: round herb pot; low bench; cushion basket; dry yellow-flower border; rope scratching post; left shaded leaf-cushion nook."};
    return d;
}
Definition wobblyPerch() {
    auto d=base(Family::Balance,"pip_wobbly_perch","pip-wobbly-perch-v1","Pip's wobbly perch",
        "Balance four practice setups for Pip. Drag numbered cushions onto the beam; weight times distance from the support must match on both sides.",9,6);
    for(int n=1;n<=4;++n)d.objects.push_back({"weight_"+std::to_string(n),std::to_string(n)+"-weight cushion","balance_weight",P::Placement,{n*2-1,5},0,n});
    for(int x:{1,2,3,5,6,7})d.sockets.push_back({"beam_"+std::to_string(x),"Beam position "+std::to_string(x),"balance_socket",{x,2}});
    for(int n=1;n<=4;++n)d.sockets.push_back({"home_"+std::to_string(n),"Cushion "+std::to_string(n)+" storage","balance_home",{n*2-1,5}});
    d.objects.push_back({"balance_test","Test this balance","balance_test",P::Marker,{4,4}});
    d.balances={{4,0,0},{3,2,2},{5,2,-3},{4,3,3}};
    const int arrangements[4][4]={{1,5,7,2},{3,2,5,1},{1,5,7,6},{1,6,2,3}};
    for(auto& round:arrangements){for(int n=0;n<4;++n)d.solution.push_back({V::Fit,"weight_"+std::to_string(n+1),"beam_"+std::to_string(round[n])});d.solution.push_back({V::TestBalance});}
    d.hints={"Place all four cushions on different beam positions. A 2-weight cushion two steps from the support pulls as much as a 4-weight cushion one step away. Tap the paw mat to test.",
        "The support moves between setups. Count distances from its current position, and include Pip's stated pull. Return a cushion to its matching storage mat to free a space.",
        "Positions for cushions 1,2,3,4: setup 1 = 1,5,7,2; setup 2 = 3,2,5,1; setup 3 = 1,5,7,6; setup 4 = 1,6,2,3. Test each setup."};
    return d;
}
std::vector<Definition> catalogue(bool expandedBeach) {
    auto ball=[] {
        PuzzleDefinition p;p.schemaVersion=2;p.questId="pip_friend_ball";p.contentId="pip-friend-ball-v1";
        p.title="Can you get my ball?";p.objective="Clear the planters and roll Pip's ball to the catch tray. Then find its way home.";
        p.width=8;p.height=6;p.source={0,4};p.checkpoint={3,2};p.destination={7,1};p.goal=Goal::Relay;
        p.objects={{"pot_a","Round planter",Kind::Pot,{1,4},0,{{1,4},{1,3},{1,2}}},
          {"pot_b","Tall planter",Kind::Pot,{2,3},0,{{2,3},{1,3}}},
          {"channel_a","First straight",Kind::Straight,{1,4},0},
          {"channel_b","Corner by the pots",Kind::Curve,{2,4},2},
          {"channel_c","Uphill straight",Kind::Straight,{2,3},1},
          {"channel_d","Catch-tray bend",Kind::Curve,{2,2},0},
          {"catch_tray","Safe catch tray",Kind::Catch,{3,2}},
          {"channel_e","Second straight",Kind::Straight,{4,2},0},
          {"channel_f","Lookout bend",Kind::Curve,{5,2},2},
          {"channel_g","Upper bend",Kind::Curve,{5,1},0},
          {"channel_h","Home straight",Kind::Straight,{6,1},0}};
        p.solution={{Action::Move,"pot_a",D::North},{Action::Move,"pot_a",D::North},{Action::Move,"pot_b",D::West},
          {Action::Rotate,"channel_a"},{Action::Rotate,"channel_b"},{Action::Rotate,"channel_c"},{Action::Rotate,"channel_d"},{Action::Roll,""},
          {Action::Rotate,"channel_e"},{Action::Rotate,"channel_f"},{Action::Rotate,"channel_g"},{Action::Rotate,"channel_h"},{Action::Roll,""}};
        p.hints={"The planters share a parking space. The catch tray is a safe halfway stop, not the finish.",
          "Move Round toward the house twice, then Tall left. Turn each of the first four channels once and roll.",
          "After the catch tray: turn each of the four newly revealed channels once. Click the ball in the tray to send it home."};
        return old(std::move(p));
    }();
    auto bridge=planks();bridge.quest="pip_friend_view";bridge.content="pip-friend-view-v1";bridge.title="Come and see!";
    bridge.objective="Build Pip a safe route to his lookout. One long plank cannot span both wide gaps; clear the planter and use the side platforms.";
    // Mirror the established bridge vocabulary into a new authored layout.
    auto mirror=[&](Cell& c){c.x=bridge.width-1-c.x;};mirror(bridge.source);mirror(bridge.destination);
    for(auto& o:bridge.objects){mirror(o.initial);for(auto& c:o.slots)mirror(c);}
    for(auto& s:bridge.sockets){mirror(s.cell);mirror(s.end);}
    for(auto& s:bridge.solution)if(s.verb==V::Slide&&s.direction==D::West)s.direction=D::East;
    bridge.hints={"There are two wide gaps and only one long plank. Follow the side platforms instead.",
      "Slide the planter right, then toward the house. The long plank belongs on the long side crossing.",
      "Planter east, north. Turn each plank once; fit Long, Step, Finish, Approach and Perch into their matching side-route crossings."};
    auto shared=wobblyPerch();shared.quest="pip_friend_room";shared.content="pip-friend-room-v1";shared.title="Room for two";shared.sharedLookout=true;
    shared.objective="Steady Pip's lookout with the numbered flowerpots, then make room for two cushions without blocking the approach.";
    for(auto& o:shared.objects)if(o.role=="balance_weight")o.name=std::to_string(o.length)+"-weight flowerpot";
    shared.objects.push_back({"pip_cushion","Pip's cushion","lookout_cushion",P::Placement,{0,4}});
    shared.objects.push_back({"your_cushion","Your cushion","lookout_cushion",P::Placement,{8,4}});
    shared.sockets.push_back({"pip_seat","Pip's lookout seat","lookout_seat",{7,1}});
    shared.sockets.push_back({"your_seat","Your lookout seat","lookout_seat",{1,1}});
    shared.sockets.push_back({"approach","Keep this approach clear","lookout_approach",{4,1}});
    shared.balances={{4,0,0},{4,3,3}};shared.solution.clear();
    const int arrangements[2][4]={{1,5,7,2},{1,6,2,3}};
    for(int round=0;round<2;++round){for(int n=0;n<4;++n)shared.solution.push_back({V::Fit,"weight_"+std::to_string(n+1),"beam_"+std::to_string(arrangements[round][n])});
      if(round==1){shared.solution.push_back({V::Fit,"pip_cushion","pip_seat"});shared.solution.push_back({V::Fit,"your_cushion","your_seat"});}
      shared.solution.push_back({V::TestBalance});}
    shared.hints={"First steady the empty lookout. In the second setup include Pip's pull on the right, then put both cushions on their marked seats.",
      "The middle approach must stay clear. Pip's cushion belongs on the right; the cushion he saved for you belongs on the left.",
      "Pots 1,2,3,4: first use positions 1,5,7,2 and test. Then use 1,6,2,3; add Pip's cushion right and yours left; test with Pip aboard."};
    auto sun=planks();sun.quest="mabel_friend_sun";sun.content="mabel-friend-sun-v1";sun.title="Just a little higher";
    sun.objective="Clear both overgrown planters and build Mabel a supported route to her sunny window ledge.";
    sun.objects.push_back({"planter_small","Small overgrown planter","crate",P::Crate,{3,1},0,1,{{3,1},{2,1},{1,1}}});
    sun.solution.insert(sun.solution.begin(),{{V::Slide,"planter_small","",D::West},{V::Slide,"planter_small","",D::West}});
    sun.hints={"The small planter blocks the large planter's parking space. Clear it first, then use the side platforms.","Move Small left twice. Move Landing left, then toward the house. One long plank cannot bridge both direct gaps.","Small west twice; Landing west, north. Turn each plank once and fit Long, Step, Finish, Approach and Perch on the side route."};
    auto drink=*ball.legacy;drink.questId="mabel_friend_water";drink.contentId="mabel-friend-water-v1";drink.title="Something's missing";
    drink.sourceId="rain_barrel";drink.destinationId="drinking_bowl";
    drink.objective="Fill the catch basin first. Then redirect the second gutter route into Mabel's bowl, keeping her cushion dry.";
    for(auto& o:drink.objects){if(channel(o.kind))o.name="Gutter "+o.id.substr(8);if(o.id=="catch_tray")o.name="Catch basin";}
    for(auto& o:drink.objects){if(o.id=="channel_e"){o.kind=Kind::Curve;o.orientation=2;}if(o.id=="channel_f"){o.initial={4,1};o.orientation=0;}if(o.id=="channel_g"){o.kind=Kind::Straight;o.orientation=0;}}
    drink.objects.push_back({"dry_cushion","Keep Mabel's cushion dry",Kind::Catch,{4,3}});
    drink.hints={"Move the planters away from the gutters. The catch basin is a safe halfway stop.","Round planter north twice; Tall west. Turn the first four gutters once and check the barrel.","When the basin is full, turn the four remaining gutters once. Check the basin to fill the bowl; the basin bend must point away from the cushion."};
    auto drinkQuest=old(std::move(drink),Family::Water);
    auto together=picnic();together.quest="mabel_friend_share";together.content="mabel-friend-share-v1";together.title="Stay a while?";together.sunnyNook=true;
    together.objective="Place Mabel's cushion and the cushion she saved for Pip in the sunshine. Leave the marked middle path clear between them.";
    together.objects={{"bed","Mabel's cushion","picnic_bed",P::Placement,{0,3}},{"bowl","Pip's cushion","picnic_bed",P::Placement,{4,3}}};
    together.shade={{1,0},{2,0},{3,0},{1,1}};together.quiet={{1,0}};together.walkable={{2,0},{2,1},{2,2}};
    together.sockets.clear();for(int y=0;y<3;++y)for(int x=1;x<=3;++x)together.sockets.push_back({"spot_"+std::to_string(x)+"_"+std::to_string(y),"Nook spot ("+std::to_string(x)+","+std::to_string(y)+")","picnic_spot",{x,y}});
    together.solution={{V::Fit,"bed","spot_1_0"},{V::Fit,"bowl","spot_3_0"},{V::Connect}};
    together.hints={"The golden marks show sunshine; the leaf marks show the shared approach. Both cushions need sun and space to enter.","Mabel's favourite place is the rear-left sunny spot. Keep every middle-column path marker uncovered.","Mabel's cushion at (1,0), Pip's at (3,0). The middle path between them stays open."};
    // First adventure slice: two baskets share parking, then a three-piece
    // driftwood track carries Pip's ball around the corner to the picnic mat.
    PuzzleDefinition beach;
    beach.schemaVersion=2;beach.questId="beach_pip_ball";beach.contentId="beach-pip-ball-v1";
    beach.title="Pip's runaway beach ball";
    beach.objective="Slide the beach baskets aside. Turn the driftwood channels, then roll the ball to Pip's picnic mat.";
    beach.dialogue="The postcard showed the perfect picnic spot. My ball found it first! Could you roll it back to my mat?";
    beach.width=5;beach.height=4;beach.source={0,2};beach.destination={3,1};
    beach.sourceExit=D::East;beach.destinationEntry=D::West;
    beach.objects={
      {"basket_a","Front beach basket",Kind::Crate,{1,2},0,{{1,2},{1,1},{1,0},{0,0}}},
      {"basket_b","Rear beach basket",Kind::Crate,{1,1},0,{{1,1},{1,0},{2,0}}},
      {"channel_a","Beach straight",Kind::Straight,{1,2},0},
      {"channel_b","Shore bend",Kind::Curve,{2,2},0},
      {"channel_c","Picnic bend",Kind::Curve,{2,1},0}};
    beach.solution={{Action::Move,"basket_b",D::North},{Action::Move,"basket_b",D::East},
      {Action::Move,"basket_a",D::North},{Action::Move,"basket_a",D::North},
      {Action::Rotate,"channel_a"},{Action::Rotate,"channel_b"},{Action::Rotate,"channel_b"},{Action::Rotate,"channel_b"},
      {Action::Rotate,"channel_c"},{Action::Roll,""}};
    beach.hints={"The baskets share a parking space. Move the rear basket first so the front one has somewhere to go.",
      "Rear basket toward the dunes, then right. Front basket toward the dunes twice. Connect the ball east, north, then east to the mat.",
      "Rear north, east; Front north twice. Turn Beach straight once, Shore bend three times and Picnic bend once. Roll the ball."};
    auto all=std::vector<Definition>{old(missingBall()),old(tidyToys()),old(stuckGate()),bench(),planks(),shapes(),water(),bells(),switches(),picnic(),scentHunt(),wobblyPerch(),std::move(ball),std::move(bridge),std::move(shared),std::move(sun),std::move(drinkQuest),std::move(together),old(std::move(beach))};
    appendBeachQuests(all);if(expandedBeach)expandBeachQuests(all);appendBeachOutings(all,expandedBeach);return all;
}
Definition quest(std::string_view id,std::string_view content) {
    // Authored definitions are immutable. Input/read-model queries must not
    // rebuild every quest just to copy one requested definition.
    static const auto current=catalogue();
    for(const auto& d:current)if(d.quest==id&&(content.empty()||content==d.content))return d;
    static const auto previous=[]{
        std::vector<Definition> definitions{old(missingBall(1)),old(tidyToys(1)),old(stuckGate(1)),benchV1(),planksV1(),switchesV1()};
        for(auto& d:catalogue(false))if(d.quest.starts_with("beach_"))definitions.push_back(std::move(d));
        return definitions;
    }();
    for(auto& d:previous)if(d.quest==id&&content==d.content)return d;
    throw std::invalid_argument("Unknown release-one quest.");
}
}
