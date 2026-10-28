#!/usr/bin/env python3
"""Package a built Windows VST3 bundle and its factory presets."""
from pathlib import Path
import argparse
import hashlib
import zipfile

parser = argparse.ArgumentParser()
parser.add_argument('--build', default='build-win')
args = parser.parse_args()
root = Path(__file__).resolve().parent.parent
bundle = root / args.build / 'ATR102_artefacts/Release/VST3/ATR-102 Tape Studio.vst3'
binary = bundle / 'Contents/x86_64-win/ATR-102 Tape Studio.vst3'
if not binary.is_file() or binary.read_bytes()[:2] != b'MZ':
    raise SystemExit('Windows x64 VST3 bundle missing; compile first.')
presets = sorted((root / 'presets').glob('*.atrpreset'))
if len(presets) != 8:
    raise SystemExit('Export all eight factory presets before packaging.')
out = root / 'releases/ATR102-Windows-x64-v0.1.0.zip'
out.parent.mkdir(exist_ok=True)
with zipfile.ZipFile(out, 'w', zipfile.ZIP_DEFLATED, compresslevel=9) as z:
    for f in sorted(bundle.rglob('*')):
        if f.is_file():
            z.write(f, f.relative_to(bundle.parent))
    for f in presets:
        z.write(f, 'presets/' + f.name)
    for name in ['README.md', 'LICENSE', 'THIRD_PARTY.md', 'VALIDATION.md']:
        z.write(root / name, name)
    z.writestr('INSTALL-WINDOWS.txt', 'Copy the entire ATR-102 Tape Studio.vst3 folder to:\r\nC:\\Program Files\\Common Files\\VST3\\\r\nThen rescan plugins in your 64-bit Windows DAW.\r\nDo not copy only the inner binary.\r\nFactory presets are built in; external .atrpreset files can be loaded with LOAD.\r\nThis independent model is unsigned. See README.md and VALIDATION.md.\r\n')
checksum = hashlib.sha256(out.read_bytes()).hexdigest()
(root / 'releases/SHA256SUMS.txt').write_text(f'{checksum}  {out.name}\n')
with zipfile.ZipFile(out) as z:
    if z.testzip() is not None:
        raise SystemExit('ZIP integrity failure')
print(f'{out.name}: {out.stat().st_size} bytes\nSHA256 {checksum}')
