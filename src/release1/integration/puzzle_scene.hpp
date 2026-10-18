#pragma once
#include "../../puzzle_scene.hpp"
#include "release1/contracts.hpp"
namespace paws::release1::presentation {
// Seven new boards consume committed snapshots; transforms cannot grant progress.
PuzzleScene build(const PuzzleView&,const GardenMeshes&,bool reducedMotion=true);
bool project(engine::Scene&,SceneBindings&,const PuzzleView&,bool reducedMotion);
void animate(engine::Scene&,SceneBindings&,double delta);
// Transient drag guidance; never changes the authoritative attempt.
void guidePlank(engine::Scene&,const SceneBindings&,const PuzzleView&,std::string_view object);
void guideMovement(engine::Scene&,SceneBindings&,const PuzzleView&,const std::vector<Intent>&);
engine::Vec3 world(std::string_view quest,Cell,std::string_view content={});
engine::Vec3 shapePosition(std::string_view quest,std::string_view object,Cell,int orientation,std::string_view content={});
}
