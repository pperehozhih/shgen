#include "shgen.h"
#include "shgen_private.h"

namespace {
   std::string g_last_error;
}

extern "C" sgContextPtr sgContextInit() {
   sgContextPtr context = new sgContext();
   return context;
}

extern "C" const char* sgGetLastError() {
   return g_last_error.c_str();
}

extern "C" const char* sgGetTechniqueName(sgTechniquePtr technique) {
   if (technique == 0)
      return 0;
   return technique->technique->name;
}

extern "C" const char* sgGetPassName(sgPassPtr pass) {
   if (pass == 0)
      return 0;
   return pass->pass->name;
}

extern "C" int sgGetPassParamCount(sgPassPtr pass) {
   if (pass == 0)
      return 0;
   return pass->params.size();
}

extern "C" const char* sgGetPassParamName(sgPassPtr pass, int index) {
   if (pass == 0 || index < 0)
      return 0;
   if (pass->params.size() <= index)
      return 0;
   return pass->params[index].name;
}

extern "C" const char* sgGetPassParamValue(sgPassPtr pass, int index) {
   if (pass == 0 || index < 0)
      return 0;
   if (pass->params.size() <= index)
      return 0;
   return pass->params[index].param;
}

extern "C" int sgReadShader(sgContextPtr context, const char* data) {
   if (data == 0)
      return sgErrorMemory;
   if (context == 0)
      return sgErrorBadArgument;
   context->current_context.reset(new sgContextParser(context->current_name.c_str(), data, std::strlen(data)));
   if (!context->current_context->parser.Parse(&context->current_context->tree)){
      g_last_error = GetHLSLLastError();
      return sgErrorParse;
   }
   M4::HLSLRoot* root = context->current_context->tree.GetRoot();
   if (root == 0) {
      g_last_error = GetHLSLLastError();
      return sgErrorParse;
   }
   M4::HLSLStatement* statment = root->statement;
   context->techniques.clear();
   while (statment) {
      if (statment->nodeType == M4::HLSLNodeType_Technique){
         context->techniques.push_back(std::shared_ptr<sgTechnique>(new sgTechnique((M4::HLSLTechnique*)statment)));
      }
      statment = statment->nextStatement;
   }
   context->current_effect.clear();
   context->current_effect = data;
   context->all_uniforms = GetAllUniforms(context);
   return sgErrorOk;
}

extern "C" sgTechniquePtr sgGetFirstTechnique(sgContextPtr context) {
   if (context == 0 || context->techniques.empty())
      return 0;
   return context->techniques.front().get();
}

extern "C" sgTechniquePtr sgGetNextTechnique(sgContextPtr context, sgTechniquePtr technique) {
   if (context == 0 || context->techniques.empty())
      return 0;
   std::vector<std::shared_ptr<sgTechnique> >::iterator res = context->techniques.begin();
   while( res != context->techniques.end()) {
      if (res->get() == technique)
         break;
      ++ res;
   }
   if (res == context->techniques.end())
      return 0;
   ++res;
   if (res == context->techniques.end())
      return 0;
   return res->get();
}

extern "C" sgPassPtr sgGetFirstPass(sgTechniquePtr technique) {
   if (technique == 0 || technique->passes.empty())
      return 0;
   return technique->passes.front().get();
}

extern "C" sgPassPtr sgGetNextPass(sgTechniquePtr technique, sgPassPtr pass) {
   if (technique == 0 || technique->passes.empty())
      return 0;
   std::vector<std::shared_ptr<sgPass> >::iterator res = technique->passes.begin();
   while (res != technique->passes.end()) {
      if (res->get() == pass)
         break;
      ++res;
   }
   if (res == technique->passes.end())
      return 0;
   ++res;
   if (res == technique->passes.end())
      return 0;
   return res->get();
}

extern "C" void sgSetName(sgContextPtr context, const char* name) {
   if (name == 0)
      return;
   context->current_name = name;
}

sgAttributePtr sgGetAttrubyteByEntryImpl(sgContextPtr context, const char* entry) {
   if (context->attributes.find(entry) == context->attributes.end()) {
      bool haveMoreOneColor = false;
      context->attributes[entry] = GetAttribute(context, entry, haveMoreOneColor);
   }
   return &(context->attributes[entry]);
}

extern "C" sgAttributePtr sgGetAttrubyteByEntry(sgContextPtr context, const char* entry) {
   return sgGetAttrubyteByEntryImpl(context, entry);
}

int sgGetAttributeCountImpl(sgAttributePtr attribute) {
   if (attribute == nullptr){
      return -1;
   }
   return attribute->names.size();
}

extern "C" int sgGetAttributeCount(sgAttributePtr attribute){
   return sgGetAttributeCountImpl(attribute);
}

const char* sgGetAttributeNameByLocationImpl(sgAttributePtr attribute, int location) {
   if (attribute == nullptr){
      return nullptr;
   }
   if (location < 0 || location >= attribute->names.size()){
      return nullptr;
   }
   return attribute->names[location];
}

extern "C" const char* sgGetAttributeNameByLocation(sgAttributePtr attribute, int location) {
   return sgGetAttributeNameByLocationImpl(attribute, location);
}

int sgGetAttributeSizeByLocationImpl(sgAttributePtr attribute, int location){
   if (attribute == nullptr){
      return 0;
   }
   if (location < 0 || location >= attribute->names.size()){
      return 0;
   }
   return attribute->size[location];
}

extern "C" int sgGetAttributeSizeByLocation(sgAttributePtr attribute, int location){
   return sgGetAttributeSizeByLocationImpl(attribute, location);
}

