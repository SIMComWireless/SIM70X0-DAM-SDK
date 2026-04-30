/**
 * @file rtos_demo.c
 * @brief ThreadX RTOS demo example collection.
 *
 * Contains example ThreadX usage: multiple threads, queues, semaphores,
 * mutexes, event flags, byte pools and block pools. Intended as a
 * reference/demo for ThreadX primitives on the DAM platform.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */

#if !defined(LOG_TAG)
	#define LOG_TAG    "RTOS_demo"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "txm_module.h"
#include "qapi_types.h"
#include "qapi.h"
#include "qapi_timer.h"

#include "../Easylogger/elog.h"

#define DEMO_STACK_SIZE         1024
#define DEMO_BYTE_POOL_SIZE     9120
#define DEMO_BLOCK_POOL_SIZE    100
#define DEMO_QUEUE_SIZE         100
#define DEMO_TASK_PRIORITY_BASE 160

/* Define the ThreadX object control blocks...  */

TX_THREAD               *thread_0;
TX_THREAD               *thread_1;
TX_THREAD               *thread_2;
TX_THREAD               *thread_3;
TX_THREAD               *thread_4;
TX_THREAD               *thread_5;
TX_THREAD               *thread_6;
TX_THREAD               *thread_7;
TX_QUEUE                *queue_0;
TX_SEMAPHORE            *semaphore_0;
TX_MUTEX                *mutex_0;
TX_EVENT_FLAGS_GROUP    *event_flags_0;
TX_BYTE_POOL            *byte_pool_0;
TX_BLOCK_POOL           *block_pool_0;
UCHAR                   memory_area[DEMO_BYTE_POOL_SIZE];

/* Define the counters used in the demo application...  */
ULONG                   thread_0_counter;
ULONG                   thread_1_counter;
ULONG                   thread_1_messages_sent;
ULONG                   thread_2_counter;
ULONG                   thread_2_messages_received;
ULONG                   thread_3_counter;
ULONG                   thread_4_counter;
ULONG                   thread_5_counter;
ULONG                   thread_6_counter;
ULONG                   thread_7_counter;

/* Define thread prototypes.  */
void    thread_0_entry(ULONG thread_input);
void    thread_1_entry(ULONG thread_input);
void    thread_2_entry(ULONG thread_input);
void    thread_3_and_4_entry(ULONG thread_input);
void    thread_5_entry(ULONG thread_input);
void    thread_6_and_7_entry(ULONG thread_input);

/* Define main entry point.  */
/**
 * @brief Initialize and start RTOS demo threads and resources.
 *
 * Creates byte pools, threads, queues, semaphores, mutexes, event flags
 * and block pools used by the demo threads.
 *
 * @return int 0 on success; negative on error.
 */
