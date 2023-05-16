/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "trackle_io_cloud.h"
#include <trackle_esp32.h>

static const char *TAG = "button";

#define CHECK_INPUT_INTERVAL 1000
#define MIN_PUBLISH_INTERVAL 250
unsigned long last_check_input = 0;
unsigned long last_check_publish = 0;
void *(*saveConfigCb)();

void addEventToQueue(char name[], char data[])
{
    memset(writeMessage.name, 0, EVENT_BUFSIZE);
    memset(writeMessage.data, 0, JSON_BUFSIZE);
    memcpy(writeMessage.name, name, EVENT_BUFSIZE);
    memcpy(writeMessage.data, data, JSON_BUFSIZE);
    ESP_LOGI(TAG, "addMessageToQueue %s: %s", name, data);

    xQueueSend(publishMessagesQueue, (void *)&writeMessage, (TickType_t)0);
}

void publishEventFromQueue(uint32_t millis)
{
    if (millis - last_check_publish >= MIN_PUBLISH_INTERVAL)
    {
        last_check_publish = millis;

        if (trackleConnected(trackle_s) && xQueueReceive(publishMessagesQueue, &readMessage, (TickType_t)0))
        {
            ESP_LOGI(TAG, "publishing %s: %s", readMessage.name, readMessage.data);
            tracklePublishSecure(readMessage.name, readMessage.data);
        }
    }
}

static void *getInputStatus(const char *args)
{
    uint8_t inputId = atoi(args);
    if (inputId != 1)
        return "{}";
    inputId -= 1; // convert to array index

    static char jsonBuffer[JSON_BUFSIZE] = {0};
    char *json = jsonBuffer;
    json += sprintf(json, "{");
    json += sprintf(json, "\"status\":%d,", trackle_io_inputStatus[inputId].status);
    json += sprintf(json, "\"falling_count\":%d,", trackle_io_input[inputId].falling_count);
    json += sprintf(json, "\"rising_count\":%d,", trackle_io_input[inputId].rising_count);
    json += sprintf(json, "\"single_pulse_count\":%d,", trackle_io_input[inputId].single_pulse_count);
    json += sprintf(json, "\"double_pulse_count\":%d,", trackle_io_input[inputId].double_pulse_count);
    json += sprintf(json, "\"long_pulse_count\":%d", trackle_io_input[inputId].long_pulse_count);
    json += sprintf(json, "}");

    return jsonBuffer;
}

static void *getInputConfig(const char *args)
{
    uint8_t inputId = atoi(args);
    if (inputId != 1)
        return "{}";
    inputId -= 1; // convert to array index

    static char jsonBuffer[JSON_BUFSIZE] = {0};
    char *json = jsonBuffer;
    json += sprintf(json, "{");
    json += sprintf(json, "\"publish_interval\":%d,", trackle_io_input[inputId].publish_interval);
    json += sprintf(json, "\"publish_changes\":%d", trackle_io_input[inputId].publish_changes);
    json += sprintf(json, "}");

    return jsonBuffer;
}

static void *getOutputStatus(const char *args)
{
    uint8_t outputId = atoi(args);
    if (outputId != 1 && outputId != 2)
        return "{}";
    outputId -= 1; // convert to array index

    static char jsonBuffer[JSON_BUFSIZE] = {0};
    char *json = jsonBuffer;
    json += sprintf(json, "{");
    json += sprintf(json, "\"status\":%d", trackle_io_output[outputId].status);
    json += sprintf(json, "}");

    return jsonBuffer;
}

int postResetInputCounter(const char *args)
{
    uint8_t inputId = atoi(args);
    if (inputId != 1)
        return -1;

    inputId -= 1; // convert to array index

    trackle_io_input[inputId].falling_count = 0;
    trackle_io_input[inputId].rising_count = 0;
    trackle_io_input[inputId].single_pulse_count = 0;
    trackle_io_input[inputId].double_pulse_count = 0;
    trackle_io_input[inputId].long_pulse_count = 0;

    return 1;
}

int postSetOutput(const char *args)
{
    char *key = strtok(args, ",");
    if (key == NULL)
        return -1;

    char *value = strtok(NULL, ",");
    if (value == NULL)
        return -1;

    uint8_t outputId = atoi(key);
    if (outputId != 1 && outputId != 2)
        return -1;
    outputId -= 1; // convert to array index

    uint8_t outputValue = atoi(value);
    if (outputValue != 0 && outputValue != 1)
        return -1;

    trackle_io_output[outputId].status = outputValue;
    gpio_set_level(trackle_io_output[outputId].gpio_num, trackle_io_output[outputId].status);

    return 1;
}

int postSetInputInterval(const char *args)
{
    char *key = strtok(args, ",");
    if (key == NULL)
        return -1;

    char *value = strtok(NULL, ",");
    if (value == NULL)
        return -1;

    uint8_t inputId = atoi(key);
    if (inputId != 1)
        return -1;
    inputId -= 1; // convert to array index

    uint32_t intervalValue = atoi(value);

    trackle_io_input[inputId].publish_interval = intervalValue;

    // callback to save configuration to storage
    if (saveConfigCb)
    {
        (*saveConfigCb)();
    }

    return 1;
}

