#pragma once
#include "engine/math.hpp"
#include <span>
namespace paws {
// Game presentation utility: distance-based traversal of authored route samples.
// No actor ownership, clocks, quest rules or saved state.
struct RouteSample { engine::Vec3 position; bool finished=true; };
inline RouteSample sampleRoute(std::span<const engine::Vec3> points,float distance) {
    if(points.empty()) return {};
    distance=std::max(0.f,distance);
    for(std::size_t i=1;i<points.size();++i) {
        const auto d=points[i]-points[i-1];const float length=std::sqrt(engine::dot(d,d));
        if(length>0 && distance<length) return {points[i-1]+d*(distance/length),false};
        distance-=length;
    }
    return {points.back(),true};
}
}
