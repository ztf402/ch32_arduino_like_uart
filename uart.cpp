/**
 * @file uart.cpp
 * @author ztf402 (ztf402@qq.com)
 * @brief An arduino like UART class for 32 series MCUs
 * @version 0.1
 * @date 2025-05-27
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "uart.hpp"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <ctime> // for clock()

UART::UART(USART_TypeDef* usart, GPIO_TypeDef* tx_port, uint16_t tx_pin, GPIO_TypeDef* rx_port, uint16_t rx_pin)
    : usart_(usart), tx_port_(tx_port), tx_pin_(tx_pin), rx_port_(rx_port), rx_pin_(rx_pin) {}

void UART::begin(uint32_t baud, UART_IRQHandler_t rxIrqHandler) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    // 1. 打开 GPIO 时钟
    if (tx_port_ == GPIOA) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    if (tx_port_ == GPIOB) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    if (tx_port_ == GPIOC) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    if (tx_port_ == GPIOD) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    if (tx_port_ == GPIOE) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

    if (rx_port_ == GPIOA) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    if (rx_port_ == GPIOB) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    if (rx_port_ == GPIOC) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    if (rx_port_ == GPIOD) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    if (rx_port_ == GPIOE) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

    // 2. 打开 USART 时钟
    if (usart_ == USART1) RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    if (usart_ == USART2) RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    if (usart_ == USART3) RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    if (usart_ == UART4)  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    if (usart_ == UART5)  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

    // 3. 配置 TX
    GPIO_InitStructure.GPIO_Pin = tx_pin_;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(tx_port_, &GPIO_InitStructure);

    // 4. 配置 RX
    GPIO_InitStructure.GPIO_Pin = rx_pin_;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(rx_port_, &GPIO_InitStructure);

    // 5. 配置 USART
    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(usart_, &USART_InitStructure);

    USART_Cmd(usart_, ENABLE);

    // 可选：配置接收中断
    if (rxIrqHandler) {
        _rxIrqHandler = rxIrqHandler;
        USART_ITConfig(usart_, USART_IT_RXNE, ENABLE);

        // 使能 NVIC
        if (usart_ == USART1) {
            NVIC_EnableIRQ(USART1_IRQn);
        } else if (usart_ == USART2) {
            NVIC_EnableIRQ(USART2_IRQn);
        } else if (usart_ == USART3) {
            NVIC_EnableIRQ(USART3_IRQn);
        } else if (usart_ == UART4) {
            NVIC_EnableIRQ(UART4_IRQn);
        } else if (usart_ == UART5) {
            NVIC_EnableIRQ(UART5_IRQn);
        }
    }
}

void UART::begin(uint32_t baud) {
    begin(baud, nullptr);
}
    
    
    

void UART::end() {
    usart_->CTLR1 &= ~USART_CTLR1_UE;
}

int UART::available() {
    return (usart_->STATR & USART_STATR_RXNE) ? 1 : 0;
}

int UART::peek() {
    // 没有硬件FIFO，peek无法实现，直接返回-1或上次读到的值
    return -1;
}

int UART::read() {
    if (available()) {
        // 只用USART_ReceiveData，确保读的是接收缓冲区
        return usart_->DATAR & 0xFF;
    }
    return -1;
}

void UART::flush() {
    while (!(usart_->STATR & USART_STATR_TC));
}

size_t UART::write(uint8_t data) {
    while (!(usart_->STATR & USART_STATR_TXE));
    usart_->DATAR = data;
    while (!(usart_->STATR & USART_STATR_TC)); // 等待发送完成
    return 1;
}

size_t UART::write(const uint8_t* buf, size_t len) {
    size_t sent = 0;
    for (size_t i = 0; i < len; ++i) {
        sent += write(buf[i]);
    }
    return sent;
}

// Print/println实现
size_t UART::print(const char* str) {
    return write((const uint8_t*)str, strlen(str));
}
size_t UART::print(char c) {
    return write((uint8_t)c);
}
size_t UART::print(int n, int base) {
    char buf[32];
    if (base == 10)
        snprintf(buf, sizeof(buf), "%d", n);
    else if (base == 16)
        snprintf(buf, sizeof(buf), "%x", n);
    else if (base == 8)
        snprintf(buf, sizeof(buf), "%o", n);
    else
        snprintf(buf, sizeof(buf), "%d", n);
    return print(buf);
}
size_t UART::print(unsigned int n, int base) {
    char buf[32];
    if (base == 10)
        snprintf(buf, sizeof(buf), "%u", n);
    else if (base == 16)
        snprintf(buf, sizeof(buf), "%x", n);
    else if (base == 8)
        snprintf(buf, sizeof(buf), "%o", n);
    else
        snprintf(buf, sizeof(buf), "%u", n);
    return print(buf);
}
size_t UART::print(long n, int base) {
    char buf[32];
    if (base == 10)
        snprintf(buf, sizeof(buf), "%ld", n);
    else if (base == 16)
        snprintf(buf, sizeof(buf), "%lx", n);
    else if (base == 8)
        snprintf(buf, sizeof(buf), "%lo", n);
    else
        snprintf(buf, sizeof(buf), "%ld", n);
    return print(buf);
}
size_t UART::print(unsigned long n, int base) {
    char buf[32];
    if (base == 10)
        snprintf(buf, sizeof(buf), "%lu", n);
    else if (base == 16)
        snprintf(buf, sizeof(buf), "%lx", n);
    else if (base == 8)
        snprintf(buf, sizeof(buf), "%lo", n);
    else
        snprintf(buf, sizeof(buf), "%lu", n);
    return print(buf);
}
size_t UART::print(float n, int digits) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", digits, n);
    return print(buf);
}
size_t UART::print(double n, int digits) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*lf", digits, n);
    return print(buf);
}
size_t UART::print(const std::string& s) {
    return print(s.c_str());
}

size_t UART::println() {
    return print("\r\n");
}
size_t UART::println(const char* str) {
    size_t n = print(str);
    n += println();
    return n;
}
size_t UART::println(char c) {
    size_t n = print(c);
    n += println();
    return n;
}
size_t UART::println(int n, int base) {
    size_t x = print(n, base);
    x += println();
    return x;
}
size_t UART::println(unsigned int n, int base) {
    size_t x = print(n, base);
    x += println();
    return x;
}
size_t UART::println(long n, int base) {
    size_t x = print(n, base);
    x += println();
    return x;
}
size_t UART::println(unsigned long n, int base) {
    size_t x = print(n, base);
    x += println();
    return x;
}
size_t UART::println(float n, int digits) {
    size_t x = print(n, digits);
    x += println();
    return x;
}
size_t UART::println(double n, int digits) {
    size_t x = print(n, digits);
    x += println();
    return x;
}
size_t UART::println(const std::string& s) {
    size_t n = print(s);
    n += println();
    return n;
}

// Stream-like
void UART::setTimeout(unsigned long timeout) {
    _timeout = timeout;
}

// 获取毫秒数（通用实现，适配裸机/RTOS/PC）
static unsigned long getMillis() {
    return (unsigned long)(clock() * 1000 / CLOCKS_PER_SEC);
}

int UART::timedRead() {
    unsigned long start = getMillis();
    while ((getMillis() - start) < _timeout) {
        if (available()) return read();
    }
    return -1;
}

int UART::timedPeek() {
    unsigned long start = getMillis();
    while ((getMillis() - start) < _timeout) {
        if (available()) return peek();
    }
    return -1;
}

size_t UART::readBytes(char* buffer, size_t length) {
    size_t count = 0;
    while (count < length) {
        int c = timedRead();
        if (c < 0) break;
        buffer[count++] = (char)c;
    }
    return count;
}

size_t UART::readBytesUntil(char terminator, char* buffer, size_t length) {
    if (length < 1) return 0;
    size_t index = 0;
    while (index < length) {
        int c = timedRead();
        if (c < 0 || c == terminator) break;
        buffer[index++] = (char)c;
    }
    return index;
}

std::string UART::readString() {
    std::string ret;
    int c = timedRead();
    while (c >= 0) {
        ret += (char)c;
        c = timedRead();
    }
    return ret;
}

std::string UART::readStringUntil(char terminator) {
    std::string ret;
    unsigned long start = getMillis();
    while ((getMillis() - start) < _timeout) {
        if (available()) {
            char c = read();
            if (c == terminator) break;
            ret += c;
            // 每收到一个字符就重置超时
            start = getMillis();
        }
    }
    return ret;
}

bool UART::isRxComplete() const {
    return (usart_->STATR & USART_STATR_RXNE) != 0;
}

std::string UART::readAll() {
    std::string ret;
    while (available()) {
        int c = read();
        if (c >= 0) ret += (char)c;
    }
    return ret;
}