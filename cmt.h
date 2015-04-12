#ifndef MAT_CMT_H
#define MAT_CMT_H

#include <inttypes.h>

#define CMT_MAXTASKS 2

struct cmt_task
{
	uint16_t sp;	/**< stack pointer */
	uint16_t tp;	/**< task proc */
	uint16_t d;		/**< ticks left to sleep */
	uint16_t minsp; /**< min detected task's SP */
};

struct cmt_mutex
{
	uint8_t ot;		/**< owning task */
	uint8_t ac;		/**< acquire count */
};

void cmt_delay_ticks(uint16_t d);
uint8_t cmt_setup_task(void (*task_proc)(void), uint16_t task_sp);
void cmt_tick(uint8_t ms);
uint16_t cmt_minsp(uint8_t task_num);

uint8_t cmt_try_acquire(struct cmt_mutex* m);
void cmt_acquire(struct cmt_mutex* m);
void cmt_release(struct cmt_mutex* m);

#endif
