#include "paws_ui.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace paws {
using namespace engine;
namespace {
constexpr GuiColor cream{1,248.f/255,233.f/255,1}, ink{37.f/255,62.f/255,55.f/255,1};
constexpr GuiColor teal{40.f/255,102.f/255,93.f/255,1}, linen{232.f/255,221.f/255,199.f/255,1};
constexpr GuiColor muted{89.f/255,104.f/255,91.f/255,1}, error{169.f/255,67.f/255,53.f/255,1};
constexpr GuiColor clear{0,0,0,0};
void transparent(Widget& w) { w.style.background=w.style.border=w.style.accent=clear; }
std::string focusedName(GuiSystem& gui) {
    for(const auto& n:gui.accessibilityNodes()) if(n.focused) return n.name;
    return {};
}
void focusNamed(GuiSystem& gui,const std::string& name) {
    auto nodes=gui.accessibilityNodes();
    for(const auto& n:nodes) if(!name.empty()&&n.name==name&&n.role==GuiRole::Button&&!n.disabled&&gui.focusWidget(n.id)) return;
    for(const auto& n:nodes) if(n.role==GuiRole::Button&&!n.disabled&&gui.focusWidget(n.id)) return;
}
std::shared_ptr<Texture> texture(int w,int h,const std::string& name) {
    auto t=std::make_shared<Texture>();t->width=w;t->height=h;t->name=name;
    t->wrapU=t->wrapV=TextureWrap::Clamp;t->pixels.resize(w*h*4);return t;
}
void pixel(Texture& t,int x,int y,GuiColor c,float alpha=1) {
    if(x<0||y<0||x>=int(t.width)||y>=int(t.height)) return;
    auto i=(y*t.width+x)*4;t.pixels[i]=std::uint8_t(c.r*255);t.pixels[i+1]=std::uint8_t(c.g*255);
    t.pixels[i+2]=std::uint8_t(c.b*255);t.pixels[i+3]=std::uint8_t(std::clamp(c.a*alpha,0.f,1.f)*255);
}
float roundDistance(float x,float y,float w,float h,float r) {
    auto qx=std::abs(x-w/2)-(w/2-r),qy=std::abs(y-h/2)-(h/2-r);
    return std::sqrt(std::max(qx,0.f)*std::max(qx,0.f)+std::max(qy,0.f)*std::max(qy,0.f))+std::min(std::max(qx,qy),0.f)-r;
}
}
PawsUi::PawsUi(std::array<std::vector<std::byte>,3> fonts,bool gallery,std::function<void(const UiIntent&)> dispatch)
    :fontBytes_(std::move(fonts)),dispatch_(std::move(dispatch)),gallery_(gallery) {
    gui_=std::make_unique<GuiSystem>();setFonts();
    if(gallery_) {
        screen_=UiScreen::Gallery;model_.puzzleReady=model_.canUndo=model_.canContinue=model_.shopOpen=true;
        model_.canRestart=model_.storageAvailable=model_.canExport=model_.inGarden=true;
        model_.coins=40;model_.friendship=2;model_.rewardGranted=model_.rewardsReady=true;model_.rewardCoins=20;model_.rewardFriendship=1;
        model_.objects={{"pot_left","Left terracotta pot","Slides between the two left slots."},{"pot_right","Right terracotta pot","Slides between the two right slots."},{"curve","Curved channel","Turns in quarter turns."}};
        model_.selectedId="curve";model_.selectedName="Curved channel";
        model_.actions={{UiAction::Move,"Move left","pot_left","The other pot is in the way.",false},{UiAction::Rotate,"Rotate","curve","",true},{UiAction::Roll,"Roll ball","ball","",true},{UiAction::PickUp,"Pick up","mouse","The gate is closed.",false},{UiAction::Place,"Place toy","socket","Choose a matching recess.",false}};
    }
}
void PawsUi::setFonts() {
    gui_->setFont(GuiFont::loadTrustedTrueType(fontBytes_[0],26));
    control_=GuiFont::loadTrustedTrueType(fontBytes_[1],26);
    heading_=GuiFont::loadTrustedTrueType(fontBytes_[2],36);
    wordmark_=GuiFont::loadTrustedTrueType(fontBytes_[2],68);
}
void PawsUi::project(UiModel model) { if(model.attempt!=model_.attempt) hintStep_=1;model_=std::move(model);dirty_=true; }
void PawsUi::show(UiScreen screen) {
    if(screen==screen_) return;
    if(!gallery_)emit(UiAction::ConfirmSound,"ui_panel");
    history_.push_back({screen_,focusedName(*gui_)});
    screen_=screen;pendingFocus_=screen==UiScreen::Restart?"Keep playing":"__first_control__";dirty_=true;down_=false;
}
void PawsUi::close() {
    if(history_.empty()) { show(UiScreen::Garden);return; }
    if(!gallery_)emit(UiAction::ConfirmSound,"ui_panel");
    screen_=history_.back().first;pendingFocus_=history_.back().second;history_.pop_back();
    if(screen_==UiScreen::Gallery) preview_=false;
    dirty_=true;down_=false;
}
void PawsUi::enter(UiScreen screen) {
    history_.clear();screen_=screen;pendingFocus_="__first_control__";dirty_=true;down_=false;
}
void PawsUi::escape() {
    if(preview_&&!history_.empty()) {close();return;}
    if(screen_==UiScreen::Garden||screen_==UiScreen::Puzzle) show(UiScreen::Pause);
    else if(screen_==UiScreen::ImportConfirm) {emit(UiAction::CancelImport);close();}
    else if(screen_==UiScreen::Placement) {emit(UiAction::CancelPlacement);close();}
    else if(screen_==UiScreen::Ending||screen_==UiScreen::Completion) emit(UiAction::BackToGarden);
    else if(screen_==UiScreen::LoadFailure) emit(UiAction::BackToGarden);
    else if(screen_!=UiScreen::Title&&screen_!=UiScreen::Gallery) close();
}
void PawsUi::emit(UiAction action,std::string target) {
    if(action==UiAction::SelectObject) pendingFocus_="__first_control__";
    if(preview_&&action!=UiAction::ResetView&&action!=UiAction::ZoomIn&&action!=UiAction::ZoomOut&&action!=UiAction::Settings) {
        model_.feedback="Preview only. No progress or purchases changed.";dirty_=true;return;
    }
    if(dispatch_) dispatch_({action,std::move(target),model_.attempt,model_.revision});
}
void PawsUi::pointer(Vec2 point,bool down) { pointer_=point;down_=down; }

