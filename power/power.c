/*
 * Copyright (C) 2012 The Android Open Source Project
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
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <cutils/uevent.h>
#include <sys/poll.h>
#include <pthread.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <stdbool.h>

#define LOG_TAG "Cardu PowerHAL"
#include <utils/Log.h>

#include <hardware/hardware.h>
#include <hardware/power.h>

#include "util.h" 
#include "cardhu_power_defines.h"

/*
#define BOOST_PATH      "/sys/devices/system/cpu/cpufreq/interactive/boost"
#define UEVENT_MSG_LEN 2048
#define TOTAL_CPUS 4
#define RETRY_TIME_CHANGING_FREQ 20
#define SLEEP_USEC_BETWN_RETRY 200
#define LOW_POWER_MAX_FREQ "640000"
#define LOW_POWER_MIN_FREQ "51000"
#define NORMAL_MAX_FREQ "1300000"
#define UEVENT_STRING "online@/devices/system/cpu/"
*/

static int boost_fd = -1;
static int boost_warned;

static struct pollfd pfd;
static char *cpu_path_min[] = {
    "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq",
    "/sys/devices/system/cpu/cpu1/cpufreq/scaling_min_freq",
    "/sys/devices/system/cpu/cpu2/cpufreq/scaling_min_freq",
    "/sys/devices/system/cpu/cpu3/cpufreq/scaling_min_freq",
};
static char *cpu_path_max[] = {
    "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq",
    "/sys/devices/system/cpu/cpu1/cpufreq/scaling_max_freq",
    "/sys/devices/system/cpu/cpu2/cpufreq/scaling_max_freq",
    "/sys/devices/system/cpu/cpu3/cpufreq/scaling_max_freq",
};
static bool freq_set[TOTAL_CPUS];
static bool low_power_mode = false;
static pthread_mutex_t low_power_mode_lock = PTHREAD_MUTEX_INITIALIZER;

static int max_freq = 0;
static int max_cpus = 0;

/*
Helper functions
*/

static void write_cpuquiet_max(const char* value)
{
    char buf[80];

    if (!strcmp(value, PROFILE_MAX_TAG)) {
        sprintf(buf, "%d", max_cpus);
    } else {
        strcpy(buf, value);
    }
#ifdef DEBUG
    ALOGI("write %s -> %s", CPUS_MAX_PATH, buf);
#endif
    sysfs_write(CPUS_MAX_PATH, buf);
}

static void write_cpufreq_values(const char* key, const char* value)
{
    char cpufreq_path[256];
    int i;
    char buf[80];

    if (!strcmp(value, PROFILE_MAX_TAG)) {
        sprintf(buf, "%d", max_freq);
    } else {
        strcpy(buf, value);
    }

    for (i = 0; i < max_cpus; i++) {
        write_cpufreq_value(i, key, buf);
    }
}

/*
Function to apply the power profile
*/

static void apply_profile(char* profile)
{
    char *token;
    char *separator = PROFILE_SEPARATOR;
    char max_freq_profile[80];
    char max_cpu_profile[80];

    token = strtok(profile, separator);
    while(token != NULL) {
#ifdef DEBUG
        ALOGI("token %s", token);
#endif
        if (!strcmp(token, PROFILE_MAX_CPU_NUM_TAG)) {
            token = strtok(NULL, separator);
            strcpy(max_cpu_profile, token);
        } else if (!strcmp(token, PROFILE_MAX_CPU_FREQ_TAG)) {
            token = strtok(NULL, separator);
            strcpy(max_freq_profile, token);
        }
        token = strtok(NULL, separator);
    }

#ifdef DEBUG
    ALOGI("max_freq_profile %s", max_freq_profile);
    ALOGI("max_cpu_profile %s", max_cpu_profile);
#endif

    // the API will ignore invalid values so we dont need to check it here
    if (strlen(max_freq_profile) != 0) {
        write_cpufreq_values(SCALING_MAX_PATH, max_freq_profile);
    }
    if (strlen(max_cpu_profile) != 0) {
        write_cpuquiet_max(max_cpu_profile);
    }
}

