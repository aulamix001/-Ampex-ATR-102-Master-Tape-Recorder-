# Build and validation — 0.1.2

## Delivered platform

Windows x86-64 VST3 bundle, Release cross-build with GCC MinGW-w64 14.2 POSIX and JUCE 7.0.12 (`4f43011b96eb0636104cb3e433894cda98243626`). The binary is a native PE32+ DLL. Runtime dependencies are Windows system DLLs; GCC, C++ and pthread runtimes are linked statically. No WebView or Node runtime is needed. The build is unsigned.

## Catalog and mapping

- 8 existing Factory programs remain at indices 0–7, followed by 72 supplied category entries: Master 13, Vintage 10, Lo-Fi 5, Clean 4, Pushed 11, Bright 8, Vocal 5, Bass 3, Classic 7, Drums 4, Guitars 2.
- The bank has 32 distinct names and repeated entries across categories. All 72 entries are retained. Three names have conflicting settings between categories; no variants were merged or overwritten.
- `presets/imported-bank.json` preserves the complete supplied JSON. Generated `presets/catalog.json` records source settings, mapped internal values, stable IDs and mapping notes. `Source/PresetMetadata.h` embeds the same catalog in the VST3.
- Empty EQ selects NAB; multiple EQ options select the first (NAB); Sync/Repro selects REPRO. Unspecified gains, trims and circuit switches use plugin defaults. These are documented interpretation rules, not recovered hidden original settings.
- All seven formulas are selectable at 15 IPS so the imported 900, 35-90 and 111 settings are not coerced to 456. A two-page matrix shows four formulas at a time. Loading a preset reveals its selected formula page.
- Category-qualified IDs are saved along with the legacy numeric program index. Original program order, parameter IDs and explicit saved-session values are retained.

## Executed checks

- Python catalog suite: 3 tests passed. Checks every imported field against the supplied bank, original eight program identities, all category entries, three conflicting-name variants, unique IDs and the generated native catalog.
- Windows test executable under Wine 10 / Xvfb: each of the 80 programs processed at 44.1, 48 and 96 kHz with finite bounded output. Every mapped parameter was checked after program selection and state restoration. Stereo linking, inverse autogain, calibration reset, speed/formula selection, saturation activity, preserved panel state and latency-compensated unity bypass were exercised.
- Actual Windows VST3 scan, instantiation and audio processing through JUCE's VST3 host under Wine: PASS (1,477,664 assertions including the processor suite).
- Both native editor states rendered, exactly 4 main / 14 main-plus-calibration sliders with nonempty bounds, all 80 menu items, an imported 900 selection, tape-page switching and same-formula category-variant page restoration: PASS (1,477,706 assertions including the processor suite).
- Headless UI checks explicitly deliver due JUCE timers and wait up to two seconds for observable readiness. The tape-page test calls the bound button action directly; it does not simulate a native mouse event in a DAW window.
- Export and reload of all 80 `.atrpreset` files, with category identity and generated metadata checks: PASS (1,477,982 assertions including the processor suite).
- ZIP integrity and SHA-256 are checked by the packaging script. Original files are exported to `presets/`; imported files to `presets/bank/<Category>/`, preventing same-name collisions.

## Limitations

This build has not been tested in a Windows installation or third-party Windows DAW. Wine tests do not establish compatibility with every Windows host. The GitHub Actions workflow defines a separate MSVC build; local checks do not establish its remote outcome.

The supplied bank's official origin and completeness are unverified. The listed parameters are not complete reproductions of proprietary presets: no Record/Reproduce levels, trim values or circuit-toggle states were supplied. Different names with identical supplied settings may therefore sound identical in this model. No extra sonic differences were invented from the names.

The DSP and original procedural graphics approximate tape behavior and the visual references; hardware accuracy and sonic equivalence to UAD are not established. SYNC shares the REPRO model. Measured formula impulse responses and component-level circuit simulation are not implemented. Stereo input/output only, with 2x oversampling.

## Reproduce

See README.md for the Windows MSVC workflow. Regenerate metadata with `python scripts/generate-preset-metadata.py`; verify it with `python tests/test_preset_catalog.py`. Run `ATR102Tests --vst3 <bundle>`, `--render-ui`, and `--export-presets`, then `python scripts/package-windows.py --build <build-directory>`. Native Linux checks from earlier versions were not rerun for 0.1.2.
