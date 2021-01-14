#include "multiprocessing.h"

auto multiprocessing::get_processor_count() -> size_t
{
    return ::KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
}

auto multiprocessing::get_current_processor_id() -> size_t
{
    return ::KeGetCurrentProcessorNumber();
}
