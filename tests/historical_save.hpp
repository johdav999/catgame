#pragma once
#include <stdexcept>
#include <string>
namespace paws::tests {
// Synthetic historical layout tests must declare the old format they model.
// A current envelope with contradictory kernel/content records must reject.
inline std::string version12Envelope(std::string bytes) {
    const auto at=bytes.find("QUEST_KERNEL ");
    if(at!=std::string::npos)bytes.erase(at,bytes.find("END\n",at)-at);
    const auto end=bytes.find('\n');if(end==std::string::npos)throw std::invalid_argument("Missing fixture header");
    bytes.replace(0,end,"PAWS_AFTERNOON 12");return bytes;
}
}
