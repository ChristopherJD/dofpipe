#include <stdio.h>
#include <stdlib.h>
#include <lsm9ds1.h>
#include <cjson/cJSON.h>
#include <argp.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "debug.h"

#define _BUILD_VERSION BUILD_VERSION

volatile static bool run = true;

static FILE *out = NULL;
static FILE *f = NULL;

struct sensor_data {
    float x;
    float y;
    float z;
};

const char *argp_program_version = _BUILD_VERSION;
const char *argp_program_bug_address = "jordan.denny5@gmail.com";
static char doc[] = "Continuously print out LSM9DS1 data in JSON format.";
static char args_doc[] = "[FILENAME]...";
static struct argp_option options[] = { 
    { "file", 'f', "FILENAME", 0, "Filename to output data to."},
    { "once", '1', 0, 0, "Query the LSM9DS1 once. Default is to continuously query the LSM9DS1."},
    { "time", 't', "QUERYTIME", 0, "How often to query the LSM9DS1."},
    { 0 } 
};

struct arguments {
    enum { STDOUT, FILEOUT } output;
    char *filename;
    bool run_once;
    unsigned long int sleep_time;
};

static struct arguments arguments = {0};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    switch (key) {
        case 'o': arguments->output = STDOUT; break;
        case 'f':
            DEBUG_PRINT("Copying Filename %s\n", arg);
            int arg_strlen = strlen(arg) + 1; // +1 full null terminator
            arguments->filename = malloc(arg_strlen);
            strncpy(arguments->filename, arg, arg_strlen);
            arguments->output = FILEOUT; 
            break;
        case '1':
            arguments->run_once = true;
            break;
        case 't':
            arguments->sleep_time = strtoul(arg, NULL, 0);
            if(arguments->sleep_time == ULONG_MAX) run = false;
            break;
        case ARGP_KEY_ARG: return 0;
        default: return ARGP_ERR_UNKNOWN;
    }   
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

void handle_signal(int signum) {

    DEBUG_PRINT("Handling signal %d\n", signum);
    switch(signum) {
        case SIGTERM:
            DEBUG_PRINT("Handling SIGTERM\n");
            run = false;
            break;
        case SIGHUP:
            DEBUG_PRINT("Received SIGHUP\n");
            DEBUG_PRINT("Reopenig %s\n", arguments.filename);
            if(arguments.output == FILEOUT) {
                out = freopen(arguments.filename, "w", out);
                if(out == NULL) run = false;
            }
            break;
        default:
            // Do Nothing
            break;
    }
}

cJSON *json_create_sensor_str(struct sensor_data *data) {

    cJSON *sensor = cJSON_CreateObject();
    if(sensor == NULL) {
        return NULL;
    }

    cJSON *sensor_x = cJSON_CreateNumber(data->x);
    cJSON *sensor_y = cJSON_CreateNumber(data->y);
    cJSON *sensor_z = cJSON_CreateNumber(data->z);

    if((sensor_x == NULL) || (sensor_y == NULL) || (sensor_z == NULL)) {
        DEBUG_PRINT("Freeing JSON sensor\n");
        cJSON_Delete(sensor);
        return NULL;
    }

    cJSON_AddItemToObject(sensor, "x", sensor_x);
    cJSON_AddItemToObject(sensor, "y", sensor_y);
    cJSON_AddItemToObject(sensor, "z", sensor_z);

    return sensor;
}

int main(int argc, char *argv[]) {

    DEBUG_PRINT("Debug Mode...\n");

    struct sigaction term_action = {0};
    term_action.sa_handler = handle_signal;
    sigaction(SIGTERM, &term_action, NULL);

    struct sigaction hup_action = {0};
    hup_action.sa_handler = handle_signal;
    sigaction(SIGHUP, &hup_action, NULL);

    arguments.output = STDOUT;
    arguments.filename = NULL;
    arguments.run_once = false;
    arguments.sleep_time = 100; // 100 microseconds.

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    switch(arguments.output) {
        case STDOUT:
            out = stdout;
            break;
        case FILEOUT:
            DEBUG_PRINT("Opening File: %s\n", "test3.json");
            if(arguments.filename == NULL) run = false;
            f = fopen(arguments.filename, "w+");
            if(f == NULL) run = false;
            out = f;
            break;
        default:
            out = stdout;
            break;
    };

	lsm9ds1_status_t status = LSM9DS1_UNKNOWN_ERROR;
	status = lsm9ds1_init();
	if(status < 0) {
		fprintf(stderr, "Error initializing lsm9ds1!\n");
        exit(EXIT_FAILURE);
	}

    accelerometer_converted_data_t accel_data = {0};
    gyro_converted_data_t gyro_data = {0};
    mag_converted_data_t mag_data = {0};
    cJSON *dof = NULL;
    cJSON *accelerometer = NULL;
    cJSON *magnetometer = NULL;
    cJSON *gyroscope = NULL;
    char *string = NULL;

    struct timespec ts = {0};

    while(run)
    {
        // string = NULL;
        // dof = NULL;

    	status = get_accel(&accel_data);
        if(status < 0) {
            break;
        }

    	status = get_gyro(&gyro_data);
        if(status < 0) {
            break;
        }

    	status = get_mag(&mag_data);
        if(status < 0) {
            break;
        }

        dof = cJSON_CreateObject();
        if(dof == NULL) {
            break;
        }

        accelerometer = json_create_sensor_str(
            (struct sensor_data *)&accel_data
        );
        if(accelerometer == NULL) {
            break;
        }
        magnetometer = json_create_sensor_str(
            (struct sensor_data *)&mag_data
        );
        if(magnetometer == NULL) {
            break;
        }
        gyroscope = json_create_sensor_str(
            (struct sensor_data *)&gyro_data
        );
        if(gyroscope == NULL) {
            break;
        }
        cJSON_AddItemToObject(dof, "accelerometer", accelerometer);
        cJSON_AddItemToObject(dof, "magnetometer", magnetometer);
        cJSON_AddItemToObject(dof, "gyroscope", gyroscope);

        string = cJSON_Print(dof);
        if(string == NULL) {
            break;
        }

        fprintf(out, "%s\n", string);
        fflush(out);

        if(arguments.run_once) {
            run = false;
        }
        else {
            ts.tv_nsec = arguments.sleep_time * 1000;
            nanosleep(&ts, NULL);
        }
    }

    if(arguments.output == FILEOUT) {
        fclose(out);
    }

    if(dof != NULL) {
        DEBUG_PRINT("Freeing cjson dof.\n");
        cJSON_Delete(dof);
    }

    if(string != NULL) {
        DEBUG_PRINT("Freeing cjson buffer.\n");
        cJSON_free(string);
    }

    if(arguments.filename != NULL) {
        DEBUG_PRINT("Freeing filename buffer.\n");
        free(arguments.filename);
    }

	status = lsm9ds1_close();
	if(status < 0) {
		fprintf(stderr, "Error closing lsm9ds1!\n");
	}

	return status;
}
