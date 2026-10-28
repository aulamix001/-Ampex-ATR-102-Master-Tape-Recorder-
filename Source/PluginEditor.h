// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "PluginProcessor.h"
class TapeLook final : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    void drawButtonText(juce::Graphics&,juce::TextButton&,bool,bool) override;
};
class TapeEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit TapeEditor(TapeProcessor&);
    ~TapeEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    using Attachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    struct Dial { std::unique_ptr<juce::Slider> slider;std::unique_ptr<Attachment> attachment;juce::Rectangle<int> bounds; bool secondary; };
    struct Switch {std::unique_ptr<juce::TextButton> button;juce::String id;float setting;juce::Rectangle<int> bounds;bool toggle,secondary;};
    TapeProcessor& processor;
    TapeLook look;
    std::vector<Dial> dials;
    std::vector<Switch> switches;
    juce::ComboBox presets;
    juce::Label presetDescription;
    juce::TooltipWindow tooltips {this, 500};
    juce::TextButton save{"SAVE"},load{"LOAD"};
    std::unique_ptr<juce::FileChooser> chooser;
    juce::Image texture;
    float reelAngle=0;
    std::array<float,2> meterLevel{};
    void timerCallback() override;
    void dial(const juce::String&,juce::Rectangle<int>,bool=false,float=0);
    void control(const juce::String&,const juce::String&,float,juce::Rectangle<int>,bool=false,bool=false);
    void editParameter(const juce::String&,float);
    void drawMeter(juce::Graphics&,int,float,float);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TapeEditor)
};
