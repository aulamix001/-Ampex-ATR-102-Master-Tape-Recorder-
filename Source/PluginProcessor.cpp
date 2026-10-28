// SPDX-License-Identifier: GPL-3.0-only
#include "PluginProcessor.h"
#include "PluginEditor.h"
namespace {
const juce::StringArray presetNames {"Calibrated 15 IPS", "Clean 30 IPS", "Warm Master", "Low Speed Color", "Wide Open GP9", "Vintage 250", "Gentle Glue", "Through Reference"};
constexpr float speeds[] {30,15,7.5f,3.75f};
constexpr float drives[] {2.8f,2.2f,2.0f,1.5f,1.4f,2.5f,3.2f};
constexpr float levels[] {3,6,7.5f,9};
float db(float v) { return juce::Decibels::decibelsToGain(v); }
}
juce::AudioProcessorValueTreeState::ParameterLayout TapeProcessor::makeParameters() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    auto boolean=[&](const juce::String& id,const juce::String& name,bool def){layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id,1},name,def));};
    auto choice=[&](const char* id,const char* name,juce::StringArray options,int def){layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id,1},name,options,def));};
    auto gain=[&](const juce::String& id,const juce::String& name,float extent){layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(-extent,extent,0.01f),0.0f,juce::AudioParameterFloatAttributes().withLabel("dB")));};
    boolean("open","Open calibration panel",false); boolean("link","Stereo link",true);boolean("power","Power",true);
    choice("speed","Tape speed",{"30 IPS","15 IPS","7.5 IPS","3.75 IPS"},1);
    choice("formula","Tape formula",{"250","456","468","GP9","900","35-90","111"},1);
    choice("cal","Calibration",{"+3","+6","+7.5","+9"},1);choice("head","Head width",{"1 inch","1/2 inch","1/4 inch"},1);
    choice("eq","Emphasis",{"NAB","CCIR"},0);choice("path","Path",{"SYNC","INPUT","REPRO","THRU"},2);
    for(auto id:{"crosstalk","wow","noise","transformer","autoCal"}) boolean(id,juce::String(id),true);
    for(auto suffix:{"L","R"}) {
        gain("record"+juce::String(suffix),"Record "+juce::String(suffix),12);
        gain("reproduce"+juce::String(suffix),"Reproduce "+juce::String(suffix),12);
        boolean("autogain"+juce::String(suffix),"Autogain "+juce::String(suffix),false);
        choice(("meter"+juce::String(suffix)).toRawUTF8(),("Meter "+juce::String(suffix)).toRawUTF8(),{"IN","OUT"},0);
        for(auto trim:{"recordHF","shelf","bias","reproLF","reproHF"}) gain(juce::String(trim)+suffix,juce::String(trim)+" "+suffix,6);
    }
    return layout;
}
TapeProcessor::TapeProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"ATR102",makeParameters()) {
    auto raw=[&](const char* id){return parameters.getRawParameterValue(id);};
    power=raw("power");path=raw("path");speed=raw("speed");formula=raw("formula");cal=raw("cal");head=raw("head");eq=raw("eq");crosstalk=raw("crosstalk");wow=raw("wow");noise=raw("noise");transformer=raw("transformer");
    for(int c=0;c<2;++c){auto suffix=juce::String(c==0?"L":"R");auto& ch=channels[(size_t)c];ch.record=raw(("record"+suffix).toRawUTF8());ch.reproduce=raw(("reproduce"+suffix).toRawUTF8());ch.autogain=raw(("autogain"+suffix).toRawUTF8());ch.meter=raw(("meter"+suffix).toRawUTF8());for(size_t i=0;i<5;++i)ch.trim[i]=raw((trimNames[i]+suffix).toRawUTF8());}
    for(auto* p:getParameters()) if(auto* ranged=dynamic_cast<juce::RangedAudioParameter*>(p))parameters.addParameterListener(ranged->paramID,this);
}
TapeProcessor::~TapeProcessor(){for(auto* p:getParameters())if(auto* ranged=dynamic_cast<juce::RangedAudioParameter*>(p))parameters.removeParameterListener(ranged->paramID,this);}
void TapeProcessor::setParameter(const juce::String& id,float v){if(auto* p=parameters.getParameter(id))p->setValueNotifyingHost(p->convertTo0to1(v));}
void TapeProcessor::parameterChanged(const juce::String& id,float v){
    if(changing.exchange(true))return;
    const bool linked=value("link")>0.5f;
    if(id.endsWith("L")||id.endsWith("R")){
        const bool left=id.endsWith("L");const juce::String suffix=left?"L":"R",other=left?"R":"L",stem=id.dropLastCharacters(1);
        if(linked && stem!="meter")setParameter(stem+other,v);
        if(stem=="record"&&value("autogain"+suffix)>.5f){setParameter("reproduce"+suffix,-v);if(linked)setParameter("reproduce"+other,-v);}
        if(stem=="autogain"&&v>.5f){setParameter("reproduce"+suffix,-value("record"+suffix));if(linked)setParameter("reproduce"+other,-value("record"+other));}
        for(const auto& trim:trimNames)if(stem==trim)setParameter("autoCal",0);
    }
    if(id=="link"&&v>.5f){for(auto stem:{"record","reproduce","autogain","recordHF","shelf","bias","reproLF","reproHF"})setParameter(juce::String(stem)+"R",value(juce::String(stem)+"L"));}
    if(id=="autoCal"&&v>.5f)for(auto suffix:{"L","R"})for(const auto& trim:trimNames)setParameter(trim+suffix,0);
    if(id=="speed"||id=="formula"){
        int s=(int)value("speed"),f=(int)value("formula");
        if((s==0 && (f==0||f>=5)) || (s==1&&f>=4) || (s>=2&&(f==2||f==3||f==4)))setParameter("formula",1);
    }
    changing.store(false);
}
void TapeProcessor::calibrate(){setParameter("autoCal",0);setParameter("autoCal",1);}
bool TapeProcessor::isBusesLayoutSupported(const BusesLayout& l)const{return l.getMainInputChannelSet()==juce::AudioChannelSet::stereo()&&l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
void TapeProcessor::prepareToPlay(double sr,int block){
    rate=sr;phase=humPhase=0;bypassWrite=0;for(auto& d:bypassDelay)d.fill(0);
    oversampling=std::make_unique<juce::dsp::Oversampling<float>>(2,1,juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,true,true);
    oversampling->initProcessing((size_t)juce::jmax(1,block));oversampling->reset();setLatencySamples((int)std::round(oversampling->getLatencyInSamples()));
    for(auto& ch:channels){ch.low=ch.hf=ch.head=ch.transformerDC=ch.prev=0;ch.delay.fill(0);ch.write=0;ch.rec.reset(sr*2,.02);ch.rep.reset(sr*2,.02);ch.rec.setCurrentAndTargetValue(db(ch.record->load()));ch.rep.setCurrentAndTargetValue(db(ch.reproduce->load()));}
}
void TapeProcessor::processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer&){
    for(int n=0;n<b.getNumSamples();++n){for(int c=0;c<2;++c){auto& d=bypassDelay[(size_t)c];float x=b.getSample(c,n);d[(size_t)bypassWrite]=x;b.setSample(c,n,d[(size_t)((bypassWrite-getLatencySamples()+128)%128)]);}bypassWrite=(bypassWrite+1)%128;}
    for(int c=0;c<2;++c){float rms=b.getRMSLevel(c,0,b.getNumSamples());inputMeter[(size_t)c].store(rms);outputMeter[(size_t)c].store(rms);}
}
void TapeProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi){
    juce::ScopedNoDenormals guard;
    if(!oversampling||buffer.getNumSamples()==0)return;
    if(power->load()<.5f || (int)path->load()==1 || (int)path->load()==3){processBlockBypassed(buffer,midi);return;}
    for(int c=0;c<2;++c)inputMeter[(size_t)c].store(buffer.getRMSLevel(c,0,buffer.getNumSamples()));
    juce::dsp::AudioBlock<float> block(buffer);auto up=oversampling->processSamplesUp(block);
    const double sr=rate*2,ips=speeds[juce::jlimit(0,3,(int)speed->load())];
    const float drive=drives[juce::jlimit(0,6,(int)formula->load())]*db(levels[juce::jlimit(0,3,(int)cal->load())]-6);
    const double hfCut=juce::jlimit(2500.,22000.,ips*1000.);const double hfA=1-std::exp(-juce::MathConstants<double>::twoPi*hfCut/sr);
    const double lfA=1-std::exp(-juce::MathConstants<double>::twoPi*90/sr),headA=1-std::exp(-juce::MathConstants<double>::twoPi*(ips*3)/sr);
    const float bleed=crosstalk->load()>.5f?.0025f:0;const bool mod=wow->load()>.5f, noisy=noise->load()>.5f, trans=transformer->load()>.5f;
    const double humHz=(speed->load()==0||eq->load()<.5f)?60:50;
    std::array<std::array<float,5>,2> trims{};for(int c=0;c<2;++c){auto& ch=channels[(size_t)c];ch.rec.setTargetValue(db(ch.record->load()));ch.rep.setTargetValue(db(ch.reproduce->load()));for(size_t k=0;k<5;++k)trims[(size_t)c][k]=ch.trim[k]->load();}
    for(size_t n=0;n<up.getNumSamples();++n){
        const float l=up.getSample(0,(int)n),r=up.getSample(1,(int)n);
        phase+=juce::MathConstants<double>::twoPi*.65/sr;if(phase>juce::MathConstants<double>::twoPi)phase-=juce::MathConstants<double>::twoPi;
        humPhase+=juce::MathConstants<double>::twoPi*humHz/sr;if(humPhase>juce::MathConstants<double>::twoPi)humPhase-=juce::MathConstants<double>::twoPi;
        for(int c=0;c<2;++c){auto& ch=channels[(size_t)c];const auto& t=trims[(size_t)c];double x=((c==0?l:r)+bleed*(c==0?r:l))*ch.rec.getNextValue();
            noiseSeed=noiseSeed*1664525u+1013904223u;if(noisy)x+=((double(noiseSeed)/4294967296.0-.5)*.00012+std::sin(humPhase)*.000025);
            ch.low+=lfA*(x-ch.low);ch.hf+=hfA*(x-ch.hf);const double high=x-ch.hf;
            x=ch.hf+high*db(t[0]+t[1]);const double d=drive*db(t[2]*.35f);
            x=std::tanh(x*d)/d;
            if(trans){const double asymmetric=std::tanh(x*1.12+.025)/1.12-std::tanh(.025)/1.12;ch.transformerDC+=.0001*(asymmetric-ch.transformerDC);x=asymmetric-ch.transformerDC;}
            ch.head+=headA*(x-ch.head);x+=ch.head*(db(t[3]+((int)head->load()==2?1.4f:.6f))-1);
            const float eqLift=(speed->load()!=0&&eq->load()>.5f)?0.6f:0;
            x+=high*(db(t[4]+eqLift)-1)*.5;
            ch.delay[(size_t)ch.write]=(float)x;
            const double delay=sr*(.001+(mod?.00008*(15/ips)*(std::sin(phase)+.2*std::sin(phase*13.7)):0));
            double read=ch.write-delay;while(read<0)read+=ch.delay.size();const int i=(int)read,j=(i+1)%(int)ch.delay.size();const float fraction=(float)(read-i);
            const float y=ch.delay[(size_t)i]+fraction*(ch.delay[(size_t)j]-ch.delay[(size_t)i]);ch.write=(ch.write+1)%(int)ch.delay.size();
            up.setSample(c,(int)n,y*ch.rep.getNextValue());
        }
    }
    oversampling->processSamplesDown(block);
    for(int c=0;c<2;++c)outputMeter[(size_t)c].store(buffer.getRMSLevel(c,0,buffer.getNumSamples()));
}
int TapeProcessor::getNumPrograms(){return presetNames.size();}
const juce::String TapeProcessor::getProgramName(int i){return presetNames[juce::jlimit(0,presetNames.size()-1,i)];}
void TapeProcessor::setCurrentProgram(int i){
    i=juce::jlimit(0,presetNames.size()-1,i);currentProgram.store(i);
    changing.store(true);
    for(auto* p:getParameters())if(auto* rp=dynamic_cast<juce::RangedAudioParameter*>(p))if(rp->paramID!="open")rp->setValueNotifyingHost(rp->getDefaultValue());
    auto set=[&](const char* id,float v){setParameter(id,v);};
    if(i==1){set("speed",0);set("formula",4);set("noise",0);set("wow",0);set("recordL",-3);set("recordR",-3);set("reproduceL",3);set("reproduceR",3);}
    if(i==2){set("recordL",4);set("recordR",4);set("reproduceL",-4);set("reproduceR",-4);set("formula",1);}
    if(i==3){set("speed",3);set("formula",6);set("recordL",2);set("recordR",2);set("reproduceL",-2);set("reproduceR",-2);}
    if(i==4){set("speed",0);set("formula",3);set("cal",3);set("head",0);set("noise",0);}
    if(i==5){set("speed",2);set("formula",0);set("cal",0);set("head",2);}
    if(i==6){set("formula",2);set("recordL",1.5f);set("recordR",1.5f);set("reproduceL",-1.5f);set("reproduceR",-1.5f);set("wow",0);set("noise",0);}
    if(i==7)set("path",3);
    changing.store(false);updateHostDisplay();
}
void TapeProcessor::getStateInformation(juce::MemoryBlock& dest){auto tree=parameters.copyState();tree.setProperty("program",currentProgram.load(),nullptr);if(auto xml=tree.createXml())copyXmlToBinary(*xml,dest);}
void TapeProcessor::setStateInformation(const void* data,int size){if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName("ATR102")){changing.store(true);auto tree=juce::ValueTree::fromXml(*xml);currentProgram.store(juce::jlimit(0,getNumPrograms()-1,(int)tree.getProperty("program",0)));parameters.replaceState(tree);changing.store(false);}}
void TapeProcessor::savePreset(const juce::File& file){juce::MemoryBlock data;getStateInformation(data);file.replaceWithData(data.getData(),data.getSize());}
bool TapeProcessor::loadPreset(const juce::File& file){juce::MemoryBlock data;if(!file.loadFileAsData(data)||!getXmlFromBinary(data.getData(),(int)data.getSize()))return false;auto xml=getXmlFromBinary(data.getData(),(int)data.getSize());if(!xml->hasTagName("ATR102"))return false;setStateInformation(data.getData(),(int)data.getSize());return true;}
juce::AudioProcessorEditor* TapeProcessor::createEditor(){return new TapeEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new TapeProcessor();}
