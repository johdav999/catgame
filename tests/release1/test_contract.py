import copy
import importlib.util
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('release1_validator', ROOT / 'scripts/release1/validate-contract.py')
validator = importlib.util.module_from_spec(spec)
spec.loader.exec_module(validator)


class ContractTests(unittest.TestCase):
    def setUp(self):
        self.contract = json.loads((ROOT / 'docs/release1/contract.json').read_text())

    def test_current_source_and_disjoint_ownership(self):
        self.assertEqual(validator.validate(self.contract)['friendship'],
                         {'pip': 5, 'mabel': 3, 'button': 3, 'pepper': 3})

    def test_invalid_graph_identity_economy_and_ownership_reject(self):
        for mutate in [
            lambda c: c['quests'][0]['requires'].append('garden_picnic'),
            lambda c: c['quests'][0]['requires'].append('unknown'),
            lambda c: c['quests'][3].update(id='pip_missing_ball'),
            lambda c: c['quests'][0].update(contentId='renamed-legacy'),
            lambda c: c['cats'][0].update(maximumFriendship=4),
            lambda c: c['items'][0].update(price=31),
            lambda c: c['tracks'][1].update(source='src/release1/cats/nested'),
        ]:
            candidate = copy.deepcopy(self.contract)
            mutate(candidate)
            with self.assertRaises(ValueError):
                validator.validate(candidate)


if __name__ == '__main__':
    unittest.main()
