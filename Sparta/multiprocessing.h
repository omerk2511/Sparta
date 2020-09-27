#pragma once

#include <ntddk.h>

#include "new.h"
#include "unique_pointer.h"

namespace multiprocessing
{
	template<typename Ret>
	struct CallbackResults
	{
		size_t processor_count;
		UniquePointer<Ret[]> return_values;
	};

	template<typename Ret, typename... Args>
	CallbackResults<Ret> execute_callback_in_each_processor(Ret (*callback)(Args...), Args... args)
	{
		auto processor_count = ::KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
		auto return_values = new (PagedPool) Ret[processor_count];

		CallbackResults<Ret> callback_results = {
			processor_count = processor_count,
			return_values = return_values
		};

		for (auto i = 0u; i < processor_count; i++)
		{
			PROCESSOR_NUMBER processor_number = { 0 };
			auto status = ::KeGetProcessorNumberFromIndex(i, &processor_number);

			if (!NT_SUCCESS(status))
			{
				KdPrint(("[-] could not get the processor number of processor %d\n", i));
				continue;
			}

			GROUP_AFFINITY old_affinity = { 0 };

			GROUP_AFFINITY new_affinity = { 0 };
			new_affinity.Group = processor_number.Group;
			new_affinity.Mask = 1ull << processor_number.Number;

			::KeSetSystemGroupAffinityThread(&new_affinity, &old_affinity);

			KdPrint(("[*] executing a callback function on processor %d in group %d\n",
				processor_number.Number, processor_number.Group));

			auto ret = callback(args...);
			return_values[i] = ret;

			::KeRevertToUserGroupAffinityThread(&old_affinity);
		}

		return callback_results;
	}

	template<typename... Args>
	void execute_callback_in_each_processor(void (*callback)(Args...), Args... args)
	{
		auto processor_count = ::KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);

		for (auto i = 0u; i < processor_count; i++)
		{
			PROCESSOR_NUMBER processor_number = { 0 };
			auto status = ::KeGetProcessorNumberFromIndex(i, &processor_number);

			if (!NT_SUCCESS(status))
			{
				KdPrint(("[-] could not get the processor number of processor %d\n", i));
				continue;
			}

			GROUP_AFFINITY old_affinity = { 0 };

			GROUP_AFFINITY new_affinity = { 0 };
			new_affinity.Group = processor_number.Group;
			new_affinity.Mask = 1ull << processor_number.Number;

			::KeSetSystemGroupAffinityThread(&new_affinity, &old_affinity);

			KdPrint(("[*] executing a callback function on processor %d in group %d\n",
				processor_number.Number, processor_number.Group));

			callback(args...);

			::KeRevertToUserGroupAffinityThread(&old_affinity);
		}
	}
}
