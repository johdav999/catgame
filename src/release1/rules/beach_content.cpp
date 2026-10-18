#include "content.hpp"
namespace paws::release1::rules {
namespace {
using V=Verb;using P=Piece;
Definition make(Family f,std::string id,std::string content,std::string title,std::string objective,int w,int h){
    Definition d;d.family=f;d.quest=std::move(id);d.content=std::move(content);d.title=std::move(title);d.objective=std::move(objective);d.width=w;d.height=h;return d;
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
    trail.hints={"All three clues must agree. A wrong trail keeps the clues you have already inspected.","Compare shelter, left/right position and the blanket thread, as well as the picnic scent.","Follow: sheltered rope post; left dry grassy path; behind striped shelter; shaded shell-side basket."};
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
}
