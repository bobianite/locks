#include <stdio.h>
#include "ticketlock.h"

int main()
{
	ticketlock_t mylock = TICKETLOCK_UNLOCKED;
	ticketlock_lock(&mylock);

	/* do critical magic here */

	ticketlock_unlock(&mylock);
	return 0;
}
