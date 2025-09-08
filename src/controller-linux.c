#include <errno.h>
#include <fcntl.h>
#include <libevdev/libevdev.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <systemd/sd-device.h>
#include <unistd.h>

#include "config.h"
#include "controller.h"
#include "event_queue.h"
// #define LOG_LEVEL_ERROR
#include "log.h"
#include "threads.h"
#include "utils.h"

#define CONTROLLER_AXIS_MAX 32767
#define CONTROLLER_AXIS_MIN -32768
#define CONTROLLER_RUMBLE_DURATION 500  // ms

#define DEVICE_PATH_SIZE 27

// Mapping buttons to their code
#define CONTROLLER_BUTTONS                      \
    BUTTON(CONTROLLER_BUTTON_A, BTN_EAST)       \
    BUTTON(CONTROLLER_BUTTON_B, BTN_SOUTH)      \
    BUTTON(CONTROLLER_BUTTON_Y, BTN_NORTH)      \
    BUTTON(CONTROLLER_BUTTON_X, BTN_WEST)       \
    BUTTON(CONTROLLER_BUTTON_L, BTN_TL)         \
    BUTTON(CONTROLLER_BUTTON_R, BTN_TR)         \
    BUTTON(CONTROLLER_BUTTON_MINUS, BTN_SELECT) \
    BUTTON(CONTROLLER_BUTTON_PLUS, BTN_START)   \
    BUTTON(CONTROLLER_BUTTON_HOME, BTN_MODE)    \
    BUTTON(CONTROLLER_BUTTON_LPAD, BTN_THUMBL)  \
    BUTTON(CONTROLLER_BUTTON_RPAD, BTN_THUMBR)

struct _Controller {
    struct libevdev *dev;
    pthread_t update_thread;
    pthread_t rumble_thread;
    pthread_mutex_t rumble_mutex;
    int fd;
    int16_t rumble_effect_id;
    int8_t player_index;
    bool is_rumbling;
    bool button_states[CONTROLLER_BUTTONS_COUNT];
};

static pthread_t monitor_thread;

/**
 * Check if a device has the required events to be a controller.
 *
 * \param dev The device to check.
 *
 * \returns true if the device matchs the requirements to be a controller.
 */
static bool is_controller(struct libevdev *const dev) NONNULL();

static bool is_controller(struct libevdev *const dev) {
    assert(dev != NULL);
    return (libevdev_has_event_type(dev, EV_ABS) &&
            libevdev_has_event_code(dev, EV_ABS, ABS_X) &&
            libevdev_has_event_code(dev, EV_ABS, ABS_Y) &&
            libevdev_has_event_code(dev, EV_ABS, ABS_Z) &&
            libevdev_has_event_code(dev, EV_ABS, ABS_RX) &&
            libevdev_has_event_code(dev, EV_ABS, ABS_RY) &&
            libevdev_has_event_code(dev, EV_ABS, ABS_RZ) &&
            libevdev_has_event_code(dev, EV_ABS, ABS_HAT0X) &&
            libevdev_has_event_code(dev, EV_ABS, ABS_HAT0Y) &&
            libevdev_has_event_type(dev, EV_FF) &&
            libevdev_has_event_code(dev, EV_FF, FF_RUMBLE) &&
            libevdev_has_event_type(dev, EV_KEY)
#define BUTTON(controller_button, button_code) \
    &&libevdev_has_event_code(dev, EV_KEY, button_code)
                CONTROLLER_BUTTONS
#undef BUTTON
    );
}

#ifndef PROD
#define controller_dump_info(self) _controller_dump_info(self)
/**
 * Print the informations about a controller device.
 *
 * \param self A pointer to the controller object.
 */
static inline void controller_dump_info(const Controller *const self) NONNULL();

static inline void controller_dump_info(const Controller *const self) {
    assert(self != NULL);
    log_debugf("controller name: '%s'", libevdev_get_name(self->dev));
    log_debugf("controller physical location: '%s'",
               libevdev_get_phys(self->dev));
    log_debugf("controller unique identifier: '%s'",
               libevdev_get_uniq(self->dev));
    log_debugf("controller product: %d", libevdev_get_id_product(self->dev));
    log_debugf("controller vendor: %d", libevdev_get_id_vendor(self->dev));
    log_debugf("controller bustype: %d", libevdev_get_id_bustype(self->dev));
    log_debugf("controller firmware version: %d",
               libevdev_get_id_version(self->dev));
    log_debugf("controller driver version: %d",
               libevdev_get_driver_version(self->dev));
}
#else
#define controller_dump_info(controller)
#endif

