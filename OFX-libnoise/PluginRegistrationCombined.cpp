#include "Perlin.h"
#include "Billow.h"
#include "RidgedMulti.h"

namespace OFX
{
    namespace Plugin
    {
        void getPluginIDs(OFX::PluginFactoryArray &ids)
        {
            getPerlinPluginID(ids);
            getBillowPluginID(ids);
            getRidgedMultiPluginID(ids);
        }
    }
}
