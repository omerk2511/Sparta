#pragma once

#include <ntddk.h>

#include "kstd.h"
#include "memory.h"
#include "scope_guard.h"

namespace multiprocessing
{
	size_t get_processor_count();
	size_t get_current_processor_id();

	template<typename Ret>
	struct CallbackResults
	{
		size_t processor_count;
		kstd::UniquePointer<Ret[]> return_values;
	};

	template<typename Ret>
	struct BaseIpiContext
	{
		Ret* return_values;
	};

	template<>
	struct BaseIpiContext<void>
	{};

	template<typename Ret, typename Arg>
	struct IpiContext : BaseIpiContext<Ret>
	{
		Ret (*callback)(Arg);
		Arg arg;
	};

	template<typename Ret, typename Arg>
	ULONG_PTR GenericIpiHandler(ULONG_PTR context)
	{
		auto ipi_context = reinterpret_cast<IpiContext<Ret, Arg>*>(context);

		if constexpr (kstd::is_same<Ret, void>)
		{
			ipi_context->callback(ipi_context->arg);
		}
		else
		{
			ipi_context->return_values[get_current_processor_id()] = ipi_context->callback(ipi_context->arg);
		}

		return 0ull;
	}

	template<typename Ret, typename Arg>
	kstd::conditional_t<kstd::is_same<Ret, void>, void, CallbackResults<Ret>>
		execute_callback_in_each_processor(Ret (*callback)(Arg), Arg arg)
	{
		Ret* return_values{ nullptr };
		auto processor_count = get_processor_count();

		if constexpr (!kstd::is_same<Ret, void>)
		{
			return_values = new (PagedPool) Ret[processor_count];

			if (!return_values)
			{
				KdPrint(("[-] could not allocate a return values buffer\n"));
				return { 0, nullptr };
			}
		}
		else
		{
			UNREFERENCED_PARAMETER(return_values);
		}

		IpiContext<Ret, Arg> ipi_context;

		ipi_context.callback = callback;
		ipi_context.arg = arg;
		ipi_context.return_values = return_values;

		::KeIpiGenericCall(
			&GenericIpiHandler<Ret, Arg>,
			reinterpret_cast<ULONG_PTR>(&ipi_context)
		);

		if constexpr (!kstd::is_same<Ret, void>)
		{
			return {
				processor_count = processor_count,
				return_values = return_values
			};
		}
	}
}
