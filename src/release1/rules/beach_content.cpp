#include "content.hpp"
#include <algorithm>
namespace paws::release1::rules {
namespace {
using V=Verb;using P=Piece;
Definition make(Family f,std::string id,std::string content,std::string title,std::string objective,int w,int h){
    Definition d;d.family=f;d.quest=std::move(id);d.content=std::move(content);d.title=std::move(title);d.objective=std::move(objective);d.width=w;d.height=h;return d;
}

}
// Keep the v1 definitions immutable for journal replay. New attempts use these
// longer layouts, with the same rule families and once-only progression IDs.
void expandBeachQuests(std::vector<Definition>& all){
 for(auto& d:all){
  if(!d.quest.starts_with("beach_"))continue;
  d.content.replace(d.content.size()-2,2,"v2");
  if(d.quest=="beach_pip_ball"){
   auto& p=*d.legacy;p.contentId=d.content;p.width=d.width=7;p.height=d.height=5;
   p.destination=d.destination={5,1};
   p.objective=d.objective="Clear four baskets sharing the dune parking spaces. Turn five driftwood channels to carry the ball around the bend to Pip.";
   p.objects.push_back({"basket_c","Dune parking basket",Kind::Crate,{2,0},0,{{2,0},{3,0},{4,0}}});
   p.objects.push_back({"basket_d","Picnic approach basket",Kind::Crate,{4,1},0,{{4,1},{4,0},{3,0}}});
   p.objects.push_back({"channel_d","Picnic straight",Kind::Straight,{3,1},0});
   p.objects.push_back({"channel_e","Mat approach",Kind::Straight,{4,1},0});
   using A=Action;using D=Direction;
   p.solution={{A::Move,"basket_b",D::North},{A::Move,"basket_c",D::East},{A::Move,"basket_b",D::East},
    {A::Move,"basket_a",D::North},{A::Move,"basket_a",D::North},{A::Move,"basket_d",D::North},
    {A::Rotate,"channel_a"},{A::Rotate,"channel_b"},{A::Rotate,"channel_b"},{A::Rotate,"channel_b"},
    {A::Rotate,"channel_c"},{A::Rotate,"channel_d"},{A::Rotate,"channel_e"},{A::Roll,""}};
   d.solution.clear();for(const auto& s:p.solution)d.solution.push_back({s.action==A::Move?V::Slide:s.action==A::Rotate?V::Rotate:V::Roll,s.object,s.target,s.direction});
   d.hints={"The rear basket and dune basket share parking. Make room before clearing the front basket; another basket covers the mat approach.",
    "Move Rear north, Dune right, Rear right, then Front north twice. Move the approach basket north. The route runs east, north, then east.",
    "After parking the four baskets, turn Beach straight once, Shore bend three times, Picnic bend once and both remaining straights once. Roll the ball."};
   for(int n=0;n<3;++n)p.hints[n]=d.hints[n];
  }else if(d.quest=="beach_button_bridge"){
   d.width=7;d.height=6;d.source={0,3};d.destination={5,2};
   d.objective="Clear the baskets from the landings. Turn and fit five driftwood planks of two lengths to make a winding dry route to Button.";
   d.objects={{"long_plank","First long plank","plank",P::Plank,{0,0},1,2},
    {"step_plank","Long dune crossing","plank",P::Plank,{2,0},0,2},
    {"finish_plank","Short dune turn","plank",P::Plank,{4,0},1,1},
    {"dune_plank","Short runnel turn","plank",P::Plank,{5,0},0,1},
    {"shelter_plank","Long shelter crossing","plank",P::Plank,{6,0},1,2},
    {"landing_basket","Landing basket","crate",P::Crate,{2,3},0,1,{{2,3},{2,4},{2,5}}},
    {"parking_basket","Parking basket","crate",P::Crate,{2,4},0,1,{{2,4},{3,4},{4,4}}}};
   d.sockets={{"long_socket","First runnel crossing","plank_socket",{0,3},{2,3},2,0},
    {"step_socket","Long dune crossing","plank_socket",{2,3},{2,1},2,1},
    {"finish_socket","Short dune turn","plank_socket",{2,1},{3,1},1,0},
    {"dune_socket","Short runnel turn","plank_socket",{3,1},{3,2},1,1},
    {"shelter_socket","Shelter crossing","plank_socket",{3,2},{5,2},2,0},
    {"unsupported_socket","Loose sand without supports","plank_socket",{0,3},{0,1},2,1,false},
    {"detour_socket","Firm dead-end landing","plank_socket",{2,3},{4,3},2,0}};
   d.solution={{V::Slide,"parking_basket","",Direction::East},{V::Slide,"landing_basket","",Direction::South}};
   for(auto pair:{std::pair{"long_plank","long_socket"},std::pair{"step_plank","step_socket"},std::pair{"finish_plank","finish_socket"},std::pair{"dune_plank","dune_socket"},std::pair{"shelter_plank","shelter_socket"}}){d.solution.push_back({V::PickUp,pair.first});d.solution.push_back({V::Rotate,pair.first});d.solution.push_back({V::Fit,pair.first,pair.second});}
   d.solution.push_back({V::Connect});
   d.hints={"Clear both ends of every crossing. Short planks span one step, long planks span two. Firm supports can still lead to a dead end.",
    "Move Parking right, then Landing toward the front. Follow the connected landings: right two, back two, right one, forward one, right two.",
    "Turn each plank once. Fit First to First, Long dune to Long dune, Short dune to Short dune, Short runnel to Short runnel, and Shelter to Shelter."};
  }else if(d.quest=="beach_mabel_shade"){
   d.width=7;d.height=6;d.destination={5,3};
   d.objective="Turn and fit four shade panels on firm supports. Cover all eight marked mat cells; supported posts outside the mat do not help.";
   d.objects={{"front_panel","Front shade panel","shade_panel",P::Plank,{0,5},1,2},
    {"rear_panel","Rear shade panel","shade_panel",P::Plank,{2,5},1,2},
    {"side_panel","Front extension panel","shade_panel",P::Plank,{4,5},1,2},
    {"corner_panel","Rear extension panel","shade_panel",P::Plank,{6,5},1,2}};
   d.sockets={{"front_support","Front canopy supports","shade_socket",{1,3},{3,3},2,0},
    {"rear_support","Rear canopy supports","shade_socket",{1,2},{3,2},2,0},
    {"side_support","Front extension supports","shade_socket",{3,3},{5,3},2,0},
    {"corner_support","Rear extension supports","shade_socket",{3,2},{5,2},2,0},
    {"outside_support","Firm posts beyond the mat","shade_socket",{1,1},{3,1},2,0},
    {"loose_support","Loose dune posts","shade_socket",{0,0},{0,2},2,1,false}};
   d.shade.clear();for(int y:{2,3})for(int x=1;x<=4;++x)d.shade.push_back({x,y});
   d.solution.clear();for(auto pair:{std::pair{"front_panel","front_support"},std::pair{"rear_panel","rear_support"},std::pair{"side_panel","side_support"},std::pair{"corner_panel","corner_support"}}){d.solution.push_back({V::PickUp,pair.first});d.solution.push_back({V::Rotate,pair.first});d.solution.push_back({V::Fit,pair.first,pair.second});}
   d.solution.push_back({V::Connect});
   d.hints={"Each panel covers two marked cells. Four panels must cover the whole mat; a firm pair of posts can still be in the wrong place.",
    "The mat has two rows, each four cells wide. Cover the left and right half of both rows. Ignore the posts beyond the mat.",
    "Turn all four panels once. Fit Front and Rear to their canopy supports, then both extension panels to their matching extension supports."};
  }else if(d.quest=="beach_pepper_basket"){
   d.trail.insert(d.trail.begin()+2,{
    {{{"Prints pass RIGHT of a forked piece of driftwood.","Blue thread is snagged on its LOW branch.","Picnic scent follows the sheltered sand, away from wet stones."}},{{"Left low branch on dry sand","Right high branch on dry sand","Right low branch on dry sand"}},2},
    {{{"Prints circle the SMOOTH side of a pale boulder.","Blue thread points toward a pair of shells.","The picnic scent stays on the dry path."}},{{"Rough side by two shells","Smooth side by two shells","Smooth side beside the runnel"}},1}});
   d.solution.clear();for(const auto& stage:d.trail){for(int n=0;n<3;++n)d.solution.push_back({V::Inspect,"clue_"+std::to_string(n)});d.solution.push_back({V::FollowTrail,"trail_"+std::to_string(stage.answer)});}
   d.objective="Inspect all three clues at each of six dune stops. Compare side, shelter and blanket threads: each wrong trail matches only part of the evidence.";
   d.hints={"Read all three clues at every stop. Each wrong path matches some evidence, but only one matches all of it.",
    "Check left or right, high or low, and dry or wet separately. A familiar landmark alone is not enough.",
    "Rule out any path that disagrees with even one clue. Recheck the side, the ground and where the blue thread caught."};
  }else if(d.quest=="beach_shell_chimes"){
   d.sequence={"star","paw","leaf","paw","star","leaf","leaf","star"};
   d.solution.clear();for(const auto& symbol:d.sequence)d.solution.push_back({V::TapSymbol,symbol});
   d.objective="Watch eight pictures on the shelter board, then play the shell chimes in order. Remember the repeated leaf near the end. Replay pictures whenever you like.";
   d.hints={"Watch all eight pictures before answering. You can replay freely, and every sound has a matching picture.",
    "Remember two groups: star, paw, leaf, paw; then star, leaf, leaf, star. The double leaf matters.",
    "Play star, paw, leaf, paw, star, leaf, leaf, star. A different picture clears your answer for a fresh try."};
  }else if(d.quest=="beach_picnic"){
   d.width=6;d.height=5;
   d.objective="Park the three baskets off the marked approach, then place the four named mats. Button needs shade and quiet beside Mabel; Pip wants sun beside Pepper.";
   d.objects.push_back({"mat_basket","Mat basket","crate",P::Crate,{1,0},0,1,{{1,0},{1,1},{1,2},{0,2}}});
   d.objects.push_back({"path_basket","Path basket","crate",P::Crate,{1,1},0,1,{{1,1},{1,2},{0,2},{0,1}}});
   d.objects.push_back({"picnic_basket","Picnic basket","crate",P::Crate,{2,1},0,1,{{2,1},{3,1},{4,1}}});
   d.walkable={{1,1},{1,2},{2,1},{2,2}};
   d.solution.insert(d.solution.begin(),{{V::Slide,"path_basket","",Direction::South},{V::Slide,"path_basket","",Direction::West},{V::Slide,"path_basket","",Direction::North},
    {V::Slide,"mat_basket","",Direction::South},{V::Slide,"mat_basket","",Direction::South},{V::Slide,"mat_basket","",Direction::West},
    {V::Slide,"picnic_basket","",Direction::East},{V::Slide,"picnic_basket","",Direction::East}});
   d.hints={"Baskets and mats cannot share a spot. Park baskets off the marked approach before checking the four cats' clues.",
    "Path basket goes forward, left, back into side storage. Mat basket can then go forward twice and left. Move Picnic basket right twice.",
    "With baskets parked: Button (1,0), Mabel (2,0), Pip (3,0), Pepper (3,1). Keep the marked approach empty, then check everyone's places."};
  }
 }
}
void appendBeachQuests(std::vector<Definition>& all){
    auto bridge=make(Family::Planks,"beach_button_bridge","beach-button-bridge-v1","A little bridge for Button",
        "Turn and fit three driftwood planks between the supports. Build Button a dry route across the shallow runnel.",6,4);
    bridge.source={0,2};bridge.destination={4,1};
    bridge.objects={{"long_plank","Long driftwood plank","plank",P::Plank,{0,0},1,2},
        {"step_plank","Short turning plank","plank",P::Plank,{2,0},0,1},
        {"finish_plank","Shelter plank","plank",P::Plank,{4,0},1,2}};
    bridge.sockets={{"long_socket","Runnel crossing","plank_socket",{0,2},{2,2},2,0},
        {"step_socket","Turn into the dunes","plank_socket",{2,2},{2,1},1,1},
        {"finish_socket","Quiet nook crossing","plank_socket",{2,1},{4,1},2,0},
        {"unsupported_socket","Loose sand without supports","plank_socket",{0,2},{0,0},2,1,false}};
    for(auto pair:{std::pair{"long_plank","long_socket"},std::pair{"step_plank","step_socket"},std::pair{"finish_plank","finish_socket"}}){bridge.solution.push_back({V::PickUp,pair.first});bridge.solution.push_back({V::Rotate,pair.first});bridge.solution.push_back({V::Fit,pair.first,pair.second});}
    bridge.solution.push_back({V::Connect});
    bridge.hints={"A long plank spans two steps. Every end needs a firm support; loose sand is not a landing.","Cross the runnel, turn toward the dunes, then cross to Button's quiet nook.","Turn each plank once. Fit Long to Runnel, Short to Turn, Shelter to Quiet nook. Check the bridge."};
    all.push_back(std::move(bridge));

    auto shade=make(Family::Shade,"beach_mabel_shade","beach-mabel-shade-v1","Mabel's shady corner",
        "Turn the two shade panels and fit them between firm supports. Cover all four marked mat cells, then check the shade.",5,5);
    shade.objects={{"front_panel","Front shade panel","shade_panel",P::Plank,{0,4},1,2},
        {"rear_panel","Rear shade panel","shade_panel",P::Plank,{3,4},0,2}};
    shade.sockets={{"front_support","Front canopy supports","shade_socket",{1,2},{3,2},2,0},
        {"rear_support","Rear canopy supports","shade_socket",{1,1},{3,1},2,0},
        {"loose_support","Loose dune posts","shade_socket",{0,0},{0,2},2,1,false}};
    shade.shade={{1,1},{2,1},{1,2},{2,2}};shade.destination={3,2};
    shade.solution={{V::PickUp,"front_panel"},{V::Rotate,"front_panel"},{V::Fit,"front_panel","front_support"},
        {V::PickUp,"rear_panel"},{V::Fit,"rear_panel","rear_support"},{V::Connect}};
    shade.hints={"The cream outlines mark the four cells of Mabel's mat. Each panel shades the two cells between its supports.","Both panels must run across the mat. The posts in loose sand cannot support a canopy.","Turn Front once and fit it to Front canopy supports. Fit Rear to Rear canopy supports, then check the shade."};
    all.push_back(std::move(shade));

    auto trail=make(Family::Scent,"beach_pepper_basket","beach-pepper-basket-v1","Pepper's missing picnic basket",
        "Inspect all three clues at each dune stop. Follow the one trail that matches the prints, blanket threads and picnic scent.",7,4);
    trail.destination={6,2}; // Requester apron, clear of every clue and choice.
    for(int n=0;n<3;++n){trail.objects.push_back({"clue_"+std::to_string(n),std::array{"Paw prints","Blanket threads","Picnic scent"}[n],"trail_clue",P::Marker,{n*2+1,0}});trail.objects.push_back({"trail_"+std::to_string(n),"Dune trail "+std::to_string(n+1),"trail_choice",P::Marker,{n*2+1,3}});}
    trail.trail={
      {{{"Fresh prints turn toward a wooden post.","Blue blanket thread caught on ROPE.","The picnic scent is sheltered from the sea."}},{{"Open shoreline rocks","Sheltered rope post","Sheltered smooth stone"}},1},
      {{{"Prints pass LEFT of a pair of dune hummocks.","Blue thread is caught on low grass.","The scent follows dry sand, away from the runnel."}},{{"Left dry grassy path","Right dry grassy path","Left wet runnel path"}},0},
      {{{"The prints circle a small striped shelter.","Blue thread hangs from its rear support.","The picnic scent is strongest behind the shelter."}},{{"Front of striped shelter","Behind a plain shelter","Behind striped shelter"}},2},
      {{{"Small prints stop at a woven basket.","Blue blanket thread crosses the lid.","The basket rests beside a shell, in the shade."}},{{"Shaded shell-side basket","Sunny shell-side basket","Shaded grass-side basket"}},0}};
    for(const auto& stage:trail.trail){for(int n=0;n<3;++n)trail.solution.push_back({V::Inspect,"clue_"+std::to_string(n)});trail.solution.push_back({V::FollowTrail,"trail_"+std::to_string(stage.answer)});}
    trail.hints={"All three clues must agree. A wrong trail keeps the clues you have already inspected.","Compare shelter, left/right position and the blanket thread, as well as the picnic scent.","Rule out any path that disagrees with even one clue. Recheck the side, the ground and where the blue thread caught."};
    all.push_back(std::move(trail));

    auto chimes=pictureSequence("beach_shell_chimes","beach-shell-chimes-v1","The shell-chime melody",
        {{"leaf","Leaf-marked shell","bell_leaf",P::Bell,{0,0},0,1,{}, {},"leaf"},
         {"star","Star-marked shell","bell_star",P::Bell,{1,0},0,1,{}, {},"star"},
         {"paw","Paw-marked shell","bell_paw",P::Bell,{2,0},0,1,{}, {},"paw"}},
        {"star","paw","leaf","paw","star","leaf"},1);
    chimes.objective="Watch six pictures on the beach shelter's board. Play the marked shell chimes in the same order; pictures carry every sound clue.";
    chimes.hints={"Watch the whole picture sequence before playing. Replay pictures is always available.","The first three pictures are star, paw, leaf. Listen if you like; the pictures give the same clue.","Play star, paw, leaf, paw, star, leaf. A different picture clears the answer so you can try again."};
    all.push_back(std::move(chimes));

    auto picnic=make(Family::Picnic,"beach_picnic","beach-picnic-v1","Everyone has a place",
        "Button's star mat needs shade and quiet; Mabel's leaf mat goes beside him in shade. Pip's paw mat wants sun, with Pepper's paw-and-star mat beside it. Keep the approach clear.",5,4);
    picnic.beachPicnic=true;
    picnic.objects={{"button_mat","Button's star mat","picnic_bed",P::Placement,{0,3}},
        {"mabel_mat","Mabel's leaf mat","picnic_bed",P::Placement,{1,3}},
        {"pip_mat","Pip's paw mat","picnic_bed",P::Placement,{3,3}},
        {"pepper_mat","Pepper's paw-and-star mat","picnic_bed",P::Placement,{4,3}}};
    for(int y=0;y<3;++y)for(int x=1;x<=3;++x)picnic.sockets.push_back({"spot_"+std::to_string(x)+"_"+std::to_string(y),"Picnic spot ("+std::to_string(x)+","+std::to_string(y)+")","picnic_spot",{x,y}});
    picnic.shade={{1,0},{2,0},{1,1}};picnic.quiet={{1,0}};picnic.walkable={{2,1},{2,2}};
    picnic.solution={{V::Fit,"button_mat","spot_1_0"},{V::Fit,"mabel_mat","spot_2_0"},
        {V::Fit,"pip_mat","spot_3_0"},{V::Fit,"pepper_mat","spot_3_1"},{V::Connect}};
    picnic.hints={"Each cat has a named mat. Shade and quiet have different markers. Mats must share an edge to count as beside each other.","Button's only quiet shaded place is rear-left. Mabel can sit beside him in shade; leave the middle approach clear.","Button at (1,0), Mabel at (2,0), Pip at (3,0), Pepper at (3,1). Check everyone's places, then watch the beach ending."};
    all.push_back(std::move(picnic));
}

// Appended IDs keep all original journals and the earned picnic postcard stable.
void appendBeachOutings(std::vector<Definition>& all,bool expanded){
    using D=Direction;
    auto crossing=make(Family::Planks,"beach_tidepool_crossing","beach-tidepool-crossing-v1","Button's tidepool expedition",
        "Help Button reach the cushion across the tidepool. Tap a basket or plank to see its moves. Clear the landings, then build around the pool. Tap a selected plank again to rotate; drag to a white gap to fit.",8,7);
    crossing.source={0,4};crossing.destination={6,2};
    crossing.objects={{"entry","Entry plank","plank",P::Plank,{0,0},0,1},
      {"west","West long plank","plank",P::Plank,{1,0},1,2},
      {"turn","Tidepool turn","plank",P::Plank,{3,0},0,1},
      {"middle","Middle long plank","plank",P::Plank,{4,0},1,2},
      {"step","Shell-side step","plank",P::Plank,{6,0},1,1},
      {"finish","Lookout plank","plank",P::Plank,{7,0},1,1},
      {"landing","Pool-side basket","crate",P::Crate,{2,3},0,1,{{2,3},{2,4},{2,5}}},
      {"parking","Middle basket","crate",P::Crate,{2,4},0,1,{{2,4},{3,4},{4,4}}},
      {"supply","Outer basket","crate",P::Crate,{3,4},0,1,{{3,4},{4,4},{5,4}}}};
    crossing.sockets={{"entry_socket","Entry step","plank_socket",{0,4},{0,3},1,1},
      {"west_socket","West crossing","plank_socket",{0,3},{2,3},2,0},
      {"turn_socket","Turn beside the pool","plank_socket",{2,3},{2,2},1,1},
      {"middle_socket","Middle crossing","plank_socket",{2,2},{4,2},2,0},
      {"step_socket","Shell-side crossing","plank_socket",{4,2},{5,2},1,0},
      {"finish_socket","Lookout crossing","plank_socket",{5,2},{6,2},1,0},
      {"wide_a","Direct wide gap one","plank_socket",{0,4},{2,4},2,0},
      {"wide_b","Direct wide gap two","plank_socket",{2,4},{4,4},2,0},
      {"wide_c","Direct wide gap three","plank_socket",{4,4},{6,4},2,0},
      {"loose","Loose tidepool edge","plank_socket",{6,4},{6,2},2,1,false}};
    crossing.solution={{V::Slide,"supply","",D::East},{V::Slide,"supply","",D::East},
      {V::Slide,"parking","",D::East},{V::Slide,"landing","",D::South}};
    for(auto id:{"entry","west","turn","middle","step","finish"}){
      crossing.solution.push_back({V::PickUp,id});crossing.solution.push_back({V::Rotate,id});
      crossing.solution.push_back({V::Fit,id,std::string(id)+"_socket"});}
    crossing.solution.push_back({V::Connect});
    crossing.hints={"Start with the outer basket, nearest the cushion. Tap a basket: its highlighted spaces show where it can move. The straight route needs more long planks than you have.",
      "Move the outer basket two spaces toward the cushion. Move the middle basket one space the same way. Slide the pool-side basket into the space the middle basket left. Build around the pool.",
      "Turn all six planks once. Fit Entry, West, Turn, Middle, Shell-side and Lookout to their matching crossings, then check the route."};
    all.push_back(std::move(crossing));

    auto water=*std::find_if(all.begin(),all.end(),[](const auto& d){return d.quest=="mabel_friend_water";});
    water.quest="beach_fresh_water";water.content="beach-fresh-water-v1";water.title="Fresh water around the dunes";
    water.objective="Clear the shared basket parking, fill the catch basin, then turn the second route toward Mabel's bowl. Keep the spare mat dry.";
    auto& p=*water.legacy;p.questId=water.quest;p.contentId=water.content;p.title=water.title;p.objective=water.objective;
    for(auto& o:p.objects)if(sliding(o.kind)){o.kind=Kind::Crate;o.name=o.id=="pot_a"?"Channel basket":"Bend basket";}
    p.objects.push_back({"supply_basket","Basin supply basket",Kind::Crate,{1,2},0,{{1,2},{0,2},{0,1}}});
    p.solution.insert(p.solution.begin(),{{Action::Move,"supply_basket",D::West},{Action::Move,"supply_basket",D::North}});
    water.solution.insert(water.solution.begin(),{{V::Slide,"supply_basket","",D::West},{V::Slide,"supply_basket","",D::North}});
    water.hints={"The supply basket blocks the channel basket's parking space. Water pauses safely in the basin; it needs a second release.",
      "Supply left then back; Channel back twice; Bend left. Turn the first four gutters once and release the barrel into the basin.",
      "After filling the basin, turn the four remaining gutters once. The first bend must point away from the spare mat. Release the basin to fill Mabel's bowl."};
    for(int n=0;n<3;++n)p.hints[n]=water.hints[n];all.push_back(std::move(water));

    auto shade=make(Family::Shade,"beach_windbreak","beach-windbreak-v1","A sheltered shell workshop",
      "Make a shaded windbreak for Button and Mabel. Turn six panels to cover every marked cell; some firm supports cover sand outside the workshop.",8,7);
    shade.destination={7,3};
    const char* names[]={"Left rear","Left front","Middle rear","Middle front","Right rear","Right front"};
    for(int n=0;n<6;++n){const auto id="panel_"+std::to_string(n);const int x=1+(n/2)*2,y=1+(n%2)*2;
      shade.objects.push_back({id,std::string(names[n])+" panel","shade_panel",P::Plank,{1+(n/2)*2,5+n%2},0,2});
      shade.sockets.push_back({"support_"+std::to_string(n),std::string(names[n])+" supports","shade_socket",{x,y},{x,y+2},2,1});
      shade.shade.push_back({x,y});shade.shade.push_back({x,y+1});
      shade.solution.push_back({V::PickUp,id});shade.solution.push_back({V::Rotate,id});shade.solution.push_back({V::Fit,id,"support_"+std::to_string(n)});}
    shade.sockets.push_back({"outside_a","Firm supports beyond the workshop","shade_socket",{7,0},{7,2},2,1});
    shade.sockets.push_back({"outside_b","Firm supports on bare sand","shade_socket",{0,1},{0,3},2,1});
    shade.sockets.push_back({"loose","Loose posts by the sea","shade_socket",{0,0},{2,0},2,0,false});
    shade.solution.push_back({V::Connect});
    shade.hints={"Count twelve marked workshop cells. Six panels cover two cells each; firm supports alone do not guarantee useful coverage.",
      "The workshop has three narrow strips, each four cells long. Cover both halves of every strip. Panels must run toward the sea.",
      "Turn every panel once. Fit Left rear/front, Middle rear/front and Right rear/front to the matching supports. Leave the two outside supports empty."};
    if(expanded){
      shade.content="beach-windbreak-v2";shade.height=9;
      shade.objective="Shade all sixteen marked workshop cells with eight panels. Choose between crossing support pairs: overlaps waste shade and can leave a corner uncovered. Tap a panel to see fits; tap again to turn it.";
      shade.objects.clear();shade.sockets.clear();shade.shade.clear();shade.solution.clear();
      // An irregular domino cover with one solution, independent of panel names.
      // The six alternate pairs are individually supported but strand other cells.
      const Cell cells[]={{0,0},{1,0},{3,0},{1,1},{2,1},{3,1},{4,1},{1,2},{2,2},{3,2},{4,2},{1,3},{2,3},{0,4},{1,4},{2,4}};
      for(auto c:cells)shade.shade.push_back({c.x+1,c.y+1});
      const Cell starts[]={{0,4},{2,3},{1,2},{0,0},{1,1},{3,0},{4,1},{2,2}};
      const int directions[]={0,1,1,0,0,1,1,0};
      // Store loose panels in parallel rows so no cloth hides another grab point.
      const int initial[]={0,0,0,0,0,0,0,0};
      const char* supports[]={"Front-left edge","Front elbow","Inner-left upright","Rear-left edge","Upper crossbar","Rear upright","Right upright","Middle crossbar"};
      for(int n=0;n<8;++n){
        const auto id="panel_"+std::to_string(n),socket="support_"+std::to_string(n);
        const Cell a{starts[n].x+1,starts[n].y+1},b{a.x+(directions[n]?0:2),a.y+(directions[n]?2:0)};
        shade.objects.push_back({id,"Workshop panel "+std::to_string(n+1),"shade_panel",P::Plank,{(n%4)*2,7+n/4},initial[n],2});
        shade.sockets.push_back({socket,supports[n],"shade_socket",a,b,2,directions[n]});
        shade.solution.push_back({V::PickUp,id});
        if(initial[n]!=directions[n])shade.solution.push_back({V::Rotate,id});
        shade.solution.push_back({V::Fit,id,socket});
      }
      const Cell alternatives[]={{1,1},{2,1},{2,1},{3,1},{3,1},{1,2}};
      const int alternateDirections[]={1,1,0,0,1,0};
      const char* alternateNames[]={"Left crossing upright","Centre crossing upright","Upper inner crossing","Upper right crossing","Right crossing upright","Lower left crossing"};
      for(int n=0;n<6;++n){const Cell a{alternatives[n].x+1,alternatives[n].y+1};const int v=alternateDirections[n];
        shade.sockets.push_back({"alternative_"+std::to_string(n),alternateNames[n],"shade_socket",a,{a.x+(v?0:2),a.y+(v?2:0)},2,v});}
      shade.solution.push_back({V::Connect});
      shade.hints={"Eight panels cover exactly sixteen cells. A supported fit may still be the wrong choice. Start with the outer cells that have only one partner; do not spend two panels shading the same cell.",
        "Cover the rear-left and front-left edges across. Their neighbours force two upright panels up the left and front elbow. Keep the upper middle pair free for a crossbar.",
        "Use Front-left edge, Front elbow, Inner-left upright, Rear-left edge, Upper crossbar, Rear upright, Right upright and Middle crossbar. Any panel works; match its orientation. Leave the six crossing alternatives empty."};
    }
    all.push_back(std::move(shade));

    auto song=pictureSequence("beach_shell_encore",expanded?"beach-shell-encore-v2":"beach-shell-encore-v1","Pepper's answering shell song",
      {{"leaf","Leaf shell","bell_leaf",P::Bell,{0,0},0,1,{}, {},"leaf"},
       {"star","Star shell","bell_star",P::Bell,{1,0},0,1,{}, {},"star"},
       {"paw","Paw shell","bell_paw",P::Bell,{2,0},0,1,{}, {},"paw"}},
      expanded?std::vector<std::string>{"leaf","paw","star","leaf","paw","leaf","star","paw"}:std::vector<std::string>{"leaf","paw","star","star","paw","leaf","paw","leaf","star","star","leaf","paw"},.85);
    song.objective="Watch two answering phrases of six pictures. The first phrase folds back on itself; the second swaps leaf and paw. Play all twelve shells in order.";
    song.hints={"Replay the board freely. Both six-picture phrases have two stars in the middle and mirror their first three pictures.",
      "First phrase: leaf, paw, star, star, paw, leaf. The answer swaps every leaf and paw while keeping both stars.",
      "Play leaf, paw, star, star, paw, leaf; then paw, leaf, star, star, leaf, paw."};
    if(expanded){
      song.objective="Watch two answering phrases of four pictures. The second swaps leaf and paw while keeping the star. Play all eight shells in order.";
      song.hints={"Remember two groups of four pictures. Replay the board whenever you like.",
        "First: leaf, paw, star, leaf. The answer swaps leaf and paw, keeping the star.",
        "Play leaf, paw, star, leaf; then paw, leaf, star, paw."};
    }
    all.push_back(std::move(song));

    auto balance=*std::find_if(all.begin(),all.end(),[](const auto& d){return d.quest=="pip_wobbly_perch";});
    balance.quest="beach_lantern_balance";balance.content="beach-lantern-balance-v1";balance.title="Pip's lantern landing";
    balance.objective="Balance four numbered lanterns for the cove. The support and Pip's position change at each of four landings; weight times distance must balance.";
    // Mirror each authored equilibrium and change the order: new loads retain
    // exact integer torque while needing a fresh arrangement at every stop.
    balance.balances={{3,2,3},{4,3,-3},{5,2,-2},{4,0,0}};
    const int placements[4][4]={{7,3,1,2},{7,2,6,5},{5,6,3,7},{7,3,1,6}};
    balance.solution.clear();for(auto& row:placements){for(int n=0;n<4;++n)balance.solution.push_back({V::Fit,"weight_"+std::to_string(n+1),"beam_"+std::to_string(row[n])});balance.solution.push_back({V::TestBalance});}
    for(auto& o:balance.objects)if(o.role=="balance_weight")o.name=std::to_string(o.length)+"-weight seaside lantern";
    for(auto& s:balance.sockets)if(s.role=="balance_home")s.name="Lantern "+s.id.substr(5)+" storage";
    balance.hints={"All four lanterns need different beam positions. Multiply each lantern's weight by its distance from the support; include Pip's pull too.",
      "The support moves: positions 3, 4, 5, then 4. Pip pulls right, left, left, then steps off. Test each balanced landing before arranging the next.",
      "Lanterns 1,2,3,4: use 7,3,1,2; then 7,2,6,5; then 5,6,3,7; finally 7,3,1,6. Test after each arrangement."};
    all.push_back(std::move(balance));
}
}
