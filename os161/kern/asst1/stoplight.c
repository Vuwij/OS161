/* 
 * stoplight.c
 *
 * 31-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
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
 * Number of cars created.
 */

#define NCARS 20


/*
 *
 * Function Definitions
 *
 */

static const char *directions[] = { "N", "E", "S", "W" };

static const char *msgs[] = {
        "approaching:",
        "region1:    ",
        "region2:    ",
        "region3:    ",
        "leaving:    "
};

/* use these constants for the first parameter of message */
enum { APPROACHING, REGION1, REGION2, REGION3, LEAVING };

static struct semaphore *mutex;

static volatile int NW_cars = 0;
static volatile int NE_cars = 0;
static volatile int SW_cars = 0;
static volatile int SE_cars = 0;


static void
message(int msg_nr, int carnumber, int cardirection, int destdirection)
{
        kprintf("%s car = %2d, direction = %s, destination = %s\n",
                msgs[msg_nr], carnumber,
                directions[cardirection], directions[destdirection]);
}

 
/*
 * gostraight()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
gostraight(unsigned long cardirection,
           unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */
        
        (void) cardirection;
        (void) carnumber;
}


/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnleft(unsigned long cardirection,
         unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) cardirection;
        (void) carnumber;
}


/*
 * turnright()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnright(unsigned long cardirection,
          unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) cardirection;
        (void) carnumber;
}

void
print_one_region(unsigned long carnumber, int source, int destination) 
{
    message(APPROACHING, carnumber, source, destination);
    message(REGION1, carnumber, source, destination); 
    message(LEAVING, carnumber, source, destination); 
}

void
print_two_regions(unsigned long carnumber, int source, int destination) 
{
    message(APPROACHING, carnumber, source, destination);
    message(REGION1, carnumber, source, destination); 
    message(REGION2, carnumber, source, destination); 
    message(LEAVING, carnumber, source, destination); 
}

void
print_three_regions(unsigned long carnumber, int source, int destination) 
{
    message(APPROACHING, carnumber, source, destination);
    message(REGION1, carnumber, source, destination); 
    message(REGION2, carnumber, source, destination); 
    message(REGION3, carnumber, source, destination); 
    message(LEAVING, carnumber, source, destination); 
}


/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
 
static
void
approachintersection(void * unusedpointer,
                     unsigned long carnumber)
{
    int source, destination;

    /*
     * Avoid unused variable and function warnings.
     */

    (void) unusedpointer;
    (void) gostraight;
    (void) turnleft;
    (void) turnright;

    /*
     * cardirection is set randomly.
     */

    source = random() % 4;        

    // can't have same value for starting and ending points
    do {
        destination = random() % 4;
    }
    while (destination == source);

    const char* src = directions[source];
    const char* dest = directions[destination];           

    if (strcmp(src,"N") == 0) {

        if (strcmp(dest,"W") == 0) {

            while (1) {
                P(mutex);
                if (NW_cars == 0){
                    NW_cars = 1;
                    break;
                }                        
                V(mutex);
            }                
            V(mutex);              

            print_one_region(carnumber, source, destination);

            P(mutex);
            NW_cars = 0;
            V(mutex);
        }                       

        if (strcmp(dest,"S") == 0) {

            while (1) {
                P(mutex);
                if (NW_cars == 0 && SW_cars == 0){
                    NW_cars = 1;
                    SW_cars = 1;
                    break;
                }                        
                V(mutex);
            }                
            V(mutex); 

            print_two_regions(carnumber, source, destination);               

            P(mutex);
            NW_cars = 0;
            SW_cars = 0;
            V(mutex); 
        }

        if (strcmp(dest,"E") == 0) {

            while (1) {
                P(mutex);
                if (NW_cars == 0 && SW_cars == 0 && SE_cars == 0){
                    NW_cars = 1;
                    SW_cars = 1;
                    SE_cars = 1;
                    break;
                }                        
                V(mutex);
            }                
            V(mutex); 

            print_three_regions(carnumber, source, destination);  

            P(mutex);
            NW_cars = 0;
            SW_cars = 0;
            SE_cars = 0;
            V(mutex); 
        }            
    }

    if (strcmp(src,"S") == 0) {

        if (strcmp(dest,"E") == 0) {

            while (1) {
                P(mutex);
                if (SE_cars == 0){
                    SE_cars = 1;
                    break;
                }                        
                V(mutex);
            }                
            V(mutex);   

            print_one_region(carnumber, source, destination);              

            P(mutex);
            SE_cars = 0;
            V(mutex);
        }                       

        if (strcmp(dest,"N") == 0) {

            while (1) {
                P(mutex);
                if (SE_cars == 0 && NE_cars == 0){
                    SE_cars = 1;
                    NE_cars = 1;
                    break;
                }                        
                V(mutex);
            }                
            V(mutex); 

            print_two_regions(carnumber, source, destination);  

            P(mutex);
            SE_cars = 0;
            NE_cars = 0;
            V(mutex); 
        }

        if (strcmp(dest,"W") == 0) {

            while (1) {
                P(mutex);
                if (SE_cars == 0 && NE_cars == 0 && NW_cars == 0){
                    SE_cars = 1;
                    NE_cars = 1;
                    NW_cars = 1;
                    break;
                }                        
                V(mutex);
            }                
            V(mutex); 

            print_three_regions(carnumber, source, destination);      

            P(mutex);
            SE_cars = 0;
            NE_cars = 0;
            NW_cars = 0;
            V(mutex); 
        }
    }

    if (strcmp(src,"E") == 0) {

        if (strcmp(dest,"N") == 0) {

            while (1) {
                P(mutex);
                if (NE_cars == 0){
                    NE_cars = 1;                        
                    break;
                }                        
                V(mutex);
            }                
            V(mutex); 

            print_one_region(carnumber, source, destination);  

            P(mutex);
            NE_cars = 0;
            V(mutex);
        }                       

        if (strcmp(dest,"W") == 0) {

            while (1) {
                P(mutex);
                if (NE_cars == 0 && NW_cars == 0){
                    NE_cars = 1; 
                    NW_cars = 1;
                    break;
                }                        
                V(mutex);
            }                
            V(mutex);    

            print_two_regions(carnumber, source, destination);

            P(mutex);
            NW_cars = 0;
            NE_cars = 0;
            V(mutex); 
        }

        if (strcmp(dest,"S") == 0) {

            while (1) {
                P(mutex);
                if (NE_cars == 0 && NW_cars == 0 && SW_cars == 0){
                    NE_cars = 1; 
                    NW_cars = 1;
                    SW_cars = 1;
                    break;
                }                        
                V(mutex);
            }                
            V(mutex); 

            print_three_regions(carnumber, source, destination);

            P(mutex);
            NW_cars = 0;
            NE_cars = 0;
            SW_cars = 0;
            V(mutex);                           
        }
    }

    if (strcmp(src,"W") == 0) {

        if (strcmp(dest,"S") == 0) {

            while (1) {
                P(mutex);
                if (SW_cars == 0){
                    SW_cars = 1;                         
                    break;
                }                        
                V(mutex);
            }                
            V(mutex); 

            print_one_region(carnumber, source, destination);                

            P(mutex);
            SW_cars = 0;
            V(mutex);
        }                       

        if (strcmp(dest,"E") == 0) {

            while (1) {
                P(mutex);
                if (SW_cars == 0 && SE_cars == 0){
                    SW_cars = 1; 
                    SE_cars = 1;                        
                    break;
                }                        
                V(mutex);
            }                
            V(mutex); 

            print_two_regions(carnumber, source, destination);   

            P(mutex);
            SW_cars = 0;
            SE_cars = 0;
            V(mutex); 
        }

        if (strcmp(dest,"N") == 0) {

            while (1) {
                P(mutex);
                if (SW_cars == 0 && SE_cars == 0 && NE_cars == 0){
                    SW_cars = 1; 
                    SE_cars = 1;       
                    NE_cars = 1;
                    break;
                }                        
                V(mutex);
            }                
            V(mutex); 

            print_three_regions(carnumber, source, destination);  

            P(mutex);
            NE_cars = 0;
            SW_cars = 0;
            SE_cars = 0;
            V(mutex); 
        }
    }        
}


/*
 * createcars()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

void
init_intersection_locks(void) 
{    
    if (mutex == NULL) {
        mutex = sem_create("mutex",1);
        if (mutex == NULL) {
            panic("mutex: lock_create failed\n");
        }
    }
}

int
createcars(int nargs,
           char ** args)
{
        int index, error;

        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;
        
        init_intersection_locks();

        /*
         * Start NCARS approachintersection() threads.
         */

        for (index = 0; index < NCARS; index++) {

                error = thread_fork("approachintersection thread",
                                    NULL,
                                    index,
                                    approachintersection,
                                    NULL
                                    );

                /*
                 * panic() on error.
                 */

                if (error) {
                        
                        panic("approachintersection: thread_fork failed: %s\n",
                              strerror(error)
                              );
                }
        }

        return 0;
}
