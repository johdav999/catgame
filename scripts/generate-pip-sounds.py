"""Generate the ten user-requested Pip effects once using the installed skill.

Provider originals and credential-free provenance stay in audio-production.
Existing completed downloads are preserved; failed calls are never retried here.
"""
import argparse
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(Path.home()/'.codex/skills/elevenlabs/scripts'))
from elevenlabs import generate, repo_root

JOBS = [
    ('pip_request_01', 1.2, False, 'One soft curious requesting meow from a small friendly domestic cat, gentle upward inflection, close dry recording, isolated, no distress, no human voices, no other animals, no music or background ambience.'),
    ('pip_request_02', 1.2, False, 'One gentle short questioning meow from a friendly small domestic cat waiting patiently for help, slightly different pitch from a usual meow, natural and affectionate, isolated close dry recording, no distress, no speech, no music or background.'),
    ('pip_happy_01', .8, False, 'One tiny happy domestic cat chirrup trill, soft bright affectionate greeting, natural feline vocalization, close dry isolated recording, no speech, no music, no other animals or background.'),
    ('pip_happy_02', .8, False, 'One soft pleased domestic cat trill with a gently rolling chirrup, warm contented friendly kitten-like voice, natural not cartoon, isolated close dry recording, no speech, no music or background.'),
    ('pip_purr_01', 10, True, 'Very gentle even continuous domestic cat purring close up, calm warm low soft rumble with subtle natural breathing, seamless sustained loop with no start or stop, no meows, no speech, no music, no room ambience.'),
    ('pip_paws_stone_01', .5, False, 'One single extremely soft padded domestic cat paw step on smooth sandstone, a light muted dry tap with tiny fur contact, isolated close recording, no claws, no hard shoes, no repeated steps, no voices or music.'),
    ('pip_paws_stone_02', .5, False, 'One single quiet small cat paw footfall on garden sandstone paving, very light soft low padded tap, natural subtle contact, isolated dry recording, no claws, no shoes, no sequence, no voices, no music.'),
    ('pip_paws_stone_03', .5, False, 'One single delicate padded cat paw contact on pale stone paving, tiny soft dry thup with faint fur movement, short isolated gentle footstep, no human shoes, no claw clicks, no extra steps, no voices, no music.'),
    ('pip_settle_01', .9, False, 'A small cat gently settling down into a soft cotton garden cushion, one brief quiet cloth rustle and muted cushion compression, cosy delicate natural contact, no meow or purr, no human voices, no music, no background.'),
    ('pip_drink_01', 3, False, 'A small domestic cat quietly lapping fresh water from a ceramic bowl, gentle delicate rhythmic tongue laps with tiny wet ripples, intimate dry recording, no pouring, no gulping, no meows, no speech, no music or background.'),
]

if __name__ == '__main__':
    root = repo_root(str(ROOT))
    for name, duration, loop, prompt in JOBS:
        output = ROOT/'artifacts/audio-production/sources/pip-sounds'/f'{name}.mp3'
        if output.exists():
            if not output.with_suffix('.mp3.provenance.json').exists():
                raise ValueError('Existing download lacks provenance: '+name)
            print('Preserving completed download: '+name, flush=True)
            continue
        print('Generating '+name, flush=True)
        generate(root, argparse.Namespace(prompt=prompt, duration=duration, loop=loop,
                 prompt_influence=.3, output=str(output)))
