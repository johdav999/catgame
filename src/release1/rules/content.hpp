#pragma once
#include "puzzles.hpp"
namespace paws::release1::rules {
std::vector<Definition> catalogue(bool expandedBeach=true);
void appendBeachQuests(std::vector<Definition>&);
void expandBeachQuests(std::vector<Definition>&);
void appendBeachOutings(std::vector<Definition>&,bool expanded=true);
Definition quest(std::string_view id,std::string_view content={});
Definition pictureSequence(std::string quest,std::string content,std::string title,
    std::vector<Object> responses,std::vector<std::string> pictures,double pictureSeconds=1);
}
