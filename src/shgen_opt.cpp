//
//  shgen_opt.cpp
//  shgen
//
//  Created by Paul Perekhozhikh on 01.04.16.
//
//

#include "shgen.h"
#include "shgen_private.h"
#include "glsl_optimizer.h"

glslopt_target get_version(GLSLTarget version) {
   if (version <= GLSLTarget100)
      return kGlslTargetOpenGL;
   if (version > GLSLTarget100 && version <= GLSLTarget140)
      return kGlslTargetOpenGLES20;
   return kGlslTargetOpenGLES30;
}

std::string GetGLSLShaderOptimize(sgContextPtr context, const char* shader, sgShaderType type, GLSLTarget version) {
   glslopt_ctx* ctx = glslopt_initialize(get_version(version));
   std::string result;
   glslopt_shader* sh = glslopt_optimize (ctx, type == sgShaderTypeVertex ? kGlslOptShaderVertex : kGlslOptShaderFragment,
                                          shader, 0);
   if (glslopt_get_status (sh)) {
      result = glslopt_get_output (sh);
   } else {
      glslopt_cleanup (ctx);
      throw GenerateException(glslopt_get_log (sh));
   }
   glslopt_cleanup (ctx);
   return result;
}

std::string GetGLSLShaderMettal(sgContextPtr context, const char* shader, sgShaderType type) {
   glslopt_ctx* ctx = glslopt_initialize(kGlslTargetMetal);
   std::string result;
   glslopt_shader* sh = glslopt_optimize (ctx, type == sgShaderTypeVertex ? kGlslOptShaderVertex : kGlslOptShaderFragment,
                                          shader, 0);
   if (glslopt_get_status (sh)) {
      result = glslopt_get_output (sh);
   } else {
      glslopt_cleanup (ctx);
      throw GenerateException(glslopt_get_log (sh));
   }
   glslopt_cleanup (ctx);
   return result;
}
