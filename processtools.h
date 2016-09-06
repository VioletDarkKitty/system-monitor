#ifndef PROCESSTOOLS
#define PROCESSTOOLS
#include <QString>
#include <proc/readproc.h>

namespace processTools {
QString getProcessName(proc_t* p);
double calculateCPUPercentage(proc_t* before, proc_t* after, unsigned long long &cpuTime);
QString getProcessCmdline(pid_t pid);
QString getProcessStartDate(unsigned long long start_time);
}

#endif // PROCESSTOOLS

