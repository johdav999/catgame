#pragma once
#include "release1/contracts.hpp"
#include <algorithm>

namespace paws::release1::audio {
// Only accepted before/after projections enter here. No rule or reward ownership.
inline std::vector<CommittedEvent> effects(const PuzzleView& before,const PuzzleView& after,const Intent& intent){
    std::vector<CommittedEvent> out;
    auto add=[&](std::string cue,std::string object={}){out.push_back({after.generation,after.attempt,after.revision,after.quest,object.empty()?intent.object:std::move(object),std::move(cue)});};
    const auto piece=std::find_if(after.objects.begin(),after.objects.end(),[&](auto& o){return o.id==intent.object;});
    const std::string_view role=piece==after.objects.end()?std::string_view{}:piece->role;
    using V=Verb;
    switch(intent.verb){
        case V::Select:add("ui_select");break;
        case V::Inspect:case V::FollowTrail:case V::TestBalance:add("ui_select");break;
        case V::Slide:add(role=="blocking_cushion"?"cushion_move":after.quest=="button_mouse_bench"?"crate_slide":"pot_slide");break;
        case V::Rotate:add(after.quest=="mabel_sunny_spot"?"plank_place":after.quest=="button_cushion_bed"?"cushion_move":"channel_rotate");break;
        case V::PickUp:case V::Retrieve:add(after.quest=="mabel_sunny_spot"?"plank_place":after.quest=="button_cushion_bed"?"cushion_move":"toy_pickup");break;
        case V::Fit:add(after.quest=="mabel_sunny_spot"?"plank_place":after.quest=="button_cushion_bed"?"shape_fit":after.quest=="garden_picnic"?(intent.object=="bowl"?"bowl_place":"furnishing_place"):"basket_place");break;
        case V::Connect:if(after.quest=="mabel_sunny_spot")add("plank_place");break;
        case V::TapSymbol:add("bell_"+intent.object);if(after.enteredSymbols.empty())add("bell_reset");break;
        case V::Hint:if(after.revealedHints>before.revealedHints)add("ui_hint");break;
        case V::Undo:add("ui_undo");break;
        case V::Buy:add("shop_purchase");break;
        case V::PlaceFurnishing:add(intent.object=="drinking_bowl"?"bowl_place":"furnishing_place");break;
        default:break;
    }
    for(const auto& o:after.objects){
        const auto previous=std::find_if(before.objects.begin(),before.objects.end(),[&](auto& old){return old.id==o.id;});
        if(previous==before.objects.end()||previous->active==o.active)continue;
        if(o.role=="switch"||o.role.starts_with("switch_"))add(o.active?"switch_down":"switch_up",o.id);
        if(o.role=="gate"||o.role=="playhouse_door")add(o.active?"gate_open":"gate_close",o.id);
    }
    return out;
}
}
