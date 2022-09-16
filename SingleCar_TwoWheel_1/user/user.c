/**
 * @file user.c
 * @brief 
 * @author ZheWana
 * @date 2021/10/14 014
 */

#include <stdlib.h>
#include <math.h>
#include "user.h"

pid leftspd = {0}, rightspd = {0}, dir = {0};
uint8_t mainState = 0, subState = 0, Tflag = 0, TStateflag = 0, delayFlag1 = 0, delayPreFlag1 = 0, delayFlag2, delayPreFlag2, turnFlag = 0, dottedFlag = 0, getCross = 0, getT = 0, crossState = 0, forceTurn = 0, dataFrame = 0;
double shadowDirOut = 0, basicLeftSpd = 0, basicRightSpd = 0;
char command = 0;


extern uint8_t data, crossHistroy;

uint8_t Get_DottedLine(void) {
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_9);
}

void CalibrationDelay(int CNT) {
    uint32_t leftCntLoad = 0;
    leftCntLoad = TIM2->CNT;
    while (TIM2->CNT - leftCntLoad < CNT);
}

void My_UART_IDLECallback(UART_HandleTypeDef *huart) {
    HAL_UART_Receive_IT(&huart1, &data, sizeof(data));
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
}

void ForceTurn(uint8_t dirMask) {
    ForceTurn_Start;
    static uint32_t leftCntLoad = 0, rightCntLoad = 0;
    leftCntLoad = TIM2->CNT;
    rightCntLoad = TIM5->CNT;

    switch (dirMask) {
        case DIR_BACK:
            Set_Speed(MAXSpeed, -MAXSpeed);
            while (TIM2->CNT - leftCntLoad < 680);
            break;
        case DIR_LEFT:
            Set_Speed(0, MAXSpeed);
            while (TIM5->CNT - rightCntLoad < 700);
            break;
        case DIR_RIGHT:
            Set_Speed(MAXSpeed, 0);
            while (TIM2->CNT - leftCntLoad < 700);
            break;
        default:;
    }
    ForceTurn_Stop;
}

void Set_Speed(float leftSpd, float rightSpd) {
    basicLeftSpd = leftSpd;
    basicRightSpd = rightSpd;
}

void LED_ON(uint8_t ledMask) {
    if (ledMask & LED_YELLOW)
        HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_RESET);
    else
        HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_SET);
    if (ledMask & LED_RED)
        HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET);
    else
        HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);
    if (ledMask & LED_GREEN)
        HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);
    else
        HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET);
}

double PID_Realize(pid *ctrl) {
    ctrl->error.cur = ctrl->ctr.aim - ctrl->ctr.cur;
    ctrl->error.sum += ctrl->error.cur;
    ctrl->error.bia = ctrl->error.cur - ctrl->error.pre;
    ctrl->error.pre = ctrl->error.cur;
    return ctrl->kp * ctrl->error.cur + ctrl->ki * ctrl->error.sum + ctrl->kd * ctrl->error.bia;
}

void PID_Init(pid *ctrl, double kp, double ki, double kd, double aim) {
    ctrl->kp = kp;
    ctrl->ki = ki;
    ctrl->kd = kd;
    ctrl->ctr.aim = aim;
}

uint8_t Get_MedicineState(void) {
    if (HAL_GPIO_ReadPin(Medicine_GPIO_Port, Medicine_Pin) == GPIO_PIN_RESET) {
        return MedicineExist;
    } else {
        return MedicineNone;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        HAL_UART_Receive_IT(&huart1, &data, sizeof(data));
        static char charflag;
        static int negtiveflag, pointflag;
        static float valint, valpoint;
        do {
            if (data == '\n' || data == 'x' || data == 'y') {
                while (valpoint >= 1)valpoint /= 10;
                switch (charflag) {
                    case 'x':
                        dir.ctr.cur = negtiveflag ? (-1) * (valint + valpoint) : (valint + valpoint);
                        charflag = 0;
                        break;
                    case 'y':
                        dataFrame = (uint8_t) valint;//negtiveflag ? (-1) * (valint + valpoint) : (valint + valpoint);
                        charflag = 0;
                        break;
                    default:;
                }
            }
            if (data == 'x') {
                charflag = 'x';
                negtiveflag = 0;
                pointflag = 0;
                valint = 0;
                valpoint = 0;
                break;
            }
            if (data == 'y') {
                charflag = 'y';
                negtiveflag = 0;
                pointflag = 0;
                valint = 0;
                valpoint = 0;
                break;
            }
            if (data == '-')negtiveflag = 1;
            if (data == '.')pointflag = 1;
            if (data <= '9' && data >= '0') {
                if (pointflag) {
                    valpoint = valpoint * 10 + (data - '0') * 1;
                } else {
                    valint = valint * 10 + (data - '0') * 1;
                }
            }
        } while (0);
    }
}

