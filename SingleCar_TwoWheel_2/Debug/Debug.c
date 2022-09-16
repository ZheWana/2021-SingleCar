/**
 * @file Debug.c
 * @brief Debug相关函数定义
 * @author ZheWana
 * @date 2021/9/30
 */

#include "Debug.h"

int UART_printf(UART_HandleTypeDef *huart, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int length;
    char buffer[128];

    length = vsnprintf(buffer, 128, fmt, ap);

    HAL_UART_Transmit(huart, (uint8_t *) buffer, length, HAL_MAX_DELAY);

    va_end(ap);
    return length;
}