#pragma once  
#include "Module.h"  

class ModuleStd : public Module  
{  
public:  
   void RegisterFunctions(FunctionRegistry& registry) override;  
};
