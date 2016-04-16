//
//  shgen_compiler_helper.cpp
//  shgen
//
//  Created by Paul Perekhozhikh on 12.04.16.
//
//

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <picojson.h>
#include "shgen_compiler_helper.h"

namespace shgen {
   namespace compiler {
      std::string GeneratorConfigWriterImpl::get_shader_lang(unsigned s){
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
      
      void GeneratorConfigWriterImpl::Write(shgen::compiler::Generator* generator,
                                            const std::string& tehnique, const std::string& pass,
                                            const std::string& name, const std::string& value) {
         m_config[tehnique][pass][name] = value;
      }
      
      void GeneratorConfigWriterImpl::WriteShader(shgen::compiler::Generator* generator,
                       const std::string& tehnique, const std::string& pass,
                       const std::string& source, unsigned format,
                       shgen::compiler::Generator::ShaderType shader) {
         std::string file_name = GenerateFileName(tehnique, pass, shader, format);
         generator->WriteTextToContainer(file_name, source);
      }
      
      void GeneratorConfigWriterImpl::EndWrite(shgen::compiler::Generator* generator){
         picojson::array root;
         for (technique_container::iterator techniquer_iter = m_config.begin();
              techniquer_iter != m_config.end();
              ++techniquer_iter){
            picojson::object technique_json;
            technique_json["name"] = picojson::value(techniquer_iter->first);
            picojson::array passes;
            for(pass::iterator pass_iter = techniquer_iter->second.begin();
                pass_iter != techniquer_iter->second.end();
                ++pass_iter) {
               picojson::object pass;
               pass["name"] = picojson::value(pass_iter->first);
               
               picojson::array param;
               
               for (pass_param::iterator pass_param_iter = pass_iter->second.begin();
                    pass_param_iter != pass_iter->second.end();
                    ++pass_param_iter){
                  picojson::object p;
                  p["name"] = picojson::value(pass_param_iter->first);
                  p["value"] = picojson::value(pass_param_iter->second);
                  param.push_back(picojson::value(p));
               }
               
               pass["param"] = picojson::value(param);
               passes.push_back(picojson::value(pass));
            }
            technique_json["pass"] = picojson::value(passes);
            root.push_back(picojson::value(technique_json));
         }
         std::stringstream str;
         picojson::object _root_object;
         _root_object["config"] = picojson::value(root);
         str << picojson::value(_root_object);
         generator->WriteTextToContainer("config.json", str.str());
      }
      
   }
}
