"""Synthetic unit fixtures in temporary build directories, never production audio."""
import copy
import importlib.util
import json
import math
from pathlib import Path
import struct
import tempfile
import unittest
import wave

ROOT=Path(__file__).resolve().parents[3]
spec=importlib.util.spec_from_file_location('import_audio',ROOT/'scripts/release1/audio/import_audio.py')
audio=importlib.util.module_from_spec(spec);spec.loader.exec_module(audio)

class ImportTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory(dir=ROOT/'build',prefix='release1-audio-test-')
        self.root=Path(self.temp.name);self.job,self.runtime=audio.paths(self.root)
        catalog=json.loads((ROOT/'artifacts/audio-jobs/release1/catalog.json').read_text())
        self.clip=next(c for c in catalog['clips'] if c['id']=='water_flow_01')
        audio.dump(self.job/'catalog.json',dict(clips=[self.clip]))
        audio.dump(self.root/'assets/runtime/audio/catalog.json',dict(clips=[]))
        (self.job/'rights.txt').write_text('Synthetic tests only, no provider or production claim.')
        self.write_wave(.1)
        self.meta=dict(id=self.clip['id'],edited='fixture.wav',loop=dict(startFrame=0,endFrame=48000),
            source=dict(provider='ElevenLabs',assetId='test-only',url='test-only',generatedAt='test-only',prompt='unit fixture',settings={'test':True},
            original='fixture.wav',rightsRecord='rights.txt',processing=['test fixture'],licenseNotice='Synthetic unit fixture only'),
            review=dict(approved=True,sourceAuditioned=True,reviewer='test-only',loopAuditioned=True,offlineRepeats=5,journeyAuditioned=False))
    def tearDown(self): self.temp.cleanup()
    def write_wave(self,amp,seconds=1):
        with wave.open(str(self.job/'fixture.wav'),'wb') as w:
            w.setparams((1,2,48000,int(48000*seconds),'NONE','not compressed'))
            w.writeframes(b''.join(struct.pack('<h',int(math.sin(i*2*math.pi*440/48000)*32767*amp)) for i in range(int(48000*seconds))))
    def test_valid_measured_but_not_listened(self):
        c=audio.import_clip(self.root,self.meta);self.assertEqual(c['pcm']['pcmBytesAtDeviceRate'],192000)
        report=audio.audit(self.root,96000);self.assertEqual(report['finalCatalogPcmBytes'],384000)
        self.assertFalse(report['finalListeningAccepted'])
        fragment=json.loads((self.root/'docs/release1/fragments/audio.json').read_text())
        self.assertEqual(fragment['assets'][0]['bytes'],96044)
        with self.assertRaises(ValueError):audio.import_clip(self.root,self.meta)
    def test_invalid_metadata_does_not_publish(self):
        for field,value in [('provider','other'),('rightsRecord','../outside.txt'),('licenseNotice','REPLACE_NOTICE')]:
            m=copy.deepcopy(self.meta);m['source'][field]=value
            with self.assertRaises(ValueError):audio.import_clip(self.root,m)
        m=copy.deepcopy(self.meta);m['review']['approved']=False
        with self.assertRaises(ValueError):audio.import_clip(self.root,m)
        self.assertFalse(self.runtime.exists())
    def test_silence_clipping_and_duration(self):
        for amp,seconds in [(0,1),(1,1),(.1,.5)]:
            self.write_wave(amp,seconds)
            with self.assertRaises(ValueError):audio.import_clip(self.root,self.meta)
        self.assertFalse(self.runtime.exists())
    def test_loops_need_actual_review_and_full_buffer(self):
        for change in ('frames','review'):
            m=copy.deepcopy(self.meta)
            if change=='frames':m['loop']['startFrame']=1
            else:m['review']['offlineRepeats']=4
            with self.assertRaises(ValueError):audio.import_clip(self.root,m)
    def test_tampering_rejected(self):
        audio.import_clip(self.root,self.meta)
        (self.runtime/self.clip['file']).write_bytes(b'changed')
        with self.assertRaises(ValueError):audio.audit(self.root)
    def test_frozen_families_and_old_slots(self):
        expected=json.loads((ROOT/'docs/release1/contract.json').read_text())['audio']['newFamilies']
        new=json.loads((ROOT/'artifacts/audio-jobs/release1/catalog.json').read_text())['clips']
        self.assertEqual(set(expected),{c['family'] for c in new}|{'picnic_complete'})
        combined=json.loads((ROOT/'web/release1/audio/catalog.json').read_text())['clips']
        old=json.loads((ROOT/'assets/runtime/audio/catalog.json').read_text())['clips']
        self.assertEqual(combined[:47],old);self.assertEqual(len(combined),66)

if __name__=='__main__':unittest.main()
