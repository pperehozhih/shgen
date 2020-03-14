#ifndef SHGEN_INCLUDED_H
#define SHGEN_INCLUDED_H

#ifdef _WIN32
#define SHGEN_EXPORT __declspec (dllexport)
#else
#define SHGEN_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif
   struct sgContext;

   struct sgTechnique;

   struct sgPass;

   struct sgAttribute;

   struct sgUniforms;

   typedef sgContext* sgContextPtr;

   typedef sgTechnique* sgTechniquePtr;

   typedef sgPass* sgPassPtr;
   
   typedef sgAttribute* sgAttributePtr;

   typedef sgUniforms* sgUniformsPtr;

   SHGEN_EXPORT sgContextPtr sgContextInit();

   enum sgError {
      sgErrorOk = 0,
      sgErrorMemory = -1,
      sgErrorBadArgument = -2,
      sgErrorParse = -3
   };

   enum sgGenerator {
      sgGeneratorHLSL,
      sgGeneratorGLSL_ES,
      sgGeneratorGLSL110,
      sgGeneratorGLSL120,
      sgGeneratorGLSL140,
      sgGeneratorGLSL300,
      sgGeneratorGLSL_ES_OPT,
      sgGeneratorGLSL110_OPT,
      sgGeneratorGLSL120_OPT,
      sgGeneratorGLSL140_OPT,
      sgGeneratorGLSL300_OPT,
      sgGeneratorMetal
   };

   enum sgShaderType {
      sgShaderTypeVertex,
      sgShaderTypeFrag
   };

   SHGEN_EXPORT int sgReadShader(sgContextPtr context, const char* data);

   SHGEN_EXPORT void sgSetName(sgContextPtr context, const char* name);

   SHGEN_EXPORT sgTechniquePtr sgGetFirstTechnique(sgContextPtr context);

   SHGEN_EXPORT sgTechniquePtr sgGetNextTechnique(sgContextPtr context, sgTechniquePtr technique);

   SHGEN_EXPORT sgPassPtr sgGetFirstPass(sgTechniquePtr technique);

   SHGEN_EXPORT sgPassPtr sgGetNextPass(sgTechniquePtr technique, sgPassPtr pass);

   SHGEN_EXPORT const char* sgGetTechniqueName(sgTechniquePtr technique);

   SHGEN_EXPORT const char* sgGetPassName(sgPassPtr pass);

   SHGEN_EXPORT int sgGetPassParamCount(sgPassPtr pass);

   SHGEN_EXPORT const char* sgGetPassParamName(sgPassPtr pass, int index);

   SHGEN_EXPORT const char* sgGetPassParamValue(sgPassPtr pass, int index);

   SHGEN_EXPORT sgAttributePtr sgGetAttrubyteByEntry(sgContextPtr context, const char* entry);

   SHGEN_EXPORT int sgGetAttributeCount(sgAttributePtr attribute);

   SHGEN_EXPORT const char* sgGetAttributeNameByLocation(sgAttributePtr attribute, int location);

   SHGEN_EXPORT int sgGetAttributeSizeByLocation(sgAttributePtr attribute, int location);

   SHGEN_EXPORT sgUniformsPtr sgGetUniformsByEntery(sgContextPtr context, const char* entry);

   SHGEN_EXPORT int sgGetUniformsCount(sgUniformsPtr uniforms);

   SHGEN_EXPORT int sgGetUniformsSize(sgUniformsPtr uniforms, int index);

   SHGEN_EXPORT const char* sgGetUniformsName(sgUniformsPtr uniforms, int index);

   SHGEN_EXPORT const char* sgGetShader(sgContextPtr context, const char* entry, sgGenerator generator, sgShaderType type);

   SHGEN_EXPORT const char* sgGetLastError();

   SHGEN_EXPORT void sgContextDeinit(sgContextPtr*);

#ifdef __cplusplus
}
#endif

#endif