std::shared_ptr<const Texture> PawsUi::textArt(const std::string& text,GuiFont& font,bool light) {
    std::string key="text:"+text+":"+std::to_string(font.lineHeight())+":"+std::to_string(light);
    if(art_.contains(key)) return art_.at(key);
    float maxWidth=0;int lines=0;std::istringstream stream(text);std::string line;
    while(std::getline(stream,line)) {maxWidth=std::max(maxWidth,font.measure(line));++lines;}
    auto t=texture(std::max(1,int(std::ceil(maxWidth))+4),int(font.lineHeight())*std::max(1,lines)+4,key);
    float x=2,top=0;
    for(std::size_t i=0;i<text.size();) {
        unsigned char c=text[i++];std::uint32_t cp=c;
        if(c=='\n') {x=2;top+=font.lineHeight();continue;}
        if(c>=0xc0) { int n=c<0xe0?1:c<0xf0?2:3;cp=c&((1<<(6-n))-1);while(n--&&i<text.size()) cp=(cp<<6)|(text[i++]&63); }
        const auto glyph=font.glyph(cp);auto atlas=font.atlas(glyph.page);
        if(atlas) for(int y=0;y<int(glyph.bounds.height);++y) for(int gx=0;gx<int(glyph.bounds.width);++gx) {
            int sx=int(std::round(glyph.uv.x*atlas->width))+gx,sy=int(std::round(glyph.uv.y*atlas->height))+y;
            if(sx<int(atlas->width)&&sy<int(atlas->height)) pixel(*t,int(x+glyph.bounds.x)+gx,int(top+font.ascent()+glyph.bounds.y)+y,
                light?cream:ink,atlas->pixels[(sy*atlas->width+sx)*4+3]/255.f);
        }
        x+=glyph.advance;
    }
    art_[key]=t;return t;
}
// Original rounded, 24-unit line icon family and cream stationery skin. Code is
// the editable master; no external or generated screenshot supplies controls.
std::shared_ptr<const Texture> PawsUi::tile(int w,int h,int state,bool primary,const std::string& icon) {
    std::string key="tile:"+std::to_string(w)+":"+std::to_string(h)+":"+std::to_string(state)+":"+std::to_string(primary)+icon;
    if(art_.contains(key)) return art_.at(key);
    auto t=texture(w,h,key);GuiColor fill=primary?teal:cream,edge=teal;
    if(state==4) {fill=linen;edge=muted;}
    if(state==1) fill=primary?GuiColor{.12f,.32f,.29f,1}:linen;
    if(state==2) fill=primary?ink:GuiColor{.85f,.81f,.73f,1};
    for(int y=0;y<h;++y) for(int x=0;x<w;++x) {
        float d=roundDistance(x+.5f,y+.5f,float(w),float(h),state==5?16.f:12.f);
        GuiColor c=d>-2?edge:fill;
        if(state==3) c=d>-3?teal:d>-5?cream:fill;
        pixel(*t,x,y,c,std::clamp(.5f-d,0.f,1.f));
    }
    if(!icon.empty()) {
        GuiColor color=primary?cream:ink;
        const auto line=[&](float x1,float y1,float x2,float y2) {
            for(int y=0;y<24;++y) for(int x=0;x<24;++x) {
                float dx=x2-x1,dy=y2-y1,u=std::clamp(((x-x1)*dx+(y-y1)*dy)/std::max(.001f,dx*dx+dy*dy),0.f,1.f);
                float d=std::hypot(x-x1-u*dx,y-y1-u*dy);
                if(d<1.5f) pixel(*t,x+16,y+(h-24)/2,color,std::clamp(1.5f-d,0.f,1.f));
            }
        };
        if(icon=="menu") {line(3,5,21,5);line(3,12,21,12);line(3,19,21,19);}
        else if(icon=="home") {line(3,11,12,3);line(12,3,21,11);line(6,9,6,21);line(6,21,18,21);line(18,21,18,9);}
        else if(icon=="hint") {line(8,17,16,17);line(9,21,15,21);line(8,17,5,10);line(5,10,8,4);line(8,4,16,4);line(16,4,19,10);line(19,10,16,17);}
        else if(icon=="undo"||icon=="rotate") {line(3,8,17,8);line(17,8,21,12);line(21,12,21,17);line(21,17,15,20);line(3,8,8,3);line(3,8,8,13);}
        else if(icon=="coin") {for(int i=0;i<24;++i){float a=i*6.283185f/24,b=(i+1)*6.283185f/24;line(12+8*std::cos(a),12+8*std::sin(a),12+8*std::cos(b),12+8*std::sin(b));}line(12,7,12,17);}
        else if(icon=="paw") {line(6,17,12,11);line(12,11,18,17);line(18,17,6,17);line(4,8,5,6);line(10,5,10,3);line(15,5,15,3);line(20,8,19,6);}
        else {line(5,12,10,17);line(10,17,20,6);}
    }
    art_[key]=t;return t;
}
Panel& PawsUi::surface(float x,float y,float w,float h,bool scroll) {
    auto& outer=gui_->root().add<Panel>();transparent(outer);outer.setPosition({x,y});outer.setSize({w,h});
    auto& bg=outer.add<Image>(tile(int(w),int(h),5,false));bg.setSize({w,h});bg.setAccessibilityHidden(true);
    Panel* inner=scroll?static_cast<Panel*>(&outer.add<ScrollPanel>()):&outer.add<Panel>();
    transparent(*inner);inner->style.background=cream;inner->style.accent=teal;inner->setPosition({16,12});inner->setSize({w-32,h-24});
    inner->setLayout(VerticalLayout{12});contentWidth_=w-32;return *inner;
}
TextLabel& PawsUi::copy(Widget& parent,std::string text,bool invalid) {
    auto& l=parent.add<TextLabel>(std::move(text));l.setStretch(true);l.setWrap(true);l.style.foreground=invalid?error:ink;return l;
}
void PawsUi::heading(Widget& parent,const std::string& text,bool wordmark) {
    auto img=textArt(text,wordmark?*wordmark_:*heading_);
    // Long headings wrap as native text instead of shrinking the font.
    if(img->width>contentWidth_) {
        const auto split=text.rfind(' ',text.size()/2);
        if(split!=std::string::npos) {heading(parent,text.substr(0,split),wordmark);heading(parent,text.substr(split+1),wordmark);}
        else copy(parent,text);
        return;
    }
    auto& label=parent.add<Image>(img);label.setSize({float(img->width),float(img->height)});label.setAccessibleName(text);
}
Button& PawsUi::button(Widget& parent,const std::string& label,std::function<void()> action,bool primary,bool enabled,std::string icon,std::string sound) {
    std::istringstream words(label);std::string word,line,wrapped;float available=contentWidth_-(icon.empty()?28:68);
    while(words>>word) {auto candidate=line.empty()?word:line+" "+word;if(!line.empty()&&control_->measure(candidate)>available){wrapped+=line+"\n";line=word;}else line=candidate;}
    wrapped+=line;auto lettering=textArt(wrapped,*control_,primary&&enabled);
    float height=std::max(52.f,float(lettering->height)+12);
    auto& row=parent.add<Panel>();transparent(row);row.setStretch(true);row.setSize({contentWidth_,height});
    auto& bg=row.add<Image>();bg.setStretch(true);bg.setSize({contentWidth_,height});bg.setAccessibilityHidden(true);
    auto& b=row.add<Button>();transparent(b);b.setStretch(true);b.setSize({contentWidth_,height});
    b.setAccessibleName(label);b.setEnabled(enabled);b.onClick([this,action=std::move(action),sound=std::move(sound)]{if(!gallery_)emit(UiAction::ConfirmSound,sound);action();});
    auto& text=row.add<Image>(lettering);
    text.setAccessibilityHidden(true);text.setSize({float(text.texture()->width),float(text.texture()->height)});
    text.setPosition({icon.empty()?12.f:52.f,(height-text.texture()->height)/2});
    skins_.push_back({&b,&bg,&text,label,std::move(icon),primary,-1,0,height});return b;
}
void PawsUi::previewImage(Widget& parent,std::shared_ptr<const Texture> image,const std::string& name) {
    if(image) {auto& i=parent.add<Image>(image);i.setSize({96,96});i.setAccessibleName(name);}
    else if(preview_) copy(parent,name+" will appear here after art production.");
}

