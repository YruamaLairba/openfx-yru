/*
OFX FractalNoise, a plugin using libnoise librairy. The Invert.cpp source was used as a template.

Copyleft : GPL Version 3
Author Amaury ABRIAL yruama_lairba@hotmail.com

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifdef _WINDOWS
#include <windows.h>
#endif

//include libnoise lib
#include "noise/noise.h"

#include <stdio.h>
#include "ofxsImageEffect.h"
#include "ofxsMultiThread.h"

#include "../include/ofxsProcessing.H"



// Base class for the RGBA and the Alpha processor
class FractalNoiseBase : public OFX::ImageProcessor {
protected :
  OFX::Image *_srcImg;
  
  //position of noise
  float posX, posY;
  
  //evolution 
  float posZ;
  
  //freqency X and Y
  float freqX, freqY;
  /*
  //lacunarity
  float lacunarity;
  
  //number of Octave
  int nbOctave;
  
  //persistence
  float persistence;
  */
  
  //noise generator
  noise::module::Perlin noiseGenerator;
public :
  /** @brief no arg ctor */
  FractalNoiseBase(OFX::ImageEffect &instance)
    : OFX::ImageProcessor(instance)
    , _srcImg(0)
    , posX(0)
    , posY(0)
    , posZ(0)
    , freqX(0)
    , freqY(0)
    /*
    , lacunarity(0)
    , nbOctave(0)
    , persistence(0)
    */
  {        
  }
  //setters
  void setPosX(float value){posX = value ;}
  void setPosY(float value){posY = value ;}
  void setPosZ(float value){posZ = value ;}
  void setFreqX(float value){freqX = value ;}
  void setFreqY(float value){freqY = value ;}
  
  void setLacunarity(double value){noiseGenerator.SetLacunarity(value);}
  void setNbOctave(int value){noiseGenerator.SetOctaveCount(value);}
  void setPersistence(double value){noiseGenerator.SetPersistence(value);}
  
  
  
  /** @brief set the src image */
  void setSrcImg(OFX::Image *v) {_srcImg = v;}
};

// template to do the RGBA processing
template <class PIX, int nComponents, int max>
class ImageInverter : public FractalNoiseBase {
public :
  // ctor
  ImageInverter(OFX::ImageEffect &instance) 
    : FractalNoiseBase(instance)
  {}

  // and do some processing
  void multiThreadProcessImages(OfxRectI procWindow)
  {
    //noise::module::Perlin noiseGenerator;
    
    for(int y = procWindow.y1; y < procWindow.y2; y++) {
      if(_effect.abort()) break;

      PIX *dstPix = (PIX *) _dstImg->getPixelAddress(procWindow.x1, y);

      for(int x = procWindow.x1; x < procWindow.x2; x++) {

        PIX *srcPix = (PIX *)  (_srcImg ? _srcImg->getPixelAddress(x, y) : 0);

        // do we have a source image to scale up
        if(srcPix) {
          for(int c = 0; c < nComponents; c++) {
            dstPix[c]=(1.0+this->noiseGenerator.GetValue((x-posX)*freqX,(y-posY)*freqY,posZ))/2.0;
            //dstPix[c] = max - srcPix[c];
          }
        }
        else {
          // no src pixel here, be black and transparent
          for(int c = 0; c < nComponents; c++) {
            dstPix[c] = 0;
          }
        }

        // increment the dst pixel
        dstPix += nComponents;
      }
    }
  }
};

////////////////////////////////////////////////////////////////////////////////
/** @brief The plugin that does our work */
class FractalNoisePlugin : public OFX::ImageEffect {
protected :
  // do not need to delete these, the ImageEffect is managing them for us
  OFX::Clip *dstClip_;
  OFX::Clip *srcClip_;
  
