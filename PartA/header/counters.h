#define _GNU_SOURCE
#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <inttypes.h>
#include <time.h>

struct eventcount{
	struct perf_event_attr pe;
	int fd;
	char name[10];
};

struct eventcount e[4];
//FILE *fp = fopen("anyfile","a");;
unsigned long long pagefaults,l1dataread,tlb,lldataread;
clock_t start;


static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,int cpu, int group_fd, unsigned long flags)
{
    int ret;

    ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                    group_fd, flags);
    return ret;
}

void initilize_events(struct eventcount* e)
{
	memset(&e[0].pe, 0, sizeof(struct perf_event_attr));
	e[0].pe.type = PERF_TYPE_SOFTWARE;
    e[0].pe.size = sizeof(struct perf_event_attr);
    e[0].pe.config = PERF_COUNT_SW_PAGE_FAULTS;
    e[0].pe.disabled = 1;
    e[0].pe.exclude_kernel = 1;
    e[0].pe.exclude_hv = 1;
    e[0].fd = perf_event_open(&e[0].pe, 0, -1, -1, 0);
    if (e[0].fd == -1) {
        fprintf(stderr, "Error opening leader of pagefaults %llx\n", e[0].pe.config);
        //exit(EXIT_FAILURE);
    }

    memset(&e[1].pe, 0, sizeof(struct perf_event_attr));
    e[1].pe.type = PERF_TYPE_HW_CACHE;
    e[1].pe.size = sizeof(struct perf_event_attr);
    e[1].pe.config = PERF_COUNT_HW_CACHE_L1D | PERF_COUNT_HW_CACHE_OP_READ << 8 | PERF_COUNT_HW_CACHE_RESULT_MISS << 16;
    e[1].pe.disabled = 1;
    e[1].pe.exclude_kernel = 1;
    e[1].pe.exclude_hv = 1;
    e[1].fd = perf_event_open(&e[1].pe, 0, -1, -1, 0);
    if (e[0].fd == -1) {
        fprintf(stderr, "Error opening leader L1D %llx\n", e[1].pe.config);
        //exit(EXIT_FAILURE);
    }

    memset(&e[2].pe, 0, sizeof(struct perf_event_attr));
    e[2].pe.type = PERF_TYPE_HW_CACHE;
    e[2].pe.size = sizeof(struct perf_event_attr);
    e[2].pe.config = PERF_COUNT_HW_CACHE_LL | PERF_COUNT_HW_CACHE_OP_READ << 8 | PERF_COUNT_HW_CACHE_RESULT_MISS << 16;
    e[2].pe.disabled = 1;
    e[2].pe.exclude_kernel = 1;
    e[2].pe.exclude_hv = 1;
    e[2].fd = perf_event_open(&e[2].pe, 0, -1, -1, 0);
    if (e[2].fd == -1) {
        fprintf(stderr, "Error opening leader of LL %llx\n", e[2].pe.config);
        //exit(EXIT_FAILURE);
    }

    memset(&e[3].pe, 0, sizeof(struct perf_event_attr));
    e[3].pe.type = PERF_TYPE_HW_CACHE;
    e[3].pe.size = sizeof(struct perf_event_attr);
    e[3].pe.config = PERF_COUNT_HW_CACHE_DTLB | PERF_COUNT_HW_CACHE_OP_READ << 8 | PERF_COUNT_HW_CACHE_RESULT_MISS << 16;
    e[3].pe.disabled = 1;
    e[3].pe.exclude_kernel = 1;
    e[3].pe.exclude_hv = 1;
    e[3].fd = perf_event_open(&e[3].pe, 0, -1, -1, 0);
    if (e[3].fd == -1) {
        fprintf(stderr, "Error opening leader of DTLB %llx\n", e[3].pe.config);
        //exit(EXIT_FAILURE);
    }
}

void start_counters()
{
	initilize_events(e);
    start = clock();
    ioctl(e[0].fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(e[1].fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(e[2].fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(e[3].fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(e[0].fd, PERF_EVENT_IOC_ENABLE, 0);
    ioctl(e[1].fd, PERF_EVENT_IOC_ENABLE, 0);
    ioctl(e[2].fd, PERF_EVENT_IOC_ENABLE, 0);
    ioctl(e[3].fd, PERF_EVENT_IOC_ENABLE, 0);
}

void stop_counters()
{
    ioctl(e[0].fd, PERF_EVENT_IOC_DISABLE, 0);
    ioctl(e[1].fd, PERF_EVENT_IOC_DISABLE, 0);
    ioctl(e[2].fd, PERF_EVENT_IOC_DISABLE, 0);
    ioctl(e[3].fd, PERF_EVENT_IOC_DISABLE, 0);

    start = clock()-start;
    double time_taken = ((double)start);

    read(e[0].fd, &pagefaults, sizeof(long long));
    read(e[1].fd, &l1dataread, sizeof(long long));
    read(e[2].fd, &lldataread, sizeof(long long));
    read(e[3].fd, &tlb, sizeof(long long));

    //fprintf(fp,"ijk,%lld,%lld,%lld,%lld,%f\n",pagefaults,l1dataread,lldataread,tlb,time_taken);

    cout<<"Pagefaults : "<<pagefaults<<endl;
    cout<<"L1 data read Miss: "<<l1dataread<<endl;
    cout<<"LL data read Miss: "<<lldataread<<endl;
    cout<<"TLB miss : "<<tlb<<endl;
    cout<<"Execution Time : "<<time_taken<<endl;

}
