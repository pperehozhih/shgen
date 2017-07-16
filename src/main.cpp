#include "shgen_compiler_helper.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

template<typename Out>
void split(const std::string &s, char delim, Out result) {
   std::stringstream ss;
   ss.str(s);
   std::string item;
   while (std::getline(ss, item, delim)) {
      *(result++) = item;
   }
}


std::vector<std::string> split(const std::string &s, char delim) {
   std::vector<std::string> elems;
   split(s, delim, std::back_inserter(elems));
   return elems;
}

int main(int argc, char** argv) {
   try {
      std::string inputfile;
      std::string outputdir;
      unsigned outputformat = 0;
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
                  outputdir = argv[index + 1];
                  break;
               case 'g':
               case 'G':
               {
                  std::string arg(argv[index + 1]);
                  std::vector<std::string> tokens = split(arg, ',');
                  for (std::string token : tokens) {
                     if (token == "gles") {
                        outputformat |= shgen::compiler::Generator::GeneratorTypeGLES;
                     } else if (token == "gl") {
                        outputformat |= shgen::compiler::Generator::GeneratorTypeGL;
                     } else if (token == "gl2") {
                        outputformat |= shgen::compiler::Generator::GeneratorTypeGL2;
                     } else if (token == "gl3") {
                        outputformat |= shgen::compiler::Generator::GeneratorTypeGL3;
                     } else if (token == "metal") {
                        outputformat |= shgen::compiler::Generator::GeneratorTypeMetal;
                     } else if (token == "hlsl") {
                        outputformat |= shgen::compiler::Generator::GeneratorTypeHLSL;
                     }
                  }
               }
                  break;
               default:
                  break;
            }
         }
         index++;
      }
      if (outputformat == 0) {
         outputformat = 0xffffffff;
      }
      shgen::compiler::GeneratorConfigWriterImpl generator_delegate;
      shgen::compiler::Generator generator(outputdir);
      generator.SetGenerator(&generator_delegate);
      generator.ImportFXFile(inputfile);
      generator.SetOutputFormat(outputformat);
      generator.Generate();
      return 0;
   } catch (std::string ex) {
      std::cerr << ex;
      return -1;
   }
}
