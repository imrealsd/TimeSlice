/*
 * time_slice.c
 *
 *  Created on: Mar 1, 2025
 *      Author: subhadeep Dhang
 */
//#include "core_cm4.h"
#include "task.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "time_slice.h"


typedef struct
{
	volatile uint32_t CTRL;                   /*!< Offset: 0x000 (R/W)  SysTick Control and Status Register */
    volatile uint32_t LOAD;                   /*!< Offset: 0x004 (R/W)  SysTick Reload Value Register */
    volatile uint32_t VAL;                    /*!< Offset: 0x008 (R/W)  SysTick Current Value Register */
    volatile uint32_t CALIB;                  /*!< Offset: 0x00C (R/ )  SysTick Calibration Register */
} SysTick_Type;

typedef struct
{
  volatile  uint32_t CPUID;                  /*!< Offset: 0x000 (R/ )  CPUID Base Register */
  volatile  uint32_t ICSR;                   /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register */
  volatile  uint32_t VTOR;                   /*!< Offset: 0x008 (R/W)  Vector Table Offset Register */
  volatile  uint32_t AIRCR;                  /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register */
  volatile  uint32_t SCR;                    /*!< Offset: 0x010 (R/W)  System Control Register */
  volatile  uint32_t CCR;                    /*!< Offset: 0x014 (R/W)  Configuration Control Register */
  volatile  uint8_t  SHP[12U];               /*!< Offset: 0x018 (R/W)  System Handlers Priority Registers (4-7, 8-11, 12-15) */
  volatile  uint32_t SHCSR;                  /*!< Offset: 0x024 (R/W)  System Handler Control and State Register */
  volatile  uint32_t CFSR;                   /*!< Offset: 0x028 (R/W)  Configurable Fault Status Register */
  volatile  uint32_t HFSR;                   /*!< Offset: 0x02C (R/W)  HardFault Status Register */
  volatile  uint32_t DFSR;                   /*!< Offset: 0x030 (R/W)  Debug Fault Status Register */
  volatile  uint32_t MMFAR;                  /*!< Offset: 0x034 (R/W)  MemManage Fault Address Register */
  volatile  uint32_t BFAR;                   /*!< Offset: 0x038 (R/W)  BusFault Address Register */
  volatile  uint32_t AFSR;                   /*!< Offset: 0x03C (R/W)  Auxiliary Fault Status Register */
  volatile  uint32_t PFR[2U];                /*!< Offset: 0x040 (R/ )  Processor Feature Register */
  volatile  uint32_t DFR;                    /*!< Offset: 0x048 (R/ )  Debug Feature Register */
  volatile  uint32_t ADR;                    /*!< Offset: 0x04C (R/ )  Auxiliary Feature Register */
  volatile  uint32_t MMFR[4U];               /*!< Offset: 0x050 (R/ )  Memory Model Feature Register */
  volatile  uint32_t ISAR[5U];               /*!< Offset: 0x060 (R/ )  Instruction Set Attributes Register */
            uint32_t RESERVED0[5U];
  volatile  uint32_t CPACR;                  /*!< Offset: 0x088 (R/W)  Coprocessor Access Control Register */
} SCB_Type;


// Core Registers
#define SYSTICK ((SysTick_Type *) SYSTICK_BASE_ADDR)
#define SCB     ((SCB_Type     *) SCB_BASE_ADDR    )


// Global Variables
static uint32_t taskStackAddress[MAX_TASK]   = {(uint32_t)TASK1_STACK_START_ADDR, (uint32_t)TASK2_STACK_START_ADDR};
static uint32_t taskHandlerAddress[MAX_TASK] = {userTask1, userTask2};
static uint8_t  currTaskIndex = 0;


// Private functions prototypes
static void timeSlice_initSysTick(void);
static void timeSlice_initTaskStack(void);
static void timeSlice_enableProcessorFaults(void);
static void timeSlice_setCurrTaskPSP(uint32_t stackTopAddr);
static uint32_t timeSlice_getCurrTaskPSP(void);
static void timeSlice_updateNextTask(void);
__attribute__((naked)) void timeSlice_initSchedStack(uint32_t stackTopAddr);
__attribute__((naked)) void timeSlice_switchToPSP(void);