int postSetInputChange(const char *args)
{
    char *key = strtok(args, ",");
    if (key == NULL)
        return -1;

    char *value = strtok(NULL, ",");
    if (value == NULL)
        return -1;

    uint8_t inputId = atoi(key);
    if (inputId != 1)
        return -1;
    inputId -= 1; // convert to array index

    uint8_t onChangeValue = atoi(value);
    if (onChangeValue != 0 && onChangeValue != 1)
        return -1;

    trackle_io_input[inputId].publish_changes = onChangeValue;

    // callback to save configuration to storage
    if (saveConfigCb)
    {
        (*saveConfigCb)();
    }

    return 1;
}

void checkPublishInput(uint32_t millis)
{
    if (millis - last_check_input >= CHECK_INPUT_INTERVAL)
    {
        last_check_input = millis;

        for (int i = 0; i < INPUT_NUMBER; i++)
        {
            // check if changed
            if (trackle_io_input[i].publish_changes && trackle_io_inputStatus[i].status != trackle_io_inputStatus[i].last_published_status)
            {
                trackle_io_inputStatus[i].last_published_status = trackle_io_inputStatus[i].status;

                char ev_name[EVENT_BUFSIZE];
                char ev_data[JSON_BUFSIZE];
                sprintf(ev_name, "input%d/change", i + 1);
                sprintf(ev_data, "{\"status\":%d}", trackle_io_inputStatus[i].status);
                addEventToQueue(ev_name, ev_data);
            }

            // check if publish interval
            if (trackle_io_input[i].publish_interval > 0 && (millis - trackle_io_inputStatus[i].last_publish_interval_ms >= trackle_io_input[i].publish_interval * 1000))
            {
                trackle_io_inputStatus[i].last_publish_interval_ms = millis;

                char input_id[JSON_BUFSIZE];
                char ev_name[EVENT_BUFSIZE];
                char ev_data[JSON_BUFSIZE];
                sprintf(input_id, "%d", (i + 1));
                sprintf(ev_name, "input%d/interval", i + 1);
                sprintf(ev_data, "%s", (const char *)getInputStatus(input_id));
                addEventToQueue(ev_name, ev_data);
            }
        }
    }
}

void TrackleIo_registerCallbacks()
{
    // Initialize publish cache queue
    publishMessagesQueue = xQueueCreate(10, sizeof(struct PublishMessage));

    tracklePost(trackle_s, "ResetInputCounter", postResetInputCounter, ALL_USERS);
    tracklePost(trackle_s, "SetOutput", postSetOutput, ALL_USERS);
    tracklePost(trackle_s, "SetInputPublishInterval", postSetInputInterval, ALL_USERS);
    tracklePost(trackle_s, "SetInputPublishOnChange", postSetInputChange, ALL_USERS);

    trackleGet(trackle_s, "GetInputStatus", getInputStatus, VAR_JSON);
    trackleGet(trackle_s, "GetInputConfig", getInputConfig, VAR_JSON);
    trackleGet(trackle_s, "GetOutputStatus", getOutputStatus, VAR_JSON);
}

void TrackleIo_loop(uint32_t millis)
{
    checkPublishInput(millis);
    publishEventFromQueue(millis);
}

bool TrackleIo_createInput(uint8_t id, gpio_num_t num_gpio, uint16_t long_press_time, uint16_t short_press_time)
{
    // validate id
    if (id >= INPUT_NUMBER)
    {
        ESP_LOGE(TAG, "input %d does not exists!", id);
        return false;
    }

    // set gpio
    gpio_set_direction(num_gpio, GPIO_MODE_INPUT);

    // set default values
    if (long_press_time == KEEP_DEFAULT_VALUE)
        long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS;

    if (short_press_time == KEEP_DEFAULT_VALUE)
        short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS;

    // create button object
    button_config_t gpio_btn_cfg = {
        .id = id,
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = long_press_time,
        .short_press_time = short_press_time,
        .gpio_button_config = {
            .gpio_num = num_gpio,
            .active_level = 0,
        },
    };

    trackle_io_inputStatus[id].gpio_btn = iot_button_create(&gpio_btn_cfg);
    trackle_io_inputStatus[id].last_published_status = -1;

    return true;
}

bool TrackleIo_createOutput(uint8_t id, gpio_num_t num_gpio)
{
    gpio_set_direction(num_gpio, GPIO_MODE_OUTPUT);
    trackle_io_output[id].gpio_num = num_gpio;
    return true;
}

esp_err_t TrackleInput_register_cb(button_handle_t btn_handle, button_event_t event, button_cb_t cb, void *usr_data)
{
    return iot_button_register_cb(btn_handle, event, cb, usr_data);
}

void TrackleIo_setSaveConfigCb(void *(*cb)())
{
    saveConfigCb = cb;
}