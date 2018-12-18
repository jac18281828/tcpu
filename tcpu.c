#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <mach/mach.h>

static void printtcpu(pid_t pid);

static void printthread(thread_t threadid, task_info_data_t taskinfo);

int main(int argc, char* argv[]) {
    if(argc > 1) {
        for(int i=1; i<argc; i++) {
            pid_t pid = atoi(argv[i]);
            printtcpu(pid);
        }

        exit(EXIT_SUCCESS);
    } else {
        printf("%s: pid...\n", argv[0]);
    }
    
}

void printtcpu(pid_t pid) {
    task_t port;
    task_for_pid(mach_task_self(), pid, &port);

    task_info_data_t taskinfo;
    mach_msg_type_number_t task_info_count;

    kern_return_t kr;

    task_info_count = TASK_INFO_MAX;
    kr = task_info(port, TASK_BASIC_INFO, (task_info_t)taskinfo, &task_info_count);
    if (kr == KERN_SUCCESS) {

        thread_array_t thread_array;
        mach_msg_type_number_t thread_count;

        uint32_t stat_thread = 0; // Mach threads

        // get threads in the task
        kr = task_threads(port, &thread_array, &thread_count);
        if (kr != KERN_SUCCESS) {
            perror("Unable to get threads for task");
            return;
        }
        if (thread_count > 0)
            stat_thread += thread_count;

        for (int j = 0; j < thread_count; j++) {
            printthread(thread_array[j], taskinfo);
        } 

    }

}


void printthread(thread_t threadid, task_info_data_t taskinfo) {
    task_basic_info_t basic_info;

    thread_info_data_t threadinfo;
    
    mach_msg_type_number_t thread_info_count;

    thread_basic_info_t basic_info_th;

    kern_return_t kr;

    basic_info = (task_basic_info_t)taskinfo;

    thread_info_count = THREAD_INFO_MAX;
    
    kr = thread_info(threadid, THREAD_BASIC_INFO, (thread_info_t)threadinfo, &thread_info_count);
    if (kr != KERN_SUCCESS) {
        perror("Unable to read thread info");
        return;
    }
    
    basic_info_th = (thread_basic_info_t)threadinfo;

    long tot_sec = 0;
    long tot_usec = 0;
    long tot_cpu = 0;

    if (!(basic_info_th->flags & TH_FLAGS_IDLE)) {
        tot_sec = tot_sec + basic_info_th->user_time.seconds + basic_info_th->system_time.seconds;
        tot_usec = tot_usec + basic_info_th->system_time.microseconds + basic_info_th->system_time.microseconds;
        tot_cpu = tot_cpu + basic_info_th->cpu_usage;

        long tid = (long)threadid;

        printf("%ld %lx %ld %ld %ld\n", tid, tid, tot_sec, tot_usec, tot_cpu);
    }
    
}

