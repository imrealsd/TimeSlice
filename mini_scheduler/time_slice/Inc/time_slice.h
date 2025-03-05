/*
 * time_slice.h
 *
 *  Created on: Mar 1, 2025
 *      Author: subhadeep Dhang
 */

#ifndef INC_TIME_SLICE_H_
#define INC_TIME_SLICE_H_

// ARM core register address
#define SYSTICK_BASE_ADDR    (0xE000E010U)
#define SCB_BASE_ADDR        (0xE000ED24U)
// Core registers value
#define DUMMY_XPSR            (0x01000000U)
#define DUMMY_LR              (0xFFFFFFFDU)

// System Clock
#define SYSCLK_FREQ            (16 * 1000000)

// Ram address
#define SRAM_START_ADDR        (0x20000000)
#define SRAM_SIZE              (128 * 1024)
#define SRAM_END_ADDR          (SRAM_START_ADDR + SRAM_SIZE)


// Stack memory allocation
#define TASKS_STACK_SIZE_BYTES (1024U)
#define SCHED_STACK_SIZE_BYTES (1024U)

// Tasks
#define MAX_TASK               (2)

// Stack pointer address
#define TASK1_STACK_START_ADDR (SRAM_END_ADDR - (1 * TASKS_STACK_SIZE_BYTES))
#define TASK2_STACK_START_ADDR (SRAM_END_ADDR - (2 * TASKS_STACK_SIZE_BYTES))
#define SCHED_STACK_START_ADDR (SRAM_END_ADDR - (3 * TASKS_STACK_SIZE_BYTES))

// Scheduler APIs
void timeSlice_initScheduler(void);
void timeSlice_taskDelay(uint32_t ticks);

#endif /* INC_TIME_SLICE_H_ */
