#include "starter_scene.hpp"
#include "puzzle_scene.hpp"
#include "engine/application.hpp"
#include <cstdlib>
#include <iostream>

int main() {
    auto starter = paws::makeStarterScene();
    if(!starter.scene || !starter.camera) return EXIT_FAILURE;
    engine::Application application;
    const auto id = application.loadScene(std::move(starter.scene));
    application.start(id);
    application.advance(0);
    application.advance(1.0/60.0);
    auto* scene = application.activeScene();
    const auto frame = scene->collect();
    if(frame.meshes.size() != 7 || frame.lights.size() != 1) {
        std::cerr << "Starter room did not publish its expected meshes/lights\n";
        return EXIT_FAILURE;
    }
    if(scene->physicsIfCreated()) {
        std::cerr << "The visual starter scene initialized optional physics\n";
        return EXIT_FAILURE;
    }
    const auto projected = starter.camera->worldToScreen({0,0.5f,0}, {0,0,800,600});
    if(projected.pixel.x < 390 || projected.pixel.x > 410 ||
       projected.pixel.y < 290 || projected.pixel.y > 310) {
        std::cerr << "Camera does not frame the starter room\n";
        return EXIT_FAILURE;
    }
    application.shutdown();
    paws::PawsSession session(paws::missingBall());
    auto puzzle=paws::makePuzzleScene(session);
    auto bindings=puzzle.bindings;
    auto other=paws::makePuzzleScene(session);
    if(paws::projectPuzzle(*other.scene,bindings,session)) return EXIT_FAILURE;
    engine::Application puzzleApp;
    const auto puzzleId=puzzleApp.loadScene(std::move(puzzle.scene));
    puzzleApp.start(puzzleId); puzzleApp.advance(0); puzzleApp.advance(0.02);
    auto* puzzleScene=puzzleApp.activeScene();
    if(puzzleScene->physicsIfCreated() || puzzleScene->collect().meshes.size()<30) return EXIT_FAILURE;
    for(const auto& step:session.content().solution) {
        if(!session.dispatch({session.state().attempt,session.state().revision,step.action,step.object,step.direction}).accepted ||
           !paws::projectPuzzle(*puzzleScene,bindings,session)) return EXIT_FAILURE;
    }
    const auto ballPosition=puzzleScene->findActor(bindings.ball)->transform.position;
    if(ballPosition.x<2.8f || ballPosition.z> -1.4f) return EXIT_FAILURE;
    puzzleApp.pause(); puzzleApp.advance(1);
    puzzleApp.setSuspended(true); puzzleApp.advance(100);
    puzzleApp.setSuspended(false); puzzleApp.advance(101);
    paws::PawsSession older(paws::missingBall());
    if(paws::projectPuzzle(*puzzleScene,bindings,older)) return EXIT_FAILURE;
    puzzleApp.shutdown();
    for(auto content:{paws::tidyToys(),paws::stuckGate()}) {
        paws::PawsSession current(content);
        auto view=paws::makePuzzleScene(current);
        auto oldBindings=view.bindings;
        for(const auto& step:content.solution) {
            if(!current.dispatch({current.state().attempt,current.state().revision,step.action,step.object,step.direction,step.target}).accepted||
               !paws::projectPuzzle(*view.scene,view.bindings,current,true)) return EXIT_FAILURE;
            paws::animatePuzzle(*view.scene,view.bindings,.02f,false); // Rapid commits replace presentation destinations.
        }
        paws::animatePuzzle(*view.scene,view.bindings,0,true);
        if(!view.bindings.motion.empty()||!current.state().puzzle.completed) return EXIT_FAILURE;
        auto replacement=paws::makePuzzleScene(current);
        if(paws::projectPuzzle(*replacement.scene,oldBindings,current)) return EXIT_FAILURE;
        paws::animatePuzzle(*replacement.scene,oldBindings,.3f,false); // Stale handles cannot animate a replacement scene.
        view.scene->findActor(view.bindings.objects[0].actor)->destroy();
        if(paws::projectPuzzle(*view.scene,view.bindings,current)) return EXIT_FAILURE;
    }
    std::cout << "Paws starter and puzzle scenes, lifecycle, camera and checked bindings passed\n";
}
