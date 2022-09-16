/**
 * @file timepiece.h
 * @brief 时间片轮询相关函数定义以及句柄
 * @author ZheWana
 * @date 2021/10/4 004
 */

#ifndef TIMEPIECE_H
#define TIMEPIECE_H

#include "Debug.h"
#include "stdio.h"

#define TaskNum(__taskArray) (sizeof(__taskArray)/sizeof(__taskArray[0]))

#define TaskListNumMAX 10

#define TaskHandlerDeclare(__handlerID) Task##__handlerID##Handler

static float ITperiod = 1;

typedef struct {
    uint8_t taskID;
    uint16_t taskTimePiece;

    void (*taskHandler)(void);

    volatile uint16_t taskTimer;
} task_t, *ptask_t;

void SetInterruptPeriod(float period_ms);

void TaskDicision(void);

void TaskRegister_Single(task_t *task);

void TaskRegister_List(task_t *task, uint8_t listLength);

#endif //TIMEPIECE_H
