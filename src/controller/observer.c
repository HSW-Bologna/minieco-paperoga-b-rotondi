#include "model/model.h"
#include "watcher.h"
#include "observer.h"
#include "bsp/relay.h"
#include "bsp/pwm.h"
#include "services/timestamp.h"


static void update_relays(void *old_value, const void *new_value, watcher_size_t size, void *user_ptr, void *arg);
static void update_pwm(void *old_value, const void *new_value, watcher_size_t size, void *user_ptr, void *arg);
static void update_alarms(void *old_value, const void *new_value, watcher_size_t size, void *user_ptr, void *arg);
static void refresh_relays(uint16_t map);


static watcher_t watcher = {0};
static struct {
    uint8_t  pwm1_percentage;
    uint8_t  pwm2_percentage;
    uint16_t relays;
    uint16_t alarms;
} state = {0};


void observer_init(mut_model_t *model) {
    WATCHER_INIT_STD(&watcher, model);

    WATCHER_ADD_ENTRY(&watcher, &state.pwm1_percentage, update_pwm, NULL);
    WATCHER_ADD_ENTRY(&watcher, &state.pwm2_percentage, update_pwm, NULL);
    WATCHER_ADD_ENTRY(&watcher, &state.relays, update_relays, NULL);
    WATCHER_ADD_ENTRY(&watcher, &state.alarms, update_alarms, NULL);
}


void observer_manage(model_t *model) {
    state.pwm1_percentage = model_get_pwm1_percentage(model);
    state.pwm2_percentage = model_get_pwm2_percentage(model);
    state.relays          = model_get_relay_map(model);
    state.alarms          = model_get_alarms(model);

    watcher_watch(&watcher, timestamp_get());
}


static void update_alarms(void *old_value, const void *new_value, watcher_size_t size, void *user_ptr, void *arg) {
    (void)old_value;
    (void)new_value;
    (void)size;
    (void)arg;

    if (state.alarms > 0) {
        model_cycle_pause(user_ptr);
    }
}


static void update_relays(void *old_value, const void *new_value, watcher_size_t size, void *user_ptr, void *arg) {
    (void)old_value;
    (void)new_value;
    (void)size;
    (void)user_ptr;
    (void)arg;

    refresh_relays(state.relays);
}


static void update_pwm(void *old_value, const void *new_value, watcher_size_t size, void *user_ptr, void *arg) {
    (void)old_value;
    (void)new_value;
    (void)size;
    (void)user_ptr;
    (void)arg;

    bsp_pwm_update(BSP_PWM_1, state.pwm1_percentage);
    bsp_pwm_update(BSP_PWM_2, state.pwm2_percentage);
}


static void refresh_relays(uint16_t map) {
    for (bsp_relay_t relay = BSP_RELAY_1; relay < BSP_RELAY_NUM; relay++) {
        bsp_relay_update(relay, (map & (1 << relay)) > 0);
    }
}
