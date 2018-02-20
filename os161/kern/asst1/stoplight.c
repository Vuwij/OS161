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

/*static struct lock *NW;
static struct lock *SW;
static struct lock *NE;
static struct lock *SE;*/

static struct semaphore *NW;
static struct semaphore *SW;
static struct semaphore *NE;
static struct semaphore *SE;

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

static void
message2(int msg_nr, char* lock_name, int carnumber, int cardirection, int destdirection)
{
        kprintf("%s %s car = %2d, direction = %s, destination = %s\n",
                msgs[msg_nr], lock_name, carnumber,
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
        
        //message(APPROACHING, carnumber, source, destination);
        
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
                
                message(APPROACHING, carnumber, source, destination); 
                message(REGION1, carnumber, source, destination); 
                //message2(REGION1, "NW", carnumber, source, destination);
                message(LEAVING, carnumber, source, destination);  
                
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
                message(APPROACHING, carnumber, source, destination); 
                message(REGION1, carnumber, source, destination); 
                message(REGION2, carnumber, source, destination); 
                //message2(REGION1, "NW", carnumber, source, destination);
                //message2(REGION2, "SW", carnumber, source, destination);
                message(LEAVING, carnumber, source, destination);                
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
                message(APPROACHING, carnumber, source, destination); 
                message(REGION1, carnumber, source, destination); 
                message(REGION2, carnumber, source, destination); 
                message(REGION3, carnumber, source, destination); 
                //message2(REGION1, "NW", carnumber, source, destination);
                //message2(REGION2, "SW", carnumber, source, destination);
                //message2(REGION3, "SE", carnumber, source, destination);
                message(LEAVING, carnumber, source, destination);                
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
                message(APPROACHING, carnumber, source, destination); 
                message(REGION1, carnumber, source, destination); 
                //message2(REGION1, "SE", carnumber, source, destination);
                message(LEAVING, carnumber, source, destination);                
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
                message(APPROACHING, carnumber, source, destination); 
                message(REGION1, carnumber, source, destination); 
                message(REGION2, carnumber, source, destination); 
//                message2(REGION1, "SE", carnumber, source, destination);
//                message2(REGION2, "NE", carnumber, source, destination);
                message(LEAVING, carnumber, source, destination);                
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
                message(APPROACHING, carnumber, source, destination); 
                message(REGION1, carnumber, source, destination); 
                message(REGION2, carnumber, source, destination); 
                message(REGION3, carnumber, source, destination);  
//                message2(REGION1, "SE", carnumber, source, destination);
//                message2(REGION2, "NE", carnumber, source, destination);
//                message2(REGION3, "NW", carnumber, source, destination);
                message(LEAVING, carnumber, source, destination);                
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
                message(APPROACHING, carnumber, source, destination); 
                message(REGION1, carnumber, source, destination); 
                //message2(REGION1, "NE", carnumber, source, destination);
                message(LEAVING, carnumber, source, destination);                
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
                message(APPROACHING, carnumber, source, destination); 
                message(REGION1, carnumber, source, destination); 
                message(REGION2, carnumber, source, destination); 
//                message2(REGION1, "NE", carnumber, source, destination);
//                message2(REGION2, "NW", carnumber, source, destination);
                message(LEAVING, carnumber, source, destination);                
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
                message(APPROACHING, carnumber, source, destination); 
                message(REGION1, carnumber, source, destination); 
                message(REGION2, carnumber, source, destination); 
                message(REGION3, carnumber, source, destination); 
//                message2(REGION1, "NE", carnumber, source, destination);
//                message2(REGION2, "NW", carnumber, source, destination);
//                message2(REGION3, "SW", carnumber, source, destination);
                message(LEAVING, carnumber, source, destination);                
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
                message(APPROACHING, carnumber, source, destination); 
                message(REGION1, carnumber, source, destination); 
                //message2(REGION1, "SW", carnumber, source, destination);
                message(LEAVING, carnumber, source, destination);                
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
                message(APPROACHING, carnumber, source, destination); 
                message(REGION1, carnumber, source, destination); 
                message(REGION2, carnumber, source, destination);  
//                message2(REGION1, "SW", carnumber, source, destination);
//                message2(REGION2, "SE", carnumber, source, destination);
                message(LEAVING, carnumber, source, destination);                
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
                message(APPROACHING, carnumber, source, destination); 
                message(REGION1, carnumber, source, destination); 
                message(REGION2, carnumber, source, destination); 
                message(REGION3, carnumber, source, destination); 
//                message2(REGION1, "SW", carnumber, source, destination);
//                message2(REGION2, "SE", carnumber, source, destination);
//                message2(REGION3, "NE", carnumber, source, destination);
                message(LEAVING, carnumber, source, destination);                
                P(mutex);
                NE_cars = 0;
                SW_cars = 0;
                SE_cars = 0;
                V(mutex); 
            }
        }
        
        //message(LEAVING, carnumber, source, destination);
                
        /*if (strcmp(src,"N") == 0) {
            
            if (strcmp(dest,"W") == 0) {
                P(NW);
                //message(REGION1, carnumber, source, destination); 
                message2(REGION1, "NW", carnumber, source, destination); 
                V(NW);
            }                       
            
            if (strcmp(dest,"S") == 0) {
                
                P(SW);
                P(NW);
                //message(REGION1, carnumber, source, destination);
                //message(REGION2, carnumber, source, destination);
                message2(REGION1, "NW", carnumber, source, destination);
                message2(REGION2, "SW", carnumber, source, destination);
                V(NW);                                
                V(SW);  
            }
                
            if (strcmp(dest,"E") == 0) {
                
                P(SW);
                P(NW);
                //message(REGION1, carnumber, source, destination);
                //message(REGION2, carnumber, source, destination);
                message2(REGION1, "NW", carnumber, source, destination);
                message2(REGION2, "SW", carnumber, source, destination);
                V(NW);
                V(SW); 
                P(SE);
                
                //message(REGION3, carnumber, source, destination);
                message2(REGION3, "SE", carnumber, source, destination);
                                            
                V(SE);              
            }            
        }
        
        if (strcmp(src,"S") == 0) {

            if (strcmp(dest,"E") == 0) {
                P(SE);
                //message(REGION1, carnumber, source, destination);
                message2(REGION1, "SE", carnumber, source, destination);
                V(SE);
            }                       
            
            if (strcmp(dest,"N") == 0) {
                
                P(NE);
                P(SE);
                //message(REGION1, carnumber, source, destination);
                //message(REGION2, carnumber, source, destination);
                message2(REGION1, "SE", carnumber, source, destination);
                message2(REGION2, "NE", carnumber, source, destination);
                V(SE);                                
                V(NE);   
            }
                
            if (strcmp(dest,"W") == 0) {
                
                P(NE);
                P(SE);
                //message(REGION1, carnumber, source, destination);
                //message(REGION2, carnumber, source, destination);
                message2(REGION1, "SE", carnumber, source, destination);
                message2(REGION2, "NE", carnumber, source, destination);
                V(SE);
                
                V(NE);
                P(NW);
                //message(REGION3, carnumber, source, destination);
                message2(REGION3, "NW", carnumber, source, destination);
                                            
                V(NW);              
            }
        }
        
        if (strcmp(src,"E") == 0) {

            if (strcmp(dest,"N") == 0) {
                P(NE);
                //message(REGION1, carnumber, source, destination); 
                message2(REGION1, "NE", carnumber, source, destination);
                V(NE);
            }                       
            
            if (strcmp(dest,"W") == 0) {
                
                P(NW);
                P(NE);
                //message(REGION1, carnumber, source, destination);
                //message(REGION2, carnumber, source, destination);
                message2(REGION1, "NE", carnumber, source, destination);
                message2(REGION2, "NW", carnumber, source, destination);
                V(NE);                         
                V(NW);   
            }
                
            if (strcmp(dest,"S") == 0) {
                
                P(NW);
                P(NE);
                //message(REGION1, carnumber, source, destination);
                //message(REGION2, carnumber, source, destination);
                message2(REGION1, "NE", carnumber, source, destination);
                message2(REGION2, "NW", carnumber, source, destination);
                V(NE);
                
                V(NW); 
                P(SW);
                //message(REGION3, carnumber, source, destination);  
                message2(REGION3, "SW", carnumber, source, destination);
                                                
                V(SW);              
            }
        }
        
        if (strcmp(src,"W") == 0) {

            if (strcmp(dest,"S") == 0) {
                P(SW);
                //message(REGION1, carnumber, source, destination); 
                message2(REGION1, "SW", carnumber, source, destination);
                
                V(SW);
            }                       
            
            if (strcmp(dest,"E") == 0) {
                
                P(SE);
                P(SW);
                //message(REGION1, carnumber, source, destination);
                //message(REGION2, carnumber, source, destination);
                message2(REGION1, "SW", carnumber, source, destination);
                message2(REGION2, "SE", carnumber, source, destination);
                V(SW);                                
                V(SE);   
            }
                
            if (strcmp(dest,"N") == 0) {
                
                P(SE);
                P(SW);
                //message(REGION1, carnumber, source, destination);
                //message(REGION2, carnumber, source, destination);
                message2(REGION1, "SW", carnumber, source, destination);
                message2(REGION2, "SE", carnumber, source, destination);
                V(SW);
                
                V(SE);
                P(NE);
                //message(REGION3, carnumber, source, destination);                
                message2(REGION3, "NE", carnumber, source, destination);
                                              
                V(NE);              
            }
        }
        
        message(LEAVING, carnumber, source, destination);*/
        
        ///////////////////////////////////////////////////////
        
        
        /*if (strcmp(src,"N") == 0) {

            P(NW);
            message(REGION1, carnumber, source, destination);            
            
            if (strcmp(dest,"S") == 0 || strcmp(dest,"E") == 0) {
                V(NW);
                message(REGION2, carnumber, source, destination);                
                
                if (strcmp(dest,"E") == 0) {
                    P(SE);
                    V(SW);
                    message(REGION3, carnumber, source, destination);
                    V(SE);              
                }
                else
                    V(SW);
            }
            else
                V(NW);
        }
        
        if (strcmp(src,"S") == 0) {

            P(SE);
            message(REGION1, carnumber, source, destination);
            
            
            if (strcmp(dest,"N") == 0 || strcmp(dest,"W") == 0) {
                P(NE);
                message(REGION2, carnumber, source, destination);
                V(SE);
                
                if (strcmp(dest,"W") == 0) {
                    P(NW);
                    V(NE);
                    message(REGION3, carnumber, source, destination);
                    V(NW);              
                }
                else
                    V(NE);
            }
            else
                V(SE);
        }
        
        if (strcmp(src,"E") == 0) {

            P(NE);
            message(REGION1, carnumber, source, destination);
            
            
            if (strcmp(dest,"W") == 0 || strcmp(dest,"S") == 0) {
                P(NW);
                V(NE);
                message(REGION2, carnumber, source, destination);
                
                
                if (strcmp(dest,"S") == 0) {
                    P(SW);
                    V(NW);
                    message(REGION3, carnumber, source, destination);
                    V(SW);              
                }
                else
                    V(NW);
            }
            else
                V(NE);
        }
        
        if (strcmp(src,"W") == 0) {

            P(SW);
            message(REGION1, carnumber, source, destination);
            
            
            if (strcmp(dest,"E") == 0 || strcmp(dest,"N") == 0) {
                P(SE);
                V(SW);
                message(REGION2, carnumber, source, destination);
                
                
                if (strcmp(dest,"N") == 0) {
                    P(NE);
                    V(SE);
                    message(REGION3, carnumber, source, destination);
                    V(NE);              
                }
                else
                    V(SE);
            }  
            else
                V(SW);
        }
        
        message(LEAVING, carnumber, source, destination);*/

