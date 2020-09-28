#pragma once

struct ProcessorContext
{
	void* vmxon_region;
	void* vmcs_region;
};
