# Prompt 9 source handoff

All final audio is currently missing. No provider generation or source audition
was performed in this task. The connected-plugin search returned no ElevenLabs
or Suno provider. Further integrations may exist in the plugin directory.

The [production checklist](../../docs/phase9-audio-production.md) lists all 47
stable filenames, provider briefs, event bindings and import destinations.
Copy [import-template.json](import-template.json) for each reviewed delivery.

Keep original downloads and actual account/asset rights records under `sources/`;
keep editable audio projects and rendered WAVs under `edited/`. Neither folder is
deployed. Use the actual original extension; transcoding MP3 to WAV does not
restore lossless detail. Supply a distribution notice separately from private
account records. Do not put credentials or account secrets in the runtime catalog.

```powershell
python scripts/import-audio.py artifacts/audio-production/amb_garden_air_01.json
python scripts/import-audio.py --check
# Final production gate fails until every clip exists and matches its hash:
python scripts/import-audio.py --check --require-complete
```

The importer accepts reviewed PCM16 mono/stereo WAVs, checks peak/DC/size and full
buffer loop frames, retains source/output hashes and writes a 10 ms waveform
envelope under `reviews/`. Numeric checks cannot establish pleasant timbre,
musical seams, natural cat sounds or speaker output. Audition the 44 ElevenLabs
clips and three Suno deliveries, then listen through the complete three-quest,
purchase, place, cat-use and ending journey. Listen to at least five repeats of
each loop through the actual backend; record browser/device, build hash, volume,
seams and clipping observations in `review`. Do not mark `journeyAuditioned` from
the synthetic automated test.