#ifndef PROD
static void controller_log_event(const Controller *const restrict self,
                                 const struct input_event *const restrict event)
    NONNULL();

static void controller_log_event(
    const Controller *const restrict self,
    const struct input_event *const restrict event) {
    assert(self != NULL);
    assert(event != NULL);

    const char *const type_name = libevdev_event_type_get_name(event->type);
    const char *const code_name =
        libevdev_event_code_get_name(event->type, event->code);

    if (type_name) {
        if (code_name) {
            log_debugf("controller '%s' event: type=%s code=%s value=%d",
                       libevdev_get_name(self->dev), type_name, code_name,
                       event->value);
            return;
        }
        log_debugf("controller '%s' event: type=%s code=%d value=%d",
                   libevdev_get_name(self->dev), type_name, event->code,
                   event->value);
        return;
    }

    if (code_name) {
        log_debugf("controller '%s' event: type=%d code=%s value=%d",
                   libevdev_get_name(self->dev), event->type, code_name,
                   event->value);
        return;
    }

    log_debugf("controller '%s' event: type=%d code=%d value=%d",
               libevdev_get_name(self->dev), event->type, event->type,
               event->value);
}
#else
#define controller_log_event(_controller, _event)
#endif

static void controller_push_button_event(const Controller *const self,
                                         const EventType event_type,
                                         const ControllerButton button)
    NONNULL(1);

static void controller_push_button_event(const Controller *const self,
                                         const EventType event_type,
                                         const ControllerButton button) {
    assert(self != NULL);
    assert(event_type == EVENT_TYPE_BUTTON_DOWN ||
           event_type == EVENT_TYPE_BUTTON_UP);
    assert(button < CONTROLLER_BUTTONS_COUNT);

    if (self->player_index == -1) return;

    event_queue_push(&(Event){
        .type = event_type,
        .button_event =
            {
                .player_index = self->player_index,
                .button = button,
            },
    });
}

static void controller_handle_button_down(Controller *const self,
                                          const ControllerButton button)
    NONNULL(1);

static void controller_handle_button_down(Controller *const self,
                                          const ControllerButton button) {
    assert(self != NULL);
    assert(button < CONTROLLER_BUTTONS_COUNT);
    self->button_states[button] = true;
    controller_push_button_event(self, EVENT_TYPE_BUTTON_DOWN, button);
}

static void controller_handle_button_up(Controller *const self,
                                        const ControllerButton button)
    NONNULL(1);

static void controller_handle_button_up(Controller *const self,
                                        const ControllerButton button) {
    assert(self != NULL);
    assert(button < CONTROLLER_BUTTONS_COUNT);
    self->button_states[button] = false;
    controller_push_button_event(self, EVENT_TYPE_BUTTON_UP, button);
}

/**
 * This function processes input events from a hat switch (D-pad), updates
 * the state of the associated buttons and triggers the appropriate button
 * callbacks.
 *
 * \param self A pointer to the Controller object that is receiving the event.
 * \param event A pointer to the input event structure containing the hat switch
 *              event.
 * \param button_positive The button associated with the positive direction of
 *                        the hat switch.
 * \param button_negative The button associated with the negative direction of
 *                        the hat switch.
 */
static void controller_handle_hat_event(
    Controller *const restrict self,
    const struct input_event *const restrict event,
    const ControllerButton button_positive,
    const ControllerButton button_negative) NONNULL(1, 2);

static void controller_handle_hat_event(
    Controller *const restrict self,
    const struct input_event *const restrict event,
    const ControllerButton button_positive,
    const ControllerButton button_negative) {
    assert(self != NULL);
    assert(event != NULL);
    assert(button_positive < CONTROLLER_BUTTONS_COUNT);
    assert(button_negative < CONTROLLER_BUTTONS_COUNT);

    if (event->value == 1) {
        if (self->button_states[button_negative]) {
            controller_handle_button_up(self, button_negative);
        }
        if (!self->button_states[button_positive]) {
            controller_handle_button_down(self, button_positive);
        }
    } else if (event->value == 0) {
        if (self->button_states[button_negative]) {
            controller_handle_button_up(self, button_negative);
        }
        if (self->button_states[button_positive]) {
            controller_handle_button_up(self, button_positive);
        }
    } else if (event->value == -1) {
        if (!self->button_states[button_negative]) {
            controller_handle_button_down(self, button_negative);
        }
        if (self->button_states[button_positive]) {
            controller_handle_button_up(self, button_positive);
        }
    }
}

