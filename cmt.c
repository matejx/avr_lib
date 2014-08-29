// ------------------------------------------------------------------
// --- cmt.c                                                    ---
// --- simple cooperative "on-sleep" multitasking                 ---
// ---                                 8.mar.2011, Matej Kogovsek ---
// ------------------------------------------------------------------

#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "cmt.h"

static volatile uint8_t cmt_curtask = 0;
static volatile struct cmt_task cmt_tasks[CMT_MAXTASKS];
static volatile uint8_t cmt_numtasks = 1;

// ------------------------------------------------------------------
// task switching is done here
// this should not be called with interrupts disabled
void cmt_delay_ticks(uint16_t d)
{
	asm(
		"push r2\n\t"
		"push r3\n\t"
		"push r4\n\t"
		"push r5\n\t"
		"push r6\n\t"
		"push r7\n\t"
		"push r8\n\t"
		"push r9\n\t"
		"push r10\n\t"
		"push r11\n\t"
		"push r12\n\t"
		"push r13\n\t"
		"push r14\n\t"
		"push r15\n\t"
		"push r16\n\t"
		"push r17\n\t"
		"push r28\n\t"
		"push r29\n\t"
	);
	cli();
	cmt_tasks[cmt_curtask].sp = SP;	// remember current task's SP
	cmt_tasks[cmt_curtask].d = d;	// and how long it wishes to sleep
	sei();
	uint8_t i = cmt_curtask;

	while(1) {
		wdt_reset();
		i =	(i + 1) % cmt_numtasks;
		cli();
		d = cmt_tasks[i].d;	// read d atomically
		sei();
		if( d == 0 ) { break; }	// found ready to run task
	}

	cli();
	cmt_curtask = i;
	SP = cmt_tasks[i].sp;	// restore stack pointer
	sei();

	uint16_t tp = cmt_tasks[i].tp;
	if( tp ) {
		cmt_tasks[i].tp = 0;
		asm("ijmp\n"::"z" (tp));
	}

	asm(
		"pop r29\n\t"
		"pop r28\n\t"
		"pop r17\n\t"
		"pop r16\n\t"
		"pop r15\n\t"
		"pop r14\n\t"
		"pop r13\n\t"
		"pop r12\n\t"
		"pop r11\n\t"
		"pop r10\n\t"
		"pop r9\n\t"
		"pop r8\n\t"
		"pop r7\n\t"
		"pop r6\n\t"
		"pop r5\n\t"
		"pop r4\n\t"
		"pop r3\n\t"
		"pop r2\n\t"
	);
}

// ------------------------------------------------------------------
// setup task, call this to setup all tasks before first sei()
uint8_t cmt_setup_task(void (*task_proc)(void), uint16_t task_sp)
{
	cmt_tasks[0].minsp = -1;	// should be in cmt_init, but can as well be here
	cmt_tasks[0].tp = 0;

	if( cmt_numtasks >= CMT_MAXTASKS ) return 0;

	cmt_tasks[cmt_numtasks].sp = task_sp;
	cmt_tasks[cmt_numtasks].tp = (uint16_t)task_proc;
	cmt_tasks[cmt_numtasks].d = 0;	// ready to run
	cmt_tasks[cmt_numtasks].minsp = -1;

	return ++cmt_numtasks;
}

// ------------------------------------------------------------------
// should be called by a timer interrupt
void cmt_tick(uint8_t ms)
{
	// keep track of current task's min SP
	if( SP < cmt_tasks[cmt_curtask].minsp ) {
		cmt_tasks[cmt_curtask].minsp = SP;
	}

	// decrease all tasks' delay count
	uint8_t i;
	for( i = 0; i < cmt_numtasks; i++ ) {
		if( cmt_tasks[i].d > ms ) {
			cmt_tasks[i].d -= ms;
		} else {
			cmt_tasks[i].d = 0;
		}
	}
}

// ------------------------------------------------------------------
// returns the task's minimal detected stack pointer
#ifdef cmt_NEED_MINSP
uint16_t cmt_minsp(uint8_t task_num)
{
	if( task_num < cmt_numtasks ) {
		return cmt_tasks[task_num].minsp;
	}
	return 0;
}
#endif

#ifdef cmt_MUTEX_FUNC
// ------------------------------------------------------------------
// tries to acquire mutex
uint8_t cmt_try_acquire(struct cmt_mutex* m)
{
	if( (m->ot == cmt_curtask) || (m->ac == 0) ) {
		m->ot = cmt_curtask;
		m->ac++;
		return 1;
	}
	return 0;
}

// ------------------------------------------------------------------
// waits until mutex acquired
void cmt_acquire(struct cmt_mutex* m)
{
	while( !cmt_try_acquire(m) ) {
		cmt_delay_ticks(0);
	}
}

// ------------------------------------------------------------------
// releases mutex
void cmt_release(struct cmt_mutex* m)
{
	if( (m->ot == cmt_curtask) && (m->ac > 0) ) {
		m->ac--;
	}
}
#endif
