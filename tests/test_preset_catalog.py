"""Check preservation and mapping against the user-supplied bank independently."""
import json
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


class CatalogTests(unittest.TestCase):
    def test_all_entries_preserved_and_mapped(self):
        source = json.loads((ROOT / 'presets/imported-bank.json').read_text())['bank_presets']
        catalog = json.loads((ROOT / 'presets/catalog.json').read_text())['presets']
        self.assertEqual(len(catalog), 80)
        self.assertEqual(len({p['id'] for p in catalog}), 80)
        self.assertEqual([p['name'] for p in catalog[:8]], [p['name'] for p in json.loads((ROOT / 'presets/presets.json').read_text())['presets']])
        self.assertEqual([p['id'] for p in catalog[:8]], [f'factory/{i}' for i in range(8)])
        entries = [dict(p, category=group['category']) for group in source for p in group['presets']]
        self.assertEqual(len(entries), 72)
        for original, imported in zip(entries, catalog[8:]):
            with self.subTest(category=original['category'], name=original['name']):
                self.assertEqual(imported['name'], original['name'])
                self.assertEqual(imported['category'], original['category'])
                self.assertEqual(imported['settings'], original['settings'])
                settings, mapped = original['settings'], imported['mapped_parameters']
                self.assertEqual(['250', '456', '468', 'GP9', '900', '35-90', '111'][mapped['formula']], settings['tape'])
                self.assertEqual(['+3', '+6', '+7.5', '+9'][mapped['cal']], settings['cal'])
                self.assertEqual(['1"', '1/2"', '1/4"'][mapped['head']], settings['head'])
                self.assertEqual([30, 15, 7.5, 3.75][mapped['speed']], settings['speed'])
                self.assertEqual(['NAB', 'CCIR'][mapped['eq']], next(iter(settings['emphasis_eq']), 'NAB'))
                self.assertEqual(mapped['link'], settings['link'])
                self.assertEqual(mapped['path'], 2)
                self.assertEqual(set(mapped), {'formula', 'cal', 'head', 'speed', 'eq', 'link', 'path'})

    def test_conflicting_names_remain_separate(self):
        presets = json.loads((ROOT / 'presets/catalog.json').read_text())['presets']
        for name, parameter in [('Clean Ultralinear Master', 'cal'), ('Vintage Type 111 Lo Fi', 'cal'), ('Pushed Bright and Sizzly', 'formula')]:
            variants = [p for p in presets if p['name'] == name]
            self.assertGreater(len({p['mapped_parameters'][parameter] for p in variants}), 1)
            self.assertEqual(len({p['id'] for p in variants}), len(variants))

    def test_embedded_catalog_is_current(self):
        header = (ROOT / 'Source/PresetMetadata.h').read_text()
        embedded = json.loads(header.split('u8R"PRESETS(', 1)[1].split(')PRESETS";', 1)[0])
        self.assertEqual(embedded, json.loads((ROOT / 'presets/catalog.json').read_text()))


if __name__ == '__main__':
    unittest.main()
