#include "../Source/PluginProcessor.h"
#include <iostream>
#include <stdexcept>
static int checks=0;
void require(bool condition,const char* message){++checks;if(!condition)throw std::runtime_error(message);}
void signal(juce::AudioBuffer<float>& b,double rate,int offset=0){for(int c=0;c<2;++c)for(int i=0;i<b.getNumSamples();++i)b.setSample(c,i,.2511886f*std::sin((float)(2*juce::MathConstants<double>::pi*440*(i+offset)/rate)));}
int main(int argc,char** argv){
 juce::ScopedJuceInitialiser_GUI init;
 try{
  TapeProcessor p;
  require(p.value("open")==0,"Default panel must be closed");require(p.value("meterL")==0&&p.value("meterR")==0,"Default meters IN");
  p.setParameter("recordL",4);require(p.value("recordR")==4,"Linked record");p.setParameter("autogainL",1);require(p.value("reproduceL")==-4&&p.value("reproduceR")==-4,"Autogain activation");p.setParameter("recordR",2);require(p.value("recordL")==2&&p.value("reproduceL")==-2,"Record R inverse linking");p.setParameter("link",0);p.setParameter("recordL",5);require(p.value("recordR")==2,"Unlinked independent gain");
  p.setParameter("recordHFL",3);require(p.value("autoCal")==0,"Manual calibration flag");p.calibrate();require(p.value("recordHFL")==0&&p.value("autoCal")==1,"Auto calibration reset");
  p.setParameter("open",1);p.setParameter("reproHFR",2.5f);juce::MemoryBlock saved;p.getStateInformation(saved);TapeProcessor restored;restored.setStateInformation(saved.getData(),(int)saved.getSize());require(restored.value("open")==1&&restored.value("reproHFR")==2.5f,"DAW state and open panel restored");require(restored.value("recordL")==5&&restored.value("recordR")==2,"Independent state restored without linking side effects");
  p.setParameter("speed",0);p.setParameter("formula",6);require(p.value("formula")==1,"30 IPS formula compatibility");
  juce::MidiBuffer midi;juce::AudioBuffer<float> b(2,256);
  for(double sr:{44100.,48000.,96000.}){
   p.prepareToPlay(sr,256);
   for(int preset=0;preset<p.getNumPrograms();++preset){p.setCurrentProgram(preset);for(int k=0;k<12;++k){signal(b,sr,k*256);p.processBlock(b,midi);for(int c=0;c<2;++c)for(int n=0;n<b.getNumSamples();++n)require(std::isfinite(b.getSample(c,n))&&std::abs(b.getSample(c,n))<8,"Preset DSP finite bounded output");}require(p.value("open")==1,"Preset preserves panel mode");}
   p.setCurrentProgram(7);p.prepareToPlay(sr,256);signal(b,sr);auto input=b; p.processBlock(b,midi);int latency=p.getLatencySamples();for(int c=0;c<2;++c)for(int n=latency;n<256;++n)require(std::abs(b.getSample(c,n)-input.getSample(c,n-latency))<1e-7,"THRU unity and reported latency");
   p.setCurrentProgram(2);signal(b,sr);input=b;p.processBlock(b,midi);require(b.getMagnitude(0,256)>0,"Processed output not silent");require(std::abs(b.getRMSLevel(0,0,256)-input.getRMSLevel(0,0,256))>.0001f,"Saturation changes signal");
  }
  if(argc>1&&juce::String(argv[1])=="--render-ui") {
   p.setCurrentProgram(0);
   auto editor=std::unique_ptr<juce::AudioProcessorEditor>(p.createEditor());
   require(editor!=nullptr,"Native editor created");
   editor->setVisible(true);
   for(int mode=0;mode<2;++mode){p.setParameter("open",(float)mode);juce::MessageManager::getInstance()->runDispatchLoopUntil(100);int visibleSliders=0;for(int i=0;i<editor->getNumChildComponents();++i)if(auto* slider=dynamic_cast<juce::Slider*>(editor->getChildComponent(i))){require(!slider->getBounds().isEmpty(),"Slider has layout bounds");if(slider->isVisible())++visibleSliders;}require(visibleSliders==(mode?14:4),"Correct calibration control visibility");auto image=editor->createComponentSnapshot(editor->getLocalBounds(),true,1.5f);require(image.isValid(),"Native editor rendered");auto file=juce::File::getCurrentWorkingDirectory().getChildFile(mode?"native-open.png":"native-closed.png");file.deleteFile();auto out=file.createOutputStream();require(out!=nullptr,"Screenshot output");juce::PNGImageFormat png;require(png.writeImageToStream(image,*out),"Screenshot encoded");}
   std::cout<<"Both native editor modes rendered\n";
  }
  if(argc>1&&juce::String(argv[1])=="--export-presets") {auto dir=juce::File::getCurrentWorkingDirectory().getChildFile("presets");dir.createDirectory();for(int i=0;i<p.getNumPrograms();++i){p.setCurrentProgram(i);p.setParameter("open",0);p.savePreset(dir.getChildFile(p.getProgramName(i)+".atrpreset"));}std::cout<<"Factory presets exported\n";}
  if(argc>2&&juce::String(argv[1])=="--vst3"){
   juce::VST3PluginFormat format;juce::OwnedArray<juce::PluginDescription> descriptions;format.findAllTypesForFile(descriptions,argv[2]);require(descriptions.size()==1,"Actual VST3 scanned");juce::String error;auto plugin=format.createInstanceFromDescription(*descriptions[0],48000,256,error);require(plugin!=nullptr,error.toRawUTF8());plugin->prepareToPlay(48000,256);signal(b,48000);plugin->processBlock(b,midi);require(b.getMagnitude(0,256)>0,"Actual VST3 processes signal");plugin->releaseResources();std::cout<<"Actual VST3 load/process passed\n";
  }
  std::cout<<"PASS "<<checks<<" checks: DSP at 44.1/48/96 kHz, all 8 presets, link, autogain, calibration, state, bypass\n";
 }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
 return 0;
}
