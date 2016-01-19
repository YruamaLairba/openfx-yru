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
class FractalNoiseProcessorBase : public OFX::ImageProcessor {
protected :
  OFX::Image *_srcImg;
  
  //position of noise
  float posX, posY;
  
  //evolution 
  float posZ;
  
  //freqency X and Y
  float freqX, freqY;
  
  //amplitude
  float amplitude;
  
  //offset
  float offset;
  
  //noise generator
  noise::module::Perlin noiseGenerator;
public :
  /** @brief no arg ctor */
  FractalNoiseProcessorBase(OFX::ImageEffect &instance)
    : OFX::ImageProcessor(instance)
    , _srcImg(0)
    , posX(0)
    , posY(0)
    , posZ(0)
    , freqX(0)
    , freqY(0)
    , amplitude(0)
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
  void setAmplitude(float value){amplitude = value ;}
  void setOffset(float value){offset = value ;}
  
  void setLacunarity(double value){noiseGenerator.SetLacunarity(value);}
  void setNbOctave(int value){noiseGenerator.SetOctaveCount(value);}
  void setPersistence(double value){noiseGenerator.SetPersistence(value);}
  
  
  
  /** @brief set the src image */
  void setSrcImg(OFX::Image *v) {_srcImg = v;}
};

// template to do the RGBA processing
template <class PIX, int nComponents, int max>
class FractalNoiseProcessor : public FractalNoiseProcessorBase {
public :
  // ctor
  FractalNoiseProcessor(OFX::ImageEffect &instance) 
    : FractalNoiseProcessorBase(instance)
  {}

