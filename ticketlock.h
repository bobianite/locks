#ifndef _TICKETLOCK_H_
#define _TICKETLOCK_H_

typedef struct ticketlock_struct {
	int queue;
	int dequeue;
} ticketlock_t;
#define TICKETLOCK_LOCKED {.queue = 1, .dequeue = 0}
#define TICKETLOCK_UNLOCKED {.queue = 0, .dequeue = 0}

static inline void ticketlock_lock(ticketlock_t *t)
{
	__asm__ __volatile__("movl $1, %%eax\n\t"
			"lock xaddl %%eax, %[q]\n\t"
			"1:\n\t"
			"cmpl %%eax, %[d]\n\t"
			"jne 1b"
			: [q] "+m" (t->queue)
			: [d] "m" (t->dequeue)
			: "cc", "eax");
}

static inline void ticketlock_unlock(ticketlock_t *t)
{
	__asm__ __volatile__("lock incl %[d]"
			: [d] "+m" (t->dequeue)
			:
			: "cc");
}

#endif /* !_TICKETLOCK_H_ */
