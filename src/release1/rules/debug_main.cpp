// Task-local inspection adapter. Temporary cubes/HTML controls never enter paws.
#include "content.hpp"
#include "engine/application.hpp"
#include "engine/actor.hpp"
#include "engine/camera_component.hpp"
#include "engine/light_component.hpp"
#include "engine/mesh_component.hpp"
#include "engine/renderer.hpp"
#include <emscripten.h>
#include <algorithm>
#include <sstream>

namespace {
using namespace paws;using namespace paws::release1;using namespace paws::release1::rules;
std::string quote(std::string_view s) {
    std::string out="\"";for(unsigned char c:s){if(c=='"'||c=='\\'){out+='\\';out+=char(c);}else if(c=='\n')out+="\\n";else if(c>=32)out+=char(c);}return out+'"';
}
void cellJson(std::ostream& out,Cell c){out<<'['<<c.x<<','<<c.y<<']';}
void stepJson(std::ostream& out,const Intent& c) {
    out<<"{\"verb\":"<<int(c.verb)<<",\"object\":"<<quote(c.object)<<",\"target\":"<<quote(c.target)<<",\"direction\":"<<int(c.direction)<<",\"destination\":";cellJson(out,c.destination);out<<'}';
}
class Debug {
public:
    Debug() {
        for(auto& d:catalogue()){auto e=validate(d);if(!e.empty())throw std::runtime_error(e);attempts_.push_back(std::make_unique<Attempt>(d));}
        auto scene=std::make_unique<engine::Scene>("Temporary release-one rules board");
        camera_=scene->spawn("Board view").addComponent<engine::CameraComponent>("Board camera").id();
        auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findComponent(camera_));
        auto& light=scene->spawn("Preview lighting").addComponent<engine::LightComponent>("Sun",engine::LightType::Directional);
        light.direction={-.5f,-1,-.3f};light.intensity=2;
        renderer_=std::make_unique<engine::Renderer>(*scene,*camera);renderer_->setShadowsEnabled(false);
        const auto id=app_.loadScene(std::move(scene));app_.start(id);renderer_->initialize();publish();
    }
    void select(int n) {if(n<0||n>=int(attempts_.size())||paused_||hidden_)return;current_=n;last_.clear();accepted_=true;active().activate(++generation_);publish();}
    void action(int i,int revision,int attempt,int generation) {
        if(i<0||i>=int(view_.actions.size()))return;
        auto c=view_.actions[i].intent;c.revision=Revision(revision);c.attempt=AttemptId(attempt);c.generation=std::uint64_t(generation);
        auto result=active().dispatch(c);last_=result.message;accepted_=result.accepted;publish();
    }
    void pause(bool value) {paused_=value;syncPause();if(value)app_.pause();else app_.resume();publish();}
    void suspend(bool value) {hidden_=value;syncPause();app_.setSuspended(value);publish();}
    void frame() {
        const auto start=emscripten_get_now();auto& f=app_.advance(start/1000);
        auto* scene=app_.activeScene();auto* camera=scene?dynamic_cast<engine::CameraComponent*>(scene->findComponent(camera_)):nullptr;
        if(camera){
            const float aspect=float(EM_ASM_DOUBLE({const c=document.querySelector('#viewport');return c.clientWidth/Math.max(1,c.clientHeight);}));
            const auto& d=active().definition();camera->setOrthographic(std::max(float(d.height)+3,(float(d.width)+3)/std::max(.1f,aspect)),.1f,100);
        }
        renderer_->serviceRecovery();renderer_->resizeCanvas();if(renderer_->ready())renderer_->render(emscripten_get_now()-start,f.interval*1000);
    }
private:
    Attempt& active(){return *attempts_[current_];}
    void syncPause(){for(auto& a:attempts_)a->setPaused(paused_||hidden_);}
    void publish() {
        view_=active().view();const auto& d=active().definition();std::ostringstream out;
        out<<"{\"title\":"<<quote(view_.title)<<",\"quest\":"<<quote(view_.quest)<<",\"objective\":"<<quote(view_.objective)
            <<",\"feedback\":"<<quote(last_.empty()?view_.feedback:last_)<<",\"completed\":"<<(view_.completed?"true":"false")
            <<",\"accepted\":"<<(accepted_?"true":"false")<<",\"paused\":"<<(paused_?"true":"false")<<",\"generation\":"<<view_.generation
            <<",\"attempt\":"<<view_.attempt<<",\"revision\":"<<view_.revision<<",\"width\":"<<d.width<<",\"height\":"<<d.height<<",\"quests\":[";
        for(std::size_t i=0;i<attempts_.size();++i){if(i)out<<',';out<<quote(attempts_[i]->definition().title);}out<<"],\"objects\":[";
        for(std::size_t i=0;i<view_.objects.size();++i){auto& o=view_.objects[i];if(i)out<<',';
            out<<"{\"id\":"<<quote(o.id)<<",\"name\":"<<quote(o.name)<<",\"role\":"<<quote(o.role)<<",\"orientation\":"<<o.orientation
                <<",\"held\":"<<(o.held?"true":"false")<<",\"active\":"<<(o.active?"true":"false")<<",\"collected\":"<<(o.collected?"true":"false")<<",\"socket\":"<<quote(o.socket)<<",\"cell\":";
            cellJson(out,o.cell);out<<",\"footprint\":[";for(std::size_t j=0;j<o.footprint.size();++j){if(j)out<<',';cellJson(out,o.footprint[j]);}out<<"]}";
        }
        out<<"],\"actions\":[";
        for(std::size_t i=0;i<view_.actions.size();++i){auto& a=view_.actions[i];if(i)out<<',';
            out<<"{\"label\":"<<quote(a.label)<<",\"reason\":"<<quote(a.reason)<<",\"enabled\":"<<(a.enabled?"true":"false")<<",\"intent\":";stepJson(out,a.intent);out<<'}';
        }
        out<<"],\"trace\":[";for(std::size_t i=0;i<d.solution.size();++i){if(i)out<<',';stepJson(out,active().command(d.solution[i]));}
        out<<"],\"hints\":[";for(unsigned i=0;i<view_.revealedHints;++i){if(i)out<<',';out<<quote(view_.hints[i]);}
        out<<"],\"symbols\":[";for(std::size_t i=0;i<view_.symbols.size();++i){if(i)out<<',';out<<quote(view_.symbols[i]);}
        out<<"],\"entered\":[";for(std::size_t i=0;i<view_.enteredSymbols.size();++i){if(i)out<<',';out<<quote(view_.enteredSymbols[i]);}
        for(auto entry:{std::pair{"basket",d.basket},std::pair{"shade",d.shade},std::pair{"quiet",d.quiet}}){out<<"],\""<<entry.first<<"\":[";for(std::size_t i=0;i<entry.second.size();++i){if(i)out<<',';cellJson(out,entry.second[i]);}}
        out<<"]}";auto json=out.str();EM_ASM({window.rulesPublish(JSON.parse(UTF8ToString($0)));},json.c_str());project();
    }
    void project() {
        auto* scene=app_.activeScene();if(!scene)return;
        for(auto id:boxes_)if(auto* actor=scene->findActor(id))actor->destroy();boxes_.clear();
        auto cube=engine::Mesh::cube();const auto& d=active().definition();
        auto box=[&](Cell p,float y,float size,engine::Vec3 color){auto& a=scene->spawn("Temporary logical-cell marker");
            a.transform.position={float(p.x)-(d.width-1)*.5f,y,float(p.y)-(d.height-1)*.5f};a.transform.scale={size,.12f,size};
            a.addComponent<engine::MeshComponent>("Temporary cube",cube).color=color;boxes_.push_back(a.id());};
        for(int y=0;y<d.height;++y)for(int x=0;x<d.width;++x)box({x,y},-.12f,.93f,{.91f,.87f,.78f});
        for(auto c:d.basket)box(c,0,.86f,{.63f,.70f,.58f});
        for(auto c:d.shade)box(c,0,.86f,{.63f,.70f,.58f});
        for(auto c:d.quiet)box(c,.03f,.6f,{.75f,.84f,.86f});
        for(auto& o:view_.objects)if(!o.collected) {
            if(o.held)continue;
            if(o.role.find("clue")!=std::string::npos)continue;
            auto cells=o.socket.empty()?std::vector<Cell>{o.cell}:o.footprint;
            for(auto p:cells)box(p,o.role.find("socket")!=std::string::npos?.04f:.25f,.62f,o.active?engine::Vec3{.92f,.77f,.42f}:engine::Vec3{.16f,.40f,.36f});
        }
        auto* camera=dynamic_cast<engine::CameraComponent*>(scene->findComponent(camera_));
        camera->transform.position={0,9,4};camera->lookAt({0,0,0});scene->synchronize();
    }
    engine::Application app_;engine::ComponentId camera_;std::vector<engine::ActorId> boxes_;
    std::unique_ptr<engine::Renderer> renderer_;
    std::vector<std::unique_ptr<Attempt>> attempts_;PuzzleView view_;
    int current_=0;std::uint64_t generation_=1;bool paused_=false,hidden_=false,accepted_=true;std::string last_;
};
std::unique_ptr<Debug> debug;
void frame(void*){debug->frame();}
}
extern "C" {
EMSCRIPTEN_KEEPALIVE void rules_action(int i,int revision,int attempt,int generation){if(debug)debug->action(i,revision,attempt,generation);}
EMSCRIPTEN_KEEPALIVE void rules_quest(int i){if(debug)debug->select(i);}
EMSCRIPTEN_KEEPALIVE void rules_pause(int value){if(debug)debug->pause(value!=0);}
EMSCRIPTEN_KEEPALIVE void rules_suspend(int value){if(debug)debug->suspend(value!=0);}
}
int main(){debug=std::make_unique<Debug>();emscripten_set_main_loop_arg(frame,nullptr,0,true);}
