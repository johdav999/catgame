// Compose Pepper's clues from the accepted Paws kit and coastal Tripo landmarks.
// Included in puzzle_scene.cpp's private presentation namespace.
#include "pepper_tags.inc"
std::shared_ptr<const engine::Material> pepperLabel(unsigned index,bool state){
 static const auto labels=[] {
  const std::span<const unsigned char> images[]={pepperTag0_0,pepperTag0_1,pepperTag1_0,pepperTag1_1,
   pepperTag2_0,pepperTag2_1,pepperTag3_0,pepperTag3_1,pepperTag4_0,pepperTag4_1,pepperTag5_0,pepperTag5_1};
  std::array<std::shared_ptr<const engine::Material>,12> result;
  for(unsigned n=0;n<12;++n){
   auto m=std::make_shared<engine::Material>();m->shader=engine::MaterialShader::Unlit;
   m->baseColorTexture=engine::Texture::loadMemory(std::as_bytes(images[n]),"Pepper interaction label "+std::to_string(n));
   m->doubleSided=true;m->alphaMode=engine::AlphaMode::Mask;m->alphaCutoff=.4f;result[n]=m;
  }return result;
 }();return labels.at(index*2+unsigned(state));
}
void pepperMarkers(engine::Scene& s,SceneBindings& b,bool completed){
 auto& p=b.pepper;
 const bool visible=!completed&&p.stage==p.pendingStage&&!(p.active&&p.reaction>=3);
 const unsigned stateKey=p.inspected|(visible?8u:0u);
 if(p.markerState==stateKey)return;
 p.markerState=stateKey;
 for(unsigned n=0;n<6;++n){
  const bool state=n<3?(p.inspected&(1u<<n))!=0:p.inspected==7;
  if(auto* m=dynamic_cast<engine::MeshComponent*>(s.findComponent(p.labels[n]))){
   m->enabled=visible;m->setMaterial(0,engine::MaterialInstance{pepperLabel(n,state)});
  }
  for(auto id:p.markers[n])if(auto* m=dynamic_cast<engine::MeshComponent*>(s.findComponent(id))){
   m->enabled=visible;m->color=state?engine::Vec3{.16f,.39f,.36f}:engine::Vec3{.60f,.48f,.30f};
  }
 }
}
std::size_t pepperStage(const PuzzleView& v,const rules::Definition& d){
 for(const auto& o:v.objects)if(o.id=="trail_0")
  for(std::size_t n=0;n<d.trail.size();++n)if(d.trail[n].choices[0]==o.name)return n;
 return 0;
}
engine::Vec3 pepperPoint(std::string_view id){
 const int n=id.back()-'0';
 // Follow evidence from the foreground up the beach, then compare the fork.
 return id.starts_with("clue_")?engine::Vec3{0,0,4.6f-n*1.8f}:engine::Vec3{(n-1)*2.5f,0,-1.15f};
}
// A small shared sand/water surface mask softens the edges of the authored
// paths. These are scene surface treatments, not additional imported models.
std::shared_ptr<const engine::Material> pepperSurface(bool wet){
 static const auto materials=[] {
  std::array<std::shared_ptr<const engine::Material>,2> result;
  for(unsigned n=0;n<2;++n){
   auto t=std::make_shared<engine::Texture>();t->name="Pepper trail surface";t->width=64;t->height=64;
   for(unsigned y=0;y<64;++y)for(unsigned x=0;x<64;++x){
    const float u=(x+.5f)/32-1,v=(y+.5f)/32-1;
    const float roundX=std::max(0.f,std::abs(u)-.72f),roundY=std::max(0.f,std::abs(v)-.72f);
    const float edge=.28f-std::sqrt(roundX*roundX+roundY*roundY)-.022f*std::sin(x*1.7f+y*.9f);
    const int grain=int((x*37+y*73+x*y*11)%17)-8;
    const int ripple=n&&((y+int(3*std::sin(x*.17f)))%21<3)?55:0;
    t->pixels.insert(t->pixels.end(),{std::uint8_t((n?66:193)+grain+ripple),std::uint8_t((n?132:157)+grain+ripple),std::uint8_t((n?142:102)+grain+ripple),std::uint8_t(std::clamp(edge*12,0.f,1.f)*255)});
   }
   auto m=std::make_shared<engine::Material>();m->name=n?"Shallow runnel":"Dry disturbed sand";
   m->baseColorTexture=t;m->roughness=n?.25f:.95f;m->alphaMode=engine::AlphaMode::Mask;m->alphaCutoff=.25f;
   result[n]=m;
  }return result;
 }();return materials[wet];
}
engine::Actor& pepperObject(engine::Scene& s,SceneBindings& b,const GardenMeshes& meshes,
                            const ObjectView& o,const rules::Definition& d){
 auto& a=s.spawn(o.name);a.transform.position=pepperPoint(o.id);
 auto& interaction=a.addComponent<engine::InteractionComponent>();
 interaction.proxyBounds=engine::MeshBounds{{-1.08f,0,-1.15f},{1.08f,1.4f,1.42f},true};
 if(o.role=="trail_clue")interaction.proxyBounds=engine::MeshBounds{{-1.85f,0,-.4f},{.72f,.9f,.75f},true};
 const unsigned marker=(o.role=="trail_clue"?0:3)+unsigned(o.id.back()-'0');
 auto& label=a.addComponent<engine::MeshComponent>("Clue or path interaction label",engine::Mesh::plane());
 label.transform.position=o.role=="trail_clue"?engine::Vec3{-1.15f,.43f,.20f}:engine::Vec3{0,.43f,1.32f};label.transform.rotation.x=.88f;label.transform.scale={1.24f,1,.5425f};
 label.color={1,1,1};label.castsShadow=false;label.setMaterial(0,engine::MaterialInstance{pepperLabel(marker,false)});
 b.pepper.labels[marker]=label.id();
 // Discrete ground outlines identify the selectable assemblies without giving
 // any route a stronger affordance than the others. Labels are in the same proxy.
 const bool clue=o.role=="trail_clue";
 for(unsigned edge=0;edge<4;++edge){
  auto& line=a.addComponent<engine::MeshComponent>(clue?"Clue boundary":"Selectable path boundary",engine::Mesh::cube());
  const float width=clue?1.32f:2.16f,depth=clue?.92f:2.22f;
  line.transform.position={edge<2?0:(edge==2?-1.f:1.f)*width/2,.022f,edge<2?(edge==0?-1.f:1.f)*depth/2:.0f};
  line.transform.scale=edge<2?engine::Vec3{width,.012f,.025f}:engine::Vec3{.025f,.012f,depth};
  line.castsShadow=false;
  auto material=std::make_shared<engine::Material>();material->shader=engine::MaterialShader::Unlit;
  line.setMaterial(0,engine::MaterialInstance{material});b.pepper.markers[marker].push_back(line.id());
 }
 auto part=[&](std::size_t stage,std::string asset,engine::Vec3 p,engine::Vec3 scale,
               engine::Vec3 rotation=engine::Vec3{},engine::Vec3 tint=engine::Vec3{1,1,1}) -> engine::MeshComponent& {
  auto& m=a.addComponent<engine::MeshComponent>("Trail stop "+std::to_string(stage+1)+" "+asset,meshes.at(asset));
  m.transform.scale=scale;m.transform.rotation=rotation;m.color=tint;
  m.enabled=stage==b.pepper.stage;
  const auto bounds=m.mesh->localBounds();float bottom=1000;
  for(float x:{bounds.minimum.x,bounds.maximum.x})for(float y:{bounds.minimum.y,bounds.maximum.y})for(float z:{bounds.minimum.z,bounds.maximum.z})
   bottom=std::min(bottom,engine::transformPoint(m.transform.matrix(),{x,y,z}).y);
  m.transform.position=p+engine::Vec3{0,-bottom,0};
  if(asset.starts_with("beach_grass")){
   auto world=m.transform;world.position=world.position+a.transform.position;
   m.transform.position.y=beachPlantHeight(s,*m.mesh,world)-a.transform.position.y;
  }
  b.pepper.visuals.push_back({m.id(),stage});return m;
 };
 auto cloth=[&](std::size_t stage,engine::Vec3 p){
  static const auto blue=[] {auto m=std::make_shared<engine::Material>();m->baseColor={.075f,.34f,.68f};m->roughness=.9f;return m;}();
  for(int n=0;n<2;++n){auto& ribbon=part(stage,"picnic_mat",p+engine::Vec3{n*.075f,0,n*.045f},{.075f,.3f,.42f},{0,.3f+n*.35f,0});
   for(std::size_t slot=0;slot<std::max(std::size_t(1),ribbon.mesh->materials.size());++slot)ribbon.setMaterial(slot,engine::MaterialInstance{blue});
  }
 };
   unsigned shellVariant=0;
   auto shell=[&](std::size_t stage,engine::Vec3 p){
    part(stage,std::array{"beach_shell_scallop","beach_shell_clam","beach_shell_spiral"}[shellVariant++%3],p,{1.65f,1.65f,1.65f});
 };
 auto shelter=[&](std::size_t stage,bool striped,engine::Vec3 p){
  if(striped){part(stage,"beach_shelter",p,{.88f,.8f,.88f});return;}
  // Same silhouette/footprint as the striped option: the fabric is the clue.
  for(float x:{-.62f,.62f})for(float z:{-.34f,.34f})
   part(stage,"beach_shade_post",p+engine::Vec3{x,0,z},{1.3f,1.5f,1.3f});
  auto& roof=part(stage,"beach_shade_panel",p+engine::Vec3{0,.94f,0},{.70f,1,1.35f});
  auto cream=std::make_shared<engine::Material>();cream->baseColor={.88f,.79f,.60f};cream->roughness=.95f;
  roof.setMaterial(0,engine::MaterialInstance{cream});
 };
 auto tracks=[&](std::size_t stage,std::initializer_list<engine::Vec3> points,bool wet=false){
  const std::vector<engine::Vec3> route(points);
  for(std::size_t n=1;n<route.size();++n){
   const auto from=route[n-1],to=route[n],delta=to-from;
   const float length=std::sqrt(engine::dot(delta,delta));
   auto& ground=a.addComponent<engine::MeshComponent>(wet?"Wet route with ripples":"Dry sand trail",engine::Mesh::plane());
   ground.transform.position=(from+to)*.5f+engine::Vec3{0,.018f,0};
   ground.transform.rotation.y=std::atan2(delta.x,delta.z);ground.transform.scale={wet?.40f:.34f,1,length+.15f};
   ground.color={1,1,1};ground.castsShadow=false;ground.setMaterial(0,engine::MaterialInstance{pepperSurface(wet)});
   ground.enabled=stage==b.pepper.stage;b.pepper.visuals.push_back({ground.id(),stage});
   const int count=std::max(1,int(length/.24f));
   for(int k=0;k<count;++k){
    const auto at=from+delta*((k+.5f)/count)+engine::Vec3{(k%2?.045f:-.045f),.026f,0};
    auto& paw=part(stage,"sequence_picture_paw",at,{.17f,.17f,.17f},{-engine::pi/2,-engine::pi/2,0},{.35f,.23f,.12f});
    paw.castsShadow=false;
   }
  }
 };
 auto grasses=[&](std::size_t stage,float side){
  for(int n=0;n<3;++n)part(stage,"beach_grass_short",{side*(n%2?.53f:.94f),0,.37f-n*.49f},{1.15f,1.2f,1.15f},{0,n*.8f,0},{.78f,1,.68f});
 };
 auto windbreak=[&](std::size_t stage){
  // A vertical rear screen separates the landward trail from the visible sea.
  part(stage,"beach_plank_short",{0,0,-.78f},{1.85f,1,1.45f},{engine::pi/2,0,0});
  for(float x:{-.86f,.86f})part(stage,"beach_rope_post",{x,0,-.72f},{.6f,.95f,.6f});
 };
 if(o.role=="trail_clue"){
  for(std::size_t stage=0;stage<d.trail.size();++stage){
   if(o.id=="clue_0")for(int n=0;n<4;++n)
    part(stage,"sequence_picture_paw",{n%2?.13f:-.13f,.012f,.37f-n*.24f},{.38f,.38f,.38f},{-engine::pi/2,0,0},{.48f,.36f,.24f});
   else if(o.id=="clue_1"){
    const auto& clue=d.trail[stage].clues[1];
    if(clue.find("grass")!=std::string::npos){part(stage,"beach_grass",{-.12f,0,0},{.75f,.65f,.75f});cloth(stage,{-.05f,.18f,.12f});}
    else if(clue.find("branch")!=std::string::npos){
     part(stage,"beach_branch",{0,0,0},{.45f,.45f,.45f});
     cloth(stage,{.15f,.22f,.035f});
    }else if(clue.find("shells")!=std::string::npos){shell(stage,{-.25f,0,0});shell(stage,{.15f,0,-.1f});cloth(stage,{-.12f,.04f,.15f});}
    else if(clue.find("support")!=std::string::npos){part(stage,"beach_plank_short",{-.15f,0,0},{.8f,.8f,.35f},{0,0,engine::pi/2});cloth(stage,{-.10f,.53f,-.1f});}
    else if(clue.find("lid")!=std::string::npos){part(stage,"beach_basket",{0,0,0},{.70f,.70f,.70f});cloth(stage,{-.10f,.33f,.05f});}
     else {part(stage,"beach_rope_post",{-.20f,0,0},{.70f,.70f,.70f});cloth(stage,{-.12f,.48f,.08f});}
   }else{
    part(stage,"picnic_mat",{0,0,0},{.48f,.5f,.38f},{0,.2f,0});
    part(stage,"fish",{.03f,.035f,.02f},{1.2f,1.2f,1.2f});
    part(stage,"beach_plank_short",{0,.01f,-.26f},{1.05f,.65f,.65f},{0,-.12f,0});
   }
   // Neutral footprints connect the evidence, stopping before the three-way
   // choice so their placement never singles out an answer.
   if(o.id!="clue_2")for(int n=0;n<3;++n){
    auto& paw=part(stage,"sequence_picture_paw",{n%2?.09f:-.09f,.012f,-.65f-n*.24f},{.20f,.20f,.20f},{-engine::pi/2,0,0},{.48f,.36f,.24f});
    paw.castsShadow=false;
   }
  }
  auto& check=a.addComponent<engine::MeshComponent>("Examined evidence",meshes.at("sequence_picture_paw"));
  check.transform.position={.48f,.035f,.22f};check.transform.scale={.26f,.26f,.26f};check.transform.rotation.x=-engine::pi/2;
  check.color={.15f,.4f,.36f};check.castsShadow=false;b.pepper.discoveries[o.id.back()-'0']=check.id();
 }else for(std::size_t stage=0;stage<d.trail.size();++stage){
  // Describe every alternative, independent of the answer index. All paths get
  // equally strong tracks; a match must be inferred from the physical features.
  const auto& choice=d.trail[stage].choices[o.id.back()-'0'];
  auto has=[&](std::string_view word){return choice.find(word)!=std::string::npos;};
  if(has("basket")){
   const bool shade=has("Shaded");
   if(shade)shelter(stage,false,{-.18f,0,-.26f});
   part(stage,"beach_basket",{-.18f,0,.05f},{1.05f,1.05f,1.05f});cloth(stage,{-.30f,.49f,.08f});
   if(has("shell"))shell(stage,{.65f,0,.12f});
   else for(float z:{-.10f,.25f})part(stage,"beach_grass_short",{.65f,0,z},{1.6f,1.6f,1.6f});
   tracks(stage,{{.28f,0,.92f},{.40f,0,.48f},{.28f,0,.26f}});
  }else if(has("shelter")){
   shelter(stage,has("striped"),{-.15f,0,-.08f});
   const bool rear=has("Behind");
   cloth(stage,{.50f,.63f,rear?-.53f:.35f});
   if(rear)tracks(stage,{{.55f,0,.91f},{.91f,0,.4f},{.91f,0,-.72f},{.25f,0,-.94f}});
   else tracks(stage,{{-.42f,0,.92f},{.05f,0,.61f},{.48f,0,.57f}});
  }else if(has("branch")){
   const bool right=has("Right"),high=has("high");
   const float side=right?1.f:-1.f;
   const bool flipped=right==high;
   part(stage,"beach_branch",{0,0,-.20f},{1.15f,1.15f,1.15f},{0,flipped?engine::pi:0,0});
   cloth(stage,{right?.42f:-.49f,high?.845f:.56f,-.16f});
   tracks(stage,{{0,0,.92f},{side*.74f,0,.45f},{side*.78f,0,-.45f},{side*.65f,0,-.92f}});
   windbreak(stage);
  }else if(has("side")){
   const bool rough=has("Rough"),wet=has("runnel");
   part(stage,rough?"beach_rock_cluster":"beach_rock_flat",{.24f,0,-.17f},rough?engine::Vec3{1.65f,1.65f,1.65f}:engine::Vec3{1.8f,3.6f,1.9f});
   if(rough)for(float z:{-.42f,.05f})part(stage,"beach_rock_cluster",{-.18f,0,z},{.65f,.9f,.65f});
   tracks(stage,{{-.1f,0,.93f},{-.69f,0,.49f},{-.75f,0,-.49f},{-.45f,0,-.90f}},wet);
   if(!wet){shell(stage,{.34f,0,.65f});shell(stage,{.77f,0,.56f});cloth(stage,{.55f,.10f,.57f});}
   else {part(stage,"beach_rock_flat",{-.83f,0,-.75f},{.5f,.6f,.5f},{},{.42f,.53f,.50f});cloth(stage,{-.63f,.05f,.44f});}
  }else if(has("path")){
   for(float x:{-.31f,.31f})part(stage,"beach_rock",{x,0,-.20f},{.42f,.36f,.49f},{},{1,.94f,.78f});
   const float side=has("Left")?-1.f:1.f;
   const bool wet=has("wet");
   tracks(stage,{{0,0,.93f},{side*.73f,0,.49f},{side*.78f,0,-.47f},{side*.62f,0,-.93f}},wet);
   if(!wet){grasses(stage,side);cloth(stage,{side*.57f,.16f,.20f});}
   else for(float z:{-.67f,.38f})part(stage,"beach_rock_flat",{side*.98f,0,z},{.36f,.6f,.45f},{},{.42f,.53f,.50f});
  }else{
   const bool rope=has("rope"),sheltered=has("Sheltered");
   if(sheltered)windbreak(stage);
   if(rope){part(stage,"beach_rope_post",{.18f,0,-.05f},{1.5f,1.25f,1.5f});cloth(stage,{.22f,.75f,.02f});}
   else part(stage,has("smooth")?"beach_rock_flat":"beach_rock_cluster",{.18f,0,-.12f},has("smooth")?engine::Vec3{1.65f,3,1.65f}:engine::Vec3{1.5f,1.4f,1.5f});
   tracks(stage,{{-.30f,0,.92f},{-.61f,0,.45f},{-.60f,0,-.22f},{-.32f,0,-.45f}});
   if(!sheltered)tracks(stage,{{-.88f,0,-.85f},{.88f,0,-.85f}},true);
  }
 }

 return a;
}
void projectPepper(engine::Scene& s,SceneBindings& b,const PuzzleView& v,bool reduced){
 if(v.quest!="beach_pepper_basket")return;
 const auto d=rules::quest(v.quest,v.content);auto& p=b.pepper;
 const auto next=pepperStage(v,d);
 if(v.attempt!=b.attempt||v.revision<b.revision||(!v.completed&&b.completed)){
  p.active=false;p.route.clear();b.ending.active=false;p.reaction=0;p.awaitingClues=0;p.inspectingClue=3;
 }
 if(!p.active){if(p.stage!=next)p.awaitingClues=0;p.stage=p.pendingStage=next;}
 p.inspected=0;
 for(const auto& o:v.objects)if(o.role=="trail_clue"&&o.active)p.inspected|=1u<<(o.id.back()-'0');
 p.awaitingClues&=p.inspected;
 p.inspected&=~p.awaitingClues;
 for(auto visual:p.visuals)if(auto* m=s.findComponent(visual.component))m->enabled=visual.stage==p.stage;
 for(unsigned n=0;n<3;++n)if(auto* m=s.findComponent(p.discoveries[n]))m->enabled=p.stage==next&&(p.inspected&(1u<<n));
 for(const auto& binding:b.objects)if(auto* a=s.findActor(binding.actor)){
  a->transform.position=pepperPoint(binding.object);a->transform.rotation={};
  for(const auto& o:v.objects)if(o.id==binding.object)a->name=o.name;
 }
 pepperMarkers(s,b,v.completed);
 if(b.requesters.empty())return;
 auto* cat=s.findActor(b.requesters[0].actor);if(!cat)return;
 auto* skin=cat->getComponent<engine::SkeletalMeshComponent>();if(!skin)return;
 if(!p.active){
  cat->transform.position=v.completed?pepperPoint("trail_"+std::to_string(d.trail.back().answer))+engine::Vec3{0,0,.95f}:p.home;
  cat->transform.rotation.y=v.completed?engine::pi:-engine::pi/2;
  cats::present(*skin,v.completed?14:p.inspected==7?1:0,reduced||v.completed);
 }
}
void startPepperReaction(engine::Scene& s,SceneBindings& b,const PuzzleView& before,const PuzzleView& after,
                         const Intent& intent,bool accepted,bool reduced){
 if(before.quest!="beach_pepper_basket"||before.completed||b.requesters.empty())return;
 if(intent.verb==Verb::Undo||intent.verb==Verb::Restart){b.pepper.active=false;b.pepper.route.clear();b.ending.active=false;return;}
 if(intent.verb!=Verb::Inspect&&intent.verb!=Verb::FollowTrail)return;
 if(intent.object.empty()||(!intent.object.starts_with("clue_")&&!intent.object.starts_with("trail_")))return;
 auto* cat=s.findActor(b.requesters[0].actor);if(!cat)return;
 auto& p=b.pepper;p.stage=pepperStage(before,rules::quest(before.quest,before.content));
 p.pendingStage=pepperStage(after,rules::quest(after.quest,after.content));
 p.reaction=intent.verb==Verb::Inspect?1:!accepted?2:after.completed?4:3;
 p.inspectingClue=intent.verb==Verb::Inspect&&accepted?unsigned(intent.object.back()-'0'):3;
 if(p.inspectingClue<3){
  const auto clue=std::find_if(before.objects.begin(),before.objects.end(),[&](const auto& o){return o.id==intent.object;});
  if(clue!=before.objects.end()&&!clue->active)p.awaitingClues|=1u<<p.inspectingClue;
  if(reduced)p.awaitingClues&=~(1u<<p.inspectingClue);
 }
 p.target=pepperPoint(intent.object)+engine::Vec3{0,0,intent.verb==Verb::Inspect?.72f:.93f};
 if(intent.verb==Verb::FollowTrail){
  const auto definition=rules::quest(before.quest,before.content);
  const auto& choice=definition.trail[p.stage].choices[intent.object.back()-'0'];
  if(choice.starts_with("Behind"))p.target=pepperPoint(intent.object)+engine::Vec3{0,0,-.90f};
  else if(choice.find("path")!=std::string::npos)p.target=pepperPoint(intent.object)+engine::Vec3{choice.starts_with("Left")?-.92f:.92f,0,.70f};
  else if(choice.find("branch")!=std::string::npos)p.target=pepperPoint(intent.object)+engine::Vec3{choice.starts_with("Left")?-.65f:.65f,0,.70f};
 }
 b.collision.synchronize(s);p.route=b.collision.route(s,cat->transform.position,p.target);
 p.elapsed=p.hold=0;p.segment=0;p.arrived=p.returning=false;p.active=!reduced;
 if(p.reaction==4)b.ending.active=p.active;
 if(reduced){p.stage=p.pendingStage;cat->transform.position=after.completed?p.target:p.home;}
}
void revealPepperEvidence(engine::Scene& s,SceneBindings& b){
 auto& p=b.pepper;
 if(p.inspectingClue>=3)return;
 const unsigned bit=1u<<p.inspectingClue;
 p.awaitingClues&=~bit;p.inspected|=bit;
 if(auto* m=s.findComponent(p.discoveries[p.inspectingClue]))m->enabled=true;
 pepperMarkers(s,b,b.completed);
}
void animatePepper(engine::Scene& s,SceneBindings& b,double delta){
 pepperMarkers(s,b,b.completed);
 auto& p=b.pepper;if(!p.active||delta<=0||!std::isfinite(delta)||b.requesters.empty())return;
 auto* cat=s.findActor(b.requesters[0].actor);if(!cat){p.active=b.ending.active=false;return;}
 auto* skin=cat->getComponent<engine::SkeletalMeshComponent>();if(!skin){p.active=b.ending.active=false;return;}
 if(b.reducedMotion){
  revealPepperEvidence(s,b);
  cat->transform.position=p.reaction==4?p.target:p.home;p.active=b.ending.active=false;p.stage=p.pendingStage;
  for(auto visual:p.visuals)if(auto* m=s.findComponent(visual.component))m->enabled=visual.stage==p.stage;
  cats::present(*skin,p.reaction==4?14:p.inspected==7?1:0,true);return;
 }
 p.elapsed+=float(delta);
 const bool arrival=!p.route.empty()&&b.collision.sample(s,*cat,p.route,p.segment,p.elapsed*pipWalkSpeed*1.6f);
 if(!arrival){sampleCatWalk(*cat,*skin,p.elapsed*1.6f,true);return;}
 if(p.returning){p.active=false;p.stage=p.pendingStage;pepperMarkers(s,b,b.completed);cats::present(*skin,p.inspected==7?1:0,b.reducedMotion);return;}
 if(!p.arrived)revealPepperEvidence(s,b);
 p.arrived=true;p.hold+=float(delta);turnCatTowards(*cat,engine::pi,delta);
 const char* clip=p.reaction==1||(p.reaction==4&&p.hold<1.8f)?"approach_bowl":p.reaction==2?"personality_reaction":p.reaction==4?"picnic_sit":"happy_pounce";
 sample(*skin,clip,p.reaction==4&&p.hold>=1.8f?p.hold-1.8f:p.hold);
 if(p.hold<(p.reaction==4?4.2f:p.reaction==2?2.f:1.8f))return;
 if(p.reaction==4){p.active=b.ending.active=false;cats::present(*skin,14,true);return;}
 // Correct choices reveal the next vignette only after Pepper checks the old one.
 if(p.reaction==3){p.stage=p.pendingStage;for(auto visual:p.visuals)if(auto* m=s.findComponent(visual.component))m->enabled=visual.stage==p.stage;
  for(auto id:p.discoveries)if(auto* m=s.findComponent(id))m->enabled=false;}
 b.collision.synchronize(s);p.route=b.collision.route(s,cat->transform.position,p.home);p.segment=0;p.elapsed=0;p.returning=true;
}