sgUniformsPtr sgGetUniformsByEnteryImpl(sgContextPtr context, const char* entry) {
   if (context->uniforms.find(entry) == context->uniforms.end()) {
      sgUniforms added;
      std::map<int, sgUniform> result = GetUniform(context, entry);
      for (std::pair<int, sgUniform> kvp : result) {
         added.uniforms.push_back(kvp.second);
      }
      context->uniforms[entry] = added;
   }
   return &(context->uniforms[entry]);
}

extern "C" sgUniformsPtr sgGetUniformsByEntery(sgContextPtr context, const char* entry) {
   return sgGetUniformsByEnteryImpl(context, entry);
}

int sgGetUniformsCountImpl(sgUniformsPtr uniforms){
   if (uniforms == nullptr){
      return -1;
   }
   return uniforms->uniforms.size();
}

extern "C" int sgGetUniformsCount(sgUniformsPtr uniforms){
   return sgGetUniformsCountImpl(uniforms);
}

int sgGetUniformsSizeImpl(sgUniformsPtr uniforms, int index){
   if (uniforms == nullptr || index < 0 || index >= uniforms->uniforms.size()){
      return -1;
   }
   return uniforms->uniforms[index].size;
}

extern "C" int sgGetUniformsSize(sgUniformsPtr uniforms, int index){
   return sgGetUniformsSizeImpl(uniforms, index);
}

const char* sgGetUniformsNameImpl(sgUniformsPtr uniforms, int index) {
   if (uniforms == nullptr || index < 0 || index >= uniforms->uniforms.size()){
      return nullptr;
   }
   return uniforms->uniforms[index].name.c_str();
}

extern "C" const char* sgGetUniformsName(sgUniformsPtr uniforms, int index){
   return sgGetUniformsNameImpl(uniforms, index);
}

const char* sgGetShaderImpl(sgContextPtr context, const char* entry, sgGenerator generator, sgShaderType type) {
   if (context == 0 || entry == 0)
      return 0;
   std::string first_phases_result;
   // First phases
   switch (generator) {
      case sgGeneratorHLSL:
         context->results.push_back(GetHLSLShader(context, entry, type));
         return context->results.back().c_str();
         break;
      case sgGeneratorGLSL_ES:
      case sgGeneratorGLSL110:
      case sgGeneratorGLSL120:
      case sgGeneratorGLSL140:
      case sgGeneratorGLSL300:
      case sgGeneratorGLSL_ES_OPT:
      case sgGeneratorGLSL110_OPT:
      case sgGeneratorGLSL120_OPT:
      case sgGeneratorGLSL140_OPT:
      case sgGeneratorGLSL300_OPT:
      case sgGeneratorMetal:
         first_phases_result = GetHLSLShader(context, entry, type);
         break;
      default:
         break;
   }
   // two phases
   std::string two_phases_result;
   switch (generator) {
      case sgGeneratorHLSL:
      case sgGeneratorGLSL_ES:
      case sgGeneratorGLSL110:
      case sgGeneratorGLSL120:
      case sgGeneratorGLSL140:
      case sgGeneratorGLSL300:
         context->results.push_back(GetGLSLShader(context, entry, first_phases_result.c_str(), type, to_target(generator)));
         return context->results.back().c_str();
         break;
      case sgGeneratorGLSL_ES_OPT:
      case sgGeneratorGLSL110_OPT:
      case sgGeneratorGLSL120_OPT:
      case sgGeneratorGLSL140_OPT:
      case sgGeneratorGLSL300_OPT:
         {
            two_phases_result = GetGLSLShader(context, entry, first_phases_result.c_str(), type, GLSLTarget100);
         }
         break;
      case sgGeneratorMetal:
         {
            two_phases_result = GetGLSLShader(context, entry, first_phases_result.c_str(), type, GLSLTarget110);
         }
         break;
      default:
         break;
   }
   // three phases
   switch (generator) {
      case sgGeneratorHLSL:
      case sgGeneratorGLSL_ES:
      case sgGeneratorGLSL110:
      case sgGeneratorGLSL120:
      case sgGeneratorGLSL140:
      case sgGeneratorGLSL300:
         break;
      case sgGeneratorGLSL_ES_OPT:
      case sgGeneratorGLSL110_OPT:
      case sgGeneratorGLSL120_OPT:
      case sgGeneratorGLSL140_OPT:
      case sgGeneratorGLSL300_OPT:
         context->results.push_back(GetGLSLShaderOptimize(context, two_phases_result.c_str(), type, to_target(generator)));
         return context->results.back().c_str();
         break;
      case sgGeneratorMetal:
      {
         context->results.push_back(GetGLSLShaderMettal(context, two_phases_result.c_str(), type));
         return context->results.back().c_str();
      }
         break;
      default:
         break;
   }
   return 0;
}

extern "C" const char* sgGetShader(sgContextPtr context, const char* entry, sgGenerator generator, sgShaderType type) {
   try {
      return sgGetShaderImpl(context, entry, generator, type);
   } catch (GenerateException ex) {
      g_last_error = ex.what();
   } catch (InitException ex) {
      g_last_error = ex.what();
   } catch (UnknownTargetException ex) {
      g_last_error = ex.what();
   } catch (std::exception ex) {
      g_last_error = ex.what();
   }
   return 0;
}

extern "C" void sgContextDeinit(sgContextPtr* context) {
   if (context == 0)
      return;
   if (*context == 0)
      return;
   delete context;
   *context = 0;
   GLSLShaderDeinit();
}

