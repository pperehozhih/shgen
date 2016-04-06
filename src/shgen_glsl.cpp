//
//  shgen_glsl.cpp
//  shgen
//
//  Created by Paul Perekhozhikh on 31.03.16.
//
//

#include "shgen.h"
#include <string>
#include <sstream>
#include <hlsl2glsl.h>
#include "shgen_private.h"

namespace {
   bool hlsl_inited = false;
   
   struct sgAttribute{
      std::vector<EAttribSemantic>  sematic;
      std::vector<const char*>      names;
   };
   
   bool ___inited() {
      if (!hlsl_inited) {
         hlsl_inited = Hlsl2Glsl_Initialize();
      }
      if (!hlsl_inited) {
         return false;
      }
      return true;
   }
   bool ___deinit() {
      if (hlsl_inited) {
         Hlsl2Glsl_Shutdown();
      }
      hlsl_inited = false;
      return true;
   }
   sgAttribute GetAttribute(sgContextPtr context, const char* entry, bool &haveMoreOneColor);
   
   std::string GetShader(sgContextPtr context, ShHandle handle, const char* entry, const char* shader, GLSLTarget version) {
      ETargetVersion vers = ETargetGLSL_ES_100;
      switch (version) {
         case GLSLTarget100:
            vers = ETargetGLSL_ES_100;
            break;
         case GLSLTarget110:
            vers = ETargetGLSL_110;
            break;
         case GLSLTarget120:
            vers = ETargetGLSL_120;
            break;
         case GLSLTarget140:
            vers = ETargetGLSL_140;
            break;
         case GLSLTarget300:
            vers = ETargetGLSL_ES_300;
            break;
         default:
            break;
      }
      unsigned option = ETranslateOpPropogateOriginalAttribNames | ETranslateOpForceBuiltinAttribNames;
      {
         bool haveMoreOneColor = false;
         sgAttribute attr = GetAttribute(context, entry, haveMoreOneColor);
         Hlsl2Glsl_SetUserAttributeNames (handle,
                                          &attr.sematic[0],
                                          &attr.names[0],
                                          attr.names.size());
         if (!haveMoreOneColor) {
            option |= ETranslateOpUseGlFragColor;
         }
      }
      if (!Hlsl2Glsl_Parse(handle, shader, vers, 0, 0)) {
         Hlsl2Glsl_DestructCompiler(handle);
         throw GenerateException(std::string("Cannot parse HLSL shader:") + Hlsl2Glsl_GetInfoLog(handle));
      }
      if (!Hlsl2Glsl_Translate(handle, entry, vers, option)) {
         Hlsl2Glsl_DestructCompiler(handle);
         throw GenerateException(std::string("Cannot translate HLSL shader") + Hlsl2Glsl_GetInfoLog(handle));
      }
      std::string result = Hlsl2Glsl_GetShader(handle);
      Hlsl2Glsl_DestructCompiler(handle);
      return result;
   }
   
   M4::HLSLFunction* FindFunction(M4::HLSLRoot* root, const char* name)
   {
      M4::HLSLStatement* statement = root->statement;
      while (statement != NULL)
      {
         if (statement->nodeType == M4::HLSLNodeType_Function)
         {
            M4::HLSLFunction* function = static_cast<M4::HLSLFunction*>(statement);
            if (strcmp(function->name, name) == 0)
            {
               return function;
            }
         }
         statement = statement->nextStatement;
      }
      return NULL;
   }
   
   M4::HLSLStruct* FindStruct(M4::HLSLRoot* root, const char* name)
   {
      M4::HLSLStatement* statement = root->statement;
      while (statement != NULL)
      {
         if (statement->nodeType == M4::HLSLNodeType_Struct)
         {
            M4::HLSLStruct* structDeclaration = static_cast<M4::HLSLStruct*>(statement);
            if (strcmp(structDeclaration->name, name) == 0)
            {
               return structDeclaration;
            }
         }
         statement = statement->nextStatement;
      }
      return NULL;
   }
   struct SemanticPair {
      const char*             name;
      const EAttribSemantic   semantic;
      SemanticPair(const char* name, const EAttribSemantic semantic):name(name), semantic(semantic){}
   };
   SemanticPair semantics_map[] = {
      SemanticPair("POSITION", EAttrSemPosition),
      SemanticPair("POSITION0", EAttrSemPosition),
      SemanticPair("POSITION1", EAttrSemPosition1),
      SemanticPair("POSITION2", EAttrSemPosition2),
      SemanticPair("POSITION3", EAttrSemPosition3),
      SemanticPair("NORMAL", EAttrSemNormal),
      SemanticPair("NORMAL0", EAttrSemNormal),
      SemanticPair("NORMAL1", EAttrSemNormal1),
      SemanticPair("NORMAL2", EAttrSemNormal2),
      SemanticPair("NORMAL3", EAttrSemNormal3),
      SemanticPair("COLOR", EAttrSemColor0),
      SemanticPair("COLOR0", EAttrSemColor0),
      SemanticPair("COLOR1", EAttrSemColor1),
      SemanticPair("COLOR2", EAttrSemColor2),
      SemanticPair("COLOR3", EAttrSemColor3),
      SemanticPair("TEXCOORD", EAttrSemTex0),
      SemanticPair("TEXCOORD0", EAttrSemTex0),
      SemanticPair("TEXCOORD1", EAttrSemTex1),
      SemanticPair("TEXCOORD2", EAttrSemTex2),
      SemanticPair("TEXCOORD3", EAttrSemTex3),
      SemanticPair("TEXCOORD4", EAttrSemTex4),
      SemanticPair("TEXCOORD5", EAttrSemTex5),
      SemanticPair("TEXCOORD6", EAttrSemTex6),
      SemanticPair("TEXCOORD7", EAttrSemTex7),
      SemanticPair("TEXCOORD8", EAttrSemTex8),
      SemanticPair("TEXCOORD9", EAttrSemTex9),
   };
   
