/*
 * Atomic ticketlock-based spinlock for lolz, inspired by Linux kernel's
 * own fancier ticket lock
 *
 * Copyright (C) 2014 Bryance Oyang
 *
 * Works on x86 or x64 architecture but use gcc, okay? (b/c gcc inline
 * assembly for atomic operations and preprocessor things)
 *
 * Wat is ticket lock?
 * You own a store. Every customer gets a unique ticket, a number. When
 * their number is called, they come to you and do their business things
 * with you. When they're done, you call out the next number. All
 * customers not currently being served by you spin around in circles to
 * cure their boredom.
 *
 * Locks prevent race conditions. Unlike oldschool spinlocks, ticket
 * locks are fair, first-come first-serve locks. A thread trying to
 * acquire the lock is a customer trying to get a hold of you. Analogy
 * follows.
 */

/*
 * Usage notes:
 *
 * Use "ticketlock_t mylock = TICKETLOCK_UNLOCKED;" to declare and
 * initialize your spinlock
 *
 * Never modify your spinlock after initializing, use only the following
 * function calls:
 *
 * Call "ticketlock_lock(&mylock);" to try to acquire the lock
 * ticketlock_lock will spin (infinite loop), waiting in queue, and will
 * not return until lock is acquired
 *
 * Call "ticketlock_unlock(&mylock);" to release the lock when done
 *
 * Always make sure ticketlock_lock and ticketlock_unlock come as pairs!
 * Making a second call to ticketlock_lock from the same thread before
 * calling ticketlock_unlock will cause deadlock!
 */

#ifndef _TICKETLOCK_H_
#define _TICKETLOCK_H_

#undef asm
#define asm __asm__

typedef struct ticketlock_struct {
	__UINT32_TYPE__ queue;
	__UINT32_TYPE__ dequeue;
} ticketlock_t;

#define TICKETLOCK_UNLOCKED (ticketlock_t){.queue = 0, .dequeue = 0}

/*
 * try to acquire lock, if fail, spin until acquired
 * critical parts are atomic
 */
static inline void ticketlock_lock(ticketlock_t *t)
{
	asm volatile("movl $1, %%ecx\n\t"
			"lock xaddl %%ecx, %[q]\n\t"
			"1:\n\t"
			"lock cmpxchgl %%eax, %[d]\n\t"
			"cmpl %%eax, %%ecx\n\t"
			"pause\n\t"
			"jne 1b"
			: [q] "+m" (t->queue)
			: [d] "m" (t->dequeue)
			: "memory", "cc", "eax", "ecx");
}

/*
 * DO NOT USE
 *
 * this is for proof of concept only
 *
 * try to acquire lock once, if fail return nonzero, if succeed return 0
 *
 * this does not conform to 1st come 1st serve spirit of ticketlock so
 * use ticketlock_lock instead
 */
static inline __UINT32_TYPE__ ticketlock_trylock(ticketlock_t *t)
{
	register __UINT32_TYPE__ ret;

	asm volatile("lock cmpxchgl %%eax, %[q]\n\t"
			"lock cmpxchgl %%eax, %[d]\n\t"
			"jne 1f\n\t"
			"movl %%eax, %%ecx\n\t"
			"incl %%ecx\n\t"
			"lock cmpxchgl %%ecx, %[q]\n\t"
			"je 2f\n\t"
			"1:\n\t"
			"movl $-1, %[r]\n\t"
			"jmp 3f\n\t"
			"2:\n\t"
			"xorl %[r], %[r]\n\t"
			"3:"
			: [q] "+m" (t->queue), [r] "=r" (ret)
			: [d] "m" (t->dequeue)
			: "memory", "cc", "eax", "ecx");

	return ret;
}

/* release lock atomically */
static inline void ticketlock_unlock(ticketlock_t *t)
{
	asm volatile("lock incl %[d]"
			: [d] "+m" (t->dequeue)
			:
			: "memory", "cc");
}

#endif /* !_TICKETLOCK_H_ */
