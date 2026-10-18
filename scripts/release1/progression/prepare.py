"""Prepare exact Afternoon owner patch and isolated fixture; no shared writes."""
import difflib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
def write(path,text):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(text,encoding='utf-8',newline='\n')
def remove_function(text, signature):
    start=text.index(signature);at=text.index('{',start);depth=1;i=at+1
    while depth:
        if text[i]=='{':depth+=1
        elif text[i]=='}':depth-=1
        i+=1
    return text[:start]+text[i:]
def prepare():
    h=(ROOT/'src/afternoon.hpp').read_text()
    h=h.replace('#include "furnishings.hpp"','#include "furnishings.hpp"\n#include "release1/rules/content.hpp"\n#include "release1/progression/policy.hpp"')
    h=h.replace('    Afternoon();','''    Afternoon();
    Afternoon(const Afternoon&);
    Afternoon& operator=(const Afternoon&);
    Afternoon(Afternoon&&) noexcept=default;
    Afternoon& operator=(Afternoon&&) noexcept=default;
    // Release-one owner seam. No new wallet or browser persistence coordinator.
    release1::PuzzleView puzzleView() const;
    release1::ProgressView progressView() const;
    std::vector<release1::SpotView> placementView(std::string_view item) const;
    release1::Intent intent(release1::Verb,std::string object={},std::string target={}) const;
    CommandResult dispatchIntent(const release1::Intent&);
    CommandResult returnToGarden(const release1::Intent&);
    CommandResult introduce(std::string_view cat,const release1::Intent&);
    std::vector<std::string> pendingIntroductions() const;
    void setItemAvailable(std::string_view,bool); // Verified presentation readiness; not persisted.
    void setSuspended(bool); // Host-only transient input cancellation, not user pause.
    CommandResult setUserPaused(bool,const release1::Intent&);
    bool inGarden() const { return garden_; }
    int chapter() const;
    int friendship(std::string_view cat) const;
    bool picnicEligible() const;
    bool picnicSeen() const { return picnicSeen_; }
    Revision ownerRevision() const { return ownerRevision_; }
    std::uint64_t generation() const { return generation_; }
    std::size_t questCount() const { return 10; }''')
    h=h.replace('const PawsSession& session() const { return sessions_[active_]; }', 'const PawsSession& session() const { return sessions_.at(activeQuest_); } // Checked legacy-only projection; new quests use puzzleView().')
    h=h.replace('return active_;','return activeQuest_;')
    h=h.replace('bool unlocked(std::size_t i) const { return i<3 && (i==0||rewarded_[i-1]); }','bool unlocked(std::size_t i) const;')
    h=h.replace('std::array<bool,3> rewarded_', 'std::array<bool,10> rewarded_')
    h=h.replace('std::array<bool,2> owned_', 'std::array<bool,8> owned_').replace('std::array<std::string,2> placements_', 'std::array<std::string,8> placements_')
    h=h.replace('    bool endingSeen_=false;','''    bool endingSeen_=false;
    std::array<std::unique_ptr<release1::rules::Attempt>,7> extra_;
    std::array<std::vector<release1::rules::Step>,7> extraRecords_;
    std::array<AttemptId,7> extraIds_{1,1,1,1,1,1,1};
    std::array<bool,4> introduced_{true,false,false,false};
    std::array<bool,8> available_{true,true,false,false,false,false,false,false};
    std::size_t activeQuest_=0;
    Revision ownerRevision_=0;
    std::uint64_t generation_=1;
    bool garden_=false,picnicSeen_=false,suspended_=false;
    void initializeRelease();
    bool current(const release1::Intent&) const;
    bool canAdvance() const;
    bool itemUnlocked(std::size_t) const;
    AttemptId currentAttempt() const;
    int legacyFriendship() const;
    int legacyCoins() const;
    std::string encodeLegacy() const;
    static Afternoon decodeLegacy(std::string_view);''')
    cpp=(ROOT/'src/afternoon.cpp').read_text()
    for signature in ['int Afternoon::friendship()', 'int Afternoon::coins()', 'bool Afternoon::owned(',
        'std::string_view Afternoon::placement(', 'std::string Afternoon::purchaseBlockage(',
        'std::string Afternoon::placementBlockage(', 'bool Afternoon::endingEligible()',
        'bool Afternoon::selectQuest(', 'void Afternoon::invalidateImportedCommands(']:
        cpp=remove_function(cpp,signature)
    cpp=cpp.replace('PawsSession(stuckGate())} {}','PawsSession(stuckGate())} { initializeRelease(); }')
    cpp=cpp.replace('std::string Afternoon::encode()', 'std::string Afternoon::encodeLegacy()').replace('Afternoon Afternoon::decode(', 'Afternoon Afternoon::decodeLegacy(')
    cpp=cpp.replace("<<coins()<<' '<<friendship()", "<<legacyCoins()<<' '<<legacyFriendship()")
    cpp=cpp.replace('std::array<bool,3> ledger{}','std::array<bool,10> ledger{}')
    cpp=cpp.replace('candidate.active_=i;', 'candidate.active_=i;candidate.activeQuest_=i;')
    cpp=cpp.replace('candidate.owned_=owned;', 'std::copy(owned.begin(),owned.end(),candidate.owned_.begin());')
    cpp=cpp.replace('candidate.active_=std::size_t(active);candidate.epoch_', 'candidate.active_=std::size_t(active);candidate.activeQuest_=active;candidate.garden_=candidate.session().state().inGarden;candidate.epoch_')
    cpp=cpp.replace('    const auto& s=session().state();', '    if(activeQuest_>=3||!canAdvance())return {false,"Use the current release-one command view."};\n    const auto& s=session().state();',1)
    cpp=cpp.replace('while(shopItems[index].id!=command.item)', 'while(release1::progression::items[index].id!=command.item)')
    cpp=cpp.replace('        if(command.action==GardenAction::Buy)owned_[index]=true;', '        if(!available_[index])return {false,"Presentation assets unavailable."};\n        if(command.action==GardenAction::Buy)owned_[index]=true;')
    cpp=cpp.replace('    return success;', '    ++ownerRevision_;return success;')
    cpp=cpp.replace('    // Resource limits protect', '    if(activeQuest_>=3||!canAdvance())return {false,"Use the current release-one command view."};\n    // Resource limits protect')
    cpp=cpp.replace('    SolutionStep record{', '    if(command.object.size()>128||command.target.size()>128||int(command.direction)<0||int(command.direction)>3)return {false,"Invalid command fields."};\n    SolutionStep record{')
    cpp=cpp.replace('    return result;\n}', '    ++ownerRevision_;garden_=session().state().inGarden;return result;\n}',1)
    while '\n\n\n' in cpp:cpp=cpp.replace('\n\n\n','\n\n')
    changes={'src/afternoon.hpp':h,'src/afternoon.cpp':cpp}
    patch=''
    for name,content in changes.items():
        patch+=''.join(difflib.unified_diff((ROOT/name).read_text().splitlines(True),content.splitlines(True),fromfile='a/'+name,tofile='b/'+name))
        write(ROOT/'src/release1/progression/runtime'/Path(name).name,content)
    # Include rules and policy in the existing owner target to avoid a content/owner cycle.
    cm=(ROOT/'CMakeLists.txt').read_text().replace('add_library(paws_content src/content.cpp src/afternoon.cpp)',
        '''add_library(paws_content src/content.cpp src/afternoon.cpp
  src/release1/rules/puzzles.cpp src/release1/rules/content.cpp
  src/release1/progression/policy.cpp src/release1/progression/codec.cpp)
target_compile_definitions(paws_content PUBLIC PAWS_RELEASE1_PROGRESSION_PRODUCTION=1)''')
    cm=cm.replace('    add_test(NAME paws_rules COMMAND paws_rules_tests)', '''    add_test(NAME paws_rules COMMAND paws_rules_tests)
    add_executable(paws_release1_progression_tests tests/release1/progression/tests.cpp)
    target_link_libraries(paws_release1_progression_tests PRIVATE paws_content)
    add_test(NAME paws_release1_progression COMMAND paws_release1_progression_tests)''')
    cm=cm.replace('    add_test(NAME paws_release1_progression COMMAND paws_release1_progression_tests)', '''    add_test(NAME paws_release1_progression COMMAND paws_release1_progression_tests)
    add_executable(paws_release1_rules_tests tests/release1/rules/tests.cpp)
    target_link_libraries(paws_release1_rules_tests PRIVATE paws_content)
    add_test(NAME paws_release1_rules COMMAND paws_release1_rules_tests)''')
    patch+=''.join(difflib.unified_diff((ROOT/'CMakeLists.txt').read_text().splitlines(True),cm.splitlines(True),fromfile='a/CMakeLists.txt',tofile='b/CMakeLists.txt'))
    helper='''std::string legacySection(const std::string& bytes) {
    std::istringstream input(bytes);std::string magic,tag,legacy;int version=0;
    input>>magic>>version;
    if(version!=6)return bytes;
    input>>tag>>std::quoted(legacy);
    if(tag!="LEGACY"||!input)throw std::runtime_error("Missing legacy section");
    return legacy;
}
'''
    for name in ('phase7_tests.cpp','phase8_tests.cpp'):
        original=(ROOT/'tests'/name).read_text()
        content=original.replace('#include <iostream>', '#include <iostream>\n#include <sstream>\n#include <iomanip>')
        content=content.replace('using namespace paws;','using namespace paws;\n'+helper)
        content=content.replace('}catch(const std::exception& e)', 'return 0;\n}catch(const std::exception& e)')
        if name.startswith('phase7'):
            content=content.replace('    auto bad=bytes;', '    bytes=legacySection(bytes); // Keep historical corruption fixtures on their original wire format.\n    auto bad=bytes;')
        else:
            content=content.replace('const auto bytes=a.encode();','const auto encoded=a.encode();const auto bytes=legacySection(encoded);')
            content=content.replace('b.encode()==bytes', 'b.encode()==encoded').replace('a.encode()==bytes', 'a.encode()==encoded')
        patch+=''.join(difflib.unified_diff(original.splitlines(True),content.splitlines(True),fromfile='a/tests/'+name,tofile='b/tests/'+name))
        write(ROOT/'tests/release1/progression'/('legacy_'+name.replace('.cpp','.inc')),content.replace('#include "afternoon.hpp"','#include "release1/progression/owner.hpp"'))
    write(ROOT/'scripts/release1/progression/integration.patch',patch)
if __name__=='__main__':prepare()
