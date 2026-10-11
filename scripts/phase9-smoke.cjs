// Runs the existing complete journey against the Prompt 9 build, with in-memory fixtures.
process.env.PAWS_AUDIO_TEST='1';
process.env.SITE_DIR ||= 'build/web-phase9-pinned-release/site';
process.env.EVIDENCE_DIR ||= 'build/evidence/phase9/journey';
process.env.PAWS_TEST_PORT ||= '19519';
require('./phase7-smoke.cjs');
