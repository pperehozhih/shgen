#include "shgen_compiler_helper.h"
#include <iostream>

int main(int argc, char** argv) {
   try {
      std::string inputfile;
      std::string ouputdir;
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
                  ouputdir = argv[index + 1];
                  break;
               default:
                  break;
            }
         }
         index++;
      }
      shgen::compiler::GeneratorConfigWriterImpl generator_delegate;
      shgen::compiler::Generator generator(ouputdir);
      generator.SetGenerator(&generator_delegate);
      generator.ImportFXFile(inputfile);
      generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeGLES);
      generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeGL);
      generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeGL2);
      generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeGL3);
      generator.AddOutputFormat(shgen::compiler::Generator::GeneratorTypeMetal);
      generator.Generate();
      return 0;
   } catch (std::string ex) {
      std::cerr << ex;
      return -1;
   }
}
