#include "multiprocessing.h"

size_t multiprocessing::get_processor_count()
{
    return ::KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
}
