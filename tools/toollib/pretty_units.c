#include "lib.h"
#include "common.h"


#ifdef __cplusplus
extern "C"
{
#endif

static long print_pretty_units(long secs, long unit_secs, const char* unit_name)
{
    int count = secs / unit_secs;
    if (count) {
        dbg_printf(-1, "%d %s%s", count, unit_name, count > 1? "s":"");
        secs %= unit_secs;
        if (secs) {
            dbg_printf(-1, ", ");
        }
    }

    return secs;
}


void print_pretty_seconds(long seconds)
{
    seconds = print_pretty_units(seconds, 365*24*60*60, "year");
    seconds = print_pretty_units(seconds, 24*60*60, "day");
    seconds = print_pretty_units(seconds, 60*60, "hour");
    seconds = print_pretty_units(seconds, 60, "minute");
    if (seconds)
        dbg_printf(-1, "%ld seconds", seconds);
}

#ifdef __cplusplus
}
#endif
