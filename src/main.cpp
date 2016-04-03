#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include "shgen.h"
#include "shgen_compiler.h"

struct GeneratorConfigWriterImpl : public shgen::compiler::GeneratorConfigWriter {
   typedef std::map<std::string, std::string> pass_param;
   typedef std::map<std::string,pass_param > technique_container;
   technique_container m_config;
   inline std::string to_string(shgen::compiler::Generator::ShaderType type){
      switch (type) {
         case shgen::compiler::Generator::ShaderTypeVertex:
            return "vertex";
            break;
         case shgen::compiler::Generator::ShaderTypeFragment:
            return "fragment";
            break;
         default:
            return "unknown";
            break;
      }
   }

   inline std::string get_shader_lang(unsigned s){
      if (s & shgen::compiler::Generator::GeneratorTypeGL){
         return "GL";
      } else if (s & shgen::compiler::Generator::GeneratorTypeGL2) {
         return "GL2";
      } else if (s & shgen::compiler::Generator::GeneratorTypeGL3) {
         return "GL3";
      } else if (s & shgen::compiler::Generator::GeneratorTypeGLES) {
         return "GLES";
      } else if (s & shgen::compiler::Generator::GeneratorTypeHLSL) {
         return "HLSL";
      } else if (s & shgen::compiler::Generator::GeneratorTypeMetal) {
         return "METAL";
      } else {
         return "UNKNOWN";
      }
   }
   
   inline std::string GenerateFileName(const std::string& tehnique, const std::string& pass,
                                       shgen::compiler::Generator::ShaderType shader, unsigned format){
      std::string result = tehnique + "." + pass + ".";
      result += get_shader_lang(format) + "." + to_string(shader);
      return result;
   }
   virtual void Write(shgen::compiler::Generator* generator,
                      const std::string& tehnique, const std::string& pass,
                      const std::string& name, const std::string& value) {
      m_config[tehnique + "." + pass][name] = value;
   }

   virtual void WriteShader(shgen::compiler::Generator* generator,
                            const std::string& tehnique, const std::string& pass,
                            const std::string& source, unsigned format,
                            shgen::compiler::Generator::ShaderType shader) {
      std::string file_name = GenerateFileName(tehnique, pass, shader, format);
      m_config[tehnique + "." + pass]["file_name"] = file_name;
      generator->WriteTextToContainer(file_name, source);
   }
   virtual void EndWrite(shgen::compiler::Generator* generator){
//      for (technique_container::iterator tech = m_config.begin(); tech != m_config.end(); ++tech){
//         for(pass_param::iterator pass_value = tech->second.begin(); pass_value != tech->second.end(), ++pass_value){
//            pass_value->first
//         }
//      }
   }
};

int main(int argc, char** argv) {
   GeneratorConfigWriterImpl generator_delegate;
   shgen::compiler::Generator generator("/tmp/test.shgenfx/");
   generator.SetGenerator(&generator_delegate);
   generator.ImportFXFile("/tmp/fx.fx");
   generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeGLES);
   generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeGL);
   generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeGL2);
   generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeGL3);
   generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeMetal);
   generator.Generate();
}
