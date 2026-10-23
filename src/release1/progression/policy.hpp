#pragma once
#include "release1/contracts.hpp"
#include <array>
#include <span>
#include <algorithm>

namespace paws::release1::progression {
struct QuestPolicy {
    std::string_view id,content;
    int chapter;
    unsigned requesters; // Pip/Mabel/Button/Pepper bits, only for derived friendship.
    std::vector<std::string_view> prerequisites;
    std::string_view pack; // Empty for the base garden; adventure progress is independent.
};
struct ItemPolicy {
    std::string_view id,name,asset,unlock;
    int price;
    float width,depth;
    std::vector<std::string_view> roles;
};
struct SpotPolicy {
    std::string_view id;
    float width,depth;
    std::vector<std::string_view> roles;
};
inline constexpr std::size_t baseQuestCount=18, originalBeachQuestCount=6, questCount=29;
inline constexpr std::string_view beachPackId="beach_afternoon";
inline constexpr std::string_view beachQuestId="beach_pip_ball";
inline constexpr std::array<std::string_view,11> beachQuestIds{
    beachQuestId,"beach_button_bridge","beach_mabel_shade","beach_pepper_basket","beach_shell_chimes","beach_picnic",
    "beach_tidepool_crossing","beach_fresh_water","beach_windbreak","beach_shell_encore","beach_lantern_balance"};
inline bool isBeachQuest(std::string_view id){return std::find(beachQuestIds.begin(),beachQuestIds.end(),id)!=beachQuestIds.end();}
extern const std::array<QuestPolicy,questCount> quests;
extern const std::array<ItemPolicy,8> items;
extern const std::array<SpotPolicy,9> spots;
inline constexpr std::array<std::string_view,4> catIds{"pip","mabel","button","pepper"};
inline constexpr std::array<std::string_view,4> catNames{"Pip","Mabel","Button","Pepper"};
inline constexpr std::array<std::array<int,3>,4> milestones{{{1,3,5},{1,2,3},{1,2,3},{1,2,3}}};
std::size_t questIndex(std::string_view);
std::size_t itemIndex(std::string_view);
std::size_t spotIndex(std::string_view);
bool compatible(const ItemPolicy&,const SpotPolicy&);
// Validates IDs, references and DAG without assuming array/topological order.
std::string validatePolicies(std::span<const QuestPolicy>);
}
