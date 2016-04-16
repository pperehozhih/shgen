//
//  shgen_compiler_helper.hpp
//  shgen
//
//  Created by Paul Perekhozhikh on 12.04.16.
//
//

#ifndef shgen_compiler_helper_hpp
#define shgen_compiler_helper_hpp

#include "shgen_compiler.h"
#include <map>

namespace shgen {
   namespace compiler {

      struct GeneratorConfigWriterImpl : public shgen::compiler::GeneratorConfigWriter {
         typedef std::map<std::string, std::string> pass_param;
         typedef std::map<std::string, pass_param> pass;
         typedef std::map<std::string, pass> technique_container;
         technique_container m_config;
         GeneratorConfigWriterImpl(){
         }
         std::string to_string(shgen::compiler::Generator::ShaderType type){
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
         
         std::string get_shader_lang(unsigned s);
         
         inline std::string GenerateFileName(const std::string& tehnique, const std::string& pass,
                                             shgen::compiler::Generator::ShaderType shader, unsigned format){
            std::string result = tehnique + "." + pass + ".";
            result += get_shader_lang(format) + "." + to_string(shader);
            return result;
         }
         virtual void Write(shgen::compiler::Generator* generator,
                            const std::string& tehnique, const std::string& pass,
                            const std::string& name, const std::string& value);
         
         virtual void WriteShader(shgen::compiler::Generator* generator,
                                  const std::string& tehnique, const std::string& pass,
                                  const std::string& source, unsigned format,
                                  shgen::compiler::Generator::ShaderType shader);
         
         virtual void EndWrite(shgen::compiler::Generator* generator);
      };
      
   }
}

#endif /* shgen_compiler_helper_hpp */