  OFX::Double2DParam *position_;
  OFX::DoubleParam *evolution_;
  OFX::Double2DParam *frequency_;
  OFX::DoubleParam *lacunarity_;
  OFX::IntParam *nbOctave_;
  OFX::DoubleParam *persistence_;
  
  
  //the noise generator
  //noise::module::Perlin noiseGenerator;

public :
  /** @brief ctor */
  FractalNoisePlugin(OfxImageEffectHandle handle)
    : ImageEffect(handle)
    , dstClip_(0)
    , srcClip_(0)
    , position_(0)
    , evolution_(0)
    , frequency_(0)
    , lacunarity_(0)
    , nbOctave_(0)
    , persistence_(0)
  {
    dstClip_ = fetchClip(kOfxImageEffectOutputClipName);
    srcClip_ = fetchClip(kOfxImageEffectSimpleSourceClipName);
    position_ = fetchDouble2DParam("position");
    evolution_ = fetchDoubleParam("evolution");
    frequency_ = fetchDouble2DParam("frequency");
    
    lacunarity_ = fetchDoubleParam("lacunarity");
    nbOctave_ = fetchIntParam("nbOctave");
    persistence_ = fetchDoubleParam("persistence");
    
  }

  /* Override the render */
  virtual void render(const OFX::RenderArguments &args);

  /* set up and run a processor */
  void setupAndProcess(FractalNoiseBase &, const OFX::RenderArguments &args);
};


////////////////////////////////////////////////////////////////////////////////
/** @brief render for the filter */

////////////////////////////////////////////////////////////////////////////////
// basic plugin render function, just a skelington to instantiate templates from


/* set up and run a processor */
void
FractalNoisePlugin::setupAndProcess(FractalNoiseBase &processor, const OFX::RenderArguments &args)
{
  // get a dst image
  std::auto_ptr<OFX::Image> dst(dstClip_->fetchImage(args.time));
  OFX::BitDepthEnum dstBitDepth       = dst->getPixelDepth();
  OFX::PixelComponentEnum dstComponents  = dst->getPixelComponents();

  // fetch main input image
  std::auto_ptr<OFX::Image> src(srcClip_->fetchImage(args.time));

  // make sure bit depths are sane
  if(src.get()) {
    OFX::BitDepthEnum    srcBitDepth      = src->getPixelDepth();
    OFX::PixelComponentEnum srcComponents = src->getPixelComponents();

    // see if they have the same depths and bytes and all
    if(srcBitDepth != dstBitDepth || srcComponents != dstComponents)
      throw int(1); // HACK!! need to throw an sensible exception here!
  }
  
  // set the images
  processor.setDstImg(dst.get());
  processor.setSrcImg(src.get());
  
  // set position
  double posX;
  double posY;
  position_->getValueAtTime(args.time, posX, posY);
  processor.setPosX(posX);
  processor.setPosY(posY);
  
  //set evolution
  double posZ;
  evolution_->getValueAtTime(args.time, posZ);
  processor.setPosZ(posZ);
  
  //set frequency
  double freqX, freqY;
  frequency_->getValueAtTime(args.time, freqX, freqY);
  processor.setFreqX(freqX);
  processor.setFreqY(freqY);

  //set lacunarity
  double lacunarity;
  lacunarity_->getValueAtTime(args.time, lacunarity);
  processor.setLacunarity(lacunarity);
  
  //set number of octave
  int nbOctave;
  nbOctave_->getValueAtTime(args.time, nbOctave);
  processor.setNbOctave(nbOctave);
  
  //set persistence
  double persistence;
  persistence_->getValueAtTime(args.time, persistence);
  processor.setPersistence(persistence);
  
  // set the render window
  processor.setRenderWindow(args.renderWindow);

  // Call the base class process member, this will call the derived templated process code
  processor.process();
}