// Private functions definitions
static void timeSlice_initSysTick(void) {

	/**
	 * Configuring sys-tick to have interrupt in every 1 ms
	 */
	SYSTICK->VAL  = 0;
	SYSTICK->LOAD = (SYSCLK_FREQ / 1000) - 1;
	SYSTICK->CTRL = (1 << 0) | (1 << 1) | (1 << 2);
}


__attribute__((naked)) void timeSlice_initSchedStack(uint32_t stackTopAddr) {

	/**
	 * MSP will be used for the task scheduler's stack pointer
	 * Scheduler runs in handler mode inside systick/pendsv ISR
	 */
	 __asm volatile("MSR MSP,%0": :  "r" (stackTopAddr)  :   );
	 __asm volatile("BX LR");
}


__attribute__((naked)) void timeSlice_switchToPSP(void) {

	/**
	 * PSP will be used as stack pointer for user tasks
	 */

	// Store LR
	__asm volatile ("PUSH {LR}");
	// Get curr stack pointer of first task
	__asm volatile ("BL timeSlice_getCurrTaskPSP");
	// Load PSP with the obtained value
	__asm volatile ("MSR PSP,R0");
	// Retrieve LR
	__asm volatile ("POP {LR}");
	// Change stack pointer to PSP from MSP
	__asm volatile ("MOV R0,#0X02");
	__asm volatile ("MSR CONTROL,R0");
	__asm volatile ("BX LR");
}


static void timeSlice_initTaskStack(void) {

	/**
	 * Filling each tasks private stacks with dummy stack frame
	 */
	uint32_t * ptrPSP = 0;

	for (uint8_t i = 0; i < MAX_TASK; i++) {

		ptrPSP = (uint32_t *) taskStackAddress[i];

		*(ptrPSP - 1) = DUMMY_XPSR;
		*(ptrPSP - 2) = (uint32_t) taskHandlerAddress[i];
		*(ptrPSP - 3) = DUMMY_LR;
		for (uint8_t i = 0; i < 13; i++) {
			*(ptrPSP - (4+i)) = 0;
		}
		taskStackAddress[i] = (uint32_t)(ptrPSP - 16);
	}
}


static void timeSlice_enableProcessorFaults(void) {

	/**
	 * Enabling memfault, busfault and usage fault
	 */
	SCB->SHCSR |= (1 << 16) | (1 << 17) | (1 << 18);
}


static void timeSlice_setCurrTaskPSP(uint32_t stackTopAddr) {

	if (currTaskIndex < MAX_TASK) {
		taskStackAddress[currTaskIndex] = stackTopAddr;
	}
}


static uint32_t timeSlice_getCurrTaskPSP(void) {

	if (currTaskIndex < MAX_TASK) {
		return taskStackAddress[currTaskIndex];
	}
	return 0;
}


static void timeSlice_updateNextTask(void) {

	currTaskIndex++;
	currTaskIndex %= MAX_TASK;
}



// Public functions definitions

void timeSlice_initScheduler(void) {

	timeSlice_enableProcessorFaults();
	timeSlice_initSysTick();
	timeSlice_initTaskStack();
	timeSlice_initSchedStack(SCHED_STACK_START_ADDR);
	timeSlice_switchToPSP();
	userTask1();
}


void timeSlice_taskDelay(uint32_t ticks) {


}

// Exception handlers

__attribute__((naked)) void SysTick_Handler(void) {

	/*Save the context of current task*/

	// Get current running task's PSP value
	__asm volatile("MRS R0,PSP");
	// Using that PSP value store SF2( R4 to R11)
	__asm volatile("STMDB R0!,{R4-R11}");
	__asm volatile("PUSH {LR}");
	// Save the current value of PSP
	__asm volatile("BL timeSlice_setCurrTaskPSP");

	/*Retrieve the context of next task */

	// Decide next task to run
	__asm volatile("BL timeSlice_updateNextTask");
	// Get it's PSP value
	__asm volatile ("BL timeSlice_getCurrTaskPSP");
	// Using that PSP value retrieve SF2(R4 to R11)
	__asm volatile ("LDMIA R0!,{R4-R11}");
	// Update PSP and exit
	__asm volatile("MSR PSP,R0");
	__asm volatile("POP {LR}");
	__asm volatile("BX LR");
}
