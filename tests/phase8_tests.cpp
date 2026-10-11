#include "afternoon.hpp"
#include <iostream>
#include <stdexcept>
using namespace paws;
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void act(Afternoon& a,Action action,std::string object={},Direction direction=Direction::North) {
    const auto s=a.session().state();check(a.dispatch({s.attempt,s.revision,action,object,direction}).accepted,"action accepted");
}
int main(){try {
    Afternoon a;act(a,Action::Move,"pot_a",Direction::North);
    a.saveRevision=42;a.saveSession="session-test-123";a.userPaused=true;a.settings.reducedMotion=true;
    const auto bytes=a.encode();auto b=Afternoon::decode(bytes);
    check(b.encode()==bytes&&b.saveRevision==42&&b.userPaused&&b.settings.reducedMotion,"v5 exact roundtrip and metadata");
    auto display=a;display.settings.quality=1;
    check(Afternoon::decode(display.encode()).settings.quality==1,"mobile display preference restores");
    display.settings.quality=2;check(Afternoon::decode(display.encode()).settings.quality==2,"desktop display preference restores");
    check(b.session().canUndo(),"undo restored");act(b,Action::Undo);
    check(b.session().state().puzzle.objects[0].cell==Afternoon().session().state().puzzle.objects[0].cell,"undo restores pre-move state");
    auto expectBad=[&](std::string data){bool failed=false;try{auto c=Afternoon::decode(data);(void)c;}catch(...){failed=true;}check(failed,"bad import rejected");check(a.encode()==bytes,"validation preserves current model");};
    auto bad=bytes;bad.replace(0,16,"PAWS_AFTERNOON 99");expectBad(bad);
    display.settings.quality=3;expectBad(display.encode());
    expectBad(bytes+"trailing");expectBad(bytes.substr(0,bytes.size()/2));
    bad=bytes;bad.replace(bad.find("session-test-123"),15,"bad/session");expectBad(bad);
    bad=bytes;bad.replace(bad.find("42 \"session"),2,"4294967295");expectBad(bad);
    auto v4=bytes;v4.replace(15,1,"4");auto start=v4.find('\n',v4.find('\n')+1)+1;
    v4.erase(start,v4.find('\n',start)-start+1);
    check(Afternoon::decode(v4).settings.quality==0,"v4 migrates to automatic quality");
    auto v3=v4;v3.replace(15,1,"3");start=v3.find('\n')+1;v3.erase(start,v3.find('\n',start)-start+1);
    auto legacy=Afternoon::decode(v3);check(legacy.saveRevision==0&&legacy.saveSession=="legacy"&&!legacy.userPaused&&legacy.session().canUndo(),"v3 migration retains attempt and undo");
    auto old=a.session().state();auto imported=Afternoon::decode(bytes);imported.invalidateImportedCommands(a);
    check(!imported.dispatch({old.attempt,old.revision,Action::Undo,{}}).accepted,"pre-import command rejected");
    check(imported.saveRevision==42&&Afternoon::decode(imported.encode()).encode()==imported.encode(),"rebased import remains valid");
    std::cout<<"Phase 8 portable metadata, v3 migration, undo, malformed/future data and imported-command invalidation passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
