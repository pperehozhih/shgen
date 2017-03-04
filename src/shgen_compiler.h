//
//  shgen_compiler.hpp
//  shgen
//
//  Created by Paul Perekhozhikh on 03.04.16.
//
//

#ifndef shgen_compiler_h
#define shgen_compiler_h

#include <string>

namespace shgen {
   namespace compiler {
      struct GeneratorConfigWriter;
      
      class Generator {
      public:
         enum GeneratorType {
            GeneratorTypeHLSL = (1 << 1),
            GeneratorTypeGL = (1 << 2),
            GeneratorTypeGL2 = (1 << 3),
            GeneratorTypeGL3 = (1 << 4),
            GeneratorTypeGLES = (1 << 5),
            GeneratorTypeMetal = (1 << 6)
         };
         enum ShaderType {
            ShaderTypeVertex,
            ShaderTypeFragment
         };
      public:
         Generator(const std::string& fxFolder);
         void ImportFXFile(const std::string& fxFileName);
         void ImportFX(const std::string& fxSource) { m_fx_source = fxSource; }
         void SetOutputFormat(unsigned format) {
            m_outputFormat = format;
         }
         void AddOutputFormat(unsigned format) {
            m_outputFormat |= format;
         }
         inline void SetGenerator(GeneratorConfigWriter* writer){
            m_generator_config = writer;
         }
         inline GeneratorConfigWriter* GetGenerator() const { return m_generator_config; }
         inline const std::string& GetFXFolder() const { return m_fx_folder; }
         void Generate();
         void WriteTextToContainer(const std::string& filename, const std::string& content);
      private:
         unsigned m_outputFormat =  GeneratorTypeHLSL;
         std::string                m_fx_folder;
         std::string                m_fx_source;
         GeneratorConfigWriter*     m_generator_config = nullptr;
      };
      
      struct GeneratorConfigWriter {
         virtual void Write(Generator* generator,
                            const std::string& tehnique, const std::string& pass,
                            const std::string& name, const std::string& value) = 0;
         virtual void WriteShader(Generator* generator,
                                  const std::string& tehnique, const std::string& pass,
                                  const std::string& source, unsigned format,
                                  Generator::ShaderType shader) = 0;
         virtual void EndWrite(Generator* generator) = 0;
      };
   }
}

#endif /* shgen_compiler_h */
