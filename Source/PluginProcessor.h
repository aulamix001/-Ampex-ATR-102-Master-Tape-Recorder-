// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

class TapeProcessor final : public juce::AudioProcessor, private juce::AudioProcessorValueTreeState::Listener
{
public:
    TapeProcessor();
    ~TapeProcessor() override;
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "ATR-102 Tape Studio"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.04; }
    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram.load(); }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    juce::String getProgramDescription(int);
    juce::String getProgramCategory(int);
    juce::String getProgramId(int);
    juce::var getProgramParameters(int);
    static std::vector<int> availableFormulas(int speedIndex);
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorValueTreeState parameters;
    std::array<std::atomic<float>, 2> inputMeter {}, outputMeter {};
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
    void setParameter(const juce::String&, float);
    float value(const juce::String& id) const { return parameters.getRawParameterValue(id)->load(); }
    void calibrate();
    void savePreset(const juce::File&);
    bool loadPreset(const juce::File&);
private:
    void parameterChanged(const juce::String&, float) override;
    struct Channel {
        std::atomic<float> *record{}, *reproduce{}, *autogain{}, *meter{};
        std::array<std::atomic<float>*, 5> trim{};
        juce::SmoothedValue<float> rec, rep;
        double low{}, hf{}, head{}, transformerDC{}, prev{};
        std::array<float, 16384> delay{};
        int write{};
    };
    std::array<Channel, 2> channels;
    std::array<juce::String, 5> trimNames {"recordHF", "shelf", "bias", "reproLF", "reproHF"};
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    std::array<std::array<float, 128>, 2> bypassDelay{};
    int bypassWrite{};
    double rate=44100, phase=0, humPhase=0;
    uint32_t noiseSeed=0x12345678;
    std::atomic<bool> changing{false};
    std::atomic<int> currentProgram{0};
    std::atomic<float> *power{}, *path{}, *speed{}, *formula{}, *cal{}, *head{}, *eq{}, *crosstalk{}, *wow{}, *noise{}, *transformer{};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TapeProcessor)
};
