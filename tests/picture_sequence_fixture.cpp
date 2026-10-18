// Build a legal Chapter 3 snapshot for isolated browser acceptance.
#include "afternoon.hpp"
#include "release1/progression/policy.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
int main(int argc,char** argv){try{
    if(argc!=2)throw std::runtime_error("Expected fixture output path");
    paws::Afternoon a;
    for(std::size_t q=0;q<7;++q){
        const auto id=std::string(paws::release1::progression::quests[q].id);
        auto r=a.dispatchIntent(a.intent(paws::release1::Verb::SelectQuest,{},id));if(!r.accepted)throw std::runtime_error(r.message);
        for(const auto& s:paws::release1::rules::quest(id).solution){auto i=a.intent(s.verb,s.object,s.target);i.direction=s.direction;i.destination=s.destination;
            r=a.dispatchIntent(i);if(!r.accepted)throw std::runtime_error(r.message);}
    }
    for(auto id:{"button","mabel","pepper"}){auto r=a.introduce(id,a.intent(paws::release1::Verb::Select));if(!r.accepted)throw std::runtime_error(r.message);}
    auto r=a.dispatchIntent(a.intent(paws::release1::Verb::SelectQuest,{},"pepper_garden_bells"));if(!r.accepted)throw std::runtime_error(r.message);
    a.settings.reducedMotion=true;a.settings.muted=true;
    const auto bytes=a.encode();auto restored=paws::Afternoon::decode(bytes);
    if(restored.puzzleView().quest!="pepper_garden_bells"||restored.coins()!=140)throw std::runtime_error("Invalid chapter fixture");
    auto p=std::filesystem::path(argv[1]);std::filesystem::create_directories(p.parent_path());std::ofstream out(p);out<<bytes;if(!out)throw std::runtime_error("Fixture write failed");
    std::cout<<"Legal Chapter 3 sequence fixture saved\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
