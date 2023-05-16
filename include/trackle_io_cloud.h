/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

// FreeRTOS includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"

#include <stdio.h>
#include <string.h>
#include "iot_button.h"

#define JSON_BUFSIZE 1024
#define EVENT_BUFSIZE 100
#define KEEP_DEFAULT_VALUE 0

#ifndef INPUT_NUMBER
#error "INPUT_NUMBER must be defined"
#endif

#ifndef OUTPUT_NUMBER
#error "OUTPUT_NUMBER must be defined"
#endif

typedef struct InputData_s
{
    uint32_t publish_interval;   // seconds to publish event
    bool publish_changes;        // if true, publish status on change
    uint32_t falling_count;      // button released count
    uint32_t rising_count;       // button pressed count
    uint32_t single_pulse_count; // button single pulse count
    uint32_t double_pulse_count; // button double pulse count
    uint32_t long_pulse_count;   // button long pulse count
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
    uint32_t reserved4;
} InputData_t;

typedef struct InputStatus_s
{
    button_handle_t gpio_btn;
    uint8_t status; // 0 not pressed, 1 pressed
    uint32_t last_publish_interval_ms;
    int8_t last_published_status;
} InputStatus_t;

typedef struct OutputData_s
{
    uint8_t gpio_num;
    uint8_t status; // 0 open, 1 close
    uint32_t reserved1;
    uint32_t reserved2;
    uint32_t reserved3;
    uint32_t reserved4;
} OutputData_t;

InputData_t trackle_io_input[INPUT_NUMBER];
InputStatus_t trackle_io_inputStatus[INPUT_NUMBER];
OutputData_t trackle_io_output[OUTPUT_NUMBER];

struct PublishMessage
{
    char name[EVENT_BUFSIZE];
    char data[JSON_BUFSIZE];
};
struct PublishMessage readMessage;
struct PublishMessage writeMessage;

xQueueHandle publishMessagesQueue;

void TrackleIo_registerCallbacks();
void TrackleIo_loop(uint32_t millis);
bool TrackleIo_createInput(uint8_t id, gpio_num_t num_gpio, uint16_t long_press_time, uint16_t short_press_time);
bool TrackleIo_createOutput(uint8_t id, gpio_num_t num_gpio);
esp_err_t TrackleInput_register_cb(button_handle_t btn_handle, button_event_t event, button_cb_t cb, void *usr_data);
void TrackleIo_setSaveConfigCb(void *(*cb)());

#ifdef __cplusplus
}
#endif