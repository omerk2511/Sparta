#include "intel.h"
#include "asm_helpers.h"

unsigned long long intel::get_system_segment_base(unsigned short segment_selector, unsigned long long gdt_base)
{
	if (!segment_selector)
		return 0;

	auto descriptor = reinterpret_cast<intel::SystemSegmentDescriptor*>(
		gdt_base + (segment_selector >> 3) * sizeof(intel::SegmentDescriptor));
	return descriptor->base_0 | (descriptor->base_0 << 24);
}

intel::SegmentAccessRights intel::get_segment_access_rights(unsigned long selector)
{
	intel::SegmentAccessRights access_rights = { 0 };

	if (selector)
	{
		access_rights.raw = asm_helpers::get_segment_access_rights(selector).raw >> 8;
	}
	else
	{
		access_rights.segment_unusable = 1;
	}

	return access_rights;
}
