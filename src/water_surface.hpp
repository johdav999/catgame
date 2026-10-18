#pragma once
#include "engine/mesh_component.hpp"
#include <algorithm>

namespace paws {
// Game palette and scale only; shading, waves and recovery belong to WebEngine.
// Keep the published kit meshes/materials intact for non-water instances.
inline void dressWater(engine::MeshComponent& surface,bool flowing=false) {
    surface.color={1,1,1};
    surface.castsShadow=false;
#if PAWS_WATER_SHADER
    static const auto bowl=[] {
        auto material=std::make_shared<engine::Material>();
        material->name="Paws drinking water";
        material->shader=engine::MaterialShader::Water;
        material->baseColor={.065f,.26f,.30f};
        material->roughness=.28f;
        // The flattened bowl asset retains coplanar underside triangles.
        // Cull them so opposite faces cannot compete with the visible surface.
        material->doubleSided=false;
        material->water.amplitude=.0015f;
        material->water.wavelength=.16f;
        material->water.speed=.35f;
        material->water.steepness=.08f;
        material->water.rippleScale=6;
        material->water.rippleStrength=.07f;
        material->water.reflectionStrength=.35f;
        material->water.horizon={.65f,.80f,.82f};
        material->water.zenith={.18f,.38f,.44f};
        // Tiny bowls/gutters need no canvas-sized refraction/foam captures.
        material->validate();
        return std::shared_ptr<const engine::Material>(material);
    }();
    static const auto stream=[] {
        auto material=std::make_shared<engine::Material>(*bowl);
        material->name="Paws gutter water";
        // Preserve the authored channel edges and sloping inlet ribbon.
        material->water.amplitude=0;
        material->water.speed=.6f;
        material->water.direction={1,0};
        material->validate();
        return std::shared_ptr<const engine::Material>(material);
    }();
    const auto& material=flowing?stream:bowl;
    const auto slots=std::max(std::size_t(1),surface.mesh->materials.size());
    for(std::size_t slot=0;slot<slots;++slot)
        surface.setMaterial(slot,engine::MaterialInstance{material});
#else
    (void)flowing;
#endif
}
} // namespace paws
