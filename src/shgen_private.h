//
//  shgen_private.h
//  shgen
//
//  Created by Paul Perekhozhikh on 31.03.16.
//
//

#ifndef shgen_private_h
#define shgen_private_h

#include <hlslparser.h>
#include <Engine/Allocator.h>

struct sgContextParser {
   M4::Allocator  allocator;
   M4::HLSLParser parser;
   M4::HLSLTree   tree;
   sgContextParser(const char* name, const char* data, size_t length):
   parser(&allocator, name, data, length), tree(&allocator){}
};

struct sgContext {
   std::shared_ptr<sgContextParser>                current_context;
   std::string                                     current_name;
   std::vector<std::shared_ptr<sgTechnique> >      techniques;
   std::string                                     current_effect;
   std::vector<std::string>                        results;
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

enum GLSLTarget {
   GLSLTarget100,
   GLSLTarget110,
   GLSLTarget120,
   GLSLTarget140,
   GLSLTarget300,
   GLSLTargetUnknown
};

inline GLSLTarget to_target(sgGenerator type) {
   if (sgGeneratorGLSL_ES == type || sgGeneratorGLSL_ES_OPT == type)
      return GLSLTarget100;
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

#endif /* shgen_private_h */
