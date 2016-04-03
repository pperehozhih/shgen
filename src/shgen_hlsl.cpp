//
//  shgen_hlsl.cpp
//  shgen
//
//  Created by Paul Perekhozhikh on 31.03.16.
//
//

#include "shgen.h"
#include <string>
#include <sstream>
#include "shgen_private.h"
#include "HLSLGenerator.h"
#include "Engine/Log.h"

std::string GetHLSLShader(sgContextPtr context, const char* entry, sgShaderType type){
   M4::HLSLGenerator::Target target = M4::HLSLGenerator::Target_PixelShader;
   switch (type) {
      case sgShaderTypeFrag:
         target = M4::HLSLGenerator::Target_PixelShader;
         break;
      case sgShaderTypeVertex:
         target = M4::HLSLGenerator::Target_VertexShader;
         break;
      default:
         throw UnknownTargetException("Unsupport shader type for target HLSL");
         break;
   }
   M4::HLSLGenerator generator(&context->current_context->allocator);
   if (!generator.Generate(&context->current_context->tree, target, entry, true)) {
      throw GenerateException(std::string("Cannot generate HLSL shader: " ) + M4::GetLastError());
   }

   return generator.GetResult();
}

std::string GetHLSLLastError() {
   return M4::GetLastError();
}
