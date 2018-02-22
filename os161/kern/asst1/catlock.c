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

#define bool	int
#define true	1
#define false	0


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

bool bowl1_occupied, bowl2_occupied;
enum Eater {NONE = 0, CAT = 1, MOUSE = 2};
enum Eater currentEater = NONE;

static struct lock *bowl1, *bowl2, *currentEaterOccupied;

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
    
    if (currentEaterOccupied == NULL) {
        currentEaterOccupied = lock_create("currentEaterOccupied");
        if (currentEaterOccupied == NULL) {
            panic("currentEaterOccupied: lock_create failed\n");
        }
    }
    
    currentEater = NONE;
    bowl1_occupied = false;
    bowl2_occupied = false;
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
    // (void) catnumber;

    int i;

    for (i = 0; i < NITERATIONS; i++) {
        while(1) {
            bool eaten = false;
            
            lock_acquire(bowl1);
            lock_acquire(currentEaterOccupied);
            
            if(!bowl1_occupied && currentEater != MOUSE) {
                bowl1_occupied = true;
                if(currentEater == NONE) currentEater = CAT;
                
                lock_release(currentEaterOccupied);
                lock_release(bowl1); 
                
                lock_eat("cat", catnumber, 1, i);
                eaten = true;
                
                lock_acquire(bowl1);
                lock_acquire(bowl2);
                lock_acquire(currentEaterOccupied);
                
                if(bowl2_occupied == false) {
                    kprintf("empty\n");
                    currentEater = NONE;
                }
                bowl1_occupied = false;
            }
            
            lock_release(currentEaterOccupied);
            lock_release(bowl2);
            lock_release(bowl1);
            
            lock_acquire(bowl2);
            lock_acquire(currentEaterOccupied);
            
            if(!bowl2_occupied && currentEater != MOUSE) {
                bowl2_occupied = true;
                if(currentEater == NONE) currentEater = CAT;
                
                lock_release(currentEaterOccupied);
                lock_release(bowl2);
                
                lock_eat("cat", catnumber, 2, i);
                eaten = true;
                
                lock_acquire(bowl1);
                lock_acquire(bowl2);
                lock_acquire(currentEaterOccupied);
                
                if(bowl1_occupied == false) {
                    kprintf("empty\n");
                    currentEater = NONE;
                }
                bowl2_occupied = false;
            }
            lock_release(currentEaterOccupied);
            lock_release(bowl2);
            lock_release(bowl1);
            
            if(eaten) break;
        }
    }
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

    int i;

    for (i = 0; i < NITERATIONS; i++) {
        while(1) {
            bool eaten = false;
            
            lock_acquire(bowl1);
            lock_acquire(currentEaterOccupied);
            
            if(!bowl1_occupied && currentEater != CAT) {
                bowl1_occupied = true;
                if(currentEater == NONE) currentEater = MOUSE;
                
                lock_release(currentEaterOccupied);
                lock_release(bowl1); 
                
                lock_eat("mouse", mousenumber, 1, i);
                eaten = true;
                
                lock_acquire(bowl1);
                lock_acquire(bowl2);
                lock_acquire(currentEaterOccupied);
                
                if(bowl2_occupied == false) currentEater = NONE;
                bowl1_occupied = false;
            }
            
            lock_release(currentEaterOccupied);
            lock_release(bowl2);
            lock_release(bowl1);
            
            lock_acquire(bowl2);
            lock_acquire(currentEaterOccupied);
            
            if(!bowl2_occupied && currentEater != CAT) {
                bowl2_occupied = true;
                if(currentEater == NONE) currentEater = MOUSE;
                
                lock_release(currentEaterOccupied);
                lock_release(bowl2);
                
                lock_eat("mouse", mousenumber, 2, i);
                eaten = true;
                
                lock_acquire(bowl1);
                lock_acquire(bowl2);
                lock_acquire(currentEaterOccupied);
                
                if(bowl1_occupied == false) currentEater = NONE;
                bowl2_occupied = false;
            }
            lock_release(currentEaterOccupied);
            lock_release(bowl2);
            lock_release(bowl1);
            
            if(eaten) break;
        }
    }
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