//4-4舵机
//1-1 1-3电机
void PWM_Handle(double leftSpd, double rightSpd) {
    leftSpd = LimitingOut((float) (leftSpd), 8399, -8399);
    rightSpd = LimitingOut((float) (rightSpd), 8399, -8399);
    //电机 8399
    if (leftSpd >= 0) {
        HAL_GPIO_WritePin(LDIR_GPIO_Port, LDIR_Pin, GPIO_PIN_SET);
        TIM1->CCR1 = (uint16_t) leftSpd;
    } else {
        HAL_GPIO_WritePin(LDIR_GPIO_Port, LDIR_Pin, GPIO_PIN_RESET);
        TIM1->CCR1 = (uint16_t) -leftSpd;
    }
    if (rightSpd >= 0) {
        HAL_GPIO_WritePin(RDIR_GPIO_Port, RDIR_Pin, GPIO_PIN_RESET);
        TIM1->CCR3 = (uint16_t) rightSpd;
    } else {
        HAL_GPIO_WritePin(RDIR_GPIO_Port, RDIR_Pin, GPIO_PIN_SET);
        TIM1->CCR3 = (uint16_t) -rightSpd;
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM3) {
        TaskDicision();
    }
}

double De2Du(double de) {
    if (de > 45)de = 45;
    if (de < -45)de = -45;
    return 7.5 + de * 5.0 / 90;
}

float LimitingOut(float data, float highVal, float lowVal) {
    return data > highVal ? highVal : data < lowVal ? lowVal : data;
}

void TaskHandlerDeclare(PIDControl)(void) {
    //PID控制
    leftspd.CNT_cur = TIM2->CNT;
    rightspd.CNT_cur = TIM5->CNT;
    leftspd.ctr.cur = (leftspd.CNT_cur - leftspd.CNT_pre) * 100.0 * 20 / LineNum / 20 / 2;
    rightspd.ctr.cur = (rightspd.CNT_cur - rightspd.CNT_pre) * 100.0 * 20 / LineNum / 20 / 2;
    float dirout = (float) PID_Realize(&dir);

    dirout = forceTurn ? 0 : dirout;
    leftspd.ctr.aim = basicLeftSpd - dirout;
    rightspd.ctr.aim = basicRightSpd + dirout;
    PWM_Handle(PID_Realize(&leftspd), PID_Realize(&rightspd));
    leftspd.CNT_pre = leftspd.CNT_cur;
    rightspd.CNT_pre = rightspd.CNT_cur;
}

void TaskHandlerDeclare(StateDetect)(void) {
    //状态检测
    getCross = dataFrame & 1;
    crossState = (uint8_t) (dataFrame << 5) >> 6;
    getT = dataFrame & (1 << 4);

    dottedFlag = Get_DottedLine();
}

void TaskHandlerDeclare(LogOutput)(void) {
    //日志输出
//    DebugLog("%f,%f,%d", rightspd.ctr.aim, leftspd.ctr.aim, TIM1->CCR1);
//    DebugLog("%d,%d,%d,%f,%d", crossState, getT, crossHistroy, leftspd.ctr.aim, getCross);
//    LED_ON(forceTurn << 2 | delayFlag2 << 1 | delayFlag1 << 0);
}

void TaskHandlerDeclare(CrossDelay)(void) {
    if (mainState == STATE_TransportMedicine) {
        static uint32_t leftCntLoad, rightCntLoad;
        if (delayFlag1) {
            if (delayPreFlag1) {
                delayPreFlag1 = 0;
                rightCntLoad = TIM5->CNT;
            }
            if (TIM5->CNT - rightCntLoad > CrossDelayCnt && !forceTurn) {
                delayFlag1 = 0;
                Set_Speed(MAXSpeed, MAXSpeed);
            }
        }
    }
}