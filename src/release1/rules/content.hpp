#pragma once
#include "puzzles.hpp"
namespace paws::release1::rules {
std::vector<Definition> catalogue();
void appendBeachQuests(std::vector<Definition>&);
Definition quest(std::string_view id,std::string_view content={});
Definition pictureSequence(std::string quest,std::string content,std::string title,
    std::vector<Object> responses,std::vector<std::string> pictures,double pictureSeconds=1);
}
