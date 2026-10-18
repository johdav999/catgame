#include "views.hpp"
#include "release1/rules/content.hpp"
#include "release1/progression/policy.hpp"
#include <algorithm>
#include <sstream>
namespace paws::release1::gui {
using namespace engine;
namespace {
constexpr GuiColor clear{0,0,0,0},ink{37.f/255,62.f/255,55.f/255,1},cream{1,248.f/255,233.f/255,1};
void transparent(Widget& w){w.style.background=w.style.border=w.style.accent=clear;}
std::string cell(Cell c){return "("+std::to_string(c.x)+","+std::to_string(c.y)+")";}
std::string join(const std::vector<std::string>& names){std::string s;for(auto& n:names){if(!s.empty())s+=" > ";s+=n;}return s;}
std::string readable(std::string id){std::replace(id.begin(),id.end(),'_',' ');return id;}
}
Views::Views(PawsUi& ui,std::function<void(const Intent&)> send,
    std::function<void(std::string_view,const Intent&)> introduce,
    std::function<void(std::string_view)> placement,
    std::function<void(std::string_view,std::string_view)> preview):ui_(ui),dispatch_(std::move(send)),introduce_(std::move(introduce)),placementRequested_(std::move(placement)),previewRequested_(std::move(preview)) {
    ui_.composition_=[this]{return compose();};ui_.dirty_=true;
}
Views::~Views(){ui_.composition_={};ui_.dirty_=true;}
void Views::selectObject(std::string_view id){
    if(std::none_of(model_.puzzle.objects.begin(),model_.puzzle.objects.end(),[&](auto& o){return o.id==id;}))return;
    selected_=id;
}
void Views::project(Model m){
    if(m.puzzle.quest!=model_.puzzle.quest||m.puzzle.attempt!=model_.puzzle.attempt)selected_.clear();
    model_=std::move(m);
    auto base=model_.services;base.attempt=model_.progress.attempt;base.revision=model_.progress.revision;
    base.coins=model_.progress.coins;base.questTitle=model_.puzzle.title;base.shortTitle=model_.puzzle.title;
    base.canRestart=!model_.progress.quests.empty();base.canUndo=model_.puzzle.canUndo;
    base.inGarden=model_.services.inGarden;base.endingSeen=model_.progress.picnicEndingSeen;
    base.revealedHints=model_.puzzle.revealedHints;base.hints=model_.puzzle.hints;
    ui_.project(std::move(base));
}
Intent Views::command(Verb v,std::string object,std::string target) const {
    return {model_.progress.generation,model_.progress.attempt,model_.progress.revision,
        model_.puzzle.quest,std::move(object),std::move(target),v};
}
void Views::send(const Intent& i){if(dispatch_)dispatch_(i);}
void Views::beginPlacement(std::string item){
    item_=std::move(item);spot_.clear();
    if(placementRequested_)placementRequested_(item_);
    // Opening a preview never commits a move. Prefer the current placement,
    // then a clear compatible spot, so the sole legal destination is actionable.
    const auto owned=std::find_if(model_.progress.items.begin(),model_.progress.items.end(),[this](const auto& i){return i.id==item_;});
    auto chosen=model_.placement.end();
    if(owned!=model_.progress.items.end())chosen=std::find_if(model_.placement.begin(),model_.placement.end(),[&](const auto& s){return s.compatible&&s.id==owned->spot;});
    if(chosen==model_.placement.end())chosen=std::find_if(model_.placement.begin(),model_.placement.end(),[](const auto& s){return s.compatible;});
    if(chosen!=model_.placement.end())spot_=chosen->id;
    ui_.show(UiScreen::Placement);
    if(previewRequested_&&!spot_.empty())previewRequested_(item_,spot_);
}
void Views::portrait(Widget& p,std::string_view id){
    const auto at=model_.portraits.find(std::string(id));
    if(at!=model_.portraits.end())ui_.previewImage(p,at->second,std::string(id)+" portrait");
    else if(model_.developmentReferences)ui_.copy(p,"Development identity reference: "+std::string(id)+" (portrait pending).");
}
void Views::clues(Widget& p){
    auto& v=model_.puzzle;
    ui_.copy(p,v.objective);
    if(!v.symbols.empty()&&!v.pictureSequence){ui_.copy(p,"Picture clue: "+join(v.symbols));ui_.copy(p,"Entered: "+(v.enteredSymbols.empty()?std::string("none"):join(v.enteredSymbols)));}
    for(const auto& o:v.objects){
        std::string text=o.name+" "+cell(o.cell);
        if(o.role.find("cushion_")==0){text+="; clockwise quarter turns: "+std::to_string(o.orientation)+"; cells:";for(auto c:o.footprint)text+=" "+cell(c);}
        if(o.held)text+="; picked up";
        if(!o.socket.empty())text+="; at "+readable(o.socket);
        if(o.role.find("switch")==0||o.role.find("cat_")==0||o.role=="gate"||o.role=="playhouse_door"||o.role=="adjacency_clue")text+=o.active?"; YES / held, open or satisfied":"; NO / released, closed or unsatisfied";
        if(o.role=="shade_clue"||o.role=="quiet_clue"||o.role=="cushion_basket"){text+="; marked cells:";for(auto c:o.footprint)text+=" "+cell(c);}
        ui_.copy(p,text);
    }
    if(!v.route.empty()){std::string path=v.completed?"Connected route:":"Route trace (use Check or Roll to confirm):";for(auto c:v.route)path+=" "+cell(c);ui_.copy(p,path);}
    ui_.copy(p,"Coordinates are column,row from the top left. Pick an object and use its named actions; dragging and sound are optional.");
}
void Views::board(float x,float y,float width,float height){
    auto d=rules::quest(model_.puzzle.quest,model_.puzzle.content);
    // Loose shapes live partly beyond the basket; include their whole visible
    // silhouette instead of cropping a square/elbow at the authored tray edge.
    for(const auto& o:model_.puzzle.objects)if(!o.held&&!o.collected)for(auto c:o.footprint){d.width=std::max(d.width,c.x+1);d.height=std::max(d.height,c.y+1);}
    const float size=std::min(width/d.width,height/d.height),bw=size*d.width,bh=size*d.height;
    x+=(width-bw)/2;y+=(height-bh)/2;const float scale=ui_.settings_.largerText?1.25f:1.f;
    board_={x*scale,y*scale,bw*scale,bh*scale};
    auto& map=ui_.gui_->root().add<Panel>();transparent(map);map.setPosition({x,y});map.setSize({bw,bh});
    map.setAccessibleName("Puzzle board. Column,row from top left; full names, footprints and clues in Menu, Puzzle controls.");
    for(int row=0;row<d.height;++row)for(int col=0;col<d.width;++col){
        Cell c{col,row};std::string mark;bool occupied=false;
        if(std::find(d.shade.begin(),d.shade.end(),c)!=d.shade.end())mark="S";
        if(std::find(d.quiet.begin(),d.quiet.end(),c)!=d.quiet.end())mark+="Q";
        if(std::find(d.basket.begin(),d.basket.end(),c)!=d.basket.end())mark="+";
        for(std::size_t i=0;i<model_.puzzle.objects.size();++i){const auto& o=model_.puzzle.objects[i];
            if(o.role.find("clue")!=std::string::npos||o.role=="picnic_shade"||o.role=="picnic_quiet"||o.role=="cushion_basket")continue;
            if(o.collected||o.held)continue;
            if(o.cell==c||std::find(o.footprint.begin(),o.footprint.end(),c)!=o.footprint.end()){mark=std::to_string(i+1);occupied=true;}
        }
        const float inset=std::min(2.f,size/4);
        auto& box=map.add<Panel>();box.setPosition({col*size+inset,row*size+inset});box.setSize({size-2*inset,size-2*inset});box.style.background=cream;box.style.border=ink;
        if(occupied)box.style.background={.91f,.80f,.58f,1};
        box.setAccessibleName("Cell "+cell(c)+(mark.empty()?" empty":" mark "+mark));
        if(!mark.empty()&&size>8){auto art=ui_.textArt(mark,*ui_.control_);auto& label=box.add<Image>(art);const float h=std::min(size-8,float(art->height));label.setSize({h*art->width/art->height,h});label.setPosition({4,4});label.setAccessibilityHidden(true);}
    }
}
bool Views::compose(){
    const auto screen=ui_.screen_;
    // Preserve the existing service, settings, confirmation and error implementations.
    if(screen==UiScreen::Title||screen==UiScreen::Settings||screen==UiScreen::Recovery||screen==UiScreen::ImportConfirm||screen==UiScreen::RestartAdventure||screen==UiScreen::Restart||screen==UiScreen::Loading||screen==UiScreen::LoadFailure||screen==UiScreen::Gallery)return false;
    const float scale=ui_.settings_.largerText?1.25f:1.f,w=ui_.width_/scale,h=ui_.height_/scale;
    ui_.gui_->setScale(scale);const bool hud=screen==UiScreen::Garden||screen==UiScreen::Puzzle;
    const auto save=model_.services.save==SaveState::Saved?"Saved":model_.services.save==SaveState::Saving?"Saving…":model_.services.save==SaveState::Failed?"Save failed · Menu":model_.services.save==SaveState::Loading?"Loading save":model_.services.save==SaveState::SecondTab?"Temporary tab":"Temporary";
    if(hud){
        const auto q=progression::questIndex(model_.puzzle.quest);
        static const char* titles[]={"Pip's ball","Tidy toys","The little gate","Button's mouse","Sunny spot","Button's bed","Mabel's drink","Pepper's bells","Two kittens","Garden picnic","Follow the scent","Pip's wobbly perch","Pip's ball adventure","Come and see!","Room for two","A little higher","A cool drink","Stay a while?","Beach ball","Button's bridge","Mabel's shade","Picnic trail","Shell chimes","Beach picnic"};
        const bool narrow=w<560;
        const bool bridge=screen==UiScreen::Puzzle&&(q==4||q==13||q==15||q==19);
        const bool sequence=screen==UiScreen::Puzzle&&model_.puzzle.pictureSequence;
        const bool scent=screen==UiScreen::Puzzle&&(q==10||q==21), balance=screen==UiScreen::Puzzle&&(q==11||q==14);
        const bool bridgeSide=screen==UiScreen::Puzzle&&w>h*1.45f&&h<500;
        const bool compactHeader=w<900;
        const float titleY=compactHeader?76.f:12.f;
        const float menuWidth=112.f,titleWidth=bridgeSide?270.f:narrow?w-24:std::min(w-menuWidth-260,480.f),textWidth=titleWidth-32;
        // Replay wraps to two lines at this width. Reserve its row so the
        // narrow objective never runs beneath the menu controls.
        const float replayHeight=std::max(52.f,float(ui_.textArt("Replay\npictures",*ui_.control_)->height)+12);
        const bool lookout=screen==UiScreen::Garden&&std::any_of(model_.progress.quests.begin(),model_.progress.quests.end(),[](const auto& q){return q.id=="pip_friend_room"&&q.rewarded;});
        const bool nook=screen==UiScreen::Garden&&std::any_of(model_.progress.quests.begin(),model_.progress.quests.end(),[](const auto& q){return q.id=="mabel_friend_share"&&q.rewarded;});
        const float menuHeight=24+52+(screen==UiScreen::Garden&&model_.progress.beachDevelopmentAccess?64:0)+(sequence||lookout?12+replayHeight:0)+(nook?12+replayHeight:0);
        const float headingWidth=narrow?textWidth-menuWidth-12:textWidth;
        const std::string name=screen==UiScreen::Puzzle?(!narrow&&!bridgeSide?model_.puzzle.title:q<paws::release1::progression::quests.size()?titles[q]:"Our request"):"Our garden";
        const auto fitted=std::count_if(model_.puzzle.objects.begin(),model_.puzzle.objects.end(),[](const auto& o){return o.role=="plank"&&!o.socket.empty();});
        const auto supplied=std::count_if(model_.puzzle.objects.begin(),model_.puzzle.objects.end(),[](const auto& o){return o.role=="plank";});
        const std::string status=bridge?std::to_string(fitted)+" / "+std::to_string(supplied)+" bridges · "+save:"Chapter "+std::to_string(model_.progress.chapter)+" · "+save;
        std::string objective=screen==UiScreen::Puzzle?model_.puzzle.objective:"Tap a cat with ! to start its request.";
        if(screen==UiScreen::Garden&&model_.lookoutQuiet)objective="Pip saved you a seat. Stay as long as you like.";
        if(screen==UiScreen::Garden&&model_.nookQuiet)objective="Mabel and Pip are sharing the sunshine. There is no hurry.";
        if(screen==UiScreen::Puzzle&&q==0)objective="Bring the ball back to Pip.";
        const std::string sequenceWatch="Watch picture "+std::to_string(model_.puzzle.sequenceIndex+1)+" of "+std::to_string(model_.puzzle.symbols.size())+" on the blackboard.";
        const std::string sequenceTurn="Your turn: "+std::to_string(model_.puzzle.enteredSymbols.size())+" / "+std::to_string(model_.puzzle.symbols.size())+" correct. Repeat the blackboard pictures.";
        if(sequence)objective=model_.puzzle.sequenceWatching?sequenceWatch:sequenceTurn;
        if(screen==UiScreen::Puzzle&&q==3&&!model_.puzzle.completed){
            const auto front=std::find_if(model_.puzzle.objects.begin(),model_.puzzle.objects.end(),[](const auto& o){return o.id=="crate_a";});
            const auto rear=std::find_if(model_.puzzle.objects.begin(),model_.puzzle.objects.end(),[](const auto& o){return o.id=="crate_b";});
            const auto reach=std::find_if(model_.puzzle.objects.begin(),model_.puzzle.objects.end(),[](const auto& o){return o.id=="crate_c";});
            const bool extended=reach!=model_.puzzle.objects.end();
            if(front!=model_.puzzle.objects.end()&&front->cell==Cell{1,1})
                objective=rear!=model_.puzzle.objects.end()&&rear->cell==Cell{1,0}?"Button's path is blocked. Drag the rear crate sideways to the white rectangle.":"Drag the front crate toward the house to the white rectangle.";
            else if(extended&&front!=model_.puzzle.objects.end()&&front->cell==Cell{1,0})objective="Drag the front crate left to its white rectangle to free the shared space.";
            else if(extended&&rear!=model_.puzzle.objects.end()&&rear->cell==Cell{2,0})objective="Drag the rear crate back left to its white rectangle to free the reach crate's space.";
            else if(extended&&reach->cell==Cell{2,1})objective="Drag the reach crate toward the house to the white rectangle to clear Button's path.";
            else objective="The path is clear. Click the toy mouse beside the bench so Button can collect it.";
        }
        if(screen==UiScreen::Puzzle&&q!=0&&q!=3&&!sequence&&!model_.puzzle.completed&&std::any_of(model_.puzzle.actions.begin(),model_.puzzle.actions.end(),[](const auto& a){return a.intent.verb==Verb::Slide||a.intent.verb==Verb::Fit||a.intent.verb==Verb::AssignCat;}))
            objective+=" White rectangles show available drop spaces. Hold a piece to see its spaces.";
        const auto& message=model_.services.feedback;
        const bool repeatSequenceStatus=sequence&&(message=="Watch the blackboard again. Your answer starts fresh."||message.starts_with("Your turn.")||message.starts_with("Picture matched."));
        const std::string feedback=screen==UiScreen::Puzzle&&!repeatSequenceStatus&&message!="A little request is ready."&&message!=objective&&message!=model_.puzzle.objective?message:std::string{};
        // Size the unified card for wrapped objective/status text, including
        // larger text and portrait. The sole Menu button occupies its own corner.
        const auto lines=[&](const std::string& text){
            std::istringstream words(text);std::string word,line;unsigned count=1;
            while(words>>word){auto next=line.empty()?word:line+" "+word;
                if(!line.empty()&&ui_.gui_->font()->measure(next)>textWidth){++count;line=word;}else line=next;}
            return count;
        };
        const bool headingWraps=ui_.heading_->measure(name)>headingWidth;
        const float headingHeight=float(ui_.textArt(name,*ui_.heading_)->height)*(headingWraps?2.f:1.f)+(headingWraps?12.f:0.f);
        const float headingRow=narrow?std::max(menuHeight-titleY,headingHeight):headingHeight;
        // Keep the memory quest's frame stable across watching, matching and
        // error feedback. Changing this card's height makes the host refit its
        // camera, moving the next bell out from under the player's pointer.
        const bool sequenceFrame=sequence&&!model_.puzzle.completed;
        const bool showStatus=screen==UiScreen::Garden||model_.services.save==SaveState::Failed;
        const auto statusLines=showStatus?lines(status):0;
        const auto objectiveLines=sequenceFrame?std::max(lines(sequenceWatch),lines(sequenceTurn)):lines(objective);
        const auto feedbackLines=sequenceFrame?std::max(lines("Different picture. Your answer is cleared. Try again or replay the blackboard."),lines("Watch all the pictures first. Your turn starts when the blackboard goes blank.")):(feedback.empty()?0:lines(feedback));
        const float top=24+headingRow+8+(statusLines+objectiveLines+feedbackLines)*ui_.gui_->font()->lineHeight()+(showStatus?8:0)+((sequenceFrame||!feedback.empty())?8:0);
        auto& title=ui_.surface(12,titleY,titleWidth,top,false);
        title.setLayout(VerticalLayout{8});
        auto& heading=title.add<Panel>();transparent(heading);heading.setSize({textWidth,headingRow});heading.setLayout(VerticalLayout{12});
        ui_.contentWidth_=headingWidth;ui_.heading(heading,name);ui_.contentWidth_=textWidth;
        if(showStatus){auto& state=ui_.copy(title,status,model_.services.save==SaveState::Failed);state.setLiveRegion(true);}
        auto& description=ui_.copy(title,objective);description.style.foreground={89.f/255,104.f/255,91.f/255,1};
        if(!feedback.empty()){auto& notice=ui_.copy(title,feedback);notice.setLiveRegion(true);}
        int hearts=0;for(const auto& cat:model_.progress.cats)hearts+=cat.friendship;
        const std::string coinValue=std::to_string(model_.progress.coins),heartValue=std::to_string(hearts);
        const float coinWidth=std::max(80.f,ui_.control_->measure(coinValue)+56),heartWidth=std::max(80.f,ui_.control_->measure(heartValue)+56);
        const float resourceX=w-menuWidth-20-coinWidth-heartWidth-8;
        const auto counter=[&](float x,float width,const std::string& icon,const std::string& value,const std::string& label){
            auto& row=ui_.gui_->root().add<Panel>();transparent(row);row.setPosition({x,12});row.setSize({width,52});
            auto& background=row.add<Image>(ui_.tile(int(width),52,0,false));background.setSize({width,52});background.setAccessibilityHidden(true);
            auto& symbol=row.add<Image>(ui_.resourceIcon(icon));symbol.setPosition({7,8});symbol.setSize({36,36});symbol.setAccessibilityHidden(true);
            auto art=ui_.textArt(value,*ui_.control_);auto& text=row.add<Image>(art);text.setPosition({46,(52-float(art->height))/2});text.setSize({float(art->width),float(art->height)});text.setAccessibleName(label);text.setLiveRegion(true);
        };
        counter(resourceX,coinWidth,"coin",coinValue,coinValue+" coins");
        counter(resourceX+coinWidth+8,heartWidth,"heart",heartValue,heartValue+" hearts · total friendship");
        auto& menu=ui_.gui_->root().add<Panel>();transparent(menu);
        menu.setPosition({w-menuWidth-12,12});menu.setSize({menuWidth,menuHeight});menu.setLayout(VerticalLayout{12});ui_.contentWidth_=menuWidth;
        ui_.button(menu,"Menu",[this]{ui_.show(UiScreen::Pause);},false,true,"menu");
        if(screen==UiScreen::Garden&&model_.progress.beachDevelopmentAccess)ui_.button(menu,"Adventures",[this]{ui_.show(UiScreen::Adventures);});
        if(lookout)ui_.button(menu,model_.lookoutQuiet?"Leave lookout":"Sit with Pip",[this]{ui_.emit(UiAction::UseFurnishing,model_.lookoutQuiet?"leave_pip_lookout":"pip_lookout");});
        if(nook)ui_.button(menu,model_.nookQuiet?"Leave nook":"Sunny nook",[this]{ui_.emit(UiAction::UseFurnishing,model_.nookQuiet?"leave_mabel_nook":"mabel_nook");});
        if(sequence)ui_.button(menu,"Replay pictures",[this,c=command(Verb::ReplaySequence)]{send(c);},false,!model_.puzzle.completed);
        if(balance){auto& test=ui_.surface((w-200)/2,h-88,200,76,false);ui_.button(test,"Test balance",[this,c=command(Verb::TestBalance)]{send(c);},false,!model_.puzzle.completed);}
        // Quiet, direct puzzle controls. Specialized choice trays retain their
        // space; their Undo/Hint actions remain available in the pause menu.
        const bool actionTray=screen==UiScreen::Puzzle&&!scent&&!balance;
        if(actionTray){
            const float trayWidth=std::min(w-24,360.f),column=(trayWidth-44)/2;
            auto& tray=ui_.surface((w-trayWidth)/2,h-88,trayWidth,76,false);
            tray.setLayout(HorizontalLayout{12});ui_.contentWidth_=column;
            auto& undo=tray.add<Panel>();transparent(undo);undo.setSize({column,52});
            ui_.button(undo,"Undo",[this,c=command(Verb::Undo)]{send(c);},false,model_.puzzle.canUndo,"undo");
            auto& hint=tray.add<Panel>();transparent(hint);hint.setSize({column,52});
            ui_.button(hint,"Hint",[this]{if(!model_.puzzle.revealedHints)send(command(Verb::Hint));ui_.show(UiScreen::Hints);},true,true,"hint");
        }
        if(screen==UiScreen::Puzzle){
            const float choiceHeight=scent?(narrow?280.f:164.f):balance||actionTray?88.f:0.f;
            if(scent){
                auto& choices=ui_.surface(12,h-choiceHeight-12,w-24,choiceHeight,true);
                if(!narrow)choices.setLayout(HorizontalLayout{12});
                const float column=narrow?w-56:(w-80)/3;
                const bool foundClues=std::count_if(model_.puzzle.objects.begin(),model_.puzzle.objects.end(),[](const auto& o){return o.role=="trail_clue"&&o.active;})==3;
                for(int n=0;n<3;++n){
                    auto& group=choices.add<Panel>();transparent(group);group.setSize({column,narrow?76.f:choiceHeight-32});group.setLayout(VerticalLayout{8});ui_.contentWidth_=column;
                    const std::string clue="clue_"+std::to_string(n),trail="trail_"+std::to_string(n);
                    if(!narrow||!foundClues)ui_.button(group,(q==21?std::array{"Inspect paw prints","Inspect blanket threads","Inspect picnic scent"}[n]:std::array{"Inspect paw prints","Inspect fluff","Inspect scent"}[n]),[this,c=command(Verb::Inspect,clue)]{send(c);},false,!model_.puzzle.completed);
                    const auto o=std::find_if(model_.puzzle.objects.begin(),model_.puzzle.objects.end(),[&](const auto& o){return o.id==trail;});
                    if(!narrow||foundClues)ui_.button(group,"Follow: "+o->name,[this,c=command(Verb::FollowTrail,trail)]{send(c);},false,!model_.puzzle.completed);
                }
            }
            const float boardTop=top+titleY-12;
            const float boardHeight=std::max(1.f,h-boardTop-40-choiceHeight);
            if(model_.developmentReferences)board(12,boardTop+28,w-24,boardHeight);
            else board_={12*scale,(boardTop+28)*scale,(w-24)*scale,boardHeight*scale};
            if(bridgeSide)board_={(titleWidth+24)*scale,(sequence?12.f:96.f)*scale,(w-titleWidth-36)*scale,(h-(sequence?24.f:108.f)-choiceHeight)*scale};
        }
        return true;
    }
    if(screen==UiScreen::Completion){
        // Rewards are presented above the cats; keep the world unobscured.
        const bool story=model_.puzzle.quest.starts_with("pip_friend_")||model_.puzzle.quest.starts_with("mabel_friend_")||progression::isBeachQuest(model_.puzzle.quest);
        const bool beach=progression::isBeachQuest(model_.puzzle.quest);
        const float width=std::min(w-24,420.f),height=beach?300.f:story?228.f:128.f;
        auto& bar=ui_.surface((w-width)/2,h-height-12,width,height,false);
        if(story)ui_.copy(bar,std::string(dialogue()[progression::questIndex(model_.puzzle.quest)].speaker)+": "+std::string(dialogue()[progression::questIndex(model_.puzzle.quest)].completion));
        auto& status=ui_.copy(bar,beach?"Beach quest complete":model_.rewardGranted?"20 coins · Friendship +1":"A lovely practice round");
        status.setLiveRegion(true);
        if(beach){
            const auto q=progression::questIndex(model_.puzzle.quest);
            if(q+1<progression::quests.size())ui_.button(bar,"Continue beach story",[this,c=command(Verb::SelectQuest,{},std::string(progression::quests[q+1].id))]{send(c);},true);
            else ui_.button(bar,model_.progress.beachEndingSeen?"Replay beach ending":"Watch beach ending",[this,c=command(Verb::RecordBeachEnding)]{send(c);},true);
        }
        ui_.button(bar,"Back to garden",[this]{ui_.emit(UiAction::BackToGarden);},true);
        board_={12*scale,12*scale,(w-24)*scale,(h-height-36)*scale};
        return true;
    }
    const bool ending=screen==UiScreen::Ending||screen==UiScreen::BeachEnding,side=ending&&w>700;
    if(!ending){auto& dim=ui_.gui_->root().add<Panel>();dim.setSize({w,h});dim.style.background={ink.r,ink.g,ink.b,.42f};dim.style.border=clear;}
    const float cw=ending?(side?450.f:w-24):std::min(w-24,520.f),ch=ending?(side?h-24:std::min(h*.48f,360.f)):std::min(h-24,680.f);
    const float cx=ending?(side?w-cw-12:12):(w-cw)/2,cy=ending?(side?12:h-ch-12):(h-ch)/2;
    if(ending)board_=side?GuiRect{12*scale,12*scale,(w-cw-36)*scale,(h-24)*scale}:GuiRect{12*scale,12*scale,(w-24)*scale,(h-ch-36)*scale};
    auto& p=ui_.surface(cx,cy,cw,ch,true);p.setSize({cw-32,ch-88});
    const auto q=progression::questIndex(model_.puzzle.quest);
    const auto& copy=dialogue()[std::min(q,dialogue().size()-1)];
    switch(screen){
    case UiScreen::Adventures: {
        ui_.heading(p,"A Seaside Afternoon");
        ui_.copy(p,"Pip found a postcard of a sheltered beach cove. His ball has already gone exploring.");
        ui_.copy(p,"Development preview: six quests in a greybox cove. No payment or purchased ownership is enabled.");
        ui_.copy(p,std::to_string(std::count_if(model_.progress.quests.begin(),model_.progress.quests.end(),[](const auto& q){return progression::isBeachQuest(q.id)&&q.rewarded;}))+" / 6 beach requests complete");
        const auto next=std::find_if(model_.progress.quests.begin(),model_.progress.quests.end(),[](const auto& q){return progression::isBeachQuest(q.id)&&!q.rewarded;});
        const std::string nextId=next==model_.progress.quests.end()?"beach_picnic":next->id;
        ui_.button(p,model_.progress.beachComplete?"Visit beach picnic":"Start / Continue beach",[this,c=command(Verb::SelectQuest,{},nextId)]{send(c);},true,model_.progress.beachDevelopmentAccess);
        for(const auto& q:model_.progress.quests)if(progression::isBeachQuest(q.id))
            ui_.button(p,q.title+(q.rewarded?" · Revisit":q.unlocked?" · Continue":" · Locked"),[this,c=command(Verb::SelectQuest,{},q.id)]{send(c);},false,q.unlocked);
        if(model_.progress.beachEndingSeen)ui_.copy(p,"Postcard saved: a seaside afternoon with four friends.");
        break;
    }
    case UiScreen::BeachEnding:
        ui_.heading(p,"A Seaside Afternoon");
        ui_.copy(p,"Pip: The postcard was right. This is a lovely picnic spot.");
        ui_.copy(p,"Button: The little bridge made it feel close to home.");
        ui_.copy(p,"Mabel: And someone thought to make a shady place for me.");
        ui_.copy(p,"Pepper: We found the basket, played our tune, and saved everyone a seat!");
        ui_.copy(p,"Four friends settle on their mats as the sea rests beyond the cove. Your seaside postcard is saved. You can revisit any beach quest.");
        break;
    case UiScreen::Dialogue:
        ui_.heading(p,model_.services.inGarden?"Requests & friends":model_.puzzle.title);
        if(model_.services.inGarden)ui_.copy(p,"Choose a friend to hear their request. Completed requests can be practised.");
        else if(progression::isBeachQuest(model_.puzzle.quest))ui_.copy(p,"Seaside Afternoon · request "+std::to_string(q-progression::baseQuestCount+1)+" of 6");
        else if(model_.puzzle.quest.starts_with("pip_friend_"))ui_.copy(p,"Pip's friendship story · "+std::to_string(q-11)+" of 3");
        else if(model_.puzzle.quest.starts_with("mabel_friend_"))ui_.copy(p,"A little patch of sunshine · "+std::to_string(q-14)+" of 3");
        {
            ui_.heading(p,std::string(copy.speaker));
            const auto mask=q<paws::release1::progression::quests.size()?progression::quests[q].requesters:0;
            for(unsigned i=0;i<4;++i)if(mask&(1u<<i))portrait(p,progression::catIds[i]);
            ui_.copy(p,std::string(copy.request));
            if(!model_.services.inGarden)ui_.button(p,"Help "+std::string(copy.speaker),[this]{ui_.enter(UiScreen::Puzzle);},true);
        }
        if(!model_.services.inGarden)break;
        for(const auto& quest:model_.progress.quests){if(progression::isBeachQuest(quest.id)||progression::isBeachQuest(model_.puzzle.quest))continue;if(quest.id=="pip_friend_ball"){ui_.heading(p,"Pip's friendship story");ui_.copy(p,"Three little adventures and a lookout to share.");}if(quest.id=="mabel_friend_sun"){ui_.heading(p,"A little patch of sunshine");ui_.copy(p,"Mabel learns to share her favourite place.");}ui_.button(p,quest.title+(quest.rewarded?" · Practice":quest.unlocked?" · Available":" · Locked"),[this,c=command(Verb::SelectQuest,{},quest.id)]{send(c);},false,quest.unlocked);}
        ui_.heading(p,"Our friends");
        for(const auto& cat:model_.progress.cats){const auto i=std::find(progression::catIds.begin(),progression::catIds.end(),cat.id)-progression::catIds.begin();
            const auto max=i<4?progression::milestones[i][2]:3;constexpr const char* labels[]={"Getting acquainted","Acquainted","Friendly","At home"};
            ui_.copy(p,cat.name+" · "+std::to_string(cat.friendship)+"/"+std::to_string(max)+" friendship · "+(cat.introduced?labels[std::clamp(cat.milestone,0,3)]:"On the way"));}
        break;
    case UiScreen::Objects:
        ui_.heading(p,model_.puzzle.title);ui_.copy(p,model_.puzzle.objective);break;
    case UiScreen::Hints:
        ui_.heading(p,"A little nudge");ui_.copy(p,"Hints are free. Take as much time as you like.");
        ui_.copy(p,model_.puzzle.objective);
        for(unsigned i=0;i<std::min(model_.puzzle.revealedHints,3u);++i)ui_.copy(p,std::to_string(i+1)+". "+model_.puzzle.hints[i]);
        ui_.button(p,model_.puzzle.revealedHints<3?"Next hint":"All hints revealed",[this,c=command(Verb::Hint)]{send(c);},true,model_.puzzle.revealedHints<3);break;
    case UiScreen::Completion:
        ui_.heading(p,"Thank you!");ui_.copy(p,std::string(copy.completion));
        ui_.copy(p,model_.rewardGranted?"Earned 20 coins and 1 friendship for each requesting cat: "+std::string(copy.speaker)+".":"A lovely practice round. This request's reward has already been earned.");
        break;
    case UiScreen::Pause:
        if(model_.progress.beachDevelopmentAccess)ui_.button(p,"Adventures",[this]{ui_.show(UiScreen::Adventures);});
        ui_.heading(p,"Take your time");ui_.copy(p,std::to_string(model_.progress.coins)+" coins · Chapter "+std::to_string(model_.progress.chapter)+" · "+save);
        ui_.fpsLabel_=&ui_.copy(p,ui_.fpsText_);ui_.fpsLabel_->setWrap(false);
        ui_.button(p,"Continue",[this]{ui_.close();},true);
        if(model_.services.inGarden)ui_.button(p,"Requests & friends",[this]{ui_.show(UiScreen::Dialogue);});
        else {
            ui_.button(p,"Undo",[this,c=command(Verb::Undo)]{send(c);},false,model_.puzzle.canUndo);
            ui_.button(p,"Hint",[this]{if(!model_.puzzle.revealedHints)send(command(Verb::Hint));ui_.show(UiScreen::Hints);});
        }
        ui_.button(p,"Settings",[this]{ui_.show(UiScreen::Settings);});
        ui_.button(p,"Restart puzzle",[this]{ui_.show(UiScreen::Restart);},false,ui_.model_.canRestart);
        ui_.button(p,"New Game",[this]{ui_.show(UiScreen::RestartAdventure);});
        ui_.button(p,"Back to garden",[this]{ui_.emit(UiAction::BackToGarden);});
        ui_.button(p,"Shop",[this]{ui_.show(UiScreen::Shop);},false,model_.services.inGarden);
        ui_.button(p,"Picnic celebration",[this,c=command(Verb::RecordPicnic)]{send(c);},false,model_.services.inGarden&&model_.progress.picnicEligible&&!model_.progress.picnicEndingSeen&&model_.finaleReason.empty());
        if(!model_.finaleReason.empty())ui_.copy(p,model_.finaleReason);
        ui_.button(p,"Replay picnic celebration",[this,c=command(Verb::ReplayPicnic)]{send(c);},false,model_.progress.picnicEndingSeen&&model_.finaleReason.empty());
        if(!model_.progress.picnicEligible)ui_.copy(p,"The picnic celebration opens after all ten requests and one owned furnishing placed. To restart the whole adventure, choose New Game.");
        else if(!model_.progress.picnicEndingSeen)ui_.copy(p,"Return to the garden to watch the picnic celebration. Replay becomes available after you have watched it.");
        ui_.button(p,"Save and recovery",[this]{ui_.show(UiScreen::Recovery);});
        break;
    case UiScreen::Shop:
        ui_.heading(p,"Little garden comforts");ui_.copy(p,std::to_string(model_.progress.coins)+" coins to spend");
        for(const auto& item:model_.progress.items){
            auto at=model_.thumbnails.find(item.id);if(at!=model_.thumbnails.end())ui_.previewImage(p,at->second,item.name);
            ui_.button(p,item.name+" · "+(item.owned?std::string("Owned"):!item.unlocked?std::string("Locked"):std::to_string(item.price)+" coins"),[this,id=item.id]{item_=id;ui_.show(UiScreen::Details);});
        }break;
    case UiScreen::Details:
        if(!model_.services.inGarden){ui_.heading(p,"Clues and connections");clues(p);break;}
        for(const auto& item:model_.progress.items)if(item.id==item_){
            ui_.heading(p,item.name);auto art=model_.thumbnails.find(item.id);if(art!=model_.thumbnails.end())ui_.previewImage(p,art->second,item.name+" preview");ui_.copy(p,item.owned?"Already yours. Moving costs no coins.":std::to_string(item.price)+" coins");
            static const char* use[]={"Button curls up; friends may rest.","A cool drink for Mabel.","Pip scratches and stretches.","Mabel climbs a supported route and settles.","Button rests in the hammock.","Pip and Pepper enter and look out.","Friends gather around the table.","Pepper and friends sit and rest."};
            const auto n=progression::itemIndex(item.id);if(n<8)ui_.copy(p,use[n]);
            if(!item.reason.empty())ui_.copy(p,item.reason);
            const bool canBuy=!item.owned&&item.unlocked&&item.available&&item.reason.empty();
            ui_.button(p,item.owned?"Owned":"Buy",[this,c=command(Verb::Buy,item.id)]{send(c);},true,canBuy);
            ui_.button(p,"Choose a spot",[this,id=item.id]{beginPlacement(id);},false,item.owned&&item.available);
            if(item.owned&&!item.spot.empty()){
                const auto why=model_.useReasons.find(item.id);
                ui_.button(p,"Use furnishing",[this,id=item.id]{ui_.emit(UiAction::UseFurnishing,id);ui_.enter(UiScreen::Garden);},false,why==model_.useReasons.end()||why->second.empty());
                if(why!=model_.useReasons.end()&&!why->second.empty())ui_.copy(p,why->second);
                if(item.id=="garden_cushion"||item.id=="drinking_bowl"){
                    const auto pip=model_.useReasons.find("pip:"+item.id);
                    ui_.button(p,item.id=="garden_cushion"?"Invite Pip to rest":"Invite Pip to drink",[this,id=item.id]{ui_.emit(UiAction::UseFurnishing,"pip:"+id);ui_.enter(UiScreen::Garden);},false,pip!=model_.useReasons.end()&&pip->second.empty());
                    if(pip!=model_.useReasons.end()&&!pip->second.empty())ui_.copy(p,"Pip: "+pip->second);
                }
            }
            if(!item.spot.empty())ui_.copy(p,"Placed at "+readable(item.spot));
        }break;
    case UiScreen::Placement:
        ui_.heading(p,"Choose a spot");ui_.copy(p,h<500?"Choose a spot, then Place.":"Choose a suitable spot, then Place. Cancel keeps the current position and your coins.");
        if(!spot_.empty())ui_.copy(p,"Selected spot: "+readable(spot_));
        else ui_.copy(p,"No clear spot is available. Move the furnishing occupying a suitable spot first.",true);
        for(const auto& spot:model_.placement){
            const auto item=progression::itemIndex(item_),destination=progression::spotIndex(spot.id);
            // Role/footprint mismatches are destinations for other furnishings.
            // Keep suitable occupied spots visible with their actual blockage.
            if(item>=progression::items.size()||destination>=progression::spots.size()||!progression::compatible(progression::items[item],progression::spots[destination]))continue;
            auto& b=ui_.button(p,readable(spot.id)+" · "+(spot.compatible?"Fits":"Blocked"),[this,id=spot.id]{spot_=id;if(previewRequested_)previewRequested_(item_,spot_);ui_.dirty_=true;},spot.id==spot_);
            b.setAccessibleDescription((spot.id==spot_?"Selected. ":"")+spot.reason);
            if(!spot.reason.empty())ui_.copy(p,spot.reason,!spot.compatible);
        }
        break;
    case UiScreen::Ending:
        ui_.heading(p,"A garden full of friends");
        {auto& row=p.add<Panel>();transparent(row);row.setStretch(true);row.setLayout(HorizontalLayout{8});const float edge=std::min(h<500?32.f:80.f,(ui_.contentWidth_-24)/4);row.setSize({ui_.contentWidth_,edge});
            std::string missing;for(const auto& cat:model_.progress.cats){const auto at=model_.portraits.find(cat.id);if(at!=model_.portraits.end()){auto& image=row.add<Image>(at->second);image.setSize({edge,edge});image.setAccessibleName(cat.name+" portrait");}else if(model_.developmentReferences){if(!missing.empty())missing+=", ";missing+=cat.name;}}
            if(!missing.empty()&&h>=500)ui_.copy(p,"Development portraits pending: "+missing+".");}
        ui_.copy(p,"Pip: A little garden, and all our favourite friends.");ui_.copy(p,"Mabel: A sunny afternoon tastes even sweeter together.");
        ui_.copy(p,"Button: There's a quiet, cosy place for everyone.");ui_.copy(p,"Pepper: And tomorrow we can play those pictures again!");
        ui_.copy(p,"Thank you for making everyone feel at home. Keep furnishing the garden or practise any request. The picnic can be replayed from Menu.");
        break;
    default:break;
    }
    auto& footer=ui_.gui_->root().add<Panel>();transparent(footer);footer.setPosition({cx+16,cy+ch-64});footer.setSize({cw-32,52});ui_.contentWidth_=cw-32;
    if(screen==UiScreen::Placement){
        footer.setLayout(HorizontalLayout{12});ui_.contentWidth_=(ui_.contentWidth_-12)/2;
        auto& a=footer.add<Panel>();transparent(a);a.setSize({ui_.contentWidth_,52});
        const auto at=std::find_if(model_.placement.begin(),model_.placement.end(),[this](const auto& s){return s.id==spot_;});
        ui_.button(a,"Place",[this,c=command(Verb::PlaceFurnishing,item_,spot_)]{send(c);},true,at!=model_.placement.end()&&at->compatible);
        auto& b=footer.add<Panel>();transparent(b);b.setSize({ui_.contentWidth_,52});ui_.button(b,"Cancel",[this]{spot_.clear();ui_.emit(UiAction::CancelPlacement);ui_.close();});
    }else if(screen==UiScreen::Ending||screen==UiScreen::BeachEnding||screen==UiScreen::Completion)ui_.button(footer,screen==UiScreen::Ending?"Continue in garden":"Back to garden",[this]{ui_.emit(UiAction::BackToGarden);},true);
    else ui_.button(footer,"Back",[this]{ui_.close();});
    return true;
}
}

