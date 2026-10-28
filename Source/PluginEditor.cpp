// SPDX-License-Identifier: GPL-3.0-only
#include "PluginEditor.h"
namespace {
const auto cream=juce::Colour(0xffd8d4bf), ink=juce::Colour(0xff161812), green=juce::Colour(0xffa2f66a);
void text(juce::Graphics& g,const juce::String& s,juce::Rectangle<float> r,float size,juce::Colour color=cream,int align=juce::Justification::centred){g.setColour(color);g.setFont(juce::Font(size));g.drawText(s,r,juce::Justification(align));}
float angleFor(float p){constexpr float a[]{-135,-90,-45,45,90,135};p=juce::jlimit(0.f,5.f,p/2);int i=juce::jmin(4,(int)p);return juce::degreesToRadians(a[i]+(a[i+1]-a[i])*(p-i));}
}
void TapeLook::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float proportion,float,float,juce::Slider& s){
 const bool trim=(bool)s.getProperties()["trim"];auto area=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h);auto centre=area.getCentre();const float radius=juce::jmin(w,h)*(trim?.40f:.30f);float angle=0;
 if(!trim){const float base=(float)s.getProperties()["base"],v=(float)s.getValue();float p=base+(v<0?v/12*base:v/12*(10-base));angle=angleFor(p);
  for(int i=0;i<6;++i){bool left=i<3;int num=left?i*2:(i-3)*2+6;float ty=centre.y+(left?1-i: i-4)*radius*.85f;float tx=centre.x+(left?-1:1)*(radius+13);text(g,juce::String(num),{tx-7,ty-5,14,10},8);g.setColour(cream);g.drawHorizontalLine((int)ty,left?tx+8:tx-14,left?tx+14:tx-8);}
  juce::Path sticker;if(base==6){sticker.startNewSubPath(centre.x+radius*.45f,centre.y-radius-3);sticker.lineTo(centre.x+radius*.9f,centre.y-radius-6);sticker.lineTo(centre.x+radius,centre.y-radius+1);sticker.lineTo(centre.x+radius*.55f,centre.y-radius+4);}else{sticker.startNewSubPath(centre.x+radius+2,centre.y-4);sticker.lineTo(centre.x+radius+13,centre.y-4);sticker.lineTo(centre.x+radius+13,centre.y+3);sticker.lineTo(centre.x+radius+2,centre.y+3);sticker.lineTo(centre.x+radius-2,centre.y);}sticker.closeSubPath();g.setColour(juce::Colour(0xffed643c));g.fillPath(sticker);
 }else angle=juce::degreesToRadians(-135+proportion*270);
 auto circle=juce::Rectangle<float>(centre.x-radius,centre.y-radius,radius*2,radius*2);g.setColour(juce::Colours::black.withAlpha(.7f));g.fillEllipse(circle.translated(2,3));g.setGradientFill(juce::ColourGradient(juce::Colour(trim?0xff35515b:0xff646558),centre.x-radius,centre.y-radius,juce::Colour(0xff060904),centre.x+radius,centre.y+radius,false));g.fillEllipse(circle);
 if(!trim){g.setColour(juce::Colour(0xff090c06));for(int i=0;i<48;++i){float a=i*juce::MathConstants<float>::twoPi/48;g.drawLine(centre.x+std::sin(a)*(radius-3),centre.y+std::cos(a)*(radius-3),centre.x+std::sin(a)*radius,centre.y+std::cos(a)*radius,.7f);}juce::Path grip;grip.addRoundedRectangle(-5,-radius+3,10,radius*2-6,2);g.setGradientFill(juce::ColourGradient(juce::Colour(0xff8d8d7b),centre.x-4,centre.y,juce::Colour(0xff070a04),centre.x+5,centre.y,false));g.fillPath(grip,juce::AffineTransform::rotation(angle).translated(centre.x,centre.y));}
 const float len=radius*(trim?.8f:.88f);g.setColour(cream);g.drawLine(centre.x,centre.y,centre.x+std::sin(angle)*len,centre.y-std::cos(angle)*len,trim?2.f:3.f);
}
void TapeLook::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool hover,bool down){
 if((bool)b.getProperties()["matrix"]){g.setColour(juce::Colour(0xff34372d));g.fillRect(b.getLocalBounds());g.setColour(juce::Colour(0xff70715f));g.drawRect(b.getLocalBounds());return;}
 const auto a=b.getLocalBounds().toFloat();g.setColour(juce::Colour(0xff20231b));g.fillRoundedRectangle(a,2);g.setColour(juce::Colours::black);g.drawRoundedRectangle(a.reduced(.5f),2,1);
 const bool manual=b.getComponentID()=="autoCal"&&!b.getToggleState();const auto led=b.getToggleState()?green:(manual?juce::Colour(0xffe3482c):juce::Colour(0xff3b4130));g.setColour(led.withAlpha(.18f));g.fillEllipse(a.getCentreX()-5,13,10,10);g.setColour(led);g.fillEllipse(a.getCentreX()-2,16,4,4);
 auto key=a.reduced(6).withTop(25).withBottom(a.getBottom()-5);g.setColour(juce::Colours::black);g.fillRoundedRectangle(key.expanded(2),1);const bool red=b.getButtonText()=="SYNC";g.setGradientFill(juce::ColourGradient(red?juce::Colour(0xffed3043):juce::Colour(0xfff3dfa0),key.getTopLeft(),red?juce::Colour(0xffa50521):juce::Colour(0xffc8b878),key.getBottomLeft(),false));g.fillRoundedRectangle(key.translated(0,down?1:0),1);if(hover){g.setColour(juce::Colours::white.withAlpha(.08f));g.fillRect(a);}
}
void TapeLook::drawButtonText(juce::Graphics& g,juce::TextButton& b,bool,bool){bool matrix=(bool)b.getProperties()["matrix"];text(g,b.getButtonText(),matrix?b.getLocalBounds().toFloat():b.getLocalBounds().toFloat().withHeight(14),matrix?11.f:8.f,matrix?(b.getToggleState()?juce::Colour(0xffffdc65):juce::Colour(0xffb4b3a0)):cream);}
void TapeEditor::editParameter(const juce::String& id,float v){auto* p=processor.parameters.getParameter(id);p->beginChangeGesture();processor.setParameter(id,v);p->endChangeGesture();}
void TapeEditor::dial(const juce::String& id,juce::Rectangle<int> bounds,bool secondary,float base){Dial d;d.bounds=bounds;d.secondary=secondary;d.slider=std::make_unique<juce::Slider>();auto& s=*d.slider;s.setSliderStyle(juce::Slider::RotaryVerticalDrag);s.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);s.setName(id);s.setTooltip(processor.parameters.getParameter(id)->getName(64)+" · drag vertically / double-click to reset");s.setDoubleClickReturnValue(true,0);s.getProperties().set("trim",secondary);s.getProperties().set("base",base);s.setLookAndFeel(&look);addAndMakeVisible(s);d.attachment=std::make_unique<Attachment>(processor.parameters,id,s);dials.push_back(std::move(d));}
void TapeEditor::control(const juce::String& label,const juce::String& id,float setting,juce::Rectangle<int> bounds,bool toggle,bool secondary){Switch s;s.id=id;s.setting=setting;s.bounds=bounds;s.toggle=toggle;s.secondary=secondary;s.button=std::make_unique<juce::TextButton>(label);auto* b=s.button.get();b->setComponentID(id);b->setName(processor.parameters.getParameter(id)->getName(80)+" "+label);b->setLookAndFeel(&look);if(bounds.getHeight()<=23)b->getProperties().set("matrix",true);b->onClick=[this,id,setting,toggle]{if(id=="autoCal")processor.calibrate();else editParameter(id,toggle?1-processor.value(id):setting);};addAndMakeVisible(*b);switches.push_back(std::move(s));}
TapeEditor::TapeEditor(TapeProcessor& p):AudioProcessorEditor(p),processor(p){
 setOpaque(true);setResizable(false,false);setSize(720,710);
 texture=juce::Image(juce::Image::RGB,128,128,true);juce::Random random(224102);for(int y=0;y<128;++y)for(int x=0;x<128;++x){auto v=(juce::uint8)random.nextInt({17,39});texture.setPixelAt(x,y,juce::Colour(v,v,(juce::uint8)(v-2)));}
 for(int c=0;c<2;++c){int dx=c*354;auto suffix=juce::String(c==0?"L":"R");dial("record"+suffix,{184+dx,61,78,79},false,6);dial("reproduce"+suffix,{269+dx,61,78,79},false,8);control("IN / OUT","meter"+suffix,0,{191+dx,148,63,19},true);control("OFF / ON","autogain"+suffix,0,{277+dx,148,62,19},true);}
 control("CLOSE","open",0,{35,428,38,49});control("OPEN","open",1,{93,428,38,49});control("UNLINK","link",0,{35,510,38,49});control("LINK","link",1,{93,510,38,49});control("NAB","eq",0,{35,612,38,49});control("CCIR","eq",1,{93,612,38,49});
 control("POWER","power",0,{289,437,46,19},true);
 for(int i=0;i<4;++i)control(juce::StringArray{"30","15","7.5","3.75"}[i],"speed",(float)i,{389,417+i*15,40,14});
 for(int i=0;i<7;++i)control(juce::StringArray{"250","456","468","GP9","900","35-90","111"}[i],"formula",(float)i,{559,424+i*23,43,20});
 for(int i=0;i<4;++i)control(juce::StringArray{"+3","+6","+7.5","+9"}[i],"cal",(float)i,{606,424+i*23,40,20});
 for(int i=0;i<3;++i)control(juce::StringArray{"1\"","1/2\"","1/4\""}[i],"head",(float)i,{650,424+i*23,42,20});
 for(int i=0;i<4;++i)control(juce::StringArray{"SYNC","INPUT","REPRO","THRU"}[i],"path",(float)i,{565+(i%2)*66,558+(i/2)*56,42,49});
 const juce::StringArray ids{"crosstalk","wow","autoCal","noise","transformer"},labels{"CROSSTALK","WOW & FLUTTER","AUTO CAL","HISS & HUM","TRANSFORMER"};for(int i=0;i<5;++i)control(labels[i],ids[i],0,{183+i*71,501,64,48},true,true);
 for(int c=0;c<2;++c)for(int i=0;i<5;++i)dial(juce::StringArray{"recordHF","shelf","bias","reproLF","reproHF"}[i]+(c==0?"L":"R"),{185+c*180+i*34,599,32,34},true);
 for(int i=0;i<p.getNumPrograms();++i)presets.addItem(p.getProgramName(i),i+1);presets.setSelectedId(p.getCurrentProgram()+1,juce::dontSendNotification);presets.onChange=[this]{processor.setCurrentProgram(presets.getSelectedId()-1);};addAndMakeVisible(presets);addAndMakeVisible(save);addAndMakeVisible(load);
 save.onClick=[this]{chooser=std::make_unique<juce::FileChooser>("Save tape preset",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Tape.atrpreset"),"*.atrpreset");chooser->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting,[safe=juce::Component::SafePointer<TapeEditor>(this)](const juce::FileChooser& fc){if(safe&&fc.getResult()!=juce::File())safe->processor.savePreset(fc.getResult().withFileExtension("atrpreset"));});};
 load.onClick=[this]{chooser=std::make_unique<juce::FileChooser>("Load tape preset",juce::File(),"*.atrpreset");chooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[safe=juce::Component::SafePointer<TapeEditor>(this)](const juce::FileChooser& fc){if(safe&&fc.getResult().existsAsFile())safe->processor.loadPreset(fc.getResult());});};
 timerCallback();startTimerHz(30);
}
TapeEditor::~TapeEditor(){stopTimer();for(auto& d:dials)d.slider->setLookAndFeel(nullptr);for(auto& s:switches)s.button->setLookAndFeel(nullptr);}
void TapeEditor::resized(){for(auto& d:dials)d.slider->setBounds(d.bounds);for(auto& s:switches)s.button->setBounds(s.bounds);presets.setBounds(10,7,260,27);save.setBounds(280,7,65,27);load.setBounds(350,7,65,27);}
void TapeEditor::timerCallback(){
 const bool open=processor.value("open")>.5f;for(auto& d:dials)d.slider->setVisible(!d.secondary||open);
 for(auto& s:switches){s.button->setVisible(!s.secondary||open);s.button->setToggleState(std::abs(processor.value(s.id)-(s.toggle?1.f:s.setting))<.01f,juce::dontSendNotification);s.button->setEnabled(s.id!="eq"||processor.value("speed")!=0);}
 // Reflow only available formulas into four physical selector rows.
 std::vector<int> available=processor.value("speed")==0?std::vector<int>{1,2,3,4}:processor.value("speed")==1?std::vector<int>{0,1,2,3}:std::vector<int>{0,1,5,6};
 for(auto& s:switches)if(s.id=="formula"){auto found=std::find(available.begin(),available.end(),(int)s.setting);s.button->setVisible(found!=available.end());if(found!=available.end())s.button->setBounds(559,424+(int)std::distance(available.begin(),found)*23,43,20);}
 for(int c=0;c<2;++c){float target=processor.value(c==0?"meterL":"meterR")==0?processor.inputMeter[(size_t)c].load():processor.outputMeter[(size_t)c].load();meterLevel[(size_t)c]+=(target-meterLevel[(size_t)c])*(target>meterLevel[(size_t)c]?.55f:.12f);}
 if(processor.inputMeter[0].load()+processor.inputMeter[1].load()>.0001f)reelAngle+=.014f;
 presets.setSelectedId(processor.getCurrentProgram()+1,juce::dontSendNotification);repaint();
}
void TapeEditor::drawMeter(juce::Graphics& g,int c,float x,float y){
 juce::Rectangle<float> box(x,y,156,79);g.setColour(juce::Colours::black);g.fillRect(box.expanded(5));g.setGradientFill(juce::ColourGradient(juce::Colour(0xffffdf92),box.getCentre(),juce::Colour(0xffcc8434),box.getTopLeft(),true));g.fillRect(box);
 const float px=x+78,py=y+100;const int nums[]{-20,-10,-7,-5,-3,-1,0,1,2,3};
 juce::Path arc;for(int i=0;i<=40;++i){float a=juce::degreesToRadians(-51.f+i*2.55f);float ax=px+std::sin(a)*77,ay=py-std::cos(a)*77;if(i==0)arc.startNewSubPath(ax,ay);else arc.lineTo(ax,ay);}g.setColour(juce::Colour(0xff57331d));g.strokePath(arc,juce::PathStrokeType(.6f));
 for(int i=0;i<10;++i){float a=juce::degreesToRadians(-51.f+i*11.33f);auto col=i>=6?juce::Colour(0xffd12617):juce::Colour(0xff392416);g.setColour(col);g.drawLine(px+std::sin(a)*77,py-std::cos(a)*77,px+std::sin(a)*83,py-std::cos(a)*83,.8f);text(g,juce::String(nums[i]),{px+std::sin(a)*92-9,py-std::cos(a)*92-5,18,10},7,col);}
 juce::Path red;for(int i=0;i<=15;++i){float a=juce::degreesToRadians(17.f+i*2.2f);float xx=px+std::sin(a)*74,yy=py-std::cos(a)*74;if(i==0)red.startNewSubPath(xx,yy);else red.lineTo(xx,yy);}g.setColour(juce::Colour(0xffdf2e1b));g.strokePath(red,juce::PathStrokeType(2));
 text(g,"VU",{x+55,y+56,46,12},9,juce::Colour(0xff50351e));float vu=juce::Decibels::gainToDecibels(meterLevel[(size_t)c],-60.f)+12;float a=juce::degreesToRadians(juce::jlimit(-51.f,51.f,vu*2.4f+17));g.setColour(juce::Colour(0xff392719));g.drawLine(px,py,px+std::sin(a)*89,py-std::cos(a)*89,1);g.setColour(vu>3?juce::Colour(0xffff3920):juce::Colour(0xff7e321d));g.fillEllipse(x+144,y+7,4,4);
 text(g,c==0?"LEFT":"RIGHT",{x,y+87,156,15},10);g.setColour(juce::Colour(0xff070a05));g.fillEllipse(x+72,y+108,11,11);g.setColour(juce::Colour(0xff5a5c4f));g.drawLine(x+74,y+116,x+81,y+111,.7f);
}
void TapeEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour(0xff151912));text(g,"ATR-102 · TAPE STUDIO",{435,7,275,27},13);
 const juce::Rectangle<float> chassis(3,42,714,662);g.setColour(cream);g.fillRect(chassis);g.setColour(juce::Colour(0xff878979));g.drawRect(chassis,2);g.setTiledImageFill(texture,0,0,1);g.fillRect(9,49,702,648);
 // Cropped metal reels, behind the fixed meter bridge.
 {juce::Graphics::ScopedSaveState saved(g);g.reduceClipRegion(10,190,700,498);
 for(int c=0;c<2;++c){const float cx=c==0?100.f:620.f,cy=203.f,r=175;g.setColour(juce::Colours::black.withAlpha(.6f));g.fillEllipse(cx-r+3,cy-r+6,r*2,r*2);bool red=c==1&&processor.value("open")>.5f;g.setGradientFill(juce::ColourGradient(red?juce::Colour(0xffcc5058):juce::Colour(0xffdeded4),cx-r,cy-r,red?juce::Colour(0xff77212d):juce::Colour(0xff858b7e),cx+r,cy+r,false));g.fillEllipse(cx-r,cy-r,r*2,r*2);
  for(int i=0;i<3;++i){juce::Path hole;hole.addCentredArc(0,0,125,125,0,.2f,1.2f,true);hole.lineTo(std::sin(1.2f)*70,-std::cos(1.2f)*70);hole.addCentredArc(0,0,70,70,0,1.2f,.2f,false);hole.closeSubPath();g.setColour(juce::Colour(0xff10130e));g.fillPath(hole,juce::AffineTransform::rotation(reelAngle+i*juce::MathConstants<float>::twoPi/3).translated(cx,cy));}
  g.setColour(juce::Colour(0xff060903));g.fillEllipse(cx-43,cy-43,86,86);g.setColour(juce::Colour(0xff424839));g.drawEllipse(cx-37,cy-37,74,74,3);g.setColour(juce::Colour(0xffbfc1ae));g.fillEllipse(cx-7,cy-7,14,14);
 }
 }
 // Stereo meter bridge with thin white module dividers.
 g.setTiledImageFill(texture,0,0,1);g.fillRect(9,49,702,141);
 for(int c=0;c<2;++c){float dx=c*354.f;g.setColour(cream);g.drawRect(juce::Rectangle<float>(12+dx,52,344,134),.8f);g.drawLine(178+dx,53,178+dx,185,.6f);drawMeter(g,c,24+dx,66);text(g,"RECORD",{186+dx,56,74,16},10);text(g,"REPRODUCE",{266+dx,56,82,16},10);text(g,"METER",{191+dx,135,63,12},9);text(g,"AUTOGAIN",{277+dx,135,62,12},9);}
 // Brushed pale side consoles.
 for(int x:{17,548}){g.setGradientFill(juce::ColourGradient(juce::Colour(0xffe2dfce),(float)x,400,juce::Colour(0xffbcbda9),(float)x+145,670,false));g.fillRect(x,399,148,279);g.setColour(juce::Colours::black);g.drawRect(x,399,148,279,2);text(g,"AMPEX",{(float)x+17,404,114,19},23,ink);g.setColour(ink);g.drawRect(x+15,403,118,20);}
 g.setColour(ink);g.fillRect(25,585,132,14);text(g,"EMPHASIS EQ",{25,585,132,14},9);text(g,"TAPE",{559,520,43,15},9,ink);text(g,"CAL",{606,520,40,15},9,ink);text(g,"HEAD",{650,520,42,15},9,ink);
 // Tape transport and tension rollers.
 g.setTiledImageFill(texture,0,0,1);g.fillRoundedRectangle(181,425,359,65,26);g.setColour(juce::Colours::black);g.drawRoundedRectangle(181,425,359,65,26,3);
 for(float x:{223.f,492.f}){g.setColour(juce::Colour(0xff040701));g.fillEllipse(x-26,434,52,52);g.setColour(juce::Colour(0xffc5bd91));g.fillEllipse(x-18,440,36,36);g.setColour(juce::Colour(0xff909385));g.fillEllipse(x-9,451,18,18);g.setColour(juce::Colour(0xffdedcc6));g.drawEllipse(x-9,451,18,18,1);}
 if(processor.value("open")>.5f){g.setGradientFill(juce::ColourGradient(cream,180,500,juce::Colour(0xffa0a28e),535,640,false));g.fillRect(178,495,362,154);g.setColour(ink);g.drawRect(178,495,362,154,2);g.fillRect(178,558,362,5);g.fillRect(357,565,3,84);for(int c=0;c<2;++c){float x=184+c*180.f;text(g,c==0?"LEFT":"RIGHT",{x,569,165,13},10,ink);text(g,"RECORD EQ       BIAS       REPRO EQ",{x,585,165,12},8,ink);const juce::StringArray names{"HF","SHELF","BIAS","LF","HF"};for(int i=0;i<5;++i)text(g,names[i],{x+i*34,633,32,12},8,ink);}}
 else{
  juce::Path base;base.startNewSubPath(183,540);base.lineTo(534,540);base.lineTo(441,640);base.lineTo(285,640);base.closeSubPath();g.setColour(ink.darker());g.fillPath(base);
  g.setTiledImageFill(texture,0,0,1);g.fillRoundedRectangle(178,487,364,114,42);g.setColour(juce::Colours::black);g.drawRoundedRectangle(178,487,364,114,42,3);
  g.setColour(cream.withAlpha(.12f));for(int i=0;i<4;++i)g.drawLine(285,537+i*6.f,400,534+i*6.f,.6f);
  g.setGradientFill(juce::ColourGradient(juce::Colour(0xffe4e2d6),462,500,juce::Colour(0xff7b8275),529,575,false));g.fillEllipse(450,497,84,84);g.setColour(juce::Colour(0xff090d06));g.drawEllipse(450,497,84,84,6);g.setColour(juce::Colour(0xffc8cabc));g.fillEllipse(478,525,28,28);g.setColour(juce::Colour(0xff8f9686));g.drawEllipse(478,525,28,28,1);
  juce::Path lip;lip.startNewSubPath(291,644);lip.lineTo(431,644);lip.lineTo(415,660);lip.lineTo(307,660);lip.closeSubPath();g.setColour(cream);g.fillPath(lip);
  juce::Path guide;guide.addRectangle(-18,-27,36,54);g.setGradientFill(juce::ColourGradient(juce::Colour(0xffe5e4d6),480,624,juce::Colour(0xff6e7666),520,650,false));g.fillPath(guide,juce::AffineTransform::rotation(-.48f).translated(508,637));
 }
 text(g,"INDEPENDENT TAPE MODEL · −12 dBFS = 0 VU",{175,681,370,17},8);
}