/**
 * Handles input events for the controller and triggers the appropriate button
 * callbacks.
 *
 * \param self A pointer to the Controller object that is receiving the event.
 * \param event A pointer to the input event structure containing the event
 *              data.
 */
static void controller_handle_event(Controller *const restrict self,
                                    const struct input_event *restrict event)
    NONNULL(1, 2);

static void controller_handle_event(
    Controller *const restrict self,
    const struct input_event *const restrict event) {
    assert(self != NULL);
    assert(event != NULL);

    controller_log_event(self, event);

    if (event->type == EV_KEY) {
#define BUTTON(controller_button, button_code)                      \
    if (event->code == button_code) {                               \
        if (event->value) {                                         \
            controller_handle_button_down(self, controller_button); \
        } else {                                                    \
            controller_handle_button_up(self, controller_button);   \
        }                                                           \
        return;                                                     \
    }

        CONTROLLER_BUTTONS

#undef BUTTON
    } else if (event->type == EV_ABS) {
        if (event->code == ABS_Z) {
            if (!event->value) {
                controller_handle_button_up(self, CONTROLLER_BUTTON_ZL);
            } else {
                controller_handle_button_down(self, CONTROLLER_BUTTON_ZL);
            }
            return;
        }
        if (event->code == ABS_RZ) {
            if (!event->value) {
                controller_handle_button_up(self, CONTROLLER_BUTTON_ZR);
            } else {
                controller_handle_button_down(self, CONTROLLER_BUTTON_ZR);
            }
            return;
        }

        if (event->code == ABS_HAT0Y) {
            controller_handle_hat_event(self, event, CONTROLLER_BUTTON_DOWN,
                                        CONTROLLER_BUTTON_UP);
            return;
        }
        if (event->code == ABS_HAT0X) {
            controller_handle_hat_event(self, event, CONTROLLER_BUTTON_RIGHT,
                                        CONTROLLER_BUTTON_LEFT);
            return;
        }
    }
}

static void *controller_update_thread(void *const data) NONNULL();

static void *controller_update_thread(void *const data) {
    assert(data != NULL);
    Controller *const self = (Controller *)data;

    uint32_t read_flag = LIBEVDEV_READ_FLAG_BLOCKING;
    while (true) {
        struct input_event event;
        const int return_code =
            libevdev_next_event(self->dev, read_flag, &event);
        if (return_code < 0) {
            log_errorf_errno("failed to get next event");
            event_queue_push(&(Event){
                .type = EVENT_TYPE_CONTROLLER_DISCONNECT,
                .controller_event =
                    {
                        .controller = self,
                    },
            });
            return NULL;
        } else if (return_code == LIBEVDEV_READ_STATUS_SYNC) {
            controller_log_event(self, &event);
            read_flag = LIBEVDEV_READ_FLAG_SYNC;
            controller_handle_event(self, &event);
        } else if (return_code == LIBEVDEV_READ_STATUS_SUCCESS) {
            read_flag = LIBEVDEV_READ_FLAG_BLOCKING;
            controller_handle_event(self, &event);
        }
    }

    return NULL;
}

static inline bool controller_upload_rumble_effect(Controller *const self)
    NONNULL();

static inline bool controller_upload_rumble_effect(Controller *const self) {
    assert(self != NULL);

    struct ff_effect effect;
    memset(&effect, 0, sizeof(effect));
    effect.type = FF_RUMBLE;
    effect.id = -1;
    effect.replay.length = CONTROLLER_RUMBLE_DURATION;
    effect.u.rumble.strong_magnitude = 0xffff;
    effect.u.rumble.weak_magnitude = 0xffff;
    if (ioctl(self->fd, EVIOCSFF, &effect) < 0) {
        log_errorf_errno("failed to upload rumble effect to controller '%s'",
                         libevdev_get_name(self->dev));
        return false;
    }

    self->rumble_effect_id = effect.id;

    log_debugf("uploaded rumble effect to controller '%s'",
               libevdev_get_name(self->dev));
    return true;
}