  // and do some processing
  void multiThreadProcessImages(OfxRectI procWindow)
  {
    //Renderscale management
    float ScaledFreqX = freqX /(_dstImg->getRenderScale().x);
    float ScaledFreqY = freqY /(_dstImg->getRenderScale().y);
    float ScaledPosX = posX *(_dstImg->getRenderScale().x);
    float ScaledPosY = posY *(_dstImg->getRenderScale().y);
    //go trough every lines
    for(int y = procWindow.y1; y < procWindow.y2; y++) {
      if(_effect.abort()) break;
      PIX *dstPix = (PIX *) _dstImg->getPixelAddress(procWindow.x1, y);
      //go through every pixels in the line
      for(int x = procWindow.x1; x < procWindow.x2; x++) {
        float valuef=max*(offset+amplitude*this->noiseGenerator.GetValue((x-ScaledPosX)*ScaledFreqX,(y-ScaledPosY)*ScaledFreqY,posZ));
		// max = 1 implies Float, don't clamp 
		if (max == 1){
          //go through every component in a pixel
          for(int c = 0; c < nComponents; c++) {
            //copy same value in each component
            dstPix[c]=(PIX)valuef;
          }
		}
		// clamp needed whith integer 
		else{
		  //go through every component in a pixel
          for(int c = 0; c < nComponents; c++) {
            //ceil
			if (valuef > max){
			  dstPix[c]=(PIX) max;
			}
			//floor
			else if (valuef < 0){
			  dstPix[c]=(PIX) 0;
			}
			else {
			  dstPix[c]=(PIX)valuef;
			}
          }
		}
        // increment to point the next dst pixel
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
  OFX::DoubleParam *amplitude_;
  OFX::DoubleParam *offset_;
  
  
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
	, amplitude_(0)
    , offset_(0)
  {
    dstClip_ = fetchClip(kOfxImageEffectOutputClipName);
    srcClip_ = fetchClip(kOfxImageEffectSimpleSourceClipName);
    position_ = fetchDouble2DParam("Position");
    evolution_ = fetchDoubleParam("Evolution");
    frequency_ = fetchDouble2DParam("Frequency");
    
    lacunarity_ = fetchDoubleParam("Lacunarity");
    nbOctave_ = fetchIntParam("Number of Octaves");
    persistence_ = fetchDoubleParam("Persistence");
	amplitude_ = fetchDoubleParam("Amplitude");
    offset_ = fetchDoubleParam("Offset");
    
  }

private:
  /* Internal render, used for code factorization */
  template <int nComponents>
  void renderInternal(const OFX::RenderArguments &args, OFX::BitDepthEnum dstBitDepth);
 
  /* Override the render */
  virtual void render(const OFX::RenderArguments &args);

  /* set up and run a processor */
  void setupAndProcess(FractalNoiseProcessorBase &, const OFX::RenderArguments &args);
};


////////////////////////////////////////////////////////////////////////////////
/** @brief render for the filter */

////////////////////////////////////////////////////////////////////////////////
// basic plugin render function, just a skelington to instantiate templates from


/* set up and run a processor */
void
FractalNoisePlugin::setupAndProcess(FractalNoiseProcessorBase &processor, const OFX::RenderArguments &args)
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
  
  //set amplitude
  double amplitude;
  amplitude_->getValueAtTime(args.time, amplitude);
  processor.setAmplitude(amplitude);
  
  //set offset
  double offset;
  offset_->getValueAtTime(args.time, offset);
  processor.setOffset(offset);
  
  // set the render window
  processor.setRenderWindow(args.renderWindow);

  // Call the base class process member, this will call the derived templated process code
  processor.process();
}

// the internal render function
template <int nComponents>
void
FractalNoisePlugin::renderInternal(const OFX::RenderArguments &args, OFX::BitDepthEnum dstBitDepth)
{
    switch (dstBitDepth) {
        case OFX::eBitDepthUByte: {
            FractalNoiseProcessor<unsigned char, nComponents, 255> fred(*this);
            setupAndProcess(fred, args);
            break;
        }
        case OFX::eBitDepthUShort: {
            FractalNoiseProcessor<unsigned short, nComponents, 65535> fred(*this);
            setupAndProcess(fred, args);
            break;
        }
        case OFX::eBitDepthFloat: {
            FractalNoiseProcessor<float, nComponents, 1> fred(*this);
            setupAndProcess(fred, args);
            break;
        }
        default:
		    setPersistentMessage(OFX::Message::eMessageError, "", "Unsupported bit depth");
            OFX::throwSuiteStatusException(kOfxStatErrUnsupported);
    }
}

// the overridden render function
void
FractalNoisePlugin::render(const OFX::RenderArguments &args)
{
  // instantiate the render code based on the pixel depth of the dst clip
  OFX::BitDepthEnum       dstBitDepth    = dstClip_->getPixelDepth();
  OFX::PixelComponentEnum dstComponents  = dstClip_->getPixelComponents();

    // do the rendering
	if (dstComponents == OFX::ePixelComponentRGBA) {
        renderInternal<4>(args, dstBitDepth);
    } 
	else if (dstComponents == OFX::ePixelComponentRGB) {
        renderInternal<3>(args, dstBitDepth);
    } 
	/*else if (dstComponents == OFX::ePixelComponentXY) {
        renderInternal<2>(args, dstBitDepth);
    }*/
	else if (dstComponents == OFX::ePixelComponentAlpha) {
        renderInternal<1>(args, dstBitDepth);
    }
	else {
		setPersistentMessage(OFX::Message::eMessageError, "", "Wrong number of components");
		OFX::throwSuiteStatusException(kOfxStatErrUnsupported);
	}
}


using namespace OFX;
mDeclarePluginFactory(FractalNoisePluginFactory, {}, {});

void FractalNoisePluginFactory::describe(OFX::ImageEffectDescriptor &desc)
{
  // basic labels
  desc.setLabels("FractalNoise", "FractalNoise", "FractalNoise");
  desc.setPluginGrouping("Yru");
  desc.setPluginDescription("FractalNoise generator");

  // add the supported contexts, only filter at the moment
  desc.addSupportedContext(eContextFilter);
  desc.addSupportedContext(eContextGeneral);
  desc.addSupportedContext(eContextPaint);

  // add supported pixel depths
  desc.addSupportedBitDepth(eBitDepthUByte);
  desc.addSupportedBitDepth(eBitDepthUShort);
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

void FractalNoisePluginFactory::describeInContext(OFX::ImageEffectDescriptor &desc , OFX::ContextEnum context)
{
  //Silent "unused parameter" warning on the context variable
  (void)context;
  // Source clip only in the filter context
  // create the mandated source clip
  ClipDescriptor *srcClip = desc.defineClip(kOfxImageEffectSimpleSourceClipName);
  srcClip->addSupportedComponent(ePixelComponentRGBA);
  srcClip->addSupportedComponent(ePixelComponentRGB);
  srcClip->addSupportedComponent(ePixelComponentAlpha);
  srcClip->setTemporalClipAccess(false);
  srcClip->setSupportsTiles(true);
  srcClip->setIsMask(false);
  srcClip->setOptional(true);

  // create the mandated output clip
  ClipDescriptor *dstClip = desc.defineClip(kOfxImageEffectOutputClipName);
  dstClip->addSupportedComponent(ePixelComponentRGBA);
  dstClip->addSupportedComponent(ePixelComponentRGB);
  dstClip->addSupportedComponent(ePixelComponentAlpha);
  dstClip->setSupportsTiles(true);
  
  PageParamDescriptor *page = desc.definePageParam("Controls");

    //Position
    {
        OFX::Double2DParamDescriptor* param = desc.defineDouble2DParam("Position");
        param->setLabel("Position");
        param->setHint("Change the center of the noise. This center can be visualized when changing the frequency.");
        param->setDefaultCoordinateSystem(eCoordinatesNormalised);
        param->setDefault(0.5, 0.5);
        param->setDisplayRange(-10000,-10000,10000,10000);
        
        if (page) {
            page->addChild(*param);
        }
    }
    //Evolution
    {
        OFX::DoubleParamDescriptor* param = desc.defineDoubleParam("Evolution");
        param->setLabel("Evolution");
        param->setHint("Slowly Animate this parameter give a morphing effect to the noise.");
        param->setDefault(0.0);
        param->setDisplayRange(0,10);
        if (page) {
            page->addChild(*param);
        }
    }
    //Frequency
    {
        OFX::Double2DParamDescriptor* param = desc.defineDouble2DParam("Frequency");
        param->setLabel("Frequency");
        param->setHint("Set the base frequency in pixel.");
        param->setDefault(0.05,0.05);
        param->setDisplayRange(0,0,0.5,0.5);
        if (page) {
            page->addChild(*param);
        }
    }
    //Lacunarity
    {
        OFX::DoubleParamDescriptor* param = desc.defineDoubleParam("Lacunarity");
        param->setLabel("Lacunarity");
        param->setHint("Set frequency scale between Octaves.");
        param->setDefault(noise::module::DEFAULT_PERLIN_LACUNARITY);
        param->setDisplayRange(1,5);
        if (page) {
            page->addChild(*param);
        }
    }
    //Number of Octave
    {
        OFX::IntParamDescriptor* param = desc.defineIntParam("Number of Octaves");
        param->setLabel("nbOctaves");
        param->setHint("Just the number of octaves to combine.");
        param->setDefault(noise::module::DEFAULT_PERLIN_OCTAVE_COUNT);
        param->setDisplayRange(1,10);
        if (page) {
            page->addChild(*param);
        }
    }
    //persistence
    {
        OFX::DoubleParamDescriptor* param = desc.defineDoubleParam("Persistence");
        param->setLabel("Persistence");
        param->setHint("Set the amplitude scale between Octaves");
        param->setDefault(noise::module::DEFAULT_PERLIN_PERSISTENCE);
        param->setDisplayRange(0,1);
        if (page) {
            page->addChild(*param);
        }
    }
    //amplitude
    {
        OFX::DoubleParamDescriptor* param = desc.defineDoubleParam("Amplitude");
        param->setLabel("Amplitude");
        param->setHint("Set the amplitude of the first octave.");
        param->setDefault(0.5);
        param->setDisplayRange(0,1);
        if (page) {
            page->addChild(*param);
        }
    }
    //offset
    {
        OFX::DoubleParamDescriptor* param = desc.defineDoubleParam("Offset");
        param->setLabel("Offset");
        param->setHint("Set the average value of the noise. Lower value mean Darker noise, Higher value mean lighter noise");
        param->setDefault(0.5);
        param->setDisplayRange(0,1);
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
