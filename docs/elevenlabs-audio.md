# ElevenLabs sound effects

The personal `$elevenlabs` Codex skill generates MP3 sound effects and provenance
sidecars in this repository. It uses a Python standard-library helper installed
under your Codex skills directory. No API key is included in the repository.

## One-time local setup

Create an ElevenLabs API key in your account, with sound-generation access and an
appropriate spending limit. Run this in your own PowerShell terminal from the
repository root (the prompt hides the key):

```powershell
$skillRoot = if ($env:CODEX_HOME) { Join-Path $env:CODEX_HOME 'skills/elevenlabs' } else { Join-Path $env:USERPROFILE '.codex/skills/elevenlabs' }
python "$skillRoot/scripts/elevenlabs.py" configure --repo .
python "$skillRoot/scripts/elevenlabs.py" status --repo .
```

The helper stores only Windows DPAPI-encrypted bytes in
`.secrets/elevenlabs.dpapi`. The entire `.secrets/` folder is ignored by Git, and
setup checks that it is ignored and untracked. Protection is tied to your Windows
login; configure again on a different computer. It does not protect against other
processes running as your user. Keep `.secrets` out of deployment archives.

To rotate the key, revoke the old key in ElevenLabs and run `configure` again.
For CI or other operating systems, inject `ELEVENLABS_API_KEY` from a secret
manager; it takes precedence over the encrypted local file. Do not paste keys
into chat, source files, command-line arguments or browser code.

## Generate an effect

Ask Codex: `Use $elevenlabs to generate a 1.5-second soft wooden gate latch click
and save it as assets/runtime/audio/gate-latch.mp3.` Or run:

```powershell
python "$skillRoot/scripts/elevenlabs.py" generate --repo . --prompt "One soft wooden gate latch click, close and dry, no speech or music" --duration 1.5 --output assets/runtime/audio/gate-latch.mp3
```

This makes a paid API call. Add `--loop` for seamless ambience; use a new filename
for each variant. The helper never overwrites existing audio or retries requests
automatically. Timeouts may still consume credits.

Each download has a `.mp3.provenance.json` recording generation settings and the
audio hash. Confirm the account's applicable license and attribution requirements
before shipping. Generation alone does not update the game's audio catalog or
playback code. Listen and verify the sound before integrating it.

The implementation follows the official
[ElevenLabs sound-effects API](https://elevenlabs.io/docs/api-reference/text-to-sound-effects/convert),
checked on 2026-10-11. Local automated validation uses a fake API transport and a
real Windows encryption round trip; live generation requires your configured key.