/**
 * Initialize a controller from it's fd and libevdev object. The controller need
 * to closed with controller_destroy().
 *
 * Before calling this function, ensure that the provided libevdev object
 * represents a valid controller by using the is_controller() function.
 *
 * \param fd The file descriptor of the controller device.
 * \param dev A pointer to the libevdev object associated with the controller.
 *
 * \returns a pointer to the controller or NULL on failure.
 */
static Controller *controller_from_fd_and_dev(const int fd,
                                              struct libevdev *const dev)
    NONNULL(2);

static Controller *controller_from_fd_and_dev(const int fd,
                                              struct libevdev *const dev) {
    assert(dev != NULL);

    Controller *const self =
        malloc_or_exit(sizeof(*self), "failed to create controller");

    self->fd = fd;
    self->dev = dev;

    self->player_index = -1;

    pthread_mutex_init(&self->rumble_mutex, NULL);
    self->is_rumbling = false;
    if (!controller_upload_rumble_effect(self)) {
        controller_destroy(self);
        return NULL;
    }

    controller_dump_info(self);

    memset(self->button_states, false, sizeof(self->button_states));

    const int return_code = pthread_create(&self->update_thread, NULL,
                                           controller_update_thread, self);
    if (return_code != 0) {
        log_errorf("failed to create update thread for controller '%s': %s",
                   libevdev_get_name(self->dev), strerror(return_code));
        controller_destroy(self);
        return NULL;
    }

    if (!controller_rumble(self)) {
        controller_destroy(self);
        return NULL;
    }

    return self;
}

static inline sd_device_enumerator *get_device_enumerator(void) RETURNS_NONNULL;

static inline sd_device_enumerator *get_device_enumerator(void) {
    int return_code;
    sd_device_enumerator *enumerator;
    return_code = sd_device_enumerator_new(&enumerator);
    if (return_code < 0) {
        log_errorf("failed to create device enumerator: %s",
                   strerror(-return_code));
        exit(EXIT_FAILURE);
    }
    assert(enumerator != NULL);

    return_code =
        sd_device_enumerator_add_match_subsystem(enumerator, "input", 1);
    if (return_code < 0) {
        log_errorf("failed to set subsystem match for device enumerator: %s",
                   strerror(-return_code));
        exit(EXIT_FAILURE);
    }

    return_code = sd_device_enumerator_add_match_sysname(enumerator, "event*");
    if (return_code < 0) {
        log_errorf("failed to set sysname match for device enumerator: %s",
                   strerror(-return_code));
        exit(EXIT_FAILURE);
    }

    return_code = sd_device_enumerator_add_match_property(
        enumerator, "ID_INPUT_JOYSTICK", "1");
    if (return_code < 0) {
        log_errorf("failed to set property match for device enumerator: %s",
                   strerror(-return_code));
        exit(EXIT_FAILURE);
    }

    return enumerator;
}

#ifndef PROD
static void log_sd_device(sd_device *const device) NONNULL();

static void log_sd_device(sd_device *const device) {
    log_debugf("device info:");
    const char *string;
    if (sd_device_get_syspath(device, &string) == 0)
        log_debugf("device syspath: '%s'", string);
    if (sd_device_get_subsystem(device, &string) == 0)
        log_debugf("device subsystem: '%s'", string);
    if (sd_device_get_driver_subsystem(device, &string) == 0)
        log_debugf("device driver subsystem: '%s'", string);
    if (sd_device_get_devtype(device, &string) == 0)
        log_debugf("device devtype: '%s'", string);
    dev_t devnum;
    if (sd_device_get_devnum(device, &devnum) == 0)
        log_debugf("device devnum: %lu", devnum);
    int ifindex;
    if (sd_device_get_ifindex(device, &ifindex) == 0)
        log_debugf("device ifindex: %d", ifindex);
    if (sd_device_get_driver(device, &string) == 0)
        log_debugf("device driver: '%s'", string);
    if (sd_device_get_devpath(device, &string) == 0)
        log_debugf("device devpath: '%s'", string);
    if (sd_device_get_devname(device, &string) == 0)
        log_debugf("device devname: '%s'", string);
    if (sd_device_get_sysname(device, &string) == 0)
        log_debugf("device sysname: '%s'", string);
    if (sd_device_get_sysnum(device, &string) == 0)
        log_debugf("device sysnum: '%s'", string);
    uint64_t value;
    if (sd_device_get_seqnum(device, &value) == 0)
        log_debugf("device seqnum: %lu", value);
    if (sd_device_get_diskseq(device, &value) == 0)
        log_debugf("device diskseq: %lu", value);
    if (sd_device_get_device_id(device, &string) == 0)
        log_debugf("device id: '%s'", string);

    const char *property_value;
    for (const char *property =
             sd_device_get_property_first(device, &property_value);
         property != NULL;
         property = sd_device_get_property_next(device, &property_value)) {
        log_debugf("device property: %s='%s'", property, property_value);
    }
}
#else
#define log_sd_device(device)
#endif

