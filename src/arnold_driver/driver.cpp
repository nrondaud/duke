#include <ai.h>
#include "DriverLocalData.hpp"

AI_DRIVER_NODE_EXPORT_METHODS(DriverDukeMtd);

namespace
{
  template <class T>
  void setDescription(T& msg, const char* aovname, int pixeltype, int w, int h)
  {
    duke::protocol::Description* description = msg.mutable_description();
    description->set_width(w);
    description->set_height(h);
    description->set_type(duke::protocol::Description_DataType_FLOAT);
    switch(pixeltype)
    {
      case AI_TYPE_RGB :
      description->add_channel("R");
      description->add_channel("G");
      description->add_channel("B");
      break;
      case AI_TYPE_RGBA :
      description->add_channel("R");
      description->add_channel("G");
      description->add_channel("B");
      description->add_channel("A");
      break;
      case AI_TYPE_FLOAT :
      description->add_channel(aovname);
      break;
    }
  }
}

node_parameters
{
}

node_initialize
{
  AiDriverInitialize(node, false, new DriverLocalData);
}

node_update
{
}

driver_supports_pixel_type
{
  switch(pixel_type) {
    case AI_TYPE_RGB:
    case AI_TYPE_RGBA:
    case AI_TYPE_FLOAT :
    return true;
    default:
    return false;
  }
}

driver_extension
{
 return NULL;
}

driver_open // RENDER START
{
  DriverLocalData* driver = (DriverLocalData*)AiDriverGetLocalData(node);
  if(!driver) {
    AiMsgError("[driver_duke] can't retrieve local data");
    return;
  }
  AtNode *options = AiUniverseGetOptions();
  if(!options) {
    AiMsgError("[driver_duke] can't retrieve global options");
    return;
  }
  int pixel_type;
  const char* name = 0;
  if(AiOutputIteratorGetNext(iterator, &name, &pixel_type, 0)) // first AOV
  {
    using namespace duke::protocol;
    Render render;
    setDescription(render, name, pixel_type, AiNodeGetInt(options, "xres"), AiNodeGetInt(options, "yres"));
    render.set_frame(0);
    render.set_camera("cameraname");
    driver->send(render);
  }
}

driver_prepare_bucket // BUCKET START
{
}

driver_write_bucket // BUCKET END
{
  DriverLocalData* driver = (DriverLocalData*)AiDriverGetLocalData(node);
  if(!driver) {
    AiMsgError("[driver_duke] can't retrieve local data");
    return;
  }
  int pixel_type;
  const char* name = 0;
  const void* bucket_data;
  if(AiOutputIteratorGetNext(iterator, &name, &pixel_type, &bucket_data)) // first AOV
  {
    using namespace duke::protocol;
    Bucket bucket;
    setDescription(bucket, name, pixel_type, bucket_size_x, bucket_size_y);
    if(bucket.description().channel_size()==0)
      return;
    size_t data_size = bucket_size_x * bucket_size_y * bucket.description().channel_size() * sizeof(float);
    AtRGBA* data = new AtRGBA[data_size];
    if(!data)
      return;
    AtRGBA* pDest = data;
    for(int y=(bucket_size_y-1); y>=0; y--) {
      AtRGBA* pSrc = ((AtRGBA*)bucket_data)+(y*bucket_size_x);
      for(int x=0; x<bucket_size_x; x++, pDest++, pSrc++) {
        pDest->r = pSrc->r;
        pDest->g = pSrc->g;
        pDest->b = pSrc->b;
        pDest->a = pSrc->a;
      }
    }
    bucket.set_x(bucket_xo);
    bucket.set_y(bucket_yo);
    bucket.set_data(data, data_size);
    driver->send(bucket);
    delete [] data;
  }
}

driver_close // RENDER END
{
}

node_finish
{
  DriverLocalData* driver = (DriverLocalData*)AiDriverGetLocalData(node);
  if(driver)
    delete driver;
  AiDriverDestroy(node);
}

node_loader
{
  if(i>0)
    return false;
  node->name = "driver_duke";
  node->methods = DriverDukeMtd;
  node->output_type = AI_TYPE_NONE;
  node->node_type = AI_NODE_DRIVER;
  strcpy(node->version, AI_VERSION);
  return true;
}