// the overridden render function
void
FractalNoisePlugin::render(const OFX::RenderArguments &args)
{
  // instantiate the render code based on the pixel depth of the dst clip
  OFX::BitDepthEnum       dstBitDepth    = dstClip_->getPixelDepth();
  OFX::PixelComponentEnum dstComponents  = dstClip_->getPixelComponents();

  // do the rendering
  if(dstComponents == OFX::ePixelComponentRGBA) {
    switch(dstBitDepth) {
/*
case OFX::eBitDepthUByte : {      
  ImageInverter<unsigned char, 4, 255> fred(*this);
  setupAndProcess(fred, args);
                           }
                           break;

case OFX::eBitDepthUShort : {
  ImageInverter<unsigned short, 4, 65535> fred(*this);
  setupAndProcess(fred, args);
                            }                          
                            break;
*/
case OFX::eBitDepthFloat : {
  ImageInverter<float, 4, 1> fred(*this);
  setupAndProcess(fred, args);
                           }
                           break;
default :
  OFX::throwSuiteStatusException(kOfxStatErrUnsupported);
    }
  }
  else {
    switch(dstBitDepth) {
case OFX::eBitDepthUByte : {
  ImageInverter<unsigned char, 1, 255> fred(*this);
  setupAndProcess(fred, args);
                           }
                           break;

case OFX::eBitDepthUShort : {
  ImageInverter<unsigned short, 1, 65535> fred(*this);
  setupAndProcess(fred, args);
                            }                          
                            break;

case OFX::eBitDepthFloat : {
  ImageInverter<float, 1, 1> fred(*this);
  setupAndProcess(fred, args);
                           }                          
                           break;
default :
  OFX::throwSuiteStatusException(kOfxStatErrUnsupported);
    }
  } 
}

mDeclarePluginFactory(FractalNoisePluginFactory, {}, {});

using namespace OFX;
void FractalNoisePluginFactory::describe(OFX::ImageEffectDescriptor &desc)
{
  // basic labels
  desc.setLabels("FractalNoise", "FractalNoise", "FractalNoise");
  desc.setPluginGrouping("Yru");
  desc.setPluginDescription("FractalNoise pixels value of the image");

  // add the supported contexts, only filter at the moment
  desc.addSupportedContext(eContextFilter);
  desc.addSupportedContext(eContextGeneral);
  desc.addSupportedContext(eContextPaint);

  // add supported pixel depths
  //desc.addSupportedBitDepth(eBitDepthUByte);
  //desc.addSupportedBitDepth(eBitDepthUShort);
  desc.addSupportedBitDepth(eBitDepthFloat);

  // set a few flags
  desc.setSingleInstance(false);
  desc.setHostFrameThreading(false);
  desc.setSupportsMultiResolution(true);
  desc.setSupportsTiles(true);
  desc.setTemporalClipAccess(false);
  desc.setRenderTwiceAlways(false);
  desc.setSupportsMultipleClipPARs(false);

}

