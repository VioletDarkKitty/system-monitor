#include "processtools.h"
#include <string>
#include <unistd.h>
#include <fstream>
#include <QFileInfo>
#include <proc/sysinfo.h>
#include <time.h>

namespace processTools {
    /** exe_of() - Obtain the executable path a process is running
     * @pid: Process ID
     * @sizeptr: If specified, the allocated size is saved here
     * @lenptr: If specified, the path length is saved here
     * Returns the dynamically allocated pointer to the path,
     * or NULL with errno set if an error occurs.
    */
    // from http://stackoverflow.com/questions/24581908/c-lstat-on-proc-pid-exe
    char* exe_of(const pid_t pid, size_t *const sizeptr, size_t *const lenptr)
    {
        char   *exe_path = NULL;
        size_t  exe_size = 1024;
        ssize_t exe_used;
        char    path_buf[64];
        unsigned int path_len;

        path_len = snprintf(path_buf, sizeof path_buf, "/proc/%ld/exe", (long)pid);
        if (path_len < 1 || path_len >= sizeof path_buf) {
            errno = ENOMEM;
            return NULL;
        }

        while (1) {

            exe_path = (char*)malloc(exe_size);
            if (!exe_path) {
                errno = ENOMEM;
                return NULL;
            }

            exe_used = readlink(path_buf, exe_path, exe_size - 1);
            if (exe_used == (ssize_t)-1) {
                free(exe_path);
                return NULL;
            }

            if (exe_used < (ssize_t)1) {
                /* Race condition? */
                errno = ENOENT;
                free(exe_path);
                return NULL;
            }

            if (exe_used < (ssize_t)(exe_size - 1))
                break;

            free(exe_path);
            exe_size += 1024;
        }

        /* Try reallocating the exe_path to minimum size.
         * This is optional, and can even fail without
         * any bad effects. */
        {
            char *temp;

            temp = (char*)realloc(exe_path, exe_used + 1);
            if (temp) {
                exe_path = temp;
                exe_size = exe_used + 1;
            }
        }

        if (sizeptr)
            *sizeptr = exe_size;

        if (lenptr)
            *lenptr = exe_used;

        exe_path[exe_used] = '\0';
        return exe_path;
    }

    const std::vector<std::string> explode(const std::string& s, const char& c)
    {
        std::string buff{""};
        std::vector<std::string> v;

        for(auto n:s)
        {
            if(n != c) buff+=n; else
            if(n == c && buff != "") { v.push_back(buff); buff = ""; }
        }
        if(buff != "") v.push_back(buff);

        return v;
    }

    QString getProcessNameFromCmdLine(const pid_t pid)
    {
        std::string temp;
        try {
            std::fstream fs;
            fs.open("/proc/"+std::to_string((long)pid)+"/cmdline", std::fstream::in);
            fs >> temp;
            fs.close();
        } catch(std::ifstream::failure e) {
            return "FAILED TO READ PROC";
        }

        if (temp.size()<1) {
            return "";
        }
        // maintain linux paths
        std::replace(temp.begin(),temp.end(),'\\','/');
        return QFileInfo(QString::fromStdString(explode(temp,'\0')[0])).fileName();
    }

    QString getProcessCmdline(pid_t pid)
    {
        /// TODO: remove this awful code duplication!
        std::string temp;
        try {
            std::fstream fs;
            fs.open("/proc/"+std::to_string((long)pid)+"/cmdline", std::fstream::in);
            fs >> temp;
            fs.close();
        } catch(std::ifstream::failure e) {
            return "FAILED TO READ PROC";
        }

        if (temp.size()<1) {
            return "";
        }
        return QString::fromStdString(temp);
    }

    QString getProcessName(proc_t* p)
    {
        QString processName = "ERROR";
        char* temp = NULL;//exe_of(p->tid,NULL,NULL);
        // if exe_of fails here, it will be because permission is denied
        if (temp!=NULL) {
            processName = QFileInfo(temp).fileName();
            free(temp);
        } else {
            // next try to read from /proc/*//*cmdline
            processName = getProcessNameFromCmdLine(p->tid);
            if (processName=="") {
                // fallback on /proc/*//*stat program name value
                // bad because it is limited to 16 chars
                processName = p->cmd;
            }
        }
        return processName;
    }

    unsigned long long getTotalCpuTime()
    {
        // from https://github.com/scaidermern/top-processes/blob/master/top_proc.c#L54
        FILE* file = fopen("/proc/stat", "r");
        if (file == NULL) {
            perror("Could not open stat file");
            return 0;
        }

        char buffer[1024];
        unsigned long long user = 0, nice = 0, system = 0, idle = 0;
        // added between Linux 2.5.41 and 2.6.33, see man proc(5)
        unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guestnice = 0;

        char* ret = fgets(buffer, sizeof(buffer) - 1, file);
        if (ret == NULL) {
            perror("Could not read stat file");
            fclose(file);
            return 0;
        }
        fclose(file);

        sscanf(buffer,
               "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guestnice);

        // sum everything up (except guest and guestnice since they are already included
        // in user and nice, see http://unix.stackexchange.com/q/178045/20626)
        return user + nice + system + idle + iowait + irq + softirq + steal;
    }

    /**
     * @brief calculateCPUPercentage
     * @param before - old proc_t of the process
     * @param after - new proc_t of the process
     * @param cpuTime - cpuTime storage (will be modified!)
     * @return The cpu percentage of the process
     */
    double calculateCPUPercentage(proc_t* before, proc_t* after, unsigned long long &cpuTime)
    {
        /// TODO: this cpu calculation is wrong
        cpuTime = getTotalCpuTime() - cpuTime;
        unsigned long long processcpuTime = ((after->utime + after->stime)
                - (before->utime + before->stime));
        /// TODO: GSM has an option to divide by # cpus
        return (processcpuTime / (float)cpuTime) * 100.0 * sysconf(_SC_NPROCESSORS_CONF);
    }

    QString getProcessStartDate(unsigned long long start_time)
    {
        __time_t time = getbtime() + start_time/Hertz;
        struct tm *tm = localtime(&time);
        char date[255];
        strftime(date, sizeof(date), "%Ex %ER", tm);
        return QString(date);
    }

    QString getProcessStatus(proc_t* p)
    {
        switch(p->state) {
            case 'S':
                return "Sleeping";
            break;

            case 'R':
                return "Running";
            break;

            case 'Z':
                return "Zombie";
            break;

            default:
                return "Unknown state" + QString(p->state);
        }
    }
}
