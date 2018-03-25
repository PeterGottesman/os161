/*
 * All the contents of this file are overwritten during automated
 * testing. Please consider this before changing anything in this file.
 */

#include <types.h>
#include <lib.h>
#include <clock.h>
#include <thread.h>
#include <synch.h>
#include <test.h>
#include <kern/test161.h>
#include <spinlock.h>

#define NTHREADS 12
#define CREATELOOPS 12
#define NREADLOOPS 150
#define NWRITELOOPS 15

static struct semaphore *donesem;
static struct rwlock *testrwlock;
static volatile unsigned long val;

void rwtestthread(void *, unsigned long );
void writethread(void *, unsigned long );

/* ********** */
/* Unit tests */
/* ********** */

void rwtestthread(void *junk, unsigned long num)
{
	(void)junk;
	(void)num;

	unsigned i, valstore;
	
	random_yielder(4);

	rwlock_acquire_read(testrwlock);
	
	valstore = val;	
	
	for (i = 0; i < NREADLOOPS; i++)
	{
		random_yielder(4);
		KASSERT(valstore == val);
	}

	rwlock_release_read(testrwlock);
	V(donesem);
}

void writethread(void *junk, unsigned long num)
{
	(void)junk;
	(void)num;
	rwlock_acquire_write(testrwlock);
	random_yielder(4);
	val = num;
	rwlock_release_write(testrwlock);
	V(donesem);
}


/*
 * Tests core reader-writer lock functionality
 * by reading and writing shared state.
 * Start a bunch of reader threads
 * Reader threads read value
 * Simultaneously, main thread modifies value
 * Reader threads should not see any change from when they acquired read lock
 */ 
int rwtest(int nargs, char **args) {
	(void)nargs;
	(void)args;

	int i, result;
	
 	kprintf_n("rwt1 starting...\n");
	for (i = 0; i < CREATELOOPS; i++)
	{
		testrwlock = rwlock_create("testrwlock");
		if (testrwlock == NULL)
		{
			panic("rwt1: failed to create testrwlock\n");
		}

		donesem = sem_create("donesem", 0);
		if (donesem == NULL)
		{
			panic("rwt1: sem_create failed on donesem");
		}

		if (i != CREATELOOPS-1)
		{
			rwlock_destroy(testrwlock);
			sem_destroy(donesem);
		}
	}

	rwlock_acquire_write(testrwlock);
	val = 10;
	rwlock_release_write(testrwlock);
	
	for (i = 0; i < NTHREADS; i++)
	{
		result = thread_fork("rwtest", NULL, rwtestthread, NULL, val);
		if (result)
		{
			panic("rwt1: reader fork failed: %s\n", strerror(result));
		}
		
		result = thread_fork("writetest", NULL, writethread, NULL, i);
		if (result)
		{
			panic("rwt1: write fork failed: %s\n", strerror(result));
		}
	}

	for (i = 0; i < NWRITELOOPS; i++)
	{
		rwlock_acquire_write(testrwlock);
		val = i;
		rwlock_release_write(testrwlock);
	}

	for (i = 0; i < NTHREADS*2; i++)
	{
		P(donesem);
	}

	rwlock_destroy(testrwlock);
	sem_destroy(donesem);

	success(TEST161_SUCCESS, SECRET, "rwt1");

	return 0;
}

/*
 * Error handling, panic on success
 * Release write when not held
 */ 
int rwtest2(int nargs, char **args) {
	(void)nargs;
	(void)args;

	testrwlock = rwlock_create("testrwlock");
	if (testrwlock == NULL)
	{
		panic("rwt2: rwlock_create failed");
	}
	kprintf("rwt2 should panic now");
	rwlock_release_write(testrwlock);

	success(TEST161_FAIL, SECRET, "rwt2");

	return 0;
}

/*
 * Tests reader-writer lock error handling.
 * Panic on success.
 * Release reader when there are none
 */ 
int rwtest3(int nargs, char **args) {
	(void)nargs;
	(void)args;


	testrwlock = rwlock_create("testrwlock");
	if (testrwlock == NULL)
	{
		panic("rwt3: rwlock_create failed");
	}
	/* kprintf("rwt3 should panic now\n"); */
	secprintf(SECRET, "Should panic...", "lt2");
	rwlock_release_read(testrwlock);

	success(TEST161_SUCCESS, SECRET, "rwt3");

	return 0;
}

/*
 * Tests reader-writer lock error handling.
 * Panic on success.
 */
int rwtest4(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt4 unimplemented\n");
	success(TEST161_FAIL, SECRET, "rwt4");

	return 0;
}

/*
 * Tests reader-writer lock error handling.
 * Panic on success.
 */ 
int rwtest5(int nargs, char **args) {
	(void)nargs;
	(void)args;

	kprintf_n("rwt5 unimplemented\n");
	success(TEST161_FAIL, SECRET, "rwt5");

	return 0;
}
