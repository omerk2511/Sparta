#pragma once

#include <ntddk.h>

#include "memory.h"

namespace multiprocessing
{
	size_t get_processor_count();

	template<typename Ret>
	struct CallbackResults
	{
		size_t processor_count;
		UniquePointer<Ret[]> return_values;
	};

	template<typename Ret, typename... Args>
	CallbackResults<Ret> execute_callback_in_each_processor(Ret (*callback)(unsigned int, Args...), Args... args)
	{
		auto processor_count = get_processor_count();
		auto return_values = new (PagedPool) Ret[processor_count];

		if (!return_values)
		{
			KdPrint(("[-] could not allocate a return values buffer\n"));
			return { 0, nullptr };
		}

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

			auto ret = callback(i, args...);
			return_values[i] = ret;

			::KeRevertToUserGroupAffinityThread(&old_affinity);
		}

		return callback_results;
	}

	template<typename... Args>
	void execute_callback_in_each_processor(void (*callback)(unsigned int, Args...), Args... args)
	{
		auto processor_count = get_processor_count();

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

			callback(i, args...);

			::KeRevertToUserGroupAffinityThread(&old_affinity);
		}
	}
}
