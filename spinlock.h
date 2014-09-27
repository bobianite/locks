/*
 * Stupid atomic spinlock for lolz
 *
 * Works on x86 or x64 architecture but use gcc, okay?
 *
 * Use "spinlock_t mylock;" to declare your spinlock
 *
 * Initialize your spinlock like "mylock = SPINLOCK_UNLOCKED;"
 * or "mylock = SPINLOCK_LOCKED;" accordingly
 *
 * Call "spinlock_lock(&mylock);" to try to acquire the lock
 * spinlock_lock will spin (infinite loop) and will not return until
 * lock is acquired
 *
 * Call "spinlock_unlock(&mylock);" to release the lock
 */

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

typedef char spinlock_t;
#define SPINLOCK_LOCKED 1
#define SPINLOCK_UNLOCKED 0

/* try to acquire lock using atomic xchg, spin/retry if fail */
static inline void spinlock_lock(spinlock_t *lock)
{
	__asm__ __volatile__("1:\tmovb $1, %%al\n\t"
			"xchgb %%al, %[lock]\n\t"
			"testb %%al, %%al\n\t"
			"jnz 1b"
			: [lock] "+m" (*lock)
			:
			: "cc", "al");
}

/* atomically release lock */
static inline void spinlock_unlock(spinlock_t *lock)
{
	__asm__ __volatile__("xorb %%al, %%al\n\t"
			"xchgb %%al, %[lock]"
			: [lock] "=m" (*lock)
			:
			: "cc", "al");
}

#endif /* !_SPINLOCK_H_ */
