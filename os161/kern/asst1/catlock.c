/*
 * catlock.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use LOCKS/CV'S to solve the cat syncronization problem in 
 * this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>


/*
 * 
 * Constants
 *
 */

/*
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2

/*
 * Number of times cats and mice eat
 */

#define NITERATIONS 4

/*
 * Locks
 */

static struct lock *bowl1;
static struct lock *bowl2;

static volatile int cat_bowl1;
static volatile int cat_bowl2;
static volatile int mouse_bowl1;
static volatile int mouse_bowl1;


/*
 * 
 * Function Definitions
 * 
 */

void
init_locks(void) {
    if (bowl1 == NULL) {
        bowl1 = lock_create("bowl1");
        if (bowl1 == NULL) {
            panic("bowl1: lock_create failed\n");
        }
    }

    if (bowl2 == NULL) {
        bowl2 = lock_create("bowl2");
        if (bowl2 == NULL) {
            panic("bowl2: lock_create failed\n");
        }
    }
}

/* who should be "cat" or "mouse" */
static void
lock_eat(const char *who, int num, int bowl, int iteration) {
    kprintf("%s: %d starts eating: bowl %d, iteration %d\n", who, num,
            bowl, iteration);
    clocksleep(1);
    kprintf("%s: %d ends eating: bowl %d, iteration %d\n", who, num,
            bowl, iteration);
}

/*
 * catlock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS -
 *      1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
catlock(void * unusedpointer,
        unsigned long catnumber) {
    /*
     * Avoid unused variable warnings.
     */

    (void) unusedpointer;
    //(void) catnumber;

    /*int i;

    for (i = 0; i < NITERATIONS; i++) {
        if (cat_bowl1 == 1 && cat_bowl2 == 0) {
            lock_acquire(bowl1);
            lock_eat("cat", catnumber, 1, i);
            lock_release(bowl1);
        }
    }*/
}

/*
 * mouselock()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using locks/cv's.
 *
 */

static
void
mouselock(void * unusedpointer,
        unsigned long mousenumber) {
    /*
     * Avoid unused variable warnings.
     */

    (void) unusedpointer;
    //(void) mousenumber;

    /*int i;

    for (i = 0; i < NITERATIONS; i++) {
        lock_acquire(bowl1);
        lock_eat("mouse", mousenumber, 1, i);
        lock_release(bowl1);
    }*/
}

/*
 * catmouselock()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catlock() and mouselock() threads.  Change
 *      this code as necessary for your solution.
 */

int
catmouselock(int nargs,
        char ** args) {
    int index, error;

    /*
     * Avoid unused variable warnings.
     */

    (void) nargs;
    (void) args;

    init_locks();

    /*
     * Start NCATS catlock() threads.
     */

    for (index = 0; index < NCATS; index++) {

        error = thread_fork("catlock thread",
                NULL,
                index,
                catlock,
                NULL
                );

        /*
         * panic() on error.
         */

        if (error) {

            panic("catlock: thread_fork failed: %s\n",
                    strerror(error)
                    );
        }
    }

    /*
     * Start NMICE mouselock() threads.
     */

    for (index = 0; index < NMICE; index++) {

        error = thread_fork("mouselock thread",
                NULL,
                index,
                mouselock,
                NULL
                );

        /*
         * panic() on error.
         */

        if (error) {

            panic("mouselock: thread_fork failed: %s\n",
                    strerror(error)
                    );
        }
    }

    return 0;
}

/*
 * End of catlock.c
 */