int rtos_demo_entry(void)
{
    CHAR    *pointer = TX_NULL;

    /* Create a byte memory pool from which to allocate the thread stacks.  */
    txm_module_object_allocate(&byte_pool_0, sizeof(TX_BYTE_POOL));
    tx_byte_pool_create(byte_pool_0, "byte pool 0", memory_area, DEMO_BYTE_POOL_SIZE);

    log_i("Allocate memory byte pool 0");

    /* Put system definition stuff in here, e.g. thread creates and other assorted
       create information.  */

    /* Allocate the stack for thread 0.  */
    tx_byte_allocate(byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    /* Create the main thread.  */
    txm_module_object_allocate(&thread_0, sizeof(TX_THREAD));
    tx_thread_create(thread_0, "thread 0", thread_0_entry, 0,  
            pointer, DEMO_STACK_SIZE, 
            DEMO_TASK_PRIORITY_BASE+1, DEMO_TASK_PRIORITY_BASE+1, TX_NO_TIME_SLICE, TX_AUTO_START);


    /* Allocate the stack for thread 1.  */
    tx_byte_allocate(byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    /* Create threads 1 and 2. These threads pass information through a ThreadX 
       message queue.  It is also interesting to note that these threads have a time
       slice.  */
    txm_module_object_allocate(&thread_1, sizeof(TX_THREAD));
    tx_thread_create(thread_1, "thread 1", thread_1_entry, 1,  
            pointer, DEMO_STACK_SIZE, 
            DEMO_TASK_PRIORITY_BASE+16, DEMO_TASK_PRIORITY_BASE+16, 4, TX_AUTO_START);

    /* Allocate the stack for thread 2.  */
    tx_byte_allocate(byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    txm_module_object_allocate(&thread_2, sizeof(TX_THREAD));
    tx_thread_create(thread_2, "thread 2", thread_2_entry, 2,  
            pointer, DEMO_STACK_SIZE, 
            DEMO_TASK_PRIORITY_BASE+16, DEMO_TASK_PRIORITY_BASE+16, 4, TX_AUTO_START);

    /* Allocate the stack for thread 3.  */
    tx_byte_allocate(byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    /* Create threads 3 and 4.  These threads compete for a ThreadX counting semaphore.  
       An interesting thing here is that both threads share the same instruction area.  */
    txm_module_object_allocate(&thread_3, sizeof(TX_THREAD));
    tx_thread_create(thread_3, "thread 3", thread_3_and_4_entry, 3,  
            pointer, DEMO_STACK_SIZE, 
            DEMO_TASK_PRIORITY_BASE+8, DEMO_TASK_PRIORITY_BASE+8, TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Allocate the stack for thread 4.  */
    tx_byte_allocate(byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    txm_module_object_allocate(&thread_4, sizeof(TX_THREAD));
    tx_thread_create(thread_4, "thread 4", thread_3_and_4_entry, 4,  
            pointer, DEMO_STACK_SIZE, 
            DEMO_TASK_PRIORITY_BASE+8, DEMO_TASK_PRIORITY_BASE+8, TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Allocate the stack for thread 5.  */
    tx_byte_allocate(byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    /* Create thread 5.  This thread simply pends on an event flag which will be set
       by thread_0.  */
    txm_module_object_allocate(&thread_5, sizeof(TX_THREAD));
    tx_thread_create(thread_5, "thread 5", thread_5_entry, 5,  
            pointer, DEMO_STACK_SIZE, 
            DEMO_TASK_PRIORITY_BASE+4, DEMO_TASK_PRIORITY_BASE+4, TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Allocate the stack for thread 6.  */
    tx_byte_allocate(byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    /* Create threads 6 and 7.  These threads compete for a ThreadX mutex.  */
    txm_module_object_allocate(&thread_6, sizeof(TX_THREAD));
    tx_thread_create(thread_6, "thread 6", thread_6_and_7_entry, 6,  
            pointer, DEMO_STACK_SIZE, 
            DEMO_TASK_PRIORITY_BASE+8, DEMO_TASK_PRIORITY_BASE+8, TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Allocate the stack for thread 7.  */
    tx_byte_allocate(byte_pool_0, (VOID **) &pointer, DEMO_STACK_SIZE, TX_NO_WAIT);

    txm_module_object_allocate(&thread_7, sizeof(TX_THREAD));
    tx_thread_create(thread_7, "thread 7", thread_6_and_7_entry, 7,  
            pointer, DEMO_STACK_SIZE, 
            DEMO_TASK_PRIORITY_BASE+8, DEMO_TASK_PRIORITY_BASE+8, TX_NO_TIME_SLICE, TX_AUTO_START);

    /* Allocate the message queue.  */
    tx_byte_allocate(byte_pool_0, (VOID **) &pointer, DEMO_QUEUE_SIZE*sizeof(ULONG), TX_NO_WAIT);

    /* Create the message queue shared by threads 1 and 2.  */
    txm_module_object_allocate(&queue_0, sizeof(TX_QUEUE));
    tx_queue_create(queue_0, "queue 0", TX_1_ULONG, pointer, DEMO_QUEUE_SIZE*sizeof(ULONG));

    /* Create the semaphore used by threads 3 and 4.  */
    txm_module_object_allocate(&semaphore_0, sizeof(TX_SEMAPHORE));
    tx_semaphore_create(semaphore_0, "semaphore 0", 1);

    /* Create the event flags group used by threads 1 and 5.  */
    txm_module_object_allocate(&event_flags_0, sizeof(TX_EVENT_FLAGS_GROUP));
    tx_event_flags_create(event_flags_0, "event flags 0");

    /* Create the mutex used by thread 6 and 7 without priority inheritance.  */
    txm_module_object_allocate(&mutex_0, sizeof(TX_MUTEX));
    tx_mutex_create(mutex_0, "mutex 0", TX_NO_INHERIT);

    /* Allocate the memory for a small block pool.  */
    tx_byte_allocate(byte_pool_0, (VOID **) &pointer, DEMO_BLOCK_POOL_SIZE, TX_NO_WAIT);

    /* Create a block memory pool to allocate a message buffer from.  */
    txm_module_object_allocate(&block_pool_0, sizeof(TX_BLOCK_POOL));
    tx_block_pool_create(block_pool_0, "block pool 0", sizeof(ULONG), pointer, DEMO_BLOCK_POOL_SIZE);

    /* Allocate a block and release the block memory.  */
    tx_block_allocate(block_pool_0, (VOID **) &pointer, TX_NO_WAIT);

    /* Release the block back to the pool.  */
    tx_block_release(pointer);
}


/* Define the test threads.  */

/**
 * @brief Thread 0 main loop.
 *
 * Sleeps periodically and sets an event flag to wake thread 5.
 *
 * @param[in] thread_input Integer thread input value (unused).
 */
void    thread_0_entry(ULONG thread_input)
{

UINT    status;


    /* This thread simply sits in while-forever-sleep loop.  */
    while(1)
    {

        /* Increment the thread counter.  */
        thread_0_counter++;

        /* Sleep for 10 ticks.  */
       qapi_Timer_Sleep(1000,QAPI_TIMER_UNIT_MSEC,true);

        /* Set event flag 0 to wakeup thread 5.  */
        status =  tx_event_flags_set(event_flags_0, 0x1, TX_OR);

        log_i("thread_0_counter->%d",thread_0_counter);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;
    }
}


/**
 * @brief Thread 1 entry: sends messages to shared queue.
 *
 * @param[in] thread_input Integer thread input value (unused).
 */
void    thread_1_entry(ULONG thread_input)
{

UINT    status;


    /* This thread simply sends messages to a queue shared by thread 2.  */
    while(1)
    {

        /* Increment the thread counter.  */
        thread_1_counter++;

        /* Send message to queue 0.  */
        status =  tx_queue_send(queue_0, &thread_1_messages_sent, TX_WAIT_FOREVER);

        log_i("thread_1_counter->%d",thread_1_counter);

        qapi_Timer_Sleep(1000,QAPI_TIMER_UNIT_MSEC,true);

        /* Check completion status.  */
        if (status != TX_SUCCESS)
            break;

        /* Increment the message sent.  */
        thread_1_messages_sent++;

        log_i("thread_1_messages_sent->%d",thread_1_messages_sent);
    }
}


/**
 * @brief Thread 2 entry: receives messages from shared queue.
 *
 * @param[in] thread_input Integer thread input value (unused).
 */
void    thread_2_entry(ULONG thread_input)
{

ULONG   received_message;
UINT    status;

    /* This thread retrieves messages placed on the queue by thread 1.  */
    while(1)
    {

        /* Increment the thread counter.  */
        thread_2_counter++;

        /* Retrieve a message from the queue.  */
        status = tx_queue_receive(queue_0, &received_message, TX_WAIT_FOREVER);

        log_i("thread_2_counter->%d",thread_2_counter);


        /* Check completion status and make sure the message is what we 
           expected.  */
        if ((status != TX_SUCCESS) || (received_message != thread_2_messages_received))
            break;
        
        /* Otherwise, all is okay.  Increment the received message count.  */
        thread_2_messages_received++;

        log_i("thread_2_messages_received->%d",thread_2_messages_received);
    }
}


/**
 * @brief Shared entry for threads 3 and 4 demonstrating semaphore usage.
 *
 * @param[in] thread_input Integer thread input value identifying the thread.
 */
void    thread_3_and_4_entry(ULONG thread_input)
{

UINT    status;


    /* This function is executed from thread 3 and thread 4.  As the loop
       below shows, these function compete for ownership of semaphore_0.  */
    while(1)
    {

        /* Increment the thread counter.  */
        if (thread_input == 3)
            thread_3_counter++;
        else
            thread_4_counter++;

        /* Get the semaphore with suspension.  */
        status =  tx_semaphore_get(semaphore_0, TX_WAIT_FOREVER);

        log_i("thread_3_counter->%d",thread_3_counter);

        log_i("thread_4_counter->%d",thread_4_counter);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;

        /* Sleep for 2 ticks to hold the semaphore.  */
        tx_thread_sleep(200);

        /* Release the semaphore.  */
        status =  tx_semaphore_put(semaphore_0);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;
    }
}


/**
 * @brief Thread 5 entry: waits on an event flag set by thread 0.
 *
 * @param[in] thread_input Integer thread input value (unused).
 */
void    thread_5_entry(ULONG thread_input)
{

UINT    status;
ULONG   actual_flags;


    /* This thread simply waits for an event in a forever loop.  */
    while(1)
    {

        /* Increment the thread counter.  */
        thread_5_counter++;

        /* Wait for event flag 0.  */
        status =  tx_event_flags_get(event_flags_0, 0x1, TX_OR_CLEAR, 
                                                &actual_flags, TX_WAIT_FOREVER);

        log_i("thread_5_counter->%d",thread_5_counter);

        /* Check status.  */
        if ((status != TX_SUCCESS) || (actual_flags != 0x1))
            break;
    }
}


/**
 * @brief Shared entry for threads 6 and 7 demonstrating mutex usage.
 *
 * @param[in] thread_input Integer thread input value identifying the thread.
 */
void    thread_6_and_7_entry(ULONG thread_input)
{

UINT    status;


    /* This function is executed from thread 6 and thread 7.  As the loop
       below shows, these function compete for ownership of mutex_0.  */
    while(1)
    {

        /* Increment the thread counter.  */
        if (thread_input == 6)
            thread_6_counter++;
        else
            thread_7_counter++;

        /* Get the mutex with suspension.  */
        status =  tx_mutex_get(mutex_0, TX_WAIT_FOREVER);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;

        /* Get the mutex again with suspension.  This shows
           that an owning thread may retrieve the mutex it
           owns multiple times.  */
        status =  tx_mutex_get(mutex_0, TX_WAIT_FOREVER);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;

        /* Sleep for 2 ticks to hold the mutex.  */
        tx_thread_sleep(200);

        log_i("thread_6_counter->%d",thread_6_counter);

        log_i("thread_7_counter->%d",thread_7_counter);

        /* Release the mutex.  */
        status =  tx_mutex_put(mutex_0);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;

        /* Release the mutex again.  This will actually 
           release ownership since it was obtained twice.  */
        status =  tx_mutex_put(mutex_0);

        /* Check status.  */
        if (status != TX_SUCCESS)
            break;
    }
}
