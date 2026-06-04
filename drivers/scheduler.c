#include "scheduler.h"

#include "alloc.h"
#include "klogf.h"
#include "lapic.h"
#include "mem.h"
#include "x86.h"

extern void scheduler_context_switch(task_t *old, task_t *new);

static run_queue_t run_queue = { nullptr, nullptr, SPINLOCK_INIT, 0 };
static cpu_local_t cpu_locals[MAX_CPUS];
static uint32_t next_task_id = 0;

static void task_trampoline() {
	// r12 = entry, r13 = arg - set up by task_create
	// extract them and call
	void (*entry)(void*);
	void* arg;

	__asm__ volatile(
		"mov %%r12, %0\n"
		"mov %%r13, %1\n"
		: "=r"(entry), "=r"(arg)
	);

	entry(arg);
	task_exit();
}

static void idle_task(void* arg) {
	(void)arg;
	for (;;)
		__asm__ volatile("pause");
}

void scheduler_init() {
	memset(cpu_locals, 0, sizeof(cpu_locals));

	// Create idle task for each cpu
	for (int i = 0; i < MAX_CPUS; i++) {
		cpu_locals[i].current = nullptr;
		cpu_locals[i].idle = task_create(idle_task, nullptr);
	}

	klog(LOG_INFO, "sizeof task_t: %u", sizeof(task_t));
	klog(LOG_INFO, "tasks needed: %u, total: %u bytes", 24, 24 * sizeof(task_t));

	klog(LOG_INFO, "Scheduler initialized");
}

void scheduler_enqueue(task_t* task) {
	task->state = TASK_READY;
	task->next = nullptr;
	
	spinlock_acquire(&run_queue.lock);
	if (!run_queue.tail) {
		run_queue.head = task;
		run_queue.tail = task;
	} else {
		run_queue.tail->next = task;
		run_queue.tail = task;
	}
	run_queue.count++;
	spinlock_release(&run_queue.lock);
}

task_t* scheduler_dequeue() {
	spinlock_acquire(&run_queue.lock);
	task_t *task = run_queue.head;
	if (task) {
		run_queue.head = task->next;
		if (!run_queue.head)
			run_queue.tail = nullptr;
		run_queue.count--;
		task->next = nullptr;
	}
	spinlock_release(&run_queue.lock);
	return task;
}

void scheduler_yield() {
	cpu_local_t *cpu = cpu_local();

	task_t *old = cpu->current;
	task_t *next = scheduler_dequeue();

	if (!next) {
		// nNothing to do - if we're already running something keep going
		if (old && old->state == TASK_RUNNING)
			return;
		next = cpu->idle;
	}

	if (old && old->state == TASK_RUNNING)
		scheduler_enqueue(old);

	next->state = TASK_RUNNING;
	next->cpu_id = lapic_id();
	cpu->current = next;

	scheduler_context_switch(old ? old : cpu->idle, next);
}

void scheduler_tick() {
	scheduler_yield();
}

task_t* task_create(void(* entry)(void*), void* arg) {
	task_t *task = kmalloc(sizeof(task_t));
	memset(task, 0, sizeof(task_t));

	klog(LOG_INFO, "task at 0x%x rsp=0x%x", task, task->rsp);

	task->id = __sync_fetch_and_add(&next_task_id, 1);
	task->state = TASK_READY;
	task->cpu_id = 0xFF;
	task->cr3 = x86_read_cr3();

	// Stack grows down, start at the top
	uint64_t *rsp = (uint64_t*)(task->stack + TASK_STACK_SIZE);

	// When the task returns from entry(), call task exit()
	*--rsp = (uint64_t)task_exit;

	// This is what scheduler_context_switch's `ret` jumps into.
	// We need a small trampoline that calls entry(arg)
	*--rsp = (uint64_t)task_trampoline;

	// scheduler_context_switch pops 6 registers.
	// We pass entry and arg via r12/r13 since they're callee-saved
	*--rsp = (uint64_t)0;		// r15
	*--rsp = (uint64_t)0;		// r14
	*--rsp = (uint64_t)arg;		// r13
	*--rsp = (uint64_t)entry;	// r12
	*--rsp = (uint64_t)0;		// rbx
	*--rsp = (uint64_t)0;		// rbp

	task->rsp = (uint64_t)rsp;

	klog(LOG_INFO, "task %d rsp=0x%x trampoline=0x%x", task->id, task->rsp, (uint64_t)task_trampoline);

	return task;
}

void task_exit() {
	cpu_local_t *cpu = cpu_local();
	klog(LOG_INFO, "Task %d exited on cpu %d", cpu->current->id, lapic_id());
	cpu->current->state = TASK_DEAD;
	scheduler_yield();
}

cpu_local_t* cpu_local() {
	return &cpu_locals[lapic_id()];
}

void cpu_enter_worker() {
	cpu_local_t* cpu = cpu_local();
	klog(LOG_INFO, "CPU %d entering worker", lapic_id());

	// Save current rsp into idle task so context switch back works
	uint64_t rsp;
	__asm__ volatile("mov %%rsp, %0" : "=r"(rsp));
	cpu->idle->rsp = rsp;
	cpu->current = cpu->idle;

	//__asm__ volatile("sti");

	klog(LOG_INFO, "CPU %d starting loop", lapic_id());
	while (1) {
		task_t* task = scheduler_dequeue();
		if (task) {
			klog(LOG_INFO, "CPU %d picked up task %d", lapic_id(), task->id);
			task->state  = TASK_RUNNING;
			task->cpu_id = lapic_id();
			cpu->current = task;
			scheduler_context_switch(cpu->idle, task);
			klog(LOG_INFO, "CPU %d returned from task", lapic_id());
		} else {
			__asm__ volatile("pause");
		}
	}
}