static int uevent_event()
{
    char msg[UEVENT_MSG_LEN];
    char *cp;
    int n, cpu, ret, retry = RETRY_TIME_CHANGING_FREQ;

    n = recv(pfd.fd, msg, UEVENT_MSG_LEN, MSG_DONTWAIT);
    if (n <= 0) {
        return -1;
    }
    if (n >= UEVENT_MSG_LEN) {   /* overflow -- discard */
        return -1;
    }

    cp = msg;

    if (strstr(cp, UEVENT_STRING)) {
        n = strlen(cp);
        errno = 0;
        cpu = strtol(cp + n - 1, NULL, 10);

        if (errno == EINVAL || errno == ERANGE || cpu < 0 || cpu >= TOTAL_CPUS) {
            return -1;
        }

        pthread_mutex_lock(&low_power_mode_lock);
        if (low_power_mode && !freq_set[cpu]) {
            while (retry) {
                sysfs_write(cpu_path_min[cpu], LOW_POWER_MIN_FREQ);
                ret = sysfs_write_silent(cpu_path_max[cpu], LOW_POWER_MAX_FREQ);
                if (!ret) {
                    freq_set[cpu] = true;
                    break;
                }
                usleep(SLEEP_USEC_BETWN_RETRY);
                retry--;
           }
        } else if (!low_power_mode && freq_set[cpu]) {
             while (retry) {
                  ret = sysfs_write_silent(cpu_path_max[cpu], NORMAL_MAX_FREQ);
                  if (!ret) {
                      freq_set[cpu] = false;
                      break;
                  }
                  usleep(SLEEP_USEC_BETWN_RETRY);
                  retry--;
             }
        }
        pthread_mutex_unlock(&low_power_mode_lock);
    }
    return 0;
}

void *thread_uevent(__attribute__((unused)) void *x)
{
    while (1) {
        int nevents, ret;

        nevents = poll(&pfd, 1, -1);

        if (nevents == -1) {
            if (errno == EINTR)
                continue;
            ALOGE("powerhal: thread_uevent: poll_wait failed\n");
            break;
        }
        ret = uevent_event();
        if (ret < 0)
            ALOGE("Error processing the uevent event");
    }
    return NULL;
}


static void uevent_init()
{
    struct sockaddr_nl client;
    pthread_t tid;
    pfd.fd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);

    if (pfd.fd < 0) {
        ALOGE("%s: failed to open: %s", __func__, strerror(errno));
        return;
    }
    memset(&client, 0, sizeof(struct sockaddr_nl));
    pthread_create(&tid, NULL, thread_uevent, NULL);
    client.nl_family = AF_NETLINK;
    client.nl_pid = tid;
    client.nl_groups = -1;
    pfd.events = POLLIN;
    bind(pfd.fd, (void *)&client, sizeof(struct sockaddr_nl));
    return;
}


static void cardhu_power_init( __attribute__((unused)) struct power_module *module)
{

    /* Initialize static vars */
    char freq[80];
    if(get_max_freq(freq, sizeof(freq))==0){
        max_freq = atoi(freq);
    }
    ALOGI("max_freq %s %d", freq, max_freq);
    char cpus[80];
    if(get_max_cpus(cpus, sizeof(cpus))==0){
        if (strlen(cpus) == 3) {
            max_cpus = atoi(cpus + 2) + 1;
        }
    }
    ALOGI("max_cpus %s %d", cpus, max_cpus);

    /*
     * cpufreq interactive governor: timer 20ms, min sample 100ms,
     * hispeed 700MHz at load 40%
     */

    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/timer_rate",
                "50000");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/min_sample_time",
                "500000");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/go_hispeed_load",
                "75");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/boost_factor",
		"0");
    sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/input_boost",
		"1");
    uevent_init();
}

static void cardhu_power_set_interactive(__attribute__((unused)) struct power_module *module,
                                          __attribute__((unused)) int on)
{
	if (on) {
		sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/go_hispeed_load", "75");
		sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/core_lock_period", "3000000");
		sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/core_lock_count", "2");
		sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/input_boost", "1");
	}
	else {
		sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/go_hispeed_load", "85");
		sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/core_lock_period", "200000");
		sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/core_lock_count", "0");
		sysfs_write("/sys/devices/system/cpu/cpufreq/interactive/input_boost", "0");
	}
}

static void cardhu_power_hint(__attribute__((unused)) struct power_module *module, power_hint_t hint,
                            void *data)
{
    char buf[80];
    int len, cpu, ret;

    switch (hint) {
    case POWER_HINT_VSYNC:
        break;

    case POWER_HINT_POWER_PROFILE:
#ifdef DEBUG
        ALOGI("POWER_HINT_POWER_PROFILE %s", (char*)data);
#endif
        // profile is contributed as string with key value
        // pairs separated with ":"
        apply_profile((char*)data);
        break;

    case POWER_HINT_LOW_POWER:
        // The LOW_POWER is handled by the power profiles
        // So this code is not needed
#ifdef DEBUG
        ALOGI("POWER_HINT_LOW_POWER");
#endif
        break;
    default:
            break;
    }
}

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_2,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "Cardu Power HAL",
        .author = "The Android Open Source Project",
        .methods = &power_module_methods,
    },

    .init = cardhu_power_init,
    .setInteractive = cardhu_power_set_interactive,
    .powerHint = cardhu_power_hint,
};
