#pragma once
#include "engine/scene.hpp"
#include "engine/camera_component.hpp"
#include <memory>

namespace paws {
// Greybox integration scene; level rules and authored assets will live in Paws.
struct StarterScene {
    std::unique_ptr<engine::Scene> scene;
    engine::CameraComponent* camera = nullptr; // Borrowed from scene.
};
StarterScene makeStarterScene();
}
