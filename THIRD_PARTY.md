# Third-party components

- JUCE 7.0.12, commit `4f43011b96eb0636104cb3e433894cda98243626`, copyright Raw Material Software Limited. Used under its GNU GPL v3 option. Source and notices: https://github.com/juce-framework/JUCE/tree/7.0.12 . JUCE includes additional module-specific third-party notices; these remain in the pinned source distribution.
- Steinberg VST3 SDK bundled in JUCE `modules/juce_audio_processors/format_types/VST3_SDK`. Copyright Steinberg Media Technologies GmbH; used under the GPL v3 option. Preserve the SDK's license notices when redistributing its source. VST is a trademark of Steinberg Media Technologies GmbH.
- Linux cross-build toolchain: GCC / MinGW-w64; generated binaries use GCC's runtime library exception and MinGW-w64 runtime licenses. System API DLLs are supplied by Windows. The toolchain itself is not vendored in this repository.

This is an independent effect inspired by visual hardware references. Ampex and ATR-102 names identify the inspiration; no endorsement, official circuitry, licensed graphics or hardware accuracy is claimed.
