#ifndef SHGEN_INCLUDED_H
#define SHGEN_INCLUDED_H

#ifdef __cplusplus
extern "C" {
#endif
   struct sgContext;

   struct sgTechnique;

   struct sgPass;

   typedef sgContext* sgContextPtr;

   typedef sgTechnique* sgTechniquePtr;

   typedef sgPass* sgPassPtr;

   sgContextPtr sgContextInit();

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

   int sgReadShader(sgContextPtr context, const char* data);

   void sgSetName(sgContextPtr context, const char* name);

   sgTechniquePtr sgGetFirstTechnique(sgContextPtr context);

   sgTechniquePtr sgGetNextTechnique(sgContextPtr context, sgTechniquePtr technique);

   sgPassPtr sgGetFirstPass(sgTechniquePtr technique);

   sgPassPtr sgGetNextPass(sgTechniquePtr technique, sgPassPtr pass);

   const char* sgGetTechniqueName(sgTechniquePtr technique);

   const char* sgGetPassName(sgPassPtr pass);

   int sgGetPassParamCount(sgPassPtr pass);

   const char* sgGetPassParamName(sgPassPtr pass, int index);

   const char* sgGetPassParamValue(sgPassPtr pass, int index);

   const char* sgGetShader(sgContextPtr context, const char* entry, sgGenerator generator, sgShaderType type);

   const char* sgGetLastError();

   void sgContextDeinit(sgContextPtr*);

#ifdef __cplusplus
}
#endif

#endif
