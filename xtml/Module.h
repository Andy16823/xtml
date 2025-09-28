#pragma once
#include "FunctionRegistry.h"

class Module
{
	public:
	virtual void RegisterFunctions(FunctionRegistry& registry) = 0;
	virtual ~Module() = default;
};