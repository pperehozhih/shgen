//
//  shgen_compiler.cpp
//  shgen
//
//  Created by Paul Perekhozhikh on 03.04.16.
//
//

#include "shgen_compiler.h"
#include <dirent.h>
#include <sys/stat.h>
#include <shgen.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <queue>

namespace {
   void CleanDirectory(const char* directory) {
      DIR *theFolder = opendir(directory);
      struct dirent *next_file;
      
      while ( (next_file = readdir(theFolder)) != NULL )
      {
         // build the path for each file in the folder
         remove((std::string(directory) + "/" + next_file->d_name).c_str());
      }
      closedir(theFolder);
   }
   
   bool CheckDir(const char* dir){
      DIR *theFolder = opendir(dir);
      bool res = theFolder != 0;
      closedir(theFolder);
      return res;
   }

   std::string ReadFile(const char* fileName)
   {
      std::ifstream ifs(fileName);
      std::stringstream buffer;
      buffer << ifs.rdbuf();
      return buffer.str();
   }
   bool WriteFile(const char* fileName, const char* data, size_t len) {
      std::ofstream fout;
      fout.open(fileName, std::ios::binary | std::ios::out);
      if (!fout.good())
         return false;
      fout.write(data, len);
      fout.close();
      return true;
   }
   std::string GetLastWord(const std::string& val){
      std::string::size_type offset = val.rfind(' ');
      if (val.npos == offset){
         return val;
      }
      return val.substr(offset, val.length() - offset);
   }
   
   struct EraseFuncNamePredicate {
      bool operator ()(char c){
         return !(
                  (c >= 'a' && c <= 'z') ||
                  (c >= 'A' && c <= 'Z') ||
                  c == '_'
                  );
      }
   };
   
   std::string EraseFuncName(const std::string& func){
      std::string result = func;
      result.erase(
                   std::remove_if(result.begin(), result.end(),
                                  EraseFuncNamePredicate()),
                   result.end()
      );
      return result;
   }
   
   void WriteShader(shgen::compiler::Generator* _this,
                    sgContextPtr context, const char* vertex_entry, sgGenerator sg_generator,
                    sgShaderType sg_type, shgen::compiler::Generator::GeneratorType generator,
                    shgen::compiler::Generator::ShaderType type,
                    const char* technique, const char* pass){
      const char* source = sgGetShader(context, vertex_entry,
                                       sg_generator, sg_type);
      if (source == nullptr) {
         throw std::string(sgGetLastError());
      }
      _this->GetGenerator()->WriteShader(_this, technique,
                                         pass, source,
                                         generator, type);
   }
}

namespace shgen {
   namespace compiler {
      Generator::Generator(const std::string& fxFolder):m_fx_folder(fxFolder){
         if (m_fx_folder.back() != '/') {
            m_fx_folder.insert(m_fx_folder.end(), '/');
         }
      }
      
      void Generator::WriteTextToContainer(const std::string& filename, const std::string& content){
         if (!WriteFile((m_fx_folder + filename).c_str(), content.c_str(), content.length())){
            std::cerr << "Cannot write file " << filename << std::endl;
         }
      }

      void Generator::ImportFXFile(const std::string& fxFileName) {
         m_fx_source = ReadFile(fxFileName.c_str());
      }

