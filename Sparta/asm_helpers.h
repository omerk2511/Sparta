#pragma once

extern "C"
{
	auto _get_es_selector() -> unsigned short;
	auto _get_cs_selector() -> unsigned short;
	auto _get_ss_selector() -> unsigned short;
	auto _get_ds_selector() -> unsigned short;
	auto _get_fs_selector() -> unsigned short;
	auto _get_gs_selector() -> unsigned short;
	auto _get_ldtr_selector() -> unsigned short;
	auto _get_tr_selector() -> unsigned short;

	auto _get_segment_access_rights(unsigned long selector) -> unsigned long;

	auto _get_rsp() -> void*;

	void _vmexit_handler();
	void _restore_guest();
}

namespace asm_helpers
{
	struct SegmentSelectors
	{
		unsigned short es;
		unsigned short cs;
		unsigned short ss;
		unsigned short ds;
		unsigned short fs;
		unsigned short gs;
		unsigned short ldtr;
		unsigned short tr;
	};

	union SegmentAccessRights
	{
		unsigned long raw;

		struct
		{
			unsigned long : 8;
			unsigned long segment_type : 4;
			unsigned long descriptor_type : 1;
			unsigned long dpl : 2;
			unsigned long segment_present : 1;
			unsigned long undefined : 4;
			unsigned long avl : 1;
			unsigned long cs_64_bit_mode_active : 1;
			unsigned long default_operation_size : 1;
			unsigned long granularity : 1;
		};
	};

	inline auto get_segment_selectors() -> SegmentSelectors
	{
		return {
			_get_es_selector(),
			_get_cs_selector(),
			_get_ss_selector(),
			_get_ds_selector(),
			_get_fs_selector(),
			_get_gs_selector(),
			_get_ldtr_selector(),
			_get_tr_selector()
		};
	}

	inline auto get_segment_access_rights(unsigned long selector) -> SegmentAccessRights
	{
		SegmentAccessRights segment_access_rights = { _get_segment_access_rights(selector) };
		segment_access_rights.undefined = 0;
		return segment_access_rights;
	}

	inline auto get_rsp() -> void*
	{
		return _get_rsp();
	}
}
