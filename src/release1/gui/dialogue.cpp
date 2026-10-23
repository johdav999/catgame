#include "views.hpp"
namespace paws::release1::gui {
const std::array<QuestCopy,29>& dialogue() {
    static const std::array<QuestCopy,29> copy{{
      {"pip_missing_ball","Pip","My ball slipped behind the pots. Could you make a little path back to me?","There it is! Pip nudges his favourite ball toward you."},
      {"pip_tidy_toys","Pip","The toys have wandered out of their basket. Shall we find each one a cosy place?","All tucked away. Pip gives the tidy basket a pleased little sniff."},
      {"pip_stuck_gate","Pip","The gate needs a steady weight on its switch. My toy is waiting on the other side.","The gate is open and the toy is safe. Pip gives you a slow blink."},
      {"button_mouse_bench","Button","My little mouse is under the bench. Clear the near lane so we can reach it gently.","Button nestles beside his mouse. That was just the right reach."},
      {"mabel_sunny_spot","Mabel","That sunny perch looks lovely. Could you fit a supported path all the way across?","Mabel settles in the warm spot. Every step felt safe."},
      {"button_cushion_bed","Button","A square, a bar and a little elbow: can they fill my basket without squashing each other?","A perfect fit. Button pats his new bed and settles down."},
      {"mabel_drinking_route","Mabel","A cool drink would be nice. Turn the gutters so the rain barrel can fill my bowl.","The bowl is full. Mabel takes a happy little sip."},
      {"pepper_garden_bells","Pepper","Watch my blackboard: five pictures, one at a time. Then play the bells in that order! Tap the board whenever you want another look.","Pepper beams at your remembered melody. A lovely garden tune."},
      {"pip_pepper_playhouse","Pip and Pepper","We both want to get inside, but someone has to hold each switch. Help us find a way to keep the doorway open.","A crate, a latch, and a little teamwork. Pip and Pepper are cosy inside together."},
      {"garden_picnic","All four cats","Button likes shade and quiet. Mabel wants her bowl beside his bed, sharing an edge in the shade.","Everything has a comfortable place. The four friends are ready for a picnic."},
      {"pepper_scent_trail","Pepper","My catnip pouch went on a little adventure. Follow six trail stops, comparing paw prints, fluff and scent at every turn.","There it is, in the leaf cushion! Pepper nuzzles her safely recovered pouch."},
      {"pip_wobbly_perch","Pip","My perch wobbles! Try four cushion arrangements as the support and my weight change. A heavy cushion close in can balance a light one farther out.","Four steady setups! Pip settles on his carefully balanced perch."},
      {"pip_friend_ball","Pip","Pip is waiting beside the planters, watching a gap between the pots. He notices you, then looks back at his ball. He has not quite decided whether to ask. 'Could you get it? The little tray will catch it halfway.' He makes a little space beside himself while you look.","You found it! ...Would you like a turn?"},
      {"pip_friend_view","Pip","This time, Pip comes looking for you. He trots toward the fence, stops, and checks that you are following. 'Come and see! There is something beyond it.' The old stepping boards do not quite reach his lookout. 'Could we make a path up together?' He waits for you at the first step.","There! Beyond the fence! I knew there was something."},
      {"pip_friend_room","Pip","Pip has brought two cushions to the lookout. When you arrive, he nudges one toward you, then quickly pretends to inspect the flowerpots. 'It wobbles when I climb up. Could we make it steady?' After a moment, he adds, 'And perhaps put that other cushion beside mine?'","This one's yours. I saved it."},
      {"mabel_friend_sun","Mabel","Mabel watches a patch of sunlight above the overgrown planters. She stretches toward it, then sits back down. 'That used to be my favourite place.' Across the garden, Pip is resting on his new lookout. Mabel glances at you. 'Perhaps we could reach the window ledge too?'","Oh. It's warm here. You found it."},
      {"mabel_friend_water","Mabel","Mabel has found exactly the warmest part of her new cushion. She opens one eye when you approach. 'It is lovely here. Only... a cool drink would be nice.' A gutter ends beside the empty bowl, but another points straight at her seat. 'Could the catch basin help us bring the water around my cushion?'","Sunshine and a cool drink. Almost perfect."},
      {"mabel_friend_share","Mabel and Pip","Mabel's bowl is full and her cushion is warm, but she keeps looking across at Pip. Today, a second cushion has appeared beside her. 'I thought he might like the sunshine too.' She smooths its corner with one paw. 'Could we put them both in the light? Leave a little path between them, so he knows he can come over.'","I thought... perhaps sunshine is nicer when someone else notices it too."},
      {"beach_pip_ball","Pip","The postcard showed the perfect picnic spot. My ball found it first! Could you clear the baskets and roll it back to my mat?","There it is! Pip gives his beach ball a happy nudge. Our seaside afternoon has begun."}
      ,{"beach_button_bridge","Button","The water is only a little runnel, but I would rather keep my paws dry. Could we reach that quiet nook?","Button crosses the driftwood bridge and looks back. There is room for the others too."}
      ,{"beach_mabel_shade","Mabel","Button found a lovely nook. My mat needs a little shade before I can settle beside him.","Mabel admires her comfortably shaded mat. Now that is a pleasant corner."}
      ,{"beach_pepper_basket","Pepper","The blanket was in our picnic basket. I followed a lovely smell and lost track of where we left it!","Pepper finds the basket beside a shell. The blanket is still tucked safely inside."}
      ,{"beach_shell_chimes","Pepper","There is a tune on the shelter's picture board. Shall we play it on these marked shells?","The little melody carries across the cove. Everyone knows where to gather."}
      ,{"beach_picnic","All four cats","A bridge, some shade, our basket and a tune. All we need now is a comfortable place for everyone.","Four friends, four places, and the sea beyond. Our seaside picnic is ready."}
      ,{"beach_tidepool_crossing","Button","I can see tiny shells beside that tidepool. Could we find a dry way around it? The baskets are using each other's parking places.","Button peers into the pool from the dry landing. A whole little world, just below our paws."}
      ,{"beach_fresh_water","Mabel","Watching the tidepool has made me thirsty. There is fresh water in the barrel, but the baskets block its gutters. Could we use the basin to bring it around the dunes?","Mabel takes a cool sip. The basin did its part, and the spare mat stayed dry."}
      ,{"beach_windbreak","Mabel and Button","We would like to sort our shells together. Can you arrange the panels around our workshop's corners? Every marked patch needs shade, and overlapping panels waste it.","The whole workshop is shaded. Button and Mabel compare their shells in a comfortable little nook."}
      ,{"beach_shell_encore","Pepper","Listen to the shells answer each other! Remember the first phrase, then its answer. The second trades leaves for paws. The board will show you as often as you like.","Two answering phrases float across the cove. Pepper gives the last shell a delighted little tap."}
      ,{"beach_lantern_balance","Pip","I found four seaside lanterns for our landing. They weigh different amounts, and the support keeps moving. Could you steady each setup while I try the perch?","Every lantern is safely balanced. Pip looks over the cove: we made a lovely place together."}
    }};return copy;
}
std::string catIntroduction(std::string_view id) {
    if(id=="mabel")return "I'm Mabel. A sunny seat and a cool drink make a lovely afternoon.";
    if(id=="button")return "I'm Button. I like quiet corners, soft beds and my little toy mouse.";
    if(id=="pepper")return "I'm Pepper! Shall we try the picture bells and play together?";
    return "I'm Pip. Make yourself at home in our little garden.";
}
}
