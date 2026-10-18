"""Generate task-local catalog, production briefs and exact shared-owner patch.

No audio generation, acceptance mutation or shared file writes.
"""
import difflib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
JOB = ROOT / 'artifacts/audio-jobs/release1'
SRC = ROOT / 'src/release1/audio'

# family, variants, seconds min/max, loop, cooldown, consumer, exact provider prompt
FAMILIES = [
    ('crate_slide', 2, .4, .8, False, .12, 'Committed crate slide; Button bench and two-switch playhouse', 'One small wooden slatted crate sliding briefly over smooth sandstone, rounded wood friction and a soft stop; close, dry, no squeak or impacts.'),
    ('plank_place', 2, .3, .7, False, .12, 'Committed plank connection, removal or orientation', 'A short smooth wooden plank lowered onto two wooden supports, two soft contacts and a tiny settling creak; close and restrained.'),
    ('shape_fit', 1, .25, .5, False, .12, 'Committed cushion fit into basket', 'One cotton cushion pressed gently into a fitted wicker recess, soft fabric compression and subtle wicker contact, no sharp click.'),
    ('bell_leaf', 1, .7, 1.2, False, .12, 'Accepted leaf-symbol tap', 'One small warm ceramic garden bell struck gently, low rounded D note, short clean natural decay, dry isolated recording, no other bells or music.'),
    ('bell_star', 1, .7, 1.2, False, .12, 'Accepted star-symbol tap', 'One small warm ceramic garden bell struck gently, middle rounded F sharp note, short clean natural decay, dry isolated recording, no other bells or music.'),
    ('bell_paw', 1, .7, 1.2, False, .12, 'Accepted paw-symbol tap', 'One small warm ceramic garden bell struck gently, upper rounded A note, short clean natural decay, dry isolated recording, no other bells or music.'),
    ('bell_reset', 1, .25, .45, False, .3, 'Committed sequence reset; visual clue stays authoritative', 'Two soft low wooden taps descending gently, quiet reassuring restart feedback, no buzzer, alarm or harsh transient.'),
    ('water_flow', 1, 1, 2, True, 4, 'Committed connected route; one bounded four-second presentation', 'A small gentle stream of water running through a shallow wooden garden gutter, even soft trickle, no splashing or faucet noise; seamless steady loop.'),
    ('bowl_fill', 1, .8, 1.5, False, .5, 'Committed bowl-filled state; cancels water flow', 'A little water pouring gently into a small ceramic pet bowl and stopping with two quiet drips, intimate and soft, no ringing ceramic.'),
    ('mabel_request', 1, .7, 1.5, False, 2, 'Accepted Mabel dialogue/request', 'One calm adult domestic cat giving a low soft questioning meow, composed and relaxed, close natural recording, no distress or background.'),
    ('mabel_happy', 1, .5, 1, False, 2, 'Committed Mabel completion or first furnishing use', 'One calm adult domestic cat making a low warm contented trill, relaxed and natural, brief and close, no background.'),
    ('button_request', 1, .5, 1.2, False, 2, 'Accepted Button dialogue/request', 'One shy domestic cat giving a tiny soft questioning mew, breathy but clearly feline, gentle natural close recording, no distress or background.'),
    ('button_happy', 1, .4, .8, False, 2, 'Committed Button completion or first furnishing use', 'One shy contented domestic cat making a quiet little pleased chirrup, soft and natural, close, no background.'),
    ('pepper_request', 1, .5, 1.2, False, 2, 'Accepted Pepper dialogue/request', 'One alert friendly domestic kitten giving a bright curious questioning meow, natural and gentle, no distress or background.'),
    ('pepper_happy', 1, .4, .9, False, 2, 'Committed Pepper completion or first furnishing use', 'One lively friendly domestic kitten making a brief pleased upward trill, natural and gentle, no exaggerated cartoon voice or background.'),
    ('cat_scratch', 1, 1, 2, False, 2, 'Accepted bounded scratching-post use; shared by cats', 'A domestic cat making three gentle short scratches on a sisal scratching post, dry soft fibres, quiet and close, no hissing or harsh scraping.'),
    ('playhouse_use', 1, .4, .8, False, 1, 'Accepted playhouse entrance/lookout use; shared by Pip and Pepper', 'Soft padded cat paws landing on a small wooden playhouse floor with one tiny warm wood creak, quiet natural contact, no door slam.'),
]

