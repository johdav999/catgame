"""Exact narrow PawsUi composition seam; fixture only until Prompt 9."""
from pathlib import Path
import difflib
ROOT=Path(__file__).resolve().parents[3]
def prepare():
    changes={}
    h=(ROOT/'src/paws_ui.hpp').read_text()
    h=h.replace('namespace paws {','namespace paws {\nnamespace release1::gui { class Views; }',1)
    h=h.replace('private:\n    struct Skin','private:\n    friend class release1::gui::Views;\n    std::function<bool()> composition_;\n    struct Skin',1)
    changes['src/paws_ui.hpp']=h
    c=(ROOT/'src/paws_ui.cpp').read_text()
    c=c.replace('art_.clear();build();gui_->layout', 'art_.clear();if(!composition_||!composition_())build();gui_->layout')
    changes['src/paws_ui.cpp']=c
    patch=''
    for name,content in changes.items():
        dest=ROOT/'src/release1/gui/runtime'/Path(name).name
        dest.parent.mkdir(parents=True,exist_ok=True)
        dest.write_text(content,encoding='utf-8',newline='\n')
        patch+=''.join(difflib.unified_diff((ROOT/name).read_text().splitlines(True),content.splitlines(True),fromfile='a/'+name,tofile='b/'+name))
    cm=(ROOT/'CMakeLists.txt').read_text()
    cm=cm.replace('target_link_libraries(paws_ui PUBLIC engine_gui)', '''target_link_libraries(paws_ui PUBLIC engine_gui paws_content)
target_sources(paws_ui PRIVATE src/release1/gui/views.cpp src/release1/gui/dialogue.cpp)
target_compile_definitions(paws_ui PUBLIC PAWS_RELEASE1_GUI_PRODUCTION=1)''')
    cm=cm.replace('    add_test(NAME paws_ui COMMAND paws_ui_tests)', '''    add_test(NAME paws_ui COMMAND paws_ui_tests)
    add_executable(paws_release1_gui_tests tests/release1/gui/tests.cpp)
    target_link_libraries(paws_release1_gui_tests PRIVATE paws_ui)
    add_test(NAME paws_release1_gui COMMAND paws_release1_gui_tests)''')
    patch+=''.join(difflib.unified_diff((ROOT/'CMakeLists.txt').read_text().splitlines(True),cm.splitlines(True),fromfile='a/CMakeLists.txt',tofile='b/CMakeLists.txt'))
    (ROOT/'scripts/release1/gui/integration.patch').write_text(patch,encoding='utf-8',newline='\n')
if __name__=='__main__':prepare()
