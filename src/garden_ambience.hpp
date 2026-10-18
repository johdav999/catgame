#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace paws {
// Presentation time only. Inactive gardens freeze the countdown; no catch-up burst.
class GardenBirdSchedule {
public:
    void reset() { remaining_=18; }
    bool advance(double delta, bool audibleGarden) {
        if(!audibleGarden || !std::isfinite(delta) || delta<=0)return false;
        remaining_-=std::min(delta,.25);
        if(remaining_>0)return false;
        random_^=random_<<13;random_^=random_>>17;random_^=random_<<5;
        remaining_=18+double(random_%19001)/1000; // Sparse 18–37 second accents.
        return true;
    }
private:
    double remaining_=18;
    std::uint32_t random_=0x50415753u;
};
}