static Controller *controller_from_devname(const char *const devname) NONNULL();

static Controller *controller_from_devname(const char *const devname) {
    assert(devname != NULL);

    const int fd = open(devname, O_RDWR);
    if (fd < 0) {
        log_errorf_errno("failed to open %s", devname);
        return NULL;
    }

    struct libevdev *dev;
    const int return_code = libevdev_new_from_fd(fd, &dev);
    if (return_code != 0) {
        log_errorf_errno("failed to initialize libevdev for device %s: %s",
                         devname, strerror(-return_code));
        close(fd);
        return NULL;
    }

    if (is_controller(dev)) {
        log_debugf("connect to controller %s", devname);
        return controller_from_fd_and_dev(fd, dev);
    }

    libevdev_free(dev);
    close(fd);
    return NULL;
}

ControllerArray *controller_get_connected_controllers(void) {
    sd_device_enumerator *const enumerator = get_device_enumerator();

    ControllerArray *const array = controller_array_create(1);

    for (sd_device *device = sd_device_enumerator_get_device_first(enumerator);
         device != NULL;
         device = sd_device_enumerator_get_device_next(enumerator)) {
        log_sd_device(device);
        const char *devname;
        int return_code;
        return_code = sd_device_get_devname(device, &devname);
        if (return_code < 0) {
            log_errorf("failed to get device devname: %s",
                       strerror(-return_code));
            continue;
        }

        Controller *const controller = controller_from_devname(devname);
        if (controller == NULL) continue;
        controller_array_push(array, controller);
    }

    sd_device_enumerator_unref(enumerator);

    return array;
}

static int handle_new_device(sd_device_monitor *const restrict _monitor,
                             sd_device *const restrict device,
                             void *const restrict _userdata) NONNULL(2);

static int handle_new_device(sd_device_monitor *const restrict _monitor,
                             sd_device *const restrict device,
                             void *const restrict _data) {
    (void)_monitor;
    (void)_data;
    assert(device != NULL);

    int return_code;
    sd_device_action_t action;
    return_code = sd_device_get_action(device, &action);
    if (return_code < 0) {
        log_errorf("failed to get device action: %s", strerror(-return_code));
        exit(EXIT_FAILURE);
    }

    if (action != SD_DEVICE_ADD) return 0;

    log_debugf("new device detected");
    log_sd_device(device);

    const char *devname = NULL;
    return_code = sd_device_get_devname(device, &devname);
    if (return_code < 0) {
        log_debugf("failed to get device devname: %s", strerror(-return_code));
        return 0;
    }
    assert(devname != NULL);

    if (strncmp(devname, "/dev/input/event", sizeof("/dev/input/event") - 1) !=
        0) {
        return 0;
    }

    const char *value;
    if (sd_device_get_property_value(device, "ID_INPUT_JOYSTICK", &value) < 0) {
        return 0;
    }
    if (strcmp(value, "1") != 0) return 0;

    log_debugf("try connecting to %s", devname);
    Controller *controller = controller_from_devname(devname);
    if (controller == NULL) return 0;

    event_queue_push(&(Event){
        .type = EVENT_TYPE_CONTROLLER_CONNECT,
        .controller_event =
            {
                .controller = controller,
            },
    });

    return 0;
}

static inline void sd_device_monitor_unref_void(
    sd_device_monitor *const monitor) NONNULL();

static inline void sd_device_monitor_unref_void(
    sd_device_monitor *const monitor) {
    assert(monitor != NULL);
    sd_device_monitor_unref(monitor);
}