   EAttribSemantic GetSemantic(const char* txt) {
      if (txt == 0)
         return EAttrSemNone;
      for(SemanticPair sem : semantics_map) {
         if (strcmp(txt, sem.name) == 0)
            return sem.semantic;
      }
      return EAttrSemNone;
   }

   void FillStructure(sgAttribute &result, const M4::HLSLType& type, M4::HLSLRoot* root) {
      if (type.baseType == M4::HLSLBaseType_UserDefined) {
         M4::HLSLStruct* structDeclaration = FindStruct(root, type.typeName);
         M4::HLSLStructField* field = structDeclaration->field;
         while (field != NULL)
         {
            if (field->type.baseType == M4::HLSLBaseType_UserDefined) {
               FillStructure(result, field->type, root);
            } else {
               result.names.push_back(field->name);
               result.sematic.push_back(GetSemantic(field->semantic));
            }
            field = field->nextField;
         }
      }
   }
   
   sgAttribute GetAttribute(sgContextPtr context, const char* entry, bool& haveMoreOneColor) {
      sgAttribute result;
      M4::HLSLRoot *root = context->current_context->tree.GetRoot();
      M4::HLSLFunction* entryFunction = FindFunction(root, entry);
      if (entryFunction == 0)
         return result;
      haveMoreOneColor = false;
      M4::HLSLArgument* argument = entryFunction->argument;
      int colorSemanticCount = 0;
      while (argument != NULL)
      {
         if (argument->type.baseType == M4::HLSLBaseType_UserDefined) {
            FillStructure(result, argument->type, root);
         } else {
            result.names.push_back(argument->name);
            EAttribSemantic semantic = GetSemantic(argument->semantic);
            result.sematic.push_back(semantic);
         }
         argument = argument->nextArgument;
      }
      if(entryFunction->returnType.baseType == M4::HLSLBaseType_Float4) {
         return result;
      } else if (entryFunction->returnType.baseType == M4::HLSLBaseType_UserDefined) {
         M4::HLSLStruct* structDeclaration = FindStruct(root, entryFunction->returnType.typeName);
         M4::HLSLStructField* field = structDeclaration->field;
         while (field != NULL)
         {
            EAttribSemantic semantic = GetSemantic(field->semantic);
            if (semantic >= EAttrSemColor0 && semantic <= EAttrSemColor3) {
               ++colorSemanticCount;
            }
            field = field->nextField;
         }
         haveMoreOneColor = colorSemanticCount > 1;
      }
      return result;
   }
}

std::string GetGLSLShaderFrag(sgContextPtr context, const char* entry, const char* shader, GLSLTarget version) {
   if (!___inited()) {
      throw InitException("Cannot init hlsl2glsl");
   }
   ShHandle handle = Hlsl2Glsl_ConstructCompiler(EShLangFragment);
   
   return GetShader(context, handle, entry, shader, version);
}

std::string GetGLSLShaderVert(sgContextPtr context, const char* entry, const char* shader, GLSLTarget version) {
   if (!___inited()) {
      throw InitException("Cannot init hlsl2glsl");
   }
   ShHandle handle = Hlsl2Glsl_ConstructCompiler(EShLangVertex);
   return GetShader(context, handle, entry, shader, version);
}

std::string GetGLSLShader(sgContextPtr context, const char* entry, const char* shader, sgShaderType type, GLSLTarget version){
   switch (type) {
      case sgShaderTypeFrag:
         return GetGLSLShaderFrag(context, entry, shader, version);
         break;
      case sgShaderTypeVertex:
         return GetGLSLShaderVert(context, entry, shader, version);
         break;
      default:
         throw new std::bad_function_call();
         break;
   }
}

void GLSLShaderDeinit() {
   ___deinit();
}