void PawsUi::build() {
    const float scale=settings_.largerText?1.25f:1.f;
    const float w=width_/scale,h=height_/scale;
    gui_->setScale(scale);
    const bool hud=screen_==UiScreen::Garden||screen_==UiScreen::Puzzle;
    const bool shortView=h<500;
    if(!hud) {auto& backdrop=gui_->root().add<Panel>();backdrop.setSize({w,h});backdrop.style.background={ink.r,ink.g,ink.b,.42f};backdrop.style.border=clear;}
    const bool placing=screen_==UiScreen::Placement,ending=screen_==UiScreen::Ending;
    const float cardWidth=std::min(w-24,placing||ending?380.f:520.f);
    const float preferred=screen_==UiScreen::Title?500.f:screen_==UiScreen::Dialogue?460.f:screen_==UiScreen::Restart?360.f:680.f;
    const float cardHeight=std::min(h-24,placing?(w<600?h*.55f:600.f):ending?380.f:preferred);
    const bool puzzleSheet=screen_==UiScreen::Objects||screen_==UiScreen::Hints;
    const float cardX=puzzleSheet||(placing&&w>=600)?w-cardWidth-12:ending&&w>=600?12:(w-cardWidth)/2;
    const float cardY=placing?h-cardHeight-12:ending?12:std::max(12.f,(h-cardHeight)/2);
    const bool footer=!hud&&(screen_!=UiScreen::Title||preview_)&&screen_!=UiScreen::Gallery&&(screen_!=UiScreen::Ending||preview_);
    const float hudWidth=shortView?std::min(420.f,(w-36)/2):std::min(w-24,420.f);
    auto& p=surface(hud?12:cardX,hud?12:cardY,hud?hudWidth:cardWidth,hud?(shortView?68:w<=780?112:142):cardHeight,!hud);
    if(footer) p.setSize({cardWidth-32,cardHeight-88});
    const auto back=[](std::string ="Close") {}; // Fixed footer below the scroll region.
    if(preview_&&!hud) copy(p,"Component preview · no game progress");
    switch(screen_) {
    case UiScreen::Title:
        heading(p,"Paws",true);heading(p,"A little garden.");copy(p,"A little wonder. Make yourself at home.");
        copy(p,"Pip has a little request for you. Explore at your own pace.");
        button(p,"Play",[this]{if(preview_)show(UiScreen::Garden);else emit(UiAction::Play);},true,model_.puzzleReady,"home");
        button(p,"Continue",[this]{emit(UiAction::Continue);},false,model_.canContinue);
        copy(p,model_.canContinue?"Continue your saved afternoon.":"No saved afternoon is available.");
        button(p,"Settings",[this]{show(UiScreen::Settings);});
        button(p,"Save and recovery",[this]{show(UiScreen::Recovery);});
        if(!model_.storageNotice.empty())copy(p,model_.storageNotice);break;
    case UiScreen::Garden:
    case UiScreen::Puzzle: {
        const bool puzzle=screen_==UiScreen::Puzzle;
        heading(p,puzzle?(w<500||shortView?model_.shortTitle:model_.questTitle):"Paws");
        const auto saveLabel=model_.save==SaveState::Saved?"Saved":model_.save==SaveState::Saving?"Saving…":model_.save==SaveState::Failed?"Save failed":model_.save==SaveState::Loading?"Loading save":model_.save==SaveState::SecondTab?"Temporary tab":"Temporary";
        if(!shortView&&w<=780) {
            auto& line=copy(p,std::to_string(model_.coins)+" coins · "+std::to_string(model_.friendship)+"/3 friends · "+saveLabel,model_.save==SaveState::Failed);
            line.setLiveRegion(true);
        } else if(!shortView) copy(p,preview_?"Component preview":puzzle?(model_.feedback.empty()?"Tap to select or turn. Drag to move. Tap the ball to roll.":model_.feedback):model_.catUse.empty()?"A quiet corner for a little cat.":model_.catUse);
        // Keep the scene open; secondary actions live in a sheet.
        const float bottomHeight=shortView?76:140;
        auto& tray=surface(12,h-bottomHeight-12,shortView?std::min(w-24,620.f):std::min(w-24,420.f),bottomHeight,false);
        if(shortView) {
            tray.setLayout(HorizontalLayout{12});contentWidth_=(contentWidth_-12)/2;
            auto& a=tray.add<Panel>();transparent(a);a.setSize({contentWidth_,52});
            button(a,puzzle?"Actions":"Meet Pip",[this]{show(screen_==UiScreen::Puzzle?UiScreen::Objects:UiScreen::Dialogue);},true,true,"paw");
            auto& b=tray.add<Panel>();transparent(b);b.setSize({contentWidth_,52});
            button(b,"Menu",[this]{show(UiScreen::Pause);},false,true,"menu");
        } else {
            button(tray,puzzle?"Objects & actions":"Meet Pip",[this]{show(screen_==UiScreen::Puzzle?UiScreen::Objects:UiScreen::Dialogue);},true,true,"paw");
            button(tray,"Menu",[this]{show(UiScreen::Pause);},false,true,"menu");
        }
        if(shortView) {
            const auto statusWidth=std::min(320.f,(w-36)/2);
            auto& status=surface(w-statusWidth-12,12,statusWidth,68,false);
            auto& line=copy(status,std::to_string(model_.coins)+" coins · "+std::to_string(model_.friendship)+"/3 · "+saveLabel,model_.save==SaveState::Failed);
            line.setLiveRegion(true);
        } else if(w>780) {
            auto& status=surface(w-264,12,252,136,false);
            copy(status,std::to_string(model_.coins)+" coins");
            copy(status,"Friendship "+std::to_string(model_.friendship)+" of 3");
            copy(status,model_.save==SaveState::Saved?"Saved":model_.save==SaveState::Saving?"Saving…":model_.save==SaveState::Failed?"Save failed · open Menu":"Temporary session");
        }
        break;
    }
    case UiScreen::Dialogue:
        heading(p,model_.questTitle);previewImage(p,model_.pipPortrait,"Pip's portrait");
        if(!model_.feedback.empty()) copy(p,model_.feedback);
        copy(p,"Pip");copy(p,model_.dialogue);
        if(!model_.puzzleReady) copy(p,"Pip's puzzle is not available in this build.");
        button(p,"Help Pip",[this]{emit(UiAction::AcceptQuest);},true,model_.puzzleReady);
        for(const auto& q:model_.quests) button(p,q.title+(q.completed?" · Practice":q.unlocked?"":" · Locked"),[this,id=q.id]{emit(UiAction::SelectQuest,id);},false,q.unlocked);
        if(!model_.quests.empty()) copy(p,"Help Pip in order. Completed requests can be practised without extra rewards.");
        back();break;
    case UiScreen::Objects:
        heading(p,model_.questTitle);copy(p,model_.objective);
        if(!model_.feedback.empty()) {auto& feedback=copy(p,model_.feedback);feedback.setLiveRegion(true);}
        copy(p,"Tap a channel to turn it. Tap other objects to select them. Drag pots and toys to an allowed spot.");
        if(!model_.objects.empty()) {
            copy(p,"Selected: "+model_.selectedName);
            for(const auto& a:model_.actions) {
                button(p,a.label,[this,a]{emit(a.action,a.target);},a.enabled,a.enabled);if(!a.reason.empty()) copy(p,a.reason,!a.enabled);
            }
            heading(p,"Choose an object");
            for(const auto& o:model_.objects) {auto& b=button(p,o.name,[this,id=o.id]{emit(UiAction::SelectObject,id);},o.id==model_.selectedId);b.setAccessibleDescription(o.description+(o.id==model_.selectedId?" Selected.":""));}
        }
        button(p,"Hint",[this]{if(!model_.revealedHints)emit(UiAction::RevealHint);show(UiScreen::Hints);},false,true,"hint");
        button(p,"Undo",[this]{emit(UiAction::Undo);},false,model_.canUndo,"undo");back();break;
    case UiScreen::Hints:
        if(!preview_) hintStep_=std::max(1,int(model_.revealedHints));
        heading(p,"A little nudge");copy(p,"Hints are free. Take as much time as you like.");
        for(int i=0;i<hintStep_;++i) copy(p,std::to_string(i+1)+". "+model_.hints[i]);
        button(p,hintStep_<3?"Next hint":"All hints revealed",[this]{if(preview_)hintStep_=std::min(3,hintStep_+1);else emit(UiAction::RevealHint);dirty_=true;},true,hintStep_<3,"hint");back("Back");break;
    case UiScreen::Pause:
        heading(p,"Take your time");copy(p,std::to_string(model_.coins)+" coins · Friendship "+std::to_string(model_.friendship)+" of 3");
        button(p,"Continue",[this]{close();},true);
        button(p,"Settings",[this]{show(UiScreen::Settings);});
        button(p,"Reset view",[this]{emit(UiAction::ResetView);},false,true,"rotate");
        button(p,"Zoom in",[this]{emit(UiAction::ZoomIn);});button(p,"Zoom out",[this]{emit(UiAction::ZoomOut);});
        button(p,"Restart puzzle",[this]{show(UiScreen::Restart);},false,model_.canRestart);
        button(p,"Back to garden",[this]{emit(UiAction::BackToGarden);},false,true,"home");
        button(p,"Shop",[this]{if(preview_)show(UiScreen::Shop);else emit(UiAction::OpenShop);},false,model_.shopOpen&&model_.inGarden,"coin");
        if(!model_.shopOpen) copy(p,"The shop opens after Pip's first request.");
        else if(!model_.inGarden) copy(p,"Return to the garden to choose a comfort for Pip.");
        button(p,"Replay afternoon moment",[this]{emit(UiAction::ReplayEnding);},false,model_.endingSeen&&model_.inGarden);
        button(p,"Save and recovery",[this]{show(UiScreen::Recovery);});
        button(p,"Title screen",[this]{show(UiScreen::Title);});break;
    case UiScreen::Settings: {
        heading(p,"Make yourself at home");copy(p,"Your choices are saved with your afternoon.");
        copy(p,model_.audioStatus);button(p,"Enable sound",[this]{emit(UiAction::EnableSound);});
        button(p,settings_.largerText?"Text size: Larger":"Text size: Standard",[this]{settings_.largerText=!settings_.largerText;pendingFocus_=settings_.largerText?"Text size: Larger":"Text size: Standard";emit(UiAction::Settings,"text");dirty_=true;});
        button(p,settings_.reducedMotion?"Motion: Reduced":"Motion: Full",[this]{settings_.reducedMotion=!settings_.reducedMotion;pendingFocus_=settings_.reducedMotion?"Motion: Reduced":"Motion: Full";emit(UiAction::Settings,"motion");dirty_=true;});
        button(p,settings_.muted?"Sound: Muted":"Sound: On",[this]{settings_.muted=!settings_.muted;pendingFocus_=settings_.muted?"Sound: Muted":"Sound: On";emit(UiAction::Settings,"mute");dirty_=true;});
        const auto volume=[&](const std::string& name,int UiSettings::*field){button(p,name+": "+std::to_string(settings_.*field)+"%",[this,field,name]{settings_.*field=(settings_.*field+10)%110;pendingFocus_=name+": "+std::to_string(settings_.*field)+"%";emit(UiAction::Settings,name);dirty_=true;});};
        volume("Music",&UiSettings::music);volume("Ambience",&UiSettings::ambience);volume("Effects",&UiSettings::effects);
        copy(p,"Volume buttons increase by 10%, then return to 0%. Sound never carries the only clue.");
        const auto qualityLabel=[](int value){return value==1?"Quality: Mobile":value==2?"Quality: Desktop":"Quality: Automatic";};
        copy(p,"Automatic fits your display. Mobile uses fewer pixels and no shadow maps; Desktop adds detail. Puzzle rules stay the same.");
        button(p,qualityLabel(settings_.quality),[this,qualityLabel]{settings_.quality=(settings_.quality+1)%3;pendingFocus_=qualityLabel(settings_.quality);emit(UiAction::Settings,"quality");dirty_=true;});
        if(gallery_) copy(p,"Font coverage: Å Ä Ö · å ä ö");
        back("Back");break;
    }
    case UiScreen::Restart:
        heading(p,"Start this puzzle again?");copy(p,"Your moves in this attempt will be cleared. Earned rewards stay yours.");
        button(p,"Keep playing",[this]{close();},true);
        button(p,"Restart puzzle",[this]{emit(UiAction::Restart);},false,model_.canRestart);break;
    case UiScreen::Completion:
        heading(p,"Thank you!");previewImage(p,model_.pipPortrait,"Happy Pip");copy(p,model_.questTitle+" complete. Pip gives you a happy little nudge.");
        copy(p,!model_.rewardsReady?"The ball is back where it belongs. Thanks for helping Pip.":model_.rewardGranted?"Earned "+std::to_string(model_.rewardCoins)+" coins and "+std::to_string(model_.rewardFriendship)+" friendship.":"A lovely practice round. This request's reward has already been earned.");
        if(preview_) button(p,"Preview reward / practice",[this]{model_.rewardGranted=!model_.rewardGranted;dirty_=true;});
        button(p,"Back to garden",[this]{emit(UiAction::BackToGarden);},true,true,"home");if(preview_) back();break;
    case UiScreen::Shop:
        heading(p,"Little garden comforts");copy(p,std::to_string(model_.coins)+" coins to spend");
        if(preview_) button(p,"Preview shop state",[this]{
            if(model_.cushionOwned) {model_.cushionOwned=model_.bowlOwned=false;model_.coins=40;}
            else if(!model_.shopOpen) {model_.shopOpen=true;model_.coins=5;model_.cushionOwned=model_.bowlOwned=true;}
            else if(model_.coins<25) model_.shopOpen=false;
            else model_.coins=5;
            dirty_=true;
        });
        previewImage(p,model_.cushionPreview,"Garden cushion");
        button(p,model_.cushionOwned?"Garden cushion · Owned":"Garden cushion · 30 coins",[this]{item_="garden_cushion";show(UiScreen::Details);});
        previewImage(p,model_.bowlPreview,"Drinking bowl");
        button(p,model_.bowlOwned?"Drinking bowl · Owned":"Drinking bowl · 25 coins",[this]{item_="drinking_bowl";show(UiScreen::Details);});
        if(!model_.shopOpen) copy(p,"Help Pip with his first request to open the shop.");
        back();break;
    case UiScreen::Details: {
        bool cushion=item_=="garden_cushion",owned=cushion?model_.cushionOwned:model_.bowlOwned;int price=cushion?30:25;
        heading(p,cushion?"Garden cushion":"Drinking bowl");previewImage(p,cushion?model_.cushionPreview:model_.bowlPreview,cushion?"Cushion preview":"Bowl preview");
        copy(p,cushion?"A soft place for Pip to curl up in the afternoon sun.":"A cool little drink after a busy afternoon.");
        copy(p,owned?"Already yours.":std::to_string(price)+" coins");
        const bool canBuy=!owned&&model_.shopOpen&&model_.coins>=price;
        button(p,owned?"Owned":"Buy",[this]{emit(UiAction::Buy,item_);},true,canBuy,"coin");
        if(!owned&&!canBuy) copy(p,model_.shopOpen?"You need a few more coins.":"The shop is not open yet.");
        button(p,"Choose a spot",[this]{if(preview_)show(UiScreen::Placement);else {emit(UiAction::BeginPlacement,item_);show(UiScreen::Placement);}},false,owned);
        const auto& placed=cushion?model_.cushionSpot:model_.bowlSpot;
        if(!placed.empty()) {
            copy(p,"Placed: "+placed+". Moving it costs no coins.");
            button(p,cushion?"Invite Pip to rest":"Invite Pip to drink",[this]{emit(UiAction::UseFurnishing,item_);},false,true,"paw");
        }
        back("Back");break;
    }
    case UiScreen::Placement:
        heading(p,"Choose a spot");
        if(preview_)previewImage(p,model_.placementPreview,"Furnishing and footprint");
        copy(p,model_.placementReason,!model_.placementValid);
        if(preview_) button(p,"Preview valid / blocked",[this]{model_.placementValid=!model_.placementValid;model_.placementReason=model_.placementValid?"Clear space beside the bench.":"The bowl already occupies this spot.";dirty_=true;});
        if(preview_)for(auto name:{"Sunny corner","Beside the bench","Quiet nook"})button(p,name,[this,name]{emit(UiAction::SelectSpot,name);});
        else for(const auto& spot:model_.spots) {
            auto& b=button(p,spot.name,[this,id=spot.id]{emit(UiAction::SelectSpot,id);},spot.selected);
            b.setAccessibleDescription(spot.reason);
        }
        if(preview_) {
            button(p,"Place",[this,target=item_+"/"+model_.placementSpot]{emit(UiAction::PlaceFurnishing,target);},true,model_.placementValid);
            button(p,"Cancel",[this]{emit(UiAction::CancelPlacement);close();});
        }
        break;
    case UiScreen::Ending:
        heading(p,"A lovely first afternoon");previewImage(p,model_.pipPortrait,"Pip feels at home");copy(p,"Pip gives you a slow blink. He feels at home.");
        button(p,"Continue in garden",[this]{emit(UiAction::BackToGarden);},true,true,"home");
        if(preview_)button(p,"Replay moment",[this]{emit(UiAction::ReplayEnding);});if(preview_) back();break;
    case UiScreen::Recovery: {
        heading(p,"Your afternoon");
        constexpr const char* messages[]={"Temporary session. Progress will not be kept when you leave.","Saving your afternoon…","Saved. Your afternoon is ready for next time.","We couldn't save your afternoon. Keep this tab open and try again.","Another tab is keeping this save. Continue there, or play a temporary session here.","Loading your afternoon…"};
        auto s=preview_?SaveState(recoveryExample_):model_.save;copy(p,messages[int(s)],s==SaveState::Failed);
        if(preview_) button(p,"Next save state",[this]{recoveryExample_=(recoveryExample_+1)%6;dirty_=true;});
        if(!model_.storageNotice.empty())copy(p,model_.storageNotice);
        button(p,"Retry save",[this]{emit(UiAction::RetrySave);},true,s==SaveState::Failed||s==SaveState::Temporary);
        button(p,"Export save",[this]{emit(UiAction::ExportSave);},false,model_.canExport);
        button(p,"Import save",[this]{emit(UiAction::ImportSave);},false,model_.storageAvailable);
        if(model_.canExportOriginal)button(p,"Export original save",[this]{emit(UiAction::ExportOriginal);});
        if(!preview_) copy(p,"Export a copy before clearing browser data. Import checks the file before asking to replace this afternoon. Undo history is restored.");
        back();break;
    }
    case UiScreen::ImportConfirm:
        heading(p,"Use this saved afternoon?");
        copy(p,"The file is valid. Import replaces this tab's afternoon, including moves, purchases and settings. Export your current afternoon first if you want to keep it.");
        button(p,"Keep current afternoon",[this]{emit(UiAction::CancelImport);close();},true);
        button(p,"Export save",[this]{emit(UiAction::ExportSave);},false,model_.canExport);
        button(p,"Use imported afternoon",[this]{emit(UiAction::ConfirmImport);});break;
    case UiScreen::Gallery: {
        heading(p,"Paws component garden");copy(p,"Development previews. Actions here never buy, reward or save. Art slots await phases 3 and 4.");
        const std::pair<const char*,UiScreen> samples[]={{"Title / continue",UiScreen::Title},{"Garden HUD",UiScreen::Garden},{"Pip dialogue",UiScreen::Dialogue},{"Puzzle actions",UiScreen::Objects},{"Puzzle HUD",UiScreen::Puzzle},{"Hints",UiScreen::Hints},{"Pause",UiScreen::Pause},{"Settings",UiScreen::Settings},{"Restart confirmation",UiScreen::Restart},{"Completion / rewards",UiScreen::Completion},{"Shop / details",UiScreen::Shop},{"Placement",UiScreen::Placement},{"Ending",UiScreen::Ending},{"Save / recovery",UiScreen::Recovery}};
        for(auto [label,screen]:samples) button(p,label,[this,screen]{preview_=true;show(screen);});
        break;
    }
    case UiScreen::Loading:
        heading(p,"Preparing Pip's request…");
        copy(p,model_.feedback.empty()?"Your current attempt is kept while the next board is prepared.":model_.feedback);
        button(p,"Back to garden",[this]{emit(UiAction::BackToGarden);},true);break;
    case UiScreen::LoadFailure:
        heading(p,"Let's try that again");
        copy(p,"The garden models could not load. Your progress and previous view are safe.");
        button(p,"Retry loading",[this]{emit(UiAction::RetryLoad);},true);
        button(p,"Back to garden",[this]{emit(UiAction::BackToGarden);});break;
    }
    if(!model_.feedback.empty()&&!hud&&screen_!=UiScreen::Objects) {auto& feedback=copy(p,model_.feedback);feedback.setLiveRegion(true);}
    if(footer) {
        auto& bar=gui_->root().add<Panel>();transparent(bar);bar.setPosition({cardX+16,cardY+cardHeight-64});bar.setSize({cardWidth-32,52});
        contentWidth_=cardWidth-32;
        if(screen_==UiScreen::Placement&&!preview_) {
            bar.setLayout(HorizontalLayout{12});contentWidth_=(contentWidth_-12)/2;
            auto& left=bar.add<Panel>();transparent(left);left.setSize({contentWidth_,52});
            button(left,"Place",[this,target=item_+"/"+model_.placementSpot]{emit(UiAction::PlaceFurnishing,target);},true,model_.placementValid);
            auto& right=bar.add<Panel>();transparent(right);right.setSize({contentWidth_,52});
            button(right,"Cancel",[this]{emit(UiAction::CancelPlacement);close();});
        } else button(bar,"Back",[this]{
            if(screen_==UiScreen::Placement){emit(UiAction::CancelPlacement);close();}
            else if(screen_==UiScreen::Ending||screen_==UiScreen::Loading||screen_==UiScreen::LoadFailure||screen_==UiScreen::Completion)emit(UiAction::BackToGarden);
            else close();
        });
    }
}
void PawsUi::update(float width,float height) {
    if(willRebuild(width,height)) {
        const auto focus=pendingFocus_.empty()?focusedName(*gui_):pendingFocus_;
        width_=width;height_=height;skins_.clear();
        auto body=gui_->font();gui_=std::make_unique<GuiSystem>();gui_->setFont(body);
        // Bound resize/state texture cache; fonts and the current draw keep their own snapshots.
        art_.clear();if(!composition_||!composition_())build();gui_->layout({width,height});focusNamed(*gui_,focus);pendingFocus_.clear();dirty_=false;
    }
    gui_->layout({width,height});
    for(auto& s:skins_) {
        const auto b=s.button->bounds();float logicalWidth=b.width/gui_->scale();
        int state=!s.button->enabled()?4:b.contains(pointer_)&&down_?2:gui_->focused()==s.button?3:b.contains(pointer_)?1:0;
        if(state!=s.state||logicalWidth!=s.width) {
            s.image->setTexture(tile(int(logicalWidth),int(s.height),state,s.primary&&s.button->enabled(),s.icon));s.state=state;s.width=logicalWidth;
        }
    }
}
}
