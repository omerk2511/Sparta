#include "intel.h"
#include "asm_helpers.h"

auto intel::get_system_segment_base(unsigned short segment_selector, unsigned long long gdt_base) -> unsigned long long
{
	if (segment_selector == 0 || (segment_selector >> 3) == 0)
	{
		return 0;
	}

	auto descriptor = reinterpret_cast<intel::SystemSegmentDescriptor*>(
		gdt_base + (segment_selector >> 3) * sizeof(intel::SegmentDescriptor));

	return (descriptor->base_0 | (descriptor->base_1 << 24) | (descriptor->base_2 << 32));
}

auto intel::get_segment_access_rights(unsigned short segment_selector) -> SegmentAccessRights
{
	intel::SegmentAccessRights access_rights = { 0 };

	if (segment_selector)
	{
		access_rights.raw = asm_helpers::get_segment_access_rights(segment_selector).raw >> 8;
	}
	else
	{
		access_rights.segment_unusable = 1;
	}

	return access_rights;
}