      void Generator::Generate() {
         if (m_generator_config == 0)
            throw std::string("Cannot set generator config, please use generator.SetGenerator()");
         if (m_fx_source.empty()){
            throw std::string("Cannot set fx content");
         }
         const int dir_err = mkdir(m_fx_folder.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
         if (dir_err < 0 && CheckDir(m_fx_folder.c_str()) == false){
            throw (std::string("Cannot create dir") + m_fx_folder);
         }
         sgContextPtr context = sgContextInit();
         if (!context) {
            throw (std::string("Cannot init shgen context:") + sgGetLastError());
         }
         int result = sgReadShader(context, m_fx_source.c_str());
         if (result != sgErrorOk) {
            throw (std::string("Cannot read fx file:") + sgGetLastError());
         }
         CleanDirectory(m_fx_folder.c_str());
         sgTechniquePtr technique = sgGetFirstTechnique(context);
         if (technique == nullptr){
            throw std::string("Fx cannot contain technique section");
         }
         while(technique) {
            sgPassPtr pass = sgGetFirstPass(technique);
            std::string vertex_entry = "vs_main";
            std::string fragment_entry = "ps_main";
            while (pass) {
               int param_count = sgGetPassParamCount(pass);
               while (param_count > 0) {
                  const char* name = sgGetPassParamName(pass, param_count - 1);
                  const char* value = sgGetPassParamValue(pass, param_count - 1);
                  if (strcasecmp(name, "vertexshader") == 0) {
                     std::string func_name = EraseFuncName(GetLastWord(value));
                     if (func_name.length())
                        vertex_entry = func_name;
                     
                  } else if (strcasecmp(name, "pixelshader") == 0) {
                     std::string func_name = EraseFuncName(GetLastWord(value));
                     if (func_name.length())
                        fragment_entry = func_name;
                  } else {
                     m_generator_config->Write(this,
                                               sgGetTechniqueName(technique),
                                               sgGetPassName(pass),
                                               name, value);
                  }
                  param_count--;
               }
               sgAttributePtr attribute = sgGetAttrubyteByEntry(context, vertex_entry.c_str());
               int attribute_count = sgGetAttributeCount(attribute);
               std::stringstream attribute_stream;
               while(attribute_count >= 0) {
                  attribute_count--;
                  const char* attribute_name = sgGetAttributeNameByLocation(attribute, attribute_count);
                  int attribute_size = sgGetAttributeSizeByLocation(attribute, attribute_count);
                  if (attribute_name) {
                     if (attribute_stream.str().size() > 0){
                        attribute_stream << ";";
                     }
                     attribute_stream << attribute_name << ":" << attribute_count << ":" << attribute_size;
                  }
               }
               if (attribute_stream.str().size() != 0) {
                  m_generator_config->Write(this,
                                            sgGetTechniqueName(technique),
                                            sgGetPassName(pass),
                                            "attribute", attribute_stream.str());
               }
               {
                  std::queue<std::string> functions;
                  std::stringstream uniform_stream;
                  functions.push(fragment_entry);
                  functions.push(vertex_entry);
                  while(functions.empty() == false) {
                     sgUniformsPtr uniforms = sgGetUniformsByEntery(context, functions.front().c_str());
                     int uniform_count = sgGetUniformsCount(uniforms);
                     while(uniform_count >= 0) {
                        uniform_count--;
                        int uniform_size = sgGetUniformsSize(uniforms, uniform_count);
                        const char* uniform_name = sgGetUniformsName(uniforms, uniform_count);
                        if (uniform_name){
                           if (uniform_stream.str().size() > 0){
                              uniform_stream << ";";
                           }
                           uniform_stream << uniform_name << ":" << uniform_count << ":" << uniform_size;
                        }
                     }
                     functions.pop();
                  }
                  if (uniform_stream.str().size() != 0) {
                     m_generator_config->Write(this,
                                               sgGetTechniqueName(technique),
                                               sgGetPassName(pass),
                                               "uniform", uniform_stream.str());
                  }
               }
               if (m_outputFormat & GeneratorTypeHLSL) {
                  WriteShader(this, context,
                              vertex_entry.c_str(),
                              sgGeneratorHLSL,
                              sgShaderTypeVertex,
                              GeneratorTypeHLSL,
                              ShaderTypeVertex,
                              sgGetTechniqueName(technique),
                              sgGetPassName(pass));
                  /******************************************************************/
                  WriteShader(this, context,
                              fragment_entry.c_str(),
                              sgGeneratorHLSL,
                              sgShaderTypeFrag,
                              GeneratorTypeHLSL,
                              ShaderTypeFragment,
                              sgGetTechniqueName(technique),
                              sgGetPassName(pass));
               }
               if (m_outputFormat & GeneratorTypeGL){
                  WriteShader(this, context,
                              vertex_entry.c_str(),
                              sgGeneratorGLSL110,
                              sgShaderTypeVertex,
                              GeneratorTypeGL,
                              ShaderTypeVertex,
                              sgGetTechniqueName(technique),
                              sgGetPassName(pass));
                  /******************************************************************/
                  WriteShader(this, context,
                              fragment_entry.c_str(),
                              sgGeneratorGLSL110,
                              sgShaderTypeFrag,
                              GeneratorTypeGL,
                              ShaderTypeFragment,
                              sgGetTechniqueName(technique),
                              sgGetPassName(pass));
               }
               if (m_outputFormat & GeneratorTypeGL2){
                  WriteShader(this, context,
                              vertex_entry.c_str(),
                              sgGeneratorGLSL120,
                              sgShaderTypeVertex,
                              GeneratorTypeGL2,
                              ShaderTypeVertex,
                              sgGetTechniqueName(technique),
                              sgGetPassName(pass));
                  /******************************************************************/
                  WriteShader(this, context,
                              fragment_entry.c_str(),
                              sgGeneratorGLSL120,
                              sgShaderTypeFrag,
                              GeneratorTypeGL2,
                              ShaderTypeFragment,
                              sgGetTechniqueName(technique),
                              sgGetPassName(pass));
               }
               if (m_outputFormat & GeneratorTypeGL3) {
                  WriteShader(this, context,
                              vertex_entry.c_str(),
                              sgGeneratorGLSL300,
                              sgShaderTypeVertex,
                              GeneratorTypeGL3,
                              ShaderTypeVertex,
                              sgGetTechniqueName(technique),
                              sgGetPassName(pass));
                  /******************************************************************/
                  WriteShader(this, context,
                              fragment_entry.c_str(),
                              sgGeneratorGLSL300,
                              sgShaderTypeFrag,
                              GeneratorTypeGL3,
                              ShaderTypeFragment,
                              sgGetTechniqueName(technique),
                              sgGetPassName(pass));
               }
               if (m_outputFormat & GeneratorTypeGLES) {
                  WriteShader(this, context,
                              vertex_entry.c_str(),
                              sgGeneratorGLSL_ES_OPT,
                              sgShaderTypeVertex,
                              GeneratorTypeGLES,
                              ShaderTypeVertex,
                              sgGetTechniqueName(technique),
                              sgGetPassName(pass));
                  /******************************************************************/
                  WriteShader(this, context,
                              fragment_entry.c_str(),
                              sgGeneratorGLSL_ES_OPT,
                              sgShaderTypeFrag,
                              GeneratorTypeGLES,
                              ShaderTypeFragment,
                              sgGetTechniqueName(technique),
                              sgGetPassName(pass));
               }
               if (m_outputFormat & GeneratorTypeMetal) {
                  WriteShader(this, context,
                              vertex_entry.c_str(),
                              sgGeneratorMetal,
                              sgShaderTypeVertex,
                              GeneratorTypeMetal,
                              ShaderTypeVertex,
                              sgGetTechniqueName(technique),
                              sgGetPassName(pass));
                  /******************************************************************/
                  WriteShader(this, context,
                              fragment_entry.c_str(),
                              sgGeneratorMetal,
                              sgShaderTypeFrag,
                              GeneratorTypeMetal,
                              ShaderTypeFragment,
                              sgGetTechniqueName(technique),
                              sgGetPassName(pass));
               }
               pass = sgGetNextPass(technique, pass);
            }
            technique = sgGetNextTechnique(context, technique);
         }
         m_generator_config->EndWrite(this);
      }
   }
}