def write(path, text):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding='utf-8', newline='\n')

def dump(path, value):
    write(path, json.dumps(value, indent=2, ensure_ascii=False) + '\n')

def prepare():
    path = JOB / 'catalog.json'
    previous = {c['id']: c for c in json.loads(path.read_text())['clips']} if path.exists() else {}
    clips = []
    for family, variants, low, high, loop, cooldown, consumer, prompt in FAMILIES:
        for variant in range(1, variants + 1):
            id = f'{family}_{variant:02}'
            c = dict(id=id, family=family, provider='ElevenLabs', prompt=prompt,
                     durationRange=[low, high], target=f'{low}-{high} s', event=consumer,
                     file=id+'.wav', loop=loop, bus=2, gain=.25 if family.startswith('bell_') else .35,
                     cooldown=cooldown, accepted=False, source=None, review=None)
            # Regeneration must never discard actual import metadata.
            for key in ('accepted','source','review','sha256','pcm','loopFrames','bytes'):
                if key in previous.get(id, {}): c[key] = previous[id][key]
            clips.append(c)
    dump(path, dict(schema=1, contentId='paws-release1-audio-1', clips=clips))
    old = json.loads((ROOT/'assets/runtime/audio/catalog.json').read_text())
    merged = json.loads(json.dumps(old))
    merged['contentId'] = 'paws-release1-audio-1'
    for c in clips:
        entry = dict(c); entry['file'] = 'release1/'+c['file']; merged['clips'].append(entry)
    dump(ROOT/'web/release1/audio/catalog.json', merged)
    original = (ROOT/'src/audio_catalog.hpp').read_text()
    extra = ''.join('    {"%s","%s",2,%s,%sf,%s},\n' %
                    (c['id'],c['family'],str(c['loop']).lower(),c['gain'],c['cooldown']) for c in clips)
    catalog = original.replace('AudioClipSpec,47>',f'AudioClipSpec,{len(merged["clips"])}>').replace('}};',extra+'}};')
    header = (ROOT/'src/game_audio.hpp').read_text().replace('std::array<Slot,47>', 'std::array<Slot,audioClips.size()>')
    header = header.replace('    void music(', '    void cancel(std::string_view family);\n    void music(')
    cpp = (ROOT/'src/game_audio.cpp').read_text()
    cpp = cpp.replace('void GameAudio::stopFamily(', 'void GameAudio::cancel(std::string_view family) {\n    stopFamily(family);loops_[std::string(family)]=false;\n}\nvoid GameAudio::stopFamily(')
    cpp = cpp.replace('(s.family=="ball_roll"&&p.age>=4)', '((s.family=="ball_roll"||s.family=="water_flow")&&p.age>=4)')
    host = (ROOT/'web/paws-audio.js').read_text().replace('47', str(len(merged['clips'])))
    host = host.replace("/^[a-z0-9_]+\\.wav$/", "/^(?:release1\\/)?[a-z0-9_]+\\.wav$/")
    changes = {'src/game_audio.hpp':header,'src/game_audio.cpp':cpp,'src/audio_catalog.hpp':catalog,'web/paws-audio.js':host}
    patch = ''
    for name, content in changes.items():
        patch += ''.join(difflib.unified_diff((ROOT/name).read_text().splitlines(True),content.splitlines(True),fromfile='a/'+name,tofile='b/'+name))
        target = SRC/'runtime'/Path(name).name if name.startswith('src/') else ROOT/'web/release1/audio/paws-audio.js'
        write(target, content)
    for name, content in {
        'CMakeLists.txt': (ROOT/'CMakeLists.txt').read_text().replace('add_library(paws_audio src/game_audio.cpp)',
            'add_library(paws_audio src/game_audio.cpp src/release1/audio/cues.cpp)\ntarget_compile_definitions(paws_audio PUBLIC PAWS_RELEASE1_AUDIO_PRODUCTION=1)'),
        'src/browser_main.cpp': (ROOT/'src/browser_main.cpp').read_text().replace('index>=47', 'std::size_t(index)>=paws::audioClips.size()'),
    }.items():
        patch += ''.join(difflib.unified_diff((ROOT/name).read_text().splitlines(True),content.splitlines(True),fromfile='a/'+name,tofile='b/'+name))
    write(JOB/'shared-runtime.patch',patch)
    write(ROOT/'tests/release1/audio/legacy_tests.inc',
          (ROOT/'tests/audio_tests.cpp').read_text().replace('#include "game_audio.hpp"', '#include "release1/audio/runtime/game_audio.hpp"'))
    # Catalog path override belongs only to the isolated preview, not production patch.
    write(ROOT/'web/release1/audio/paws-audio.js',host.replace("fetch('assets/audio/catalog.json')", "fetch('catalog.json')"))
    lines = ['# Release-one additional production briefs', '',
        '19 new ElevenLabs deliveries plus the 47 existing deliveries in docs/phase9-audio-production.md. No provider output is present.', '',
        'Use the exact prompts below with ElevenLabs Sound Effects. Retain actual model/settings, original download, source link/date and account/asset rights evidence. Rights remain unknown until supplied; no license is presumed.', '',
        'The locally installed ElevenLabs helper uses eleven_text_to_sound_v2 and accepts 0.5-30 s sources. Generate at the upper target duration (at least 0.5 s), then trim short cues to the delivery ranges. Keep MP3 originals and provenance sidecars under sources/, not runtime. Local status on 11 October 2026 reported no API key configured.', '',
        'Edit copies to PCM16 WAV, mono, preferably 48 kHz. Remove DC; peak <= -2 dBFS. Aim for short-effect active RMS around -24 to -18 dBFS, water around -30 to -24 dBFS; these are starting mix targets, not listening acceptance. Match variants by perceived level. No compression pumping, clipped transients, voices in ambience, or synthetic replacements.', '',
        'Reuse garden afternoon for garden/shop in all three chapters and garden puzzles for all ten requests. Preserve music across ordinary actions. picnic_complete aliases the existing Suno music_pip_at_home resolving phrase: audition it with all four cats; if unsuitable, replace via the existing Suno brief, not another music provider. No extra chapter tracks.', '',
        'Reuse cushion_move for slides/rotations, basket_place/toy_pickup for retrieval, channel_rotate for gutters, switch_down/up and gate_open/close for held-switch edges, and existing paw/settle/purr/drink/furnishing/UI/reward cues. Distinguish new cat requests/happy responses only.', '',
        'Loops use exactly [0, frames), edited before import. Audition five offline repeats then five actual-backend repeats. Bell pitches are suggested D/F#/A; verify/edit by ear because generation cannot guarantee tuning. Leaf/star/paw symbols and sequence feedback remain authoritative without sound.', '',
        'Per clip <=24 MiB decoded; retained cache <=48 MiB, 12 effects/14 voices. Test device resampling at its actual rate. AudioBuffer/decoder/transfer duplication is additional memory, not included in retained PCM.', '']
    for f, n, lo, hi, loop, cd, consumer, prompt in FAMILIES:
        lines += [f'## {f}', '',f'ElevenLabs; {n} variant(s); {lo}-{hi} s; loop={loop}; cooldown={cd} s.', '',consumer, '',prompt, '',
                  'Files: '+', '.join(f'`{f}_{v:02}.wav`' for v in range(1,n+1)), '']
    write(JOB/'production-briefs.md','\n'.join(lines))
    print(f'Prepared {len(clips)} new clips; {len(merged["clips"])} total slots; no shared writes.')

if __name__ == '__main__': prepare()