static void *controller_monitor_thread(void *const _data) NONNULL();

static void *controller_monitor_thread(void *const _data) {
    (void)_data;
    int return_code;
    sd_device_monitor *monitor;
    return_code = sd_device_monitor_new(&monitor);
    if (return_code < 0) {
        log_errorf("failed to create monitor: %s", strerror(-return_code));
        exit(EXIT_FAILURE);
    }
    assert(monitor != NULL);

    pthread_cleanup_push((void (*)(void *))sd_device_monitor_unref_void,
                         monitor);

    return_code = sd_device_monitor_start(monitor, handle_new_device, NULL);
    if (return_code < 0) {
        log_errorf("failed to start monitor: %s", strerror(-return_code));
        exit(EXIT_FAILURE);
    }

    sd_event *const event = sd_device_monitor_get_event(monitor);
    if (event == NULL) {
        log_errorf("failed to get monitor event");
        exit(EXIT_FAILURE);
    }

    return_code = sd_event_loop(event);
    if (return_code < 0) {
        log_errorf("failed to run monitor event loop: %s",
                   strerror(-return_code));
        exit(EXIT_FAILURE);
    }

    log_debugf("quit event loop");
    pthread_cleanup_pop(1);

    return NULL;
}

void controller_start_monitor(void) {
    const int return_code =
        pthread_create(&monitor_thread, NULL, controller_monitor_thread, NULL);
    if (return_code < 0) {
        log_errorf("failed to start controller monitor thread: %s",
                   strerror(-return_code));
        exit(EXIT_FAILURE);
    }
}

void controller_stop_monitor(void) {
    int return_code;
    return_code = pthread_cancel(monitor_thread);
    if (return_code < 0) {
        log_errorf("failed to cancel controller monitor thread: %s",
                   strerror(-return_code));
        exit(EXIT_FAILURE);
    }

    return_code = pthread_join(monitor_thread, NULL);
    if (return_code < 0) {
        log_errorf("failed to join controller monitor thread: %s",
                   strerror(-return_code));
        exit(EXIT_FAILURE);
    }
}

static void controller_stop_rumble_thread(Controller *const self) NONNULL();

static void controller_stop_rumble_thread(Controller *const self) {
    assert(self != NULL);
    assert(self->is_rumbling);

    const int return_code = pthread_cancel(self->rumble_thread);
    if (return_code != 0) {
        log_errorf("failed to cancel rumble thread for controller '%s': %s",
                   libevdev_get_name(self->dev), strerror(errno));
    }
}

#ifndef PROD
const char *controller_get_name(const Controller *const self) {
    assert(self != NULL);
    return libevdev_get_name(self->dev);
}
#endif

int8_t controller_get_player_index(const Controller *const self) {
    assert(self != NULL);
    return self->player_index;
}

void controller_set_player_index(Controller *const self,
                                 const int8_t player_index) {
    assert(self != NULL);
    self->player_index = player_index;
}

void controller_destroy(Controller *const self) {
    assert(self != NULL);
    if (self->is_rumbling) {
        controller_stop_rumble_thread(self);
        const int read_return_code = pthread_join(self->rumble_thread, NULL);
        if (read_return_code != 0) {
            log_errorf("failed to join rumble thread for controller '%s': %s",
                       libevdev_get_name(self->dev),
                       strerror(read_return_code));
        }
    }
    mutex_destroy(&self->rumble_mutex);

    int return_code;
    return_code = pthread_cancel(self->update_thread);
    if (return_code != 0) {
        log_errorf("failed to cancel update thread for controller '%s': %s",
                   libevdev_get_name(self->dev), strerror(return_code));
    }
    return_code = pthread_join(self->update_thread, NULL);
    if (return_code != 0) {
        log_errorf("failed to join update thread for controller '%s': %s",
                   libevdev_get_name(self->dev), strerror(return_code));
    }
    log_debugf("disconnect controller '%s'", libevdev_get_name(self->dev));
    libevdev_free(self->dev);
    close(self->fd);
    free(self);
}