void FractalNoisePluginFactory::describeInContext(OFX::ImageEffectDescriptor &desc, OFX::ContextEnum context)
{
  // Source clip only in the filter context
  // create the mandated source clip
  ClipDescriptor *srcClip = desc.defineClip(kOfxImageEffectSimpleSourceClipName);
  srcClip->addSupportedComponent(ePixelComponentRGBA);
  srcClip->addSupportedComponent(ePixelComponentRGB);
  srcClip->addSupportedComponent(ePixelComponentAlpha);
  srcClip->setTemporalClipAccess(false);
  srcClip->setSupportsTiles(true);
  srcClip->setIsMask(false);

  // create the mandated output clip
  ClipDescriptor *dstClip = desc.defineClip(kOfxImageEffectOutputClipName);
  dstClip->addSupportedComponent(ePixelComponentRGBA);
  dstClip->addSupportedComponent(ePixelComponentRGB);
  dstClip->addSupportedComponent(ePixelComponentAlpha);
  dstClip->setSupportsTiles(true);
  
  
  if (context == eContextGeneral || context == eContextPaint) 
  {
    ClipDescriptor *maskClip = context == eContextGeneral ? desc.defineClip("Mask") : desc.defineClip("Brush");
    maskClip->addSupportedComponent(ePixelComponentAlpha);
    maskClip->setTemporalClipAccess(false);
    if (context == eContextGeneral) 
	{
      maskClip->setOptional(true);
    }
    maskClip->setSupportsTiles(true);
    maskClip->setIsMask(true);
  }

  PageParamDescriptor *page = desc.definePageParam("Controls");

    //Position
    {
        OFX::Double2DParamDescriptor* param = desc.defineDouble2DParam("position");
        param->setLabel("Position");
        param->setHint("Position");
        param->setDefault(0.0,0.0);
        if (page) {
            page->addChild(*param);
        }
    }
    //Evolution
    {
        OFX::DoubleParamDescriptor* param = desc.defineDoubleParam("evolution");
        param->setLabel("Evolution");
        param->setHint("Evolution");
        param->setDefault(0.0);
        if (page) {
            page->addChild(*param);
        }
    }
    //Frequency
    {
        OFX::Double2DParamDescriptor* param = desc.defineDouble2DParam("frequency");
        param->setLabel("Frequency");
        param->setHint("Frequency");
        param->setDefault(0.05,0.05);
        if (page) {
            page->addChild(*param);
        }
    }
    //Lacunarity
    {
        OFX::DoubleParamDescriptor* param = desc.defineDoubleParam("lacunarity");
        param->setLabel("Lacunarity");
        param->setHint("Lacunarity");
        param->setDefault(noise::module::DEFAULT_PERLIN_LACUNARITY);
        if (page) {
            page->addChild(*param);
        }
    }
    //Number of Octave
    {
        OFX::IntParamDescriptor* param = desc.defineIntParam("nbOctave");
        param->setLabel("Number of octave");
        param->setHint("Number of octave");
        param->setDefault(noise::module::DEFAULT_PERLIN_OCTAVE_COUNT);
        if (page) {
            page->addChild(*param);
        }
    }
    //persistence
    {
        OFX::DoubleParamDescriptor* param = desc.defineDoubleParam("persistence");
        param->setLabel("persistence");
        param->setHint("persistence");
        param->setDefault(noise::module::DEFAULT_PERLIN_PERSISTENCE);
        if (page) {
            page->addChild(*param);
        }
    }
    
    {
        OFX::BooleanParamDescriptor* param = desc.defineBooleanParam("R");
        param->setLabel("Rouge");
        param->setHint("Traiter le canal");
        param->setDefault(true);
        //param->setLayoutHint(eLayoutHintNoNewLine);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        OFX::BooleanParamDescriptor* param = desc.defineBooleanParam("G");
        param->setLabel("Vert");
        param->setHint("Traiter le canal");
        param->setDefault(true);
        //param->setLayoutHint(eLayoutHintNoNewLine);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        OFX::BooleanParamDescriptor* param = desc.defineBooleanParam("B");
        param->setLabel("Bleu");
        param->setHint("Traiter le canal");
        param->setDefault(true);
        //param->setLayoutHint(eLayoutHintNoNewLine);
        if (page) {
            page->addChild(*param);
        }
    }
    {
        OFX::BooleanParamDescriptor* param = desc.defineBooleanParam("A");
        param->setLabel("Alpha");
        param->setHint("Traiter le canal");
        param->setDefault(true);
        if (page) {
            page->addChild(*param);
        }
    }
	
}

OFX::ImageEffect* FractalNoisePluginFactory::createInstance(OfxImageEffectHandle handle, OFX::ContextEnum /*context*/)
{
  return new FractalNoisePlugin(handle);
}

namespace OFX 
{
  namespace Plugin 
  {  
    void getPluginIDs(OFX::PluginFactoryArray &ids)
    {
      static FractalNoisePluginFactory p("FractalNoise", 1, 0);
      ids.push_back(&p);
    }
  }
}
