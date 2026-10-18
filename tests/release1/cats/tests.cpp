#include "release1/cats/cat_assets.hpp"
#include "engine/application.hpp"
#include "engine/glb_importer.hpp"
#include <filesystem>
#include <iostream>
#include <cmath>
#include <stdexcept>

namespace cats=paws::release1::cats;
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(int argc,char** argv) {
    try {
        const auto root=std::filesystem::path(__FILE__).parent_path().parent_path().parent_path().parent_path();
        const auto authority=engine::GlbImporter::loadFile((root/"assets/runtime/models/pip/pip.glb").string());
        const auto candidates=argc>1?std::filesystem::path(argv[1]):root/"build/web-release1-cats/site/candidates";
        for(const auto& asset:cats::assets) {
            const auto mesh=engine::GlbImporter::loadFile((candidates/(std::string(asset.cat)+".glb")).string());
            cats::validate(*mesh);cats::validateCanonical(*mesh,*authority);
            auto preview=paws::makePipPreview(mesh);const auto handle=preview.cat;
            engine::Application app;const auto sceneId=app.loadScene(std::move(preview.scene.scene));app.start(sceneId);
            auto* cat=dynamic_cast<engine::SkeletalMeshComponent*>(app.activeScene()->findComponent(handle));check(cat,"Missing cat handle");
            double time=0;app.advance(time);
            for(std::size_t a=0;a<cats::actions.size();++a) {
                const auto& spec=cats::actions[a];
                cat->animation().play(paws::pipClip(*mesh,spec.name),spec.loop);
                float lowest=10,highest=-10;
                // Uniform exported frames plus half-frame samples reveal
                // interpolation penetration that keyframe-only tests miss.
                for(int f=0;f<=int(std::round(spec.duration*60));++f) {
                    cat->animation().seek(f/60.);cat->animation().pause();time+=1./60.;app.advance(time);
                    const auto skin=cat->skinPose();
                    for(const auto& bone:skin->bones)for(float v:bone.transform.m)check(std::isfinite(v),"Nonfinite animated matrix");
                    for(std::size_t v=0;v<mesh->vertices.size();++v) {
                        engine::Vec3 p{};
                        for(int k=0;k<4;++k)p=p+engine::transformPoint(skin->bones[mesh->skin[v].joints[k]].transform,mesh->vertices[v].position)*mesh->skin[v].weights[k];
                        lowest=std::min(lowest,p.y);highest=std::max(highest,p.y);
                    }
                }
                check(lowest>=-.008f&&highest<1.65f,"Animation outside supported ground/height envelope");
                cats::present(*cat,a,true);const auto still=cat->animation().time();time+=.2;app.advance(time);check(cat->animation().time()==still,"Reduced motion advanced");
                std::cout<<asset.cat<<" "<<spec.name<<" y=["<<lowest<<","<<highest<<"]\n";
            }
            cats::present(*cat,3,false);time+=.1;app.advance(time);app.pause();const auto paused=cat->animation().time();time+=.2;app.advance(time);check(cat->animation().time()==paused,"Pause advanced playback");
            app.resume();app.setSuspended(true);time+=.2;app.advance(time);check(cat->animation().time()==paused,"Suspension advanced playback");
            app.shutdown();check(!app.findScene(sceneId),"Cat scene survived shutdown");
            std::cout<<asset.cat<<": canonical bind, "<<mesh->indices.size()/3<<" triangles, 16 actions, normalized skin and lifecycle passed\n";
        }
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
