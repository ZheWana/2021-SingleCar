/**
 * @file user.h
 * @brief 
 * @author ZheWana
 * @date 2021/10/14 014
 */

#ifndef CTRLEDCAR_USER_H
#define CTRLEDCAR_USER_H

#include "usart.h"
#include "Debug.h"
#include "timepiece.h"

#define LineNum             13

#define MAXSpeed            35
#define LowSpeed            15

#define CrossCnt            600
#define TurnCnt             1000
#define BackCnt             1300
#define DottedCnt           800
#define CrossDelayCnt       400

#define STATE_Debug                 0xff
#define STATE_MedicineRoom          0
#define STATE_TransportMedicine     1
#define STATE_PacientRoom           2
#define STATE_GetMedicineMiddle     3
#define STATE_GetMedicineFar        4

#define GetMedicine_TurnAround      0
#define GetMedicine_Turn            1
#define GetMedicine_TurnBig         2
#define GetMedicine_Go              3

#define MedicineExist               0
#define MedicineNone                1

#define ForceTurn_Start             forceTurn = 1
#define ForceTurn_Stop              forceTurn = 0

#define DIR_LEFT                    0
#define DIR_RIGHT                   1
#define DIR_BACK                    2

typedef struct PID {
    double kp;
    double ki;
    double kd;
    int32_t CNT_pre;
    int32_t CNT_cur;
    struct ctr {
        double cur;
        double pre;
        double aim;
    } ctr;
    struct error {
        double cur;
        double pre;
        double sum;
        double bia;
    } error;

} pid;

#define LED_RED         1<<0
#define LED_YELLOW      1<<1
#define LED_GREEN       1<<2

uint8_t Get_DottedLine(void);

void CalibrationDelay(int CNT);

void My_UART_IDLECallback(UART_HandleTypeDef *huart);

void ForceTurn(uint8_t dirMask);

void Set_Speed(float leftSpd, float rightSpd);

void LED_ON(uint8_t ledMask);

double PID_Realize(pid *ctrl);

void PID_Init(pid *ctrl, double kp, double ki, double kd, double aim);

void PWM_Handle(double leftspd, double rightspd);

uint8_t Get_MedicineState(void);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

float LimitingOut(float data, float highVal, float lowVal);

void TaskHandlerDeclare(PIDControl)(void);

void TaskHandlerDeclare(StateDetect)(void);

void TaskHandlerDeclare(LogOutput)(void);

void TaskHandlerDeclare(CrossDelay)(void);

#endif //CTRLEDCAR_USER_H
