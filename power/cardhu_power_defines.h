/*
 * Copyright (C) 2015 OmniROM
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



/*
In this include file are all the defines located that are needed for 
the funcioning of the power hal and the power profile
*/

#define BOOST_PATH      "/sys/devices/system/cpu/cpufreq/interactive/boost"
#define UEVENT_MSG_LEN 2048
#define TOTAL_CPUS 4
#define RETRY_TIME_CHANGING_FREQ 20
#define SLEEP_USEC_BETWN_RETRY 200
#define LOW_POWER_MAX_FREQ "640000"
#define LOW_POWER_MIN_FREQ "51000"
#define NORMAL_MAX_FREQ "1300000"
#define UEVENT_STRING "online@/devices/system/cpu/"

#define SCALING_MAX_PATH "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq"
#define CPUFREQ_ROOT_PATH "/sys/devices/system/cpu/cpu"
#define CPUFREQ_TAIL_PATH "/cpufreq/"

// how much cpus ara available (for using max value) e.g. 0-3
#define CPUS_MAX_PATH "/sys/devices/system/cpu/present"

// we can handle those tags
#define PROFILE_MAX_CPU_NUM_TAG "maxCpu"
#define PROFILE_MAX_CPU_FREQ_TAG "maxFreq"
#define PROFILE_MAX_TAG "max"
#define PROFILE_SEPARATOR ":"
