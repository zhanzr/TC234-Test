#include <stdint.h>
#include "system.h"

/* gettimeofday() and getpid() */
int seed_from_timestamp_and_pid(uint32_t *seed) {
	Ifx_STM *StmBase = systime_GetStmBase();

	*seed = StmBase->TIM0.U;
	return 0;
}