////////////////////////////////////////////////////////////////        
        
        /*if (strcmp(src,"N") == 0) { //kprintf("hi\n");}
//            kprintf("N\n");
//            kprintf("src: %s\n",src);
//            kprintf("dest: %s\n",dest);
            lock_acquire(NW);
            message(REGION1, carnumber, source, destination);
            
            
            if (strcmp(dest,"S") == 0 || strcmp(dest,"E") == 0) {
                lock_acquire(SW);
                lock_release(NW);
                message(REGION2, carnumber, source, destination);                
                
                if (strcmp(dest,"E") == 0) {
                    lock_acquire(SE);
                    lock_release(SW);
                    message(REGION3, carnumber, source, destination);
                    lock_release(SE);              
                }
                else
                    lock_release(SW);
            }
            else
                lock_release(NW);
        }
        
        if (strcmp(src,"S") == 0) {
//            kprintf("S\n");
//            kprintf("src: %s\n",src);
//            kprintf("dest: %s\n",dest);
            lock_acquire(SE);
            message(REGION1, carnumber, source, destination);
            
            
            if (strcmp(dest,"N") == 0 || strcmp(dest,"W") == 0) {
                lock_acquire(NE);
                message(REGION2, carnumber, source, destination);
                lock_release(SE);
                
                if (strcmp(dest,"W") == 0) {
                    lock_acquire(NW);
                    lock_release(NE);
                    message(REGION3, carnumber, source, destination);
                    lock_release(NW);              
                }
                else
                    lock_release(NE);
            }
            else
                lock_release(SE);
        }
        
        if (strcmp(src,"E") == 0) {
//            kprintf("E\n");
//            kprintf("src: %s\n",src);
//            kprintf("dest: %s\n",dest);
            lock_acquire(NE);
            message(REGION1, carnumber, source, destination);
            
            
            if (strcmp(dest,"W") == 0 || strcmp(dest,"S") == 0) {
                lock_acquire(NW);
                lock_release(NE);
                message(REGION2, carnumber, source, destination);
                
                
                if (strcmp(dest,"S") == 0) {
                    lock_acquire(SW);
                    lock_release(NW);
                    message(REGION3, carnumber, source, destination);
                    lock_release(SW);              
                }
                else
                    lock_release(NW);
            }
            else
                lock_release(NE);
        }
        
        if (strcmp(src,"W") == 0) {
//            kprintf("W\n");
//            kprintf("src: %s\n",src);
//            kprintf("dest: %s\n",dest);
            lock_acquire(SW);
            message(REGION1, carnumber, source, destination);
            
            
            if (strcmp(dest,"E") == 0 || strcmp(dest,"N") == 0) {
                lock_acquire(SE);
                lock_release(SW);
                message(REGION2, carnumber, source, destination);
                
                
                if (strcmp(dest,"N") == 0) {
                    lock_acquire(NE);
                    lock_release(SE);
                    message(REGION3, carnumber, source, destination);
                    lock_release(NE);              
                }
                else
                    lock_release(SE);
            }  
            else
                lock_release(SW);
        }
        
        message(LEAVING, carnumber, source, destination);
        */
        
        ///////////////////////////////////////////////////////
        
        /*
         if (strcmp(src,"N") == 0) { //kprintf("hi\n");}
//            kprintf("N\n");
//            kprintf("src: %s\n",src);
//            kprintf("dest: %s\n",dest);
            lock_acquire(NW);
            message(REGION1, carnumber, source, destination);
            lock_release(NW);
            
            if (strcmp(dest,"S") == 0 || strcmp(dest,"E") == 0) {
                lock_acquire(SW);
                message(REGION2, carnumber, source, destination);
                lock_release(SW);
                
                if (strcmp(dest,"E") == 0) {
                    lock_acquire(SE);
                    message(REGION3, carnumber, source, destination);
                    lock_release(SE);              
                }
            }            
        }
        
        if (strcmp(src,"S") == 0) {
//            kprintf("S\n");
//            kprintf("src: %s\n",src);
//            kprintf("dest: %s\n",dest);
            lock_acquire(SE);
            message(REGION1, carnumber, source, destination);
            lock_release(SE);
            
            if (strcmp(dest,"N") == 0 || strcmp(dest,"W") == 0) {
                lock_acquire(NE);
                message(REGION2, carnumber, source, destination);
                lock_release(NE);
                
                if (strcmp(dest,"W") == 0) {
                    lock_acquire(NW);
                    message(REGION3, carnumber, source, destination);
                    lock_release(NW);              
                }
            }            
        }
        
        if (strcmp(src,"E") == 0) {
//            kprintf("E\n");
//            kprintf("src: %s\n",src);
//            kprintf("dest: %s\n",dest);
            lock_acquire(NE);
            message(REGION1, carnumber, source, destination);
            lock_release(NE);
            
            if (strcmp(dest,"W") == 0 || strcmp(dest,"S") == 0) {
                lock_acquire(NW);
                message(REGION2, carnumber, source, destination);
                lock_release(NW);
                
                if (strcmp(dest,"S") == 0) {
                    lock_acquire(SW);
                    message(REGION3, carnumber, source, destination);
                    lock_release(SW);              
                }
            }            
        }
        
        if (strcmp(src,"W") == 0) {
//            kprintf("W\n");
//            kprintf("src: %s\n",src);
//            kprintf("dest: %s\n",dest);
            lock_acquire(SW);
            message(REGION1, carnumber, source, destination);
            lock_release(SW);
            
            if (strcmp(dest,"E") == 0 || strcmp(dest,"N") == 0) {
                lock_acquire(SE);
                message(REGION2, carnumber, source, destination);
                lock_release(SE);
                
                if (strcmp(dest,"N") == 0) {
                    lock_acquire(NE);
                    message(REGION3, carnumber, source, destination);
                    lock_release(NE);              
                }
            }            
        }
        
        message(LEAVING, carnumber, source, destination);
        */
        
        //choose destination direction randomly
        
        //approach
        
        //have 12 cases. make turns based on start and end. for instance N to S will be NW-NS
        
        //each section has lock. aquire it (move), print, release it, aquire next
        
        
        
        
        
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
    /*if (NW == NULL) {
        NW = lock_create("NW");
        if (NW == NULL) {
            panic("NW: lock_create failed\n");
        }
    }
    if (NE == NULL) {
        NE = lock_create("NE");
        if (NE == NULL) {
            panic("NE: lock_create failed\n");
        }
    }
    if (SW == NULL) {
        SW = lock_create("SW");
        if (SW == NULL) {
            panic("SW: lock_create failed\n");
        }
    }
    if (SE == NULL) {
        SE = lock_create("SE");
        if (SE == NULL) {
            panic("SE: lock_create failed\n");
        }
    }*/
    
    if (NW == NULL) {
        NW = sem_create("NW",1);
        if (NW == NULL) {
            panic("NW: lock_create failed\n");
        }
    }
    if (NE == NULL) {
        NE = sem_create("NE",1);
        if (NE == NULL) {
            panic("NE: lock_create failed\n");
        }
    }
    if (SW == NULL) {
        SW = sem_create("SW",1);
        if (SW == NULL) {
            panic("SW: lock_create failed\n");
        }
    }
    if (SE == NULL) {
        SE = sem_create("SE",1);
        if (SE == NULL) {
            panic("SE: lock_create failed\n");
        }
    }
    
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
