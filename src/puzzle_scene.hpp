#pragma once
#include "puzzle.hpp"
#include "engine/scene.hpp"
#include "engine/camera_component.hpp"
#include "garden_kit.hpp"
#include <array>
namespace paws {
struct ObjectBinding { ObjectId object; engine::ActorId actor; std::array<engine::ActorId,2> arms; };
struct SceneBindings {
    AttemptId attempt=0;
    Revision revision=0;
    std::vector<ObjectBinding> objects;
    engine::ActorId ball, selection;
    struct Motion { engine::ActorId actor; engine::Vec3 from,to,rotationFrom,rotationTo; };
    std::vector<Motion> motion;
    float elapsed=0;
    std::string questId;
    bool production=false;
    bool garden=false;
    engine::ComponentId cat;
    engine::ActorId gardenCat;
    std::array<engine::ActorId,2> furnishings, furnishingPreviews;
    bool completed=false;
    bool reducedMotion=false;
    bool ballAtDestination=false;
    std::vector<engine::Vec3> ballRoute;
    float ballElapsed=0;
};
struct PuzzleScene {
    std::unique_ptr<engine::Scene> scene;
    engine::CameraComponent* camera=nullptr;
    SceneBindings bindings;
};
PuzzleScene makePuzzleScene(const PawsSession& session,const GardenMeshes* meshes=nullptr);
// Reject old attempt projections and foreign/deleted handles; no retained actor pointers.
bool projectPuzzle(engine::Scene&, SceneBindings&, const PawsSession&, bool animate=false);
void animatePuzzle(engine::Scene&, SceneBindings&, float delta, bool reducedMotion);
}
