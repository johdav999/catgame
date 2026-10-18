#pragma once
#include "puzzle.hpp"
#include "engine/scene.hpp"
#include "engine/camera_component.hpp"
#include "garden_kit.hpp"
#include "cat_collision.hpp"
#include "release1/rules/picture_sequence.hpp"
#include <array>
namespace paws {
struct ObjectBinding { ObjectId object; engine::ActorId actor; std::array<engine::ActorId,2> arms; };
struct SceneBindings {
    CatCollision collision;
    AttemptId attempt=0;
    Revision revision=0;
    std::vector<ObjectBinding> objects;
    engine::ActorId ball, selection;
    struct SlideGuide {std::size_t object;Cell cell;engine::ActorId actor;};
    std::vector<SlideGuide> slideGuides;
    std::vector<engine::ActorId> movementGuides;
    struct Motion { engine::ActorId actor; engine::Vec3 from,to,rotationFrom,rotationTo; };
    std::vector<Motion> motion;
    std::vector<Motion> arrivalMotion;
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
    struct CatTravel {engine::ActorId actor;engine::ComponentId skin;std::vector<engine::Vec3> route;double elapsed=0;std::size_t segment=0;};
    std::vector<CatTravel> requesters;
    engine::ActorId benchPath;
    std::vector<engine::ActorId> butterflies;
    std::array<engine::ActorId,6> beachMilestones{};
    release1::rules::PictureSequencePlayback sequencePlayback;
    unsigned sequenceReplay=0;
    std::vector<std::pair<std::string,engine::ActorId>> sequencePictures;
    struct QuestEnding {
        bool active=false,finished=false,facingComplete=false;
        int kind=0;
        float elapsed=0,walkSeconds=0;
        engine::Vec3 home{},homeRotation{},start{},target{},toyStart{};
        engine::ActorId actor,toy,basket;
        engine::ActorId tongue;
        std::vector<engine::Vec3> route,carryRoute,flowRoute;
        std::size_t segment=0,carrySegment=0;
        float leadSeconds=0,clipSeconds=0,carrySeconds=0,finalYaw=0;
    } ending;
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
