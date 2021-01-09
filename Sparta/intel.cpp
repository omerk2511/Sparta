#include "intel.h"
#include "asm_helpers.h"

auto intel::get_system_segment_base(unsigned short segment_selector, unsigned long long gdt_base) -> unsigned long long
{
	if (segment_selector == 0 || (segment_selector >> 3) == 0)
	{
		return 0;
	}

	auto descriptor = reinterpret_cast<intel::SegmentDescriptor*>(
		gdt_base + (segment_selector >> 3) * sizeof(intel::SegmentDescriptor));

	if (descriptor->attr_0 & 0x10)
	{
		auto system_descriptor = reinterpret_cast<intel::SystemSegmentDescriptor*>(descriptor);
		return (system_descriptor->base_0 | (system_descriptor->base_1 << 24));
	}

	return (descriptor->base_0 | (descriptor->base_1 << 24));
}
