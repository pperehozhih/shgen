#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include "shgen.h"
#include "shgen_compiler.h"
#include <picojson.h>

struct GeneratorConfigWriterImpl : public shgen::compiler::GeneratorConfigWriter {
   typedef std::map<std::string, std::string> pass_param;
   typedef std::map<std::string, pass_param> pass;
   typedef std::map<std::string, pass> technique_container;
   technique_container m_config;
   GeneratorConfigWriterImpl(){
      
   }
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
      m_config[tehnique][pass][name] = value;
   }

   virtual void WriteShader(shgen::compiler::Generator* generator,
                            const std::string& tehnique, const std::string& pass,
                            const std::string& source, unsigned format,
                            shgen::compiler::Generator::ShaderType shader) {
      std::string file_name = GenerateFileName(tehnique, pass, shader, format);
//      m_config[tehnique][pass]["file_name." + get_shader_lang(format) + "." + to_string(shader)] = file_name;
      generator->WriteTextToContainer(file_name, source);
   }
   virtual void EndWrite(shgen::compiler::Generator* generator){
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
      str << picojson::value(root);
      generator->WriteTextToContainer("config.json", str.str());
   }
};

int main(int argc, char** argv) {
   try {
      std::string inputfile;
      std::string ouputdir;
      int index = 1;
      while (argv[index]) {
         const char* arg = argv[index];
         if (index < argc && arg[0] == '-' && strlen(arg) > 1) {
            switch (arg[1]) {
               case 'i':
               case 'I':
                  inputfile = argv[index + 1];
                  break;
               case 'o':
               case 'O':
                  ouputdir = argv[index + 1];
                  break;
               default:
                  break;
            }
         }
         index++;
      }
      GeneratorConfigWriterImpl generator_delegate;
      shgen::compiler::Generator generator(ouputdir);
      generator.SetGenerator(&generator_delegate);
      generator.ImportFXFile(inputfile);
      generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeGLES);
      generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeGL);
      generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeGL2);
      generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeGL3);
      generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeMetal);
      generator.Generate();
      return 0;
   } catch (std::string ex) {
      std::cerr << ex;
      return -1;
   }
}