v2f controller_get_stick(const Controller *const self,
                         const ControllerStick stick) {
    assert(self != NULL);

    int x_axis, y_axis;
    switch (stick) {
        case CONTROLLER_STICK_LEFT:
            x_axis = ABS_X;
            y_axis = ABS_Y;
            break;

        case CONTROLLER_STICK_RIGHT:
            x_axis = ABS_RX;
            y_axis = ABS_RY;
            break;
    }

    const int x_value = libevdev_get_event_value(self->dev, EV_ABS, x_axis);
    const int y_value = libevdev_get_event_value(self->dev, EV_ABS, y_axis);

    v2f stick_state = {
        .x = 2.0f * (float)(x_value + CONTROLLER_AXIS_MAX) /
                 (CONTROLLER_AXIS_MAX - CONTROLLER_AXIS_MIN) -
             1.0f,
        .y = 2.0f * (float)(y_value + CONTROLLER_AXIS_MAX) /
                 (CONTROLLER_AXIS_MAX - CONTROLLER_AXIS_MIN) -
             1.0f,
    };

    if (fabsf(stick_state.x) < CONTROLLER_AXIS_ROUND) {
        stick_state.x = 0.0f;
    } else if (stick_state.x > 1.0f - CONTROLLER_AXIS_ROUND) {
        stick_state.x = 1.0f;
    } else if (stick_state.x < -1.0f + CONTROLLER_AXIS_ROUND) {
        stick_state.x = -1.0f;
    }
    if (fabsf(stick_state.y) < CONTROLLER_AXIS_ROUND) {
        stick_state.y = 0.0f;
    } else if (stick_state.y > 1.0f - CONTROLLER_AXIS_ROUND) {
        stick_state.y = 1.0f;
    } else if (stick_state.y < -1.0f + CONTROLLER_AXIS_ROUND) {
        stick_state.y = -1.0f;
    }

    const float stick_state_norm = v2f_norm(stick_state);
    if (stick_state_norm > 1.0f) {
        stick_state = v2f_div(stick_state, stick_state_norm);
    }

    return stick_state;
}

inline bool controller_get_button(const Controller *const self,
                                  const ControllerButton button) {
    assert(self != NULL);
    assert(button < CONTROLLER_BUTTONS_COUNT);
    return self->button_states[button];
}

static void controller_stop_rumble(Controller *const self) NONNULL();

static void controller_stop_rumble(Controller *const self) {
    assert(self != NULL);

    mutex_lock(&self->rumble_mutex);
    assert(self->is_rumbling);

    struct input_event stop;
    stop.type = EV_FF;
    stop.code = self->rumble_effect_id;
    stop.value = 0;

    if (write(self->fd, &stop, sizeof(stop)) < 0) {
        mutex_unlock(&self->rumble_mutex);
        log_errorf_errno("failed to send rumble stop event to controller '%s'",
                         libevdev_get_name(self->dev));
    }

    self->is_rumbling = false;
    mutex_unlock(&self->rumble_mutex);
    log_debugf("controller '%s' stop rumble", libevdev_get_name(self->dev));
}

static void *controller_rumble_thread(void *const data) NONNULL();

static void *controller_rumble_thread(void *const data) {
    assert(data != NULL);
    Controller *const controller = data;

    pthread_cleanup_push((void (*)(void *))controller_stop_rumble, controller);

    usleep(CONTROLLER_RUMBLE_DURATION * 1000);

    pthread_cleanup_pop(1);

    return NULL;
}

bool controller_rumble(Controller *const self) {
    assert(self != NULL);

    mutex_lock(&self->rumble_mutex);
    if (self->is_rumbling) controller_stop_rumble_thread(self);

    struct input_event play;
    play.type = EV_FF;
    play.code = self->rumble_effect_id;
    play.value = 1;

    if (write(self->fd, &play, sizeof(play)) < 0) {
        mutex_unlock(&self->rumble_mutex);
        log_errorf_errno("failed to send rumble play event");
        return false;
    }

    const int return_code = pthread_create(&self->rumble_thread, NULL,
                                           controller_rumble_thread, self);
    if (return_code != 0) {
        mutex_unlock(&self->rumble_mutex);
        log_errorf(
            "failed to rumble the controller '%s': failed to create thread: %s",
            libevdev_get_name(self->dev), strerror(return_code));
        return false;
    }

    self->is_rumbling = true;

    mutex_unlock(&self->rumble_mutex);

    log_debugf("controller '%s' start rumble", libevdev_get_name(self->dev));

    return true;
}
