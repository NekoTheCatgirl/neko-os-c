#pragma once
#include <stdint.h>

#include "spinlock.h"

#define TASK_STACK_SIZE 0x4000  // 16KiB
#define MAX_CPUS        16

typedef enum {
	TASK_READY,
	TASK_RUNNING,
	TASK_BLOCKED,
	TASK_DEAD,
} task_state_t;

typedef struct task {
	uint64_t rsp;
	uint64_t cr3;
	task_state_t state;
	uint32_t id;
	uint8_t cpu_id;
	struct task *next;
	uint8_t stack[TASK_STACK_SIZE];
} task_t;

typedef struct {
	task_t *current;
	task_t *idle;
	spinlock_t lock;
} cpu_local_t;

typedef struct {
	task_t *head;
	task_t *tail;
	spinlock_t lock;
	uint32_t count;
} run_queue_t;

void scheduler_init();
void scheduler_enqueue(task_t* task);
task_t *scheduler_dequeue();
void scheduler_yield();
void scheduler_tick();

task_t *task_create(void (*entry)(void*), void* arg);
void task_exit();

cpu_local_t *cpu_local();
void cpu_enter_worker();