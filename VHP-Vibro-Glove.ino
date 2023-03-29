// Copyright 2021-2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This Arduino example sketch shows how to use PWM on the slim board
// Plays a sin wave from a lookup on each channel.

#include "pwm_sleeve.h"
#include "board_defs.h"

using namespace audio_tactile;
static uint16_t sin_wave[8];
static uint16_t silence[8] = {0};  // all zeros for no vibration output.

int order_pairs[8] = {0, 3, 4, 5, 11, 9, 8, 6};

void OnPwmSequenceEnd();

void setup() {

  nrf_gpio_cfg_output(kLedPinBlue);
  nrf_gpio_cfg_output(kLedPinGreen);
  
  // Create a sin wav pattern to play, which is repeated every 4 ms.
  // Can be changed to any pattern here, with resolution of 0 to 512, where 512
  // is the loudest. Also, scale the output, so its not too loud.
  const int scale = 2;
  sin_wave[0] = 256 / scale;
  sin_wave[1] = 437 / scale;
  sin_wave[2] = 512 / scale;
  sin_wave[3] = 437 / scale;
  sin_wave[4] = 256 / scale;
  sin_wave[5] = 75 / scale;
  sin_wave[6] = 0 / scale;
  sin_wave[7] = 75 / scale;

  SleeveTactors.OnSequenceEnd(OnPwmSequenceEnd);
  SleeveTactors.Initialize();
  // Warning: issue only in Arduino. When using StartPlayback() it crashes.
  // Looks like NRF_PWM0 module is automatically triggered, and triggering it
  // again here crashes ISR. Temporary fix is to only use nrf_pwm_task_trigger
  // for NRF_PWM1 and NRF_PWM2. To fix might need a nRF52 driver update.
  nrf_pwm_task_trigger(NRF_PWM1, NRF_PWM_TASK_SEQSTART0);
  nrf_pwm_task_trigger(NRF_PWM2, NRF_PWM_TASK_SEQSTART0);
}

void loop() {
  nrf_gpio_pin_set(kLedPinBlue);

  // Loop through all channels.
  for (int i = 0; i < 3; i++) {
    shuffleOrder(order_pairs, 4);
    for (int c = 0; c < 4; c++) {
      Serial.print("Current activation: ");
      Serial.print(order_pairs[c]+1);
      Serial.print(" and ");
      Serial.println(order_pairs[c+4]+1);

      nrf_gpio_pin_set(kLedPinGreen);
      SleeveTactors.UpdateChannel(order_pairs[c], sin_wave);
      SleeveTactors.UpdateChannel(order_pairs[c+4], sin_wave);
      
      delay(100);
      nrf_gpio_pin_clear(kLedPinGreen);

      SleeveTactors.UpdateChannel(order_pairs[c], silence);
      SleeveTactors.UpdateChannel(order_pairs[c+4], silence);
      delay(67);
    }
  }
  nrf_gpio_pin_clear(kLedPinBlue);
  delay(1333);
}

// This function is triggered after sequence is finished playing.
void OnPwmSequenceEnd() {}


// Shuffle the ordering array (two sets in one array)
void shuffleOrder(int *order, int n)
{
  // Seed the random function with noise from analog pin 0
  randomSeed(analogRead(0)); 

  // Shuffle each element with a random later element
  for (int i = 0; i < n; i++)
  {
    int j = random(n-i);
    int temp = order[i];
    order[i] = order[i+j];
    order[i+j] = temp;

    temp = order[i+n];
    order[i+n] = order[i+j+n];
    order[i+j+n] = temp;
  }
}
