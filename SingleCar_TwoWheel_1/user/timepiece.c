/**
 * @file timepiece.c
 * @brief 时间片轮询函数实现
 * @author ZheWana
 * @date 2021/10/4 004
 */

#include "timepiece.h"

static ptask_t taskList[TaskListNumMAX] = {NULL};

void SetInterruptPeriod(float period_ms) {
    ITperiod = period_ms;
}

void TaskDicision(void) {
    for (int i = 0; i < TaskNum(taskList); i++) {
        if (taskList[i] != NULL) {
            taskList[i]->taskTimer += ITperiod;
            if (taskList[i]->taskTimer >= taskList[i]->taskTimePiece) {
                taskList[i]->taskTimer = 0;
                taskList[i]->taskHandler();
            }
        }
    }
}

void TaskRegister_Single(task_t *task) {
    taskList[task->taskID] = task;
}

void TaskRegister_List(task_t *taskList, uint8_t listLength) {
    for (int i = 0; i < listLength; i++)
        TaskRegister_Single(taskList + i);
}