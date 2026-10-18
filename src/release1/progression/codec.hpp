#pragma once
#include "owner.hpp"
#include <optional>
namespace paws::release1::progression {
struct SaveCandidate {
    std::optional<Afternoon> value;
    std::string original,error;
};
// Preserves caller-provided bytes even on future/corrupt input; publication is explicit.
SaveCandidate inspectSave(std::string bytes);
}
