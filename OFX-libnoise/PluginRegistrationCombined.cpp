#include "Perlin.h"
#include "Billow.h"

namespace OFX
{
    namespace Plugin
    {
        void getPluginIDs(OFX::PluginFactoryArray &ids)
        {
            getPerlinPluginID(ids);
            getBillowPluginID(ids);
        }
    }
}
