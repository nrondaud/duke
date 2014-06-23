#include "duke/gl/GL.hpp"
#include "duke/gl/GlUtils.hpp"
#include "duke/io/IO.hpp"
#include "duke/protocol/pbhelpers.hpp"

namespace duke {

using namespace protocol;

class PBImageReader : public IImageReader {
  FILE* m_pFile;
  MessageHolder m_holder;

 public:
  PBImageReader(const char* filename) : m_pFile(fopen(filename, "rb")) {
    if (!m_pFile) {
      m_Error = "Unable to open";
      return;
    }
    if (!m_holder.ParseFromFileDescriptor(fileno(m_pFile)) || !m_holder.IsInitialized()) {
      m_Error = std::string("Unable to parse protobuf object from ")+filename;
      fclose(m_pFile);
      return;
    }
    if(isType<Bucket>(m_holder)) {
      const Bucket & b = unpackTo<Bucket>(m_holder);
      ImageDescription description;
      description.channels = getChannels(GL_RGBA32F);
      description.width = b.description().width();
      description.height = b.description().height();
      m_Description.subimages.push_back(std::move(description));
      m_Description.frames = 1;
    } else if(isType<Render>(m_holder)) { 
      const Render & r = unpackTo<Render>(m_holder);
      ImageDescription description;
      description.channels = getChannels(GL_RGBA32F);
      description.width = r.description().width();
      description.height = r.description().height();
      m_Description.subimages.push_back(std::move(description));
      m_Description.frames = 1;
    }
    fclose(m_pFile);
  }

  ~PBImageReader() {
    if (m_pFile) fclose(m_pFile);
  }

  bool read(const ReadOptions& options, const Allocator& allocator, FrameData& frame) override {
    using namespace attribute;
    auto description = m_Description.subimages.at(0);
    auto data = frame.setDescriptionAndAllocate(description, allocator);
    float * out = (float*)data.begin();
    if(isType<Bucket>(m_holder)) {
      const Bucket & b = unpackTo<Bucket>(m_holder);
      if(!b.has_data()) {
        for(size_t i = 0; i < data.size()/4; i++)
          out[i] = 0.f;
        return true;
      }
      float * in = (float*)b.data().data();
      for(size_t i = 0; i < data.size()/4; i+=4){
        out[i] = in[i];
        out[i+1] = in[i+1];
        out[i+2] = in[i+2];
        out[i+3] = in[i+3];
      }
    } else if(isType<Render>(m_holder)) { 
      for(size_t i = 0; i < data.size()/4; i+=4){
        out[i] = 0.1f;
        out[i+1] = 0.1f;
        out[i+2] = 0.1f;
        out[i+3] = 1.f;
      }
    }
    return true;
  }
};

class PBDescriptor : public IIODescriptor {
  virtual ~PBDescriptor() {}
  virtual const std::vector<std::string>& getSupportedExtensions() const override {
    static std::vector<std::string> extensions = {"pb"};
    return extensions;
  }
  virtual bool supports(Capability capability) const override { return capability == Capability::READER_SINGLE_FRAME; }
  virtual const char* getName() const override { return "Protobuf"; }
  virtual IImageReader* createFileReader(const char* filename) const override { return new PBImageReader(filename); }
};

namespace {
bool registrar = IODescriptors::instance().registerDescriptor(new PBDescriptor());
}  // namespace

}  // namespace duke

