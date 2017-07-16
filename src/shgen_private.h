//
//  shgen_private.h
//  shgen
//
//  Created by Paul Perekhozhikh on 31.03.16.
//
//

#ifndef shgen_private_h
#define shgen_private_h

#include <HLSLParser.h>
#include <Engine/Allocator.h>
#include <hlsl2glsl.h>
#include <map>

struct sgContextParser {
   M4::Allocator  allocator;
   M4::HLSLParser parser;
   M4::HLSLTree   tree;
   sgContextParser(const char* name, const char* data, size_t length):
   parser(&allocator, name, data, length), tree(&allocator){}
};

struct sgAttribute;

struct sgUniform;

struct sgContext {
   std::shared_ptr<sgContextParser>                      current_context;
   std::string                                           current_name;
   std::vector<std::shared_ptr<sgTechnique> >            techniques;
   std::string                                           current_effect;
   std::vector<std::string>                              results;
   std::map<std::string, sgAttribute>                    attributes;
   std::map<std::string, sgUniforms>                     uniforms;
   std::vector<sgUniform>                                all_uniforms;
};

struct sgPass {
   M4::HLSLPass* pass;
   struct sgPassParam {
      const char* name;
      const char* param;
      sgPassParam(const char* name, const char* param):name(name), param(param) {}
   };
   std::vector<sgPassParam> params;
   sgPass(M4::HLSLPass* p):pass(p){
      M4::HLSLPassParam* param = p->passParam;
      while(param) {
         params.push_back(sgPassParam(param->name, param->value));
         param = param->nextPassParam;
      }
   }
};

struct sgTechnique {
   M4::HLSLTechnique*                     technique;
   std::vector<std::shared_ptr<sgPass> >  passes;
   sgTechnique(M4::HLSLTechnique* tech):technique(tech){
      M4::HLSLPass* pass = tech->pass;
      while(pass){
         passes.push_back(std::shared_ptr<sgPass>(new sgPass(pass)));
         pass = pass->nextPass;
      }
   }
};

class UnknownTargetException : public std::exception {
   std::string message;
public:
   UnknownTargetException(const std::string& message):std::exception(), message(message) {}
   virtual ~UnknownTargetException() throw() {}
   virtual const char* what() const throw() { return message.c_str(); }
};

class GenerateException : public std::exception {
   std::string message;
public:
   GenerateException(const std::string& message):std::exception(), message(message) {}
   virtual ~GenerateException() throw() {}
   virtual const char* what() const throw() { return message.c_str(); }
};

class InitException : public std::exception {
   std::string message;
public:
   InitException(const std::string& message):std::exception(), message(message) {}
   virtual ~InitException() throw() {}
   virtual const char* what() const throw() { return message.c_str(); }
};

struct sgAttribute{
   std::vector<EAttribSemantic>  sematic;
   std::vector<const char*>      names;
   std::vector<size_t>           size;
};


struct sgUniform{
   enum sgUniformType {
      sgUniformTypeUndefine,
      sgUniformTypeVoid,
      sgUniformTypeFloat,
      sgUniformTypeFloat2,
      sgUniformTypeFloat3,
      sgUniformTypeFloat4,
      sgUniformTypeFloat3x3,
      sgUniformTypeFloat4x4,
      sgUniformTypeHalf,
      sgUniformTypeHalf2,
      sgUniformTypeHalf3,
      sgUniformTypeHalf4,
      sgUniformTypeHalf3x3,
      sgUniformTypeHalf4x4,
      sgUniformTypeBool,
      sgUniformTypeInt,
      sgUniformTypeInt2,
      sgUniformTypeInt3,
      sgUniformTypeInt4,
      sgUniformTypeUint,
      sgUniformTypeUint2,
      sgUniformTypeUint3,
      sgUniformTypeUint4,
      sgUniformTypeTexture,
      sgUniformTypeSampler2D,
      sgUniformTypeSamplerCube,
      sgUniformTypeStruct,
   };
   std::string                name;
   sgUniformType              type = sgUniformTypeUndefine;
   int                        size = 0;
   std::vector<sgUniform>     childs;
   void UpdateSize() {
      switch(type) {
         case sgUniformTypeUint:
         case sgUniformTypeInt:
         case sgUniformTypeBool:
         case sgUniformTypeHalf:
         case sgUniformTypeFloat:
            size = 4;
            break;
         case sgUniformTypeUint2:
         case sgUniformTypeInt2:
         case sgUniformTypeHalf2:
         case sgUniformTypeFloat2:
            size = 8;
            break;
         case sgUniformTypeUint3:
         case sgUniformTypeInt3:
         case sgUniformTypeHalf3:
         case sgUniformTypeFloat3:
            size = 12;
            break;
         case sgUniformTypeUint4:
         case sgUniformTypeInt4:
         case sgUniformTypeHalf4:
         case sgUniformTypeFloat4:
            size = 16;
            break;
         case sgUniformTypeHalf3x3:
         case sgUniformTypeFloat3x3:
            size = 36;
            break;
         case sgUniformTypeHalf4x4:
         case sgUniformTypeFloat4x4:
            size = 64;
            break;
         case sgUniformTypeStruct:
         {
            size = 0;
            for (sgUniform &child : childs) {
               child.UpdateSize();
               if (child.size > 0) {
                  size += child.size;
               }
            }
            break;
         }
         default:
            size = -1;
            break;
      }
   }
};

struct sgUniforms {
   std::vector<sgUniform> uniforms;
};

enum GLSLTarget {
   GLSLTarget100,
   GLSLTargetGLES,
   GLSLTarget110,
   GLSLTarget120,
   GLSLTarget140,
   GLSLTarget300,
   GLSLTargetUnknown
};

inline GLSLTarget to_target(sgGenerator type) {
   if (sgGeneratorGLSL_ES == type || sgGeneratorGLSL_ES_OPT == type)
      return GLSLTargetGLES;
   if (sgGeneratorGLSL110 == type || sgGeneratorGLSL110_OPT == type)
      return GLSLTarget110;
   if (sgGeneratorGLSL120 == type || sgGeneratorGLSL120_OPT == type)
      return GLSLTarget120;
   if (sgGeneratorGLSL140 == type || sgGeneratorGLSL140_OPT == type)
      return GLSLTarget140;
   if (sgGeneratorGLSL300 == type || sgGeneratorGLSL300_OPT == type)
      return GLSLTarget300;
   return GLSLTargetUnknown;
}

std::string GetHLSLShader(sgContextPtr context, const char* entry, sgShaderType type);

std::string GetHLSLLastError();

std::string GetGLSLShader(sgContextPtr context, const char* entry, const char* shader, sgShaderType type, GLSLTarget version);

void GLSLShaderDeinit();

std::string GetGLSLShaderOptimize(sgContextPtr context, const char* shader, sgShaderType type, GLSLTarget version);

std::string GetGLSLShaderMettal(sgContextPtr context, const char* shader, sgShaderType type);

sgAttribute GetAttribute(sgContextPtr context, const char* entry, bool &haveMoreOneColor);

std::vector<sgUniform> GetAllUniforms(sgContextPtr context);

std::map<int, sgUniform> GetUniform(sgContextPtr context, const char* entry);

#endif /* shgen_private_h */
