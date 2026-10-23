#pragma once
#include "host_events.hpp"
#include "engine/application.hpp"
#include "engine/audio.hpp"
#include "engine/gui.hpp"
#include "engine/render_quality.hpp"
#include <type_traits>
#include <variant>

namespace paws::runtime {
// Values are owned for the duration of the synchronous adapter call. Adapters
// must copy anything queued and must never retain scene/widget references here.
struct HostValue {
    std::variant<double,std::string> value;
    template<class T> requires std::is_arithmetic_v<T>
    HostValue(T v) : value(double(v)) {}
    HostValue(const char* v) : value(std::string(v)) {}
    HostValue(std::string_view v) : value(std::string(v)) {}
    double number() const { return std::get<double>(value); }
    const std::string& text() const { return std::get<std::string>(value); }
};
// One platform host owns one Application. GameSession borrows both and owns
// Afternoon, scene composition, commands, presentation and save policy.
class RuntimeHost {
public:
    virtual ~RuntimeHost()=default;
    bool beachPreview=false,correctIdlePose=true,gallery=false;
    int msaaOverride=0;
    virtual engine::Application& application()=0;
    virtual engine::AssetLoader assetLoader(std::string url,
        std::function<void(std::span<const std::byte>,const engine::AssetLoadContext&)> decode)=0;
    virtual std::unique_ptr<engine::AudioBackend> makeAudio(bool ui)=0;
    virtual unsigned audioDevice(bool) const { return 0; }
    virtual std::string audioStatus(bool active,bool muted)=0;
    virtual void event(HostEvent,std::initializer_list<HostValue>)=0;
    virtual void viewport(double* width,double* height) const=0;
    virtual bool ready() const=0;
    virtual bool platformSuspended() const { return false; }
    virtual void setSuspended(bool value) { application().setSuspended(value); }
    virtual void bindScene(engine::SceneId,engine::CameraComponent&)=0;
    virtual engine::GuiSystem* sharedGui() { return nullptr; }
    virtual void attachGui(engine::GuiSystem*)=0;
    virtual void syncGui(bool) {}
    virtual void guiReady() {}
    virtual void setQuality(const engine::RenderQuality&)=0;
    virtual void setWaterTime(double)=0;
    virtual double waterTime() const=0;
    virtual void releaseRendering()=0;
};
}
