"""Importer regression tests use an isolated build directory, never runtime assets."""
import contextlib, copy, importlib.util, io, json, math, struct, sys, tempfile, unittest, wave
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('audio_import',ROOT/'scripts/import-audio.py')
module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)

class ImportTests(unittest.TestCase):
    def setUp(self):
        (ROOT/'build').mkdir(exist_ok=True)
        self.temp=tempfile.TemporaryDirectory(dir=ROOT/'build',prefix='audio-import-test-')
        self.root=Path(self.temp.name);module.ROOT=self.root;module.CATALOG=self.root/'assets/runtime/audio/catalog.json'
        module.CATALOG.parent.mkdir(parents=True)
        self.clip={'id':'amb_garden_air_01','file':'amb_garden_air_01.wav','provider':'ElevenLabs','accepted':False,'loop':True}
        module.CATALOG.write_text(json.dumps({'clips':[self.clip]}))
        folder=self.root/'artifacts/audio-production';folder.mkdir(parents=True)
        self.wav=folder/'signal.wav'
        self.write_wave(.1)
        (folder/'rights.txt').write_text('Synthetic unit test only, no provider-generation claim.')
        self.meta={'id':self.clip['id'],'edited':'artifacts/audio-production/signal.wav','loop':{'startFrame':0,'endFrame':4800},
            'source':{'provider':'ElevenLabs','assetId':'test-only','url':'test-only','generatedAt':'test-only','prompt':'test fixture','settings':{'test':True},'rightsRecord':'artifacts/audio-production/rights.txt','original':'artifacts/audio-production/signal.wav','processing':['test-only signal'],'licenseNotice':'Unit-test synthetic fixture, never distributed as final audio.'},
            'review':{'approved':True,'sourceAuditioned':True,'loopAuditioned':True,'offlineRepeats':5,'journeyAuditioned':False}}
    def tearDown(self): self.temp.cleanup()
    def write_wave(self,amplitude):
        with wave.open(str(self.wav),'wb') as w:
            w.setparams((1,2,48000,4800,'NONE','not compressed'))
            w.writeframes(b''.join(struct.pack('<h',int(math.sin(i*2*math.pi*440/48000)*32767*amplitude)) for i in range(4800)))
    def run_import(self):
        meta=self.root/'input.json';meta.write_text(json.dumps(self.meta));previous=sys.argv
        try:
            sys.argv=['import-audio.py',str(meta)]
            with contextlib.redirect_stdout(io.StringIO()):module.main()
        finally:sys.argv=previous
    def test_valid_delivery_keeps_journey_unverified(self):
        self.run_import();c=json.loads(module.CATALOG.read_text())['clips'][0]
        self.assertTrue(c['accepted']);self.assertFalse(c['review']['journeyAuditioned'])
        self.assertEqual(c['pcm']['frames'],4800);self.assertEqual(len(c['sha256']),64)
        self.assertTrue((module.CATALOG.parent/'amb_garden_air_01.notice.txt').exists())
        self.assertTrue((self.root/'artifacts/audio-production/reviews/amb_garden_air_01.json').exists())
    def test_unreviewed_rejected_without_publication(self):
        self.meta['review']['sourceAuditioned']=False
        with self.assertRaises(AssertionError):self.run_import()
        self.assertFalse((module.CATALOG.parent/self.clip['file']).exists())
    def test_silent_and_clipping_rejected(self):
        for amplitude in (0,1):
            self.write_wave(amplitude)
            with self.assertRaises(AssertionError):self.run_import()
    def test_untrimmed_loop_rejected(self):
        self.meta['loop']['startFrame']=100
        with self.assertRaises(AssertionError):self.run_import()
    def test_wrong_provider_rejected(self):
        self.meta['source']['provider']='Another music provider'
        with self.assertRaises(AssertionError):self.run_import()
    def test_missing_rights_rejected(self):
        self.meta['source']['rightsRecord']='artifacts/audio-production/missing.txt'
        with self.assertRaises(AssertionError):self.run_import()

if __name__=='__main__':unittest.main()
