#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <mach/mach.h>

static void printtcpu(pid_t pid);

static void printtask(pid_t taskid, task_info_data_t taskinfo);

static void printthread(thread_t threadid, task_info_data_t taskinfo);

int main(int argc, char* argv[]) {
    if(argc > 1) {
        for(int i=1; i<argc; i++) {
            pid_t pid = atoi(argv[i]);
            printf("tid tidx time usage pcpu\n");
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

    kern_return_t kresult;

    task_info_count = TASK_INFO_MAX;
    
    kresult = task_info(port, TASK_BASIC_INFO, (task_info_t)taskinfo, &task_info_count);
    if (kresult == KERN_SUCCESS) {

        printtask(pid, taskinfo);

        thread_array_t thread_array;
        mach_msg_type_number_t thread_count;

        // get threads in the task
        kresult = task_threads(port, &thread_array, &thread_count);
        if (kresult != KERN_SUCCESS) {
            perror("Unable to get threads for task");
            return;
        }

        for (int j = 0; j < thread_count; j++) {
            printthread(thread_array[j], taskinfo);
        }

        kresult = vm_deallocate(mach_task_self(), (vm_offset_t)thread_array, thread_count * sizeof(thread_t));
        assert(kresult == KERN_SUCCESS);

    }

}


void printtask(pid_t pid, task_info_data_t taskinfo) {

    task_basic_info_t basic_taskinfo = (task_basic_info_t)taskinfo;

        const int tot_usec =
            basic_taskinfo->user_time.microseconds +
            basic_taskinfo->system_time.microseconds;
        
        float tot_seconds =
            basic_taskinfo->user_time.seconds +
            basic_taskinfo->system_time.seconds +
            tot_usec*1e-6;

        printf("%d %x %f %d %f\n",
               pid,
               pid,
               tot_seconds,
               1,
               0.0F);
               
}

void printthread(thread_t threadid, task_info_data_t taskinfo) {
    task_basic_info_t basic_info;

    thread_info_data_t threadinfo;
    
    mach_msg_type_number_t thread_info_count;

    thread_basic_info_t basic_info_th;

    kern_return_t kresult;

    basic_info = (task_basic_info_t)taskinfo;

    thread_info_count = THREAD_INFO_MAX;
    
    kresult = thread_info(threadid, THREAD_BASIC_INFO, (thread_info_t)threadinfo, &thread_info_count);
    if (kresult != KERN_SUCCESS) {
        perror("Unable to read thread info");
        return;
    }
    
    basic_info_th = (thread_basic_info_t)threadinfo;

    int tid = (int)threadid;

    int thread_usec = 0;
    float thread_sec = 0.0;
    int thread_cpu = 0;
    float cpu_pct = 0.0;

    if (!(basic_info_th->flags & TH_FLAGS_IDLE)) {
        thread_usec = basic_info_th->system_time.microseconds + basic_info_th->system_time.microseconds;
        thread_sec = basic_info_th->user_time.seconds + basic_info_th->system_time.seconds + thread_usec*1e-6F;

        thread_cpu = basic_info_th->cpu_usage;

        cpu_pct =(float)thread_cpu / (float)TH_USAGE_SCALE * 100.0;

    }

    printf("%d %x %f %d %f\n", tid, tid, thread_sec, thread_cpu, cpu_pct);

    
}
