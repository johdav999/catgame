#pragma once
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>
namespace paws::release1::rules {
// Transient clue playback driven by Application time. Answers and rewards stay
// in Attempt/Afternoon; reactivation replays the clue before accepting input.
class PictureSequencePlayback {
public:
    void start(std::vector<std::string> pictures,double seconds) {
        if(pictures.empty()||pictures.size()>8||!std::isfinite(seconds)||seconds<.25||seconds>10)
            throw std::invalid_argument("Invalid picture sequence playback");
        pictures_=std::move(pictures);seconds_=seconds;elapsed_=0;
    }
    bool advance(double delta,bool paused=false) {
        const auto before=index();const bool wasWatching=watching();
        if(!paused&&std::isfinite(delta)&&delta>0)elapsed_=std::min(duration(),elapsed_+delta);
        return before!=index()||wasWatching!=watching();
    }
    void finish() {elapsed_=duration();}
    bool watching() const {return !pictures_.empty()&&elapsed_<duration();}
    std::size_t index() const {return watching()?std::min(pictures_.size()-1,std::size_t(elapsed_/seconds_)):pictures_.size();}
    std::string picture() const {return watching()?pictures_[index()]:std::string{};}
    std::size_t size() const {return pictures_.size();}
    double elapsed() const {return elapsed_;}
private:
    double duration() const {return seconds_*pictures_.size();}
    std::vector<std::string> pictures_;
    double seconds_=1,elapsed_=0;
};
}
