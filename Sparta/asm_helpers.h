#pragma once

extern "C"
{
	unsigned short _get_es_selector();
	unsigned short _get_cs_selector();
	unsigned short _get_ss_selector();
	unsigned short _get_ds_selector();
	unsigned short _get_fs_selector();
	unsigned short _get_gs_selector();
	unsigned short _get_ldtr_selector();
	unsigned short _get_tr_selector();
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

	inline SegmentSelectors get_segment_selectors()
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
}
