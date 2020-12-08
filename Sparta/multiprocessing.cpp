#include "multiprocessing.h"

size_t multiprocessing::get_processor_count()
{
    return ::KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
}

size_t multiprocessing::get_current_processor_id()
{
    return ::KeGetCurrentProcessorNumber();
}
