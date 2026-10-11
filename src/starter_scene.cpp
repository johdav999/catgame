#include "starter_scene.hpp"
#include "engine/actor.hpp"
#include "engine/mesh_component.hpp"
#include "engine/light_component.hpp"

namespace paws {
StarterScene makeStarterScene() {
    StarterScene result{std::make_unique<engine::Scene>("Paws starter room"), nullptr};
    auto& scene = *result.scene;
    const auto cube = engine::Mesh::cube();
    const auto box = [&](const char* name, engine::Vec3 position,
                         engine::Vec3 scale, engine::Vec3 color) {
        auto& actor = scene.spawn(name);
        actor.transform.position = position;
        actor.transform.scale = scale;
        actor.addComponent<engine::MeshComponent>(name, cube).color = color;
    };
    box("Room floor", {0,-0.2f,0}, {8,0.3f,8}, {0.28f,0.35f,0.4f});
    box("Puzzle block", {1.5f,0.45f,0}, {0.9f,0.9f,0.9f}, {0.68f,0.45f,0.24f});
    box("Goal marker", {1.5f,0.01f,-2}, {1.2f,0.08f,1.2f}, {0.35f,0.75f,0.6f});
    box("Cat body placeholder", {-1,0.5f,0}, {0.65f,0.7f,1.1f}, {0.94f,0.72f,0.4f});
    box("Cat head placeholder", {-1,1.04f,-0.4f}, {0.6f,0.55f,0.55f}, {0.94f,0.72f,0.4f});
    box("Cat left ear", {-1.22f,1.4f,-0.4f}, {0.16f,0.25f,0.18f}, {0.94f,0.72f,0.4f});
    box("Cat right ear", {-0.78f,1.4f,-0.4f}, {0.16f,0.25f,0.18f}, {0.94f,0.72f,0.4f});
    auto& light = scene.spawn("Lighting").addComponent<engine::LightComponent>(
        "Room sunlight", engine::LightType::Directional);
    light.direction = {-0.5f,-1,-0.3f};
    light.intensity = 2;
    auto& camera = scene.spawn("View").addComponent<engine::CameraComponent>("Puzzle camera");
    camera.transform.position = {7,7,9};
    camera.lookAt({0,0.5f,0});
    result.camera = &camera;
    return result;
}
}
