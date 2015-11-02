/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ButtonService.h"

DigitalOut  led1(LED1, 1);
InterruptIn button(BUTTON1);

const static char     DEVICE_NAME[] = "Button";
static const uint16_t uuid16_list[] = {ButtonService::BUTTON_SERVICE_UUID};

ButtonService *buttonServicePtr;

void buttonPressedCallback(void)
{
    buttonServicePtr->updateButtonState(true);
}

void buttonReleasedCallback(void)
{
    buttonServicePtr->updateButtonState(false);
}

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    BLE::Instance().gap().startAdvertising(); // restart advertising
}

void blinkCallback(void)
{
    led1 = !led1; /* Do blinky on LED1 to indicate system aliveness. */
}

void onBleInitError(BLE &ble, ble_error_t error)
{
    // Initialization error handling should go here
}

void bleInitComplete(BLE &ble, ble_error_t error)
{
    if (error != BLE_ERROR_NONE) {
        // in case of error, forward the error handling to onBleInitError
        onBleInitError(ble, error);
        return;
    }

    // ensure that it is the default instance of BLE
    if(ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        return;
    }

    ble.gap().onDisconnection(disconnectionCallback);

    button.fall(buttonPressedCallback);
    button.rise(buttonReleasedCallback);

    /* Setup primary service. */
    buttonServicePtr = new ButtonService(ble, false /* initial value for button pressed */);

    /* setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();
}

void app_start(int, char**)
{
    minar::Scheduler::postCallback(blinkCallback).period(minar::milliseconds(500));

    BLE &ble = BLE::Instance();
    ble.init(bleInitComplete);
}
