/**
 * @file uart.hpp
 * @author ztf402 (ztf402@qq.com)
 * @brief An arduino like UART class for 32 series MCUs
 * @version 0.1
 * @date 2025-05-27
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string>

#if defined(CH32V30x)||defined(CH32V30x_C)||defined(CH32V20x)||defined(CH32V10x)||defined(CH32L10x)||defined(CH32V00x)||defined(CH32VM00X)||defined(CH32X035)
#include "ch32yyxx.h" 
#include "ch32yyxx_usart.h"
#include "ch32yyxx_rcc.h"
#include "ch32yyxx_gpio.h"
#endif
//support for all ch32
#if defined(STM32F1)||defined(STM32F4)||defined(STM32F0)||defined(STM32L0)
#endif
//add hal support for stm32
typedef void (*UART_IRQHandler_t)(void);

class UART {
public:
    UART(USART_TypeDef* usart, GPIO_TypeDef* tx_port, uint16_t tx_pin, GPIO_TypeDef* rx_port, uint16_t rx_pin);

    void begin(uint32_t baud); // 无中断
    void begin(uint32_t baud, UART_IRQHandler_t rxIrqHandler); // 带中断
    void end();
    int available();
    int peek();
    int read();
    void flush();
    size_t write(uint8_t data);
    size_t write(const uint8_t* buf, size_t len);

    // Print/println for various types
    size_t print(const char* str);
    size_t print(char c);
    size_t print(int n, int base = 10);
    size_t print(unsigned int n, int base = 10);
    size_t print(long n, int base = 10);
    size_t print(unsigned long n, int base = 10);
    size_t print(float n, int digits = 2);
    size_t print(double n, int digits = 2);
    size_t print(const std::string& s);

    size_t println();
    size_t println(const char* str);
    size_t println(char c);
    size_t println(int n, int base = 10);
    size_t println(unsigned int n, int base = 10);
    size_t println(long n, int base = 10);
    size_t println(unsigned long n, int base = 10);
    size_t println(float n, int digits = 2);
    size_t println(double n, int digits = 2);
    size_t println(const std::string& s);

    // Stream-like
    void setTimeout(unsigned long timeout);
    size_t readBytes(char* buffer, size_t length);
    size_t readBytesUntil(char terminator, char* buffer, size_t length);
    std::string readString();
    std::string readStringUntil(char terminator);

    bool isRxComplete() const;
    std::string readAll();

private:
    USART_TypeDef* usart_;
    GPIO_TypeDef* tx_port_;
    uint16_t tx_pin_;
    GPIO_TypeDef* rx_port_;
    uint16_t rx_pin_;
    UART_IRQHandler_t _rxIrqHandler = nullptr;
    unsigned long _timeout = 1000;
    int timedRead();
    int timedPeek();
};