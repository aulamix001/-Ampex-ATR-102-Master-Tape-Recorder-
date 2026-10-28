#!/usr/bin/env python3
"""Build a stable, categorized native catalog while retaining the supplied bank."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FACTORY_OVERRIDES = [
    {},
    {'speed': 0, 'formula': 4, 'noise': 0, 'wow': 0, 'recordL': -3, 'recordR': -3, 'reproduceL': 3, 'reproduceR': 3},
    {'recordL': 4, 'recordR': 4, 'reproduceL': -4, 'reproduceR': -4, 'formula': 1},
    {'speed': 2, 'formula': 6, 'recordL': 2, 'recordR': 2, 'reproduceL': -2, 'reproduceR': -2},
    {'speed': 0, 'formula': 3, 'cal': 3, 'head': 0, 'noise': 0},
    {'speed': 2, 'formula': 0, 'cal': 0, 'head': 2},
    {'formula': 2, 'recordL': 1.5, 'recordR': 1.5, 'reproduceL': -1.5, 'reproduceR': -1.5, 'wow': 0, 'noise': 0},
    {'path': 3},
]
FORMULAS = ['250', '456', '468', 'GP9', '900', '35-90', '111']
CAL = ['+3', '+6', '+7.5', '+9']
HEADS = ['1"', '1/2"', '1/4"']
SPEEDS = [30, 15, 7.5, 3.75]
CATEGORIES = ['Master', 'Vintage', 'Lo-Fi', 'Clean', 'Pushed', 'Bright', 'Vocal', 'Bass', 'Classic', 'Drums', 'Guitars']


CCIR_CATEGORIES = {'Master', 'Clean', 'Bright', 'Vocal'}


def resolve_eq(category, speed, options):
    """Fill only ambiguous EQ fields; never override a single explicit setting."""
    if len(options) == 1:
        return options[0], 'Explicit source EQ retained.'
    preferred = 'CCIR' if speed == 15 and category in CCIR_CATEGORIES else 'NAB'
    selected = preferred if not options or preferred in options else options[0]
    reason = ('Clarity-oriented 15 IPS starting point' if selected == 'CCIR'
              else 'Conservative vintage/low-speed starting point')
    return selected, f'{reason}: {selected}; chosen for an ambiguous source field, not recovered UAD calibration.'


def build_catalog(factory, bank):
    if len(factory['presets']) != 8:
        raise ValueError('The original eight program indices must remain stable')
    if [group['category'] for group in bank['bank_presets']] != CATEGORIES:
        raise ValueError('Unexpected category order')
    programs = []
    for index, item in enumerate(factory['presets']):
        programs.append(dict(item, id=f'factory/{index}', category='Factory', mapped_parameters=FACTORY_OVERRIDES[index]))
    for group in bank['bank_presets']:
        category = group['category']
        seen = set()
        for item in group['presets']:
            name, settings = item['name'], item['settings']
            if name in seen:
                raise ValueError(f'Duplicate within category: {category}/{name}')
            seen.add(name)
            eq_options = settings['emphasis_eq']
            if not isinstance(settings['link'], bool) or any(eq not in ['NAB', 'CCIR'] for eq in eq_options):
                raise ValueError(f'Invalid settings: {category}/{name}')
            eq, eq_note = resolve_eq(category, settings['speed'], eq_options)
            mode = settings['mode']
            path = {'Sync/Repro': 2, 'SYNC': 0, 'INPUT': 1, 'REPRO': 2, 'THRU': 3}[mode]
            mapped = {
                'formula': FORMULAS.index(settings['tape']), 'cal': CAL.index(settings['cal']),
                'head': HEADS.index(settings['head']), 'speed': SPEEDS.index(settings['speed']),
                'eq': ['NAB', 'CCIR'].index(eq), 'link': int(settings['link']), 'path': path,
            }
            notes = []
            if not eq_options:
                notes.append('Emphasis EQ unspecified. ' + eq_note)
            elif len(eq_options) > 1:
                notes.append('Multiple emphasis choices retained in source. ' + eq_note)
            if mode == 'Sync/Repro':
                notes.append('Sync/Repro interpreted as REPRO; SYNC remains independently selectable.')
            description = f"{settings['tape']} / {settings['speed']:g} IPS / CAL {settings['cal']} / {settings['head']} / {eq} / REPRO"
            programs.append(dict(item, id=f'bank/{category}/{name}', category=category,
                                 description=description, mapped_parameters=mapped, mapping_notes=notes))
    return {'schema_version': 1, 'provenance': 'User-supplied bank; official origin and completeness not independently verified.',
            'mapping_policy': {'revision': 2, 'empty_eq': 'engineering starting point by category and speed',
                               'multiple_eq': 'prefer the engineering choice among supplied options',
                               'explicit_eq': 'retain exactly',
                               'ccir_at_15_ips': sorted(CCIR_CATEGORIES),
                               'other_ambiguous_eq': 'NAB', 'Sync/Repro': 'REPRO',
                               'unspecified_parameters': 'plugin defaults', 'duplicate_names': 'retain category-specific entries'},
            'presets': programs}


def main():
    factory = json.loads((ROOT / 'presets/presets.json').read_text(encoding='utf-8'))
    bank = json.loads((ROOT / 'presets/imported-bank.json').read_text(encoding='utf-8'))
    catalog = build_catalog(factory, bank)
    source = json.dumps(catalog, indent=2, ensure_ascii=False) + '\n'
    header = ('// Generated by scripts/generate-preset-metadata.py; edit the JSON sources.\n'
              '#pragma once\nstatic constexpr const char* presetMetadataJson = u8R"PRESETS(' + source + ')PRESETS";\n')
    (ROOT / 'presets/catalog.json').write_text(source, encoding='utf-8')
    (ROOT / 'Source/PresetMetadata.h').write_text(header, encoding='utf-8')
    print(f'Generated {len(catalog["presets"])} programs in 12 categories (8 original + 72 supplied entries).')


if __name__ == '__main__':
    main()
