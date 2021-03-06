/* Copyright (c) 2018 Willem Dijkstra. All Rights Reserved.
 *
 * Consumer keys (media keys) using nrf51822 and Nordic SDK 10.0.0
 *
 * based on Nordic's ble_sdk_app_hids_keyboard_main sample file.
 *
 */

/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_adc.h"
#include "nrf_assert.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advertising.h"
#include "ble_advdata.h"
#include "ble_hids.h"
#include "ble_bas.h"
#include "ble_dis.h"
#include "ble_conn_params.h"
#include "bsp.h"
#include "app_scheduler.h"
#include "softdevice_handler_appsh.h"
#include "app_timer_appsh.h"
#include "device_manager.h"
#include "pstorage.h"
#include "app_trace.h"


#define IS_SRVC_CHANGED_CHARACT_PRESENT  0                                              /**< Include or not the service_changed characteristic. if not enabled, the server's database cannot be changed for the lifetime of the device*/

#define INPUT_CCONTROL_KEYS_INDEX	 0
#define INPUT_CC_REP_REF_ID	         1
#define INPUT_CC_REPORT_KEYS_MAX_LEN	 1

#define DEVICE_NAME                      "WearLink"                                     /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME                "Dijkstra.xyz"                                 /**< Manufacturer. Will be passed to Device Information Service. */

#define APP_TIMER_PRESCALER              0                                              /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE          4                                              /**< Size of timer operation queues. */

#define BATTERY_LEVEL_MEAS_INTERVAL      APP_TIMER_TICKS(120000, APP_TIMER_PRESCALER)   /**< Battery level measurement interval (ticks). */
#define MAX_IDLE_BATTERY_MEAS            10                                             /**< Maximum idle battery measurement intervals until autosleep */

#define ADC_MEAS_INTERVAL                APP_TIMER_TICKS(30, APP_TIMER_PRESCALER)

#define PNP_ID_VENDOR_ID_SOURCE          0x02                                           /**< Vendor ID Source. */
#define PNP_ID_VENDOR_ID                 0x1915                                         /**< Vendor ID. */
#define PNP_ID_PRODUCT_ID                0xEEEE                                         /**< Product ID. */
#define PNP_ID_PRODUCT_VERSION           0x0001                                         /**< Product Version. */

#define APP_ADV_FAST_INTERVAL            0x0028                                         /**< Fast advertising interval (in units of 0.625 ms. This value corresponds to 25 ms.). */
#define APP_ADV_SLOW_INTERVAL            0x0C80                                         /**< Slow advertising interval (in units of 0.625 ms. This value corrsponds to 2 seconds). */
#define APP_ADV_FAST_TIMEOUT             30                                             /**< The duration of the fast advertising period (in seconds). */
#define APP_ADV_SLOW_TIMEOUT             30                                             /**< The duration of the slow advertising period (in seconds). */

/*lint -emacro(524, MIN_CONN_INTERVAL) // Loss of precision */
#define MIN_CONN_INTERVAL                MSEC_TO_UNITS(7.5, UNIT_1_25_MS)               /**< Minimum connection interval (7.5 ms) */
#define MAX_CONN_INTERVAL                MSEC_TO_UNITS(30, UNIT_1_25_MS)                /**< Maximum connection interval (30 ms). */
#define SLAVE_LATENCY                    6                                              /**< Slave latency. */
#define CONN_SUP_TIMEOUT                 MSEC_TO_UNITS(430, UNIT_10_MS)                 /**< Connection supervisory timeout (430 ms). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)     /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY    APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER)    /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT     3                                              /**< Number of attempts before giving up the connection parameter negotiation. */

#define SEC_PARAM_BOND                   1                                              /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                              /**< Man In The Middle protection not required. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                           /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                              /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                              /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16                                             /**< Maximum encryption key size. */

#define APP_FEATURE_NOT_SUPPORTED       BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2            /**< Reply when unsupported features are requested. */

#define BASE_USB_HID_SPEC_VERSION        0x0101                                         /**< Version number of base USB HID Specification implemented by this application. */

#define DEAD_BEEF                        0xDEADBEEF                                     /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define SCHED_MAX_EVENT_DATA_SIZE        MAX(APP_TIMER_SCHED_EVT_SIZE,\
                                             BLE_STACK_HANDLER_SCHED_EVT_SIZE)          /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE                 10                                             /**< Maximum number of events in the scheduler queue. */

#define KEYPAD_ADC_PIN                    NRF_ADC_CONFIG_INPUT_2                        /**< Pin P0.01 */
#define KEYPAD_SENSE_PIN                  4
#define KEYPAD_NUMKEYS                    5

#define KEYPAD_COMBINATIONS               1
#define KEYPAD_COMBINATION_MAXLEN         10

typedef enum
{
    BLE_NO_ADV,               /**< No advertising running. */
    BLE_DIRECTED_ADV,         /**< Direct advertising to the latest central. */
    BLE_FAST_ADV_WHITELIST,   /**< Advertising with whitelist. */
    BLE_FAST_ADV,             /**< Fast advertising running. */
    BLE_SLOW_ADV,             /**< Slow advertising running. */
    BLE_SLEEP,                /**< Go to system-off. */
} ble_advertising_mode_t;

typedef enum
{
    ADC_MODE_KEYPAD,          /* ADC is configured for keypad */
    ADC_MODE_BATTERY,         /* ADC is configured to measure battery */
} adc_mode_t;

static ble_hids_t                        m_hids;                                        /**< Structure used to identify the HID service. */
static ble_bas_t                         m_bas;                                         /**< Structure used to identify the battery service. */

static uint16_t                          m_conn_handle = BLE_CONN_HANDLE_INVALID;       /**< Handle of the current connection. */

APP_TIMER_DEF(m_battery_timer_id);                                                      /**< Battery timer. */

APP_TIMER_DEF(m_adc_timer_id);

static dm_application_instance_t         m_app_handle;                                  /**< Application identifier allocated by device manager. */
static dm_handle_t                       m_bonded_peer_handle;                          /**< Device reference handle to the current bonded central. */

static ble_uuid_t m_adv_uuids[] = {{BLE_UUID_HUMAN_INTERFACE_DEVICE_SERVICE, BLE_UUID_TYPE_BLE}};

static adc_mode_t                        adc_mode;
volatile int32_t                         adc_sample;
static uint8_t                           battery_level;
static float                             battery_correction = 1.0;
static uint8_t                           idle_counter = 0;
static uint8_t                           start_key = 0;

/*
  Fibretronics wearlink is a string of push button (no) switches individually
  in series with a resistor. When no button is pressed it is open
  circuit. When one button is pressed, the resistance corresponding to that
  key is present between the two feed wires.

  To determine the value for the adc we calculate ideal adc reading based on
  the known/measured resistance per button.

  Additional wrinkles;
  - ADC reference is VBG = 1.2V
  - ADC prescaling = 1/3
  - ADC input impendance is ~390k
  - ADC sampling time is 20us @ 8 bits, 68us @ 10 bit

  Schematic:

   Vcc--Rk---x----Rl--gnd
             |
            Ra
             |
           Vref/2

   Where:
   Rk = key resistance
   Rl = limit or load resistance, makes a Rk + Rl load when not sampling
   Ra = the adc impedance at sampling time
   Ua = voltage at point x

   Ik = Il + Ia = Vcc / Rk
   Ia = (Ua - (Vref / 2)) / Rain
   Il = Ua / Rl

   Solve for Ua: Ua = (-Vcc - ((Vref / 2) / Ra)) / (-1 - (Rk / Ra) - (Rk / Rl))

  #+ORGTBL: SEND keymarks orgtbl-to-generic :lstart "   { " :lend " }," :sep ", ", :skip 4 :skipcols (3 4 5 6 7)
  | ! | Key                           |       Rk |         Uau |      Ua |   Vscaled |      Adc |   A-D% |    A+D% |
  |   |                               |          |   unsampled | sampled |           |          |        |         |
  |---+-------------------------------+----------+-------------+---------+-----------+----------+--------+---------|
  | # | RELEASE_KEY                   | 10000000 |       0.007 |   0.036 |     0.012 |        3 |      3 |       3 |
  | # | CONSUMER_CTRL_VOL_DW          |    10000 |       2.400 |   2.370 |     0.790 |      169 |    161 |     177 |
  | # | CONSUMER_CTRL_SCAN_PREV_TRACK |    20000 |       1.800 |   1.770 |     0.590 |      126 |    120 |     132 |
  | # | CONSUMER_CTRL_PLAY            |    47000 |       1.075 |   1.058 |     0.353 |       75 |     71 |      79 |
  | # | CONSUMER_CTRL_SCAN_NEXT_TRACK |    30000 |       1.440 |   1.415 |     0.472 |      101 |     96 |     106 |
  | # | CONSUMER_CTRL_VOL_UP          |     5600 |       2.812 |   2.788 |     0.929 |      198 |    188 |     208 |
  |---+-------------------------------+----------+-------------+---------+-----------+----------+--------+---------|
  | $ | Vcc=3.6                       | Rl=20000 | Rain=389200 |         | scale=1/3 | Vref=1.2 | bits=8 | dev=.05 |
  #+TBLFM: $4=($Rl/($Rk+$Rl))*$Vcc;%.3f::$5=(-(($Vref/2)*$3)/$Rain-$Vcc)/((-$3/$Rain)-($3/$Rl)-1);%.3f::$6=$Ua*$scale;%.3f::$7=$Vscaled/($Vref/2^$bits);%.0f::$8=max($7-($dev*$7),0);%.0f::$9=min($7+($dev*$7),2^$bits);%0.f

  Note that the measurement is compared to the internal reference, and that the
  supply voltage will slowly drop due to battery discharge. We take this into
  account by adjusting the adc read voltage by a battery level correction.
*/

#define ADC_BATTERY_3V6  (1<<8)

typedef enum
{
    RELEASE_KEY                     = 0x00,
    CONSUMER_CTRL_PLAY              = 0x01,
    CONSUMER_CTRL_ALCCC             = 0x02,
    CONSUMER_CTRL_SCAN_NEXT_TRACK   = 0x04,
    CONSUMER_CTRL_SCAN_PREV_TRACK   = 0x08,
    CONSUMER_CTRL_VOL_DW            = 0x10,
    CONSUMER_CTRL_VOL_UP            = 0x20,
    CONSUMER_CTRL_AC_FORWARD        = 0x40,
    CONSUMER_CTRL_AC_BACK           = 0x80,
} consumer_control_t;

const struct keymark {
    uint8_t key;
    uint8_t low;
    uint8_t high;
} keymark[KEYPAD_NUMKEYS] = {
/* BEGIN RECEIVE ORGTBL keymarks */
   { CONSUMER_CTRL_VOL_DW,          161, 177 },
   { CONSUMER_CTRL_SCAN_PREV_TRACK, 120, 132 },
   { CONSUMER_CTRL_PLAY,             71,  79 },
   { CONSUMER_CTRL_SCAN_NEXT_TRACK,  96, 106 },
   { CONSUMER_CTRL_VOL_UP,          188, 208 },
/* END RECEIVE ORGTBL keymarks */
};

typedef enum
{
    NO_ACTION,
    ACTION_ERASE_BONDS
} keycombo_action_t;

struct keycombo {
    uint8_t len;
    uint8_t cur;
    keycombo_action_t action;
    uint8_t keys[KEYPAD_COMBINATION_MAXLEN];
} keycombo[KEYPAD_COMBINATIONS] = {
    {7, 0, ACTION_ERASE_BONDS,
     { CONSUMER_CTRL_PLAY,
       CONSUMER_CTRL_PLAY,
       CONSUMER_CTRL_PLAY,
       CONSUMER_CTRL_VOL_DW,
       CONSUMER_CTRL_VOL_DW,
       CONSUMER_CTRL_VOL_UP,
       CONSUMER_CTRL_VOL_UP }},
};

static uint8_t adc_process_battery_measurement(int32_t sample);
static uint8_t adc_process_keypad_measurement(int32_t sample);
static void adc_init(adc_mode_t mode_req);
static void adc_timeout_handler(void *p_context);
static void keypad_combo_handler(uint8_t key);
static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t * p_evt);

/*
 * Fibretronics wearlink is a string of push button (no) switches individually
 * in series with a resistor. When no button is pressed it is open
 * circuit. When one button is pressed, the resistance corresponding to that
 * key is present between the two feed wires.
 *
 * To determine the value for the adc we calculate ideal adc reading based on
 * the known/measured resistance per button.
 *
 */

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling Service errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void service_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling advertising errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void ble_advertising_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the battery level measurement timer expires.
 *
 * @param[in]   p_context   Pointer used for passing some arbitrary information (context) from the
 *                          app_start_timer() call to the timeout handler.
 */
static void battery_level_meas_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

    if (! nrf_adc_is_busy()) {
        adc_init(ADC_MODE_BATTERY);
        nrf_adc_start();
        app_timer_start(m_adc_timer_id, ADC_MEAS_INTERVAL, NULL);
    }
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    uint32_t err_code;

    // Initialize timer module, making it use the scheduler.
    APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true);

    // Create battery timer.
    err_code = app_timer_create(&m_battery_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                battery_level_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);

    // Create adc timer
    err_code = app_timer_create(&m_adc_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                adc_timeout_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_HID);
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing Device Information Service.
 */
static void dis_init(void)
{
    uint32_t         err_code;
    ble_dis_init_t   dis_init_obj;
    ble_dis_pnp_id_t pnp_id;

    pnp_id.vendor_id_source = PNP_ID_VENDOR_ID_SOURCE;
    pnp_id.vendor_id        = PNP_ID_VENDOR_ID;
    pnp_id.product_id       = PNP_ID_PRODUCT_ID;
    pnp_id.product_version  = PNP_ID_PRODUCT_VERSION;

    memset(&dis_init_obj, 0, sizeof(dis_init_obj));

    ble_srv_ascii_to_utf8(&dis_init_obj.manufact_name_str, MANUFACTURER_NAME);
    dis_init_obj.p_pnp_id = &pnp_id;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&dis_init_obj.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init_obj.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing Battery Service.
 */
static void bas_init(void)
{
    uint32_t       err_code;
    ble_bas_init_t bas_init_obj;

    memset(&bas_init_obj, 0, sizeof(bas_init_obj));

    bas_init_obj.evt_handler          = NULL;
    bas_init_obj.support_notification = true;
    bas_init_obj.p_report_ref         = NULL;
    bas_init_obj.initial_batt_level   = 100;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&bas_init_obj.battery_level_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&bas_init_obj.battery_level_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&bas_init_obj.battery_level_char_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&bas_init_obj.battery_level_report_read_perm);

    err_code = ble_bas_init(&m_bas, &bas_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing HID Service.
 */
static void hids_init(void)
{
    uint32_t                   err_code;
    ble_hids_init_t            hids_init_obj;
    ble_hids_inp_rep_init_t    input_report_array[1];
    ble_hids_inp_rep_init_t  * p_input_report;
    ble_hids_outp_rep_init_t   output_report_array[1];
    uint8_t                    hid_info_flags;

    memset((void *)input_report_array, 0, sizeof(ble_hids_inp_rep_init_t));
    memset((void *)output_report_array, 0, sizeof(ble_hids_outp_rep_init_t));

    static uint8_t report_map_data[] =
    {
        // Report ID 2: Advanced buttons
        0x05, 0x0C,                     // Usage Page (Consumer)
        0x09, 0x01,                     // Usage (Consumer Control)
        0xA1, 0x01,                     // Collection (Application)
        0x85, 0x01,                     //     Report Id (1)
        0x15, 0x00,                     //     Logical minimum (0)
        0x25, 0x01,                     //     Logical maximum (1)
        0x75, 0x01,                     //     Report Size (1)
        0x95, 0x01,                     //     Report Count (1)

        0x09, 0xCD,                     //     Usage (Play/Pause)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
        0x0A, 0x83, 0x01,               //     Usage (AL Consumer Control Configuration)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
        0x09, 0xB5,                     //     Usage (Scan Next Track)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
        0x09, 0xB6,                     //     Usage (Scan Previous Track)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)

        0x09, 0xEA,                     //     Usage (Volume Down)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
        0x09, 0xE9,                     //     Usage (Volume Up)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
        0x0A, 0x25, 0x02,               //     Usage (AC Forward)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
        0x0A, 0x24, 0x02,               //     Usage (AC Back)
        0x81, 0x02,                     //     Input (Data,Value,Relative,Bit Field)
        0xC0 // End Collection
    };

    // Initialize HID Consumer Control
    p_input_report                      = &input_report_array[INPUT_CCONTROL_KEYS_INDEX];
    p_input_report->max_len             = INPUT_CC_REPORT_KEYS_MAX_LEN;
    p_input_report->rep_ref.report_id   = INPUT_CC_REP_REF_ID;
    p_input_report->rep_ref.report_type = BLE_HIDS_REP_TYPE_INPUT;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&p_input_report->security_mode.write_perm);

    hid_info_flags = HID_INFO_FLAG_REMOTE_WAKE_MSK | HID_INFO_FLAG_NORMALLY_CONNECTABLE_MSK;

    memset(&hids_init_obj, 0, sizeof(hids_init_obj));

    hids_init_obj.evt_handler                    = on_hids_evt;
    hids_init_obj.error_handler                  = service_error_handler;
    hids_init_obj.is_kb                          = false;
    hids_init_obj.is_mouse                       = false;
    hids_init_obj.inp_rep_count                  = 1;
    hids_init_obj.p_inp_rep_array                = input_report_array;
    hids_init_obj.outp_rep_count                 = 0;
    hids_init_obj.p_outp_rep_array               = NULL;
    hids_init_obj.feature_rep_count              = 0;
    hids_init_obj.p_feature_rep_array            = NULL;
    hids_init_obj.rep_map.data_len               = sizeof(report_map_data);
    hids_init_obj.rep_map.p_data                 = report_map_data;
    hids_init_obj.hid_information.bcd_hid        = BASE_USB_HID_SPEC_VERSION;
    hids_init_obj.hid_information.b_country_code = 0;
    hids_init_obj.hid_information.flags          = hid_info_flags;
    hids_init_obj.included_services_count        = 0;
    hids_init_obj.p_included_services_array      = NULL;

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.rep_map.security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.rep_map.security_mode.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.hid_information.security_mode.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.hid_information.security_mode.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_protocol.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_protocol.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&hids_init_obj.security_mode_ctrl_point.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&hids_init_obj.security_mode_ctrl_point.write_perm);

    err_code = ble_hids_init(&m_hids, &hids_init_obj);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    dis_init();
    bas_init();
    hids_init();
}

/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = NULL;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for starting timers.
 */
static void timers_start(void)
{
    uint32_t err_code;

    err_code = app_timer_start(m_battery_timer_id, BATTERY_LEVEL_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

static uint32_t consumer_control_send(consumer_control_t cmd)
{
    return ble_hids_inp_rep_send(&m_hids, INPUT_CCONTROL_KEYS_INDEX, INPUT_CC_REPORT_KEYS_MAX_LEN, (uint8_t*)&cmd);
}

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code;
    bsp_indication_set(BSP_INDICATE_IDLE);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling HID events.
 *
 * @details This function will be called for all HID events which are passed to the application.
 *
 * @param[in]   p_hids  HID service structure.
 * @param[in]   p_evt   Event received from the HID service.
 */
static void on_hids_evt(ble_hids_t * p_hids, ble_hids_evt_t *p_evt)
{
    switch (p_evt->evt_type) {
    case BLE_HIDS_EVT_NOTIF_ENABLED:
    {
        dm_service_context_t   service_context;
        service_context.service_type = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;
        service_context.context_data.len = 0;
        service_context.context_data.p_data = NULL;

        if (p_evt->params.notification.char_id.rep_type == BLE_HIDS_REP_TYPE_INPUT)
        {
            // The protocol mode is Report Protocol mode. And the CCCD for the input report
            // is changed. It is now time to store all the CCCD information (system
            // attributes) into the flash.
            uint32_t err_code;

            err_code = dm_service_context_set(&m_bonded_peer_handle, &service_context);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            else
            {
                // The system attributes could not be written to the flash because
                // the connected central is not a new central. The system attributes
                // will only be written to flash only when disconnected from this central.
                // Do nothing now.
            }

            // If present, report the startup keypress
            if (start_key) {
                app_trace_log("sending startup registered key");
                consumer_control_send(start_key);
                start_key = 0;
                consumer_control_send(RELEASE_KEY);
            }
        }
        else
        {
            // The notification of the report that was enabled by the central is not interesting
            // to this application. So do nothing.
        }
        break;
    }

    default:
        // No implementation needed.
        break;
    }
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_DIRECTED:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_DIRECTED);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_SLOW:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_SLOW);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_FAST_WHITELIST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_SLOW_WHITELIST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;

        case BLE_ADV_EVT_WHITELIST_REQUEST:
        {
            ble_gap_whitelist_t whitelist;
            ble_gap_addr_t    * p_whitelist_addr[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            ble_gap_irk_t     * p_whitelist_irk[BLE_GAP_WHITELIST_IRK_MAX_COUNT];

            whitelist.addr_count = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
            whitelist.irk_count  = BLE_GAP_WHITELIST_IRK_MAX_COUNT;
            whitelist.pp_addrs   = p_whitelist_addr;
            whitelist.pp_irks    = p_whitelist_irk;

            err_code = dm_whitelist_create(&m_app_handle, &whitelist);
            APP_ERROR_CHECK(err_code);

            err_code = ble_advertising_whitelist_reply(&whitelist);
            APP_ERROR_CHECK(err_code);
            break;
        }
        case BLE_ADV_EVT_PEER_ADDR_REQUEST:
        {
            ble_gap_addr_t peer_address;

            // Only Give peer address if we have a handle to the bonded peer.
            if ((m_bonded_peer_handle.appl_id != DM_INVALID_ID) &&
                (m_bonded_peer_handle.service_id != DM_INVALID_ID))
            {
                err_code = dm_peer_addr_get(&m_bonded_peer_handle, &peer_address);
                APP_ERROR_CHECK(err_code);

                err_code = ble_advertising_peer_addr_reply(&peer_address);
                APP_ERROR_CHECK(err_code);
            }
            break;
        }
        default:
            break;
    }
}


/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                              err_code;
    ble_gatts_rw_authorize_reply_params_t auth_reply;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);

            m_conn_handle      = p_ble_evt->evt.gap_evt.conn_handle;
            break;

        case BLE_EVT_TX_COMPLETE:
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            err_code = bsp_indication_set(BSP_INDICATE_ALERT_OFF);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_EVT_USER_MEM_REQUEST:
            err_code = sd_ble_user_mem_reply(m_conn_handle, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            if(p_ble_evt->evt.gatts_evt.params.authorize_request.type
               != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
            {
                if ((p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                     == BLE_GATTS_OP_PREP_WRITE_REQ)
                    || (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                     == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW)
                    || (p_ble_evt->evt.gatts_evt.params.authorize_request.request.write.op
                     == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
                {
                    if (p_ble_evt->evt.gatts_evt.params.authorize_request.type
                        == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
                    {
                    auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
                    }
                    else
                    {
                        auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
                    }
                    auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
                    err_code = sd_ble_gatts_rw_authorize_reply(m_conn_handle,&auth_reply);
                    APP_ERROR_CHECK(err_code);
                }
            }
            break;

        case BLE_GATTC_EVT_TIMEOUT:
        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server and Client timeout events.
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief   Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the scheduler in the main loop after a BLE stack
 *          event has been received.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    dm_ble_evt_handler(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_hids_on_ble_evt(&m_hids, p_ble_evt);
    ble_bas_on_ble_evt(&m_bas, p_ble_evt);
}


/**@brief   Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the System event interrupt handler after a system
 *          event has been received.
 *
 * @param[in]   sys_evt   System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;

    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_APPSH_INIT(NRF_CLOCK_LFCLKSRC, true);

    // Enable BLE stack
    ble_enable_params_t ble_enable_params;
    memset(&ble_enable_params, 0, sizeof(ble_enable_params));
    ble_enable_params.gatts_enable_params.service_changed = IS_SRVC_CHANGED_CHARACT_PRESENT;
    err_code = sd_ble_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for the Event Scheduler initialization.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
static void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    app_trace_log("bsp event handler %8x\n\r", event);
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            err_code = ble_advertising_restart_without_whitelist();
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

    default:
            break;
    }
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t       err_code;
    uint8_t        adv_flags;
    ble_advdata_t  advdata;

    // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));

    adv_flags                       = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
    advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance      = true;
    advdata.flags                   = adv_flags;
    advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    advdata.uuids_complete.p_uuids  = m_adv_uuids;

    ble_adv_modes_config_t options =
    {
        BLE_ADV_WHITELIST_ENABLED,
        BLE_ADV_DIRECTED_ENABLED,
        BLE_ADV_DIRECTED_SLOW_DISABLED, 0,0,
        BLE_ADV_FAST_ENABLED, APP_ADV_FAST_INTERVAL, APP_ADV_FAST_TIMEOUT,
        BLE_ADV_SLOW_ENABLED, APP_ADV_SLOW_INTERVAL, APP_ADV_SLOW_TIMEOUT
    };

    err_code = ble_advertising_init(&advdata, NULL, &options, on_adv_evt, ble_advertising_error_handler);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Device Manager events.
 *
 * @param[in]   p_evt   Data associated to the device manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const    * p_handle,
                                           dm_event_t const     * p_event,
                                           ret_code_t           event_result)
{
    APP_ERROR_CHECK(event_result);

    switch(p_event->event_id)
    {
        case DM_EVT_DEVICE_CONTEXT_LOADED: // Fall through.
        case DM_EVT_SECURITY_SETUP_COMPLETE:
            m_bonded_peer_handle = (*p_handle);
            break;
    }

    return NRF_SUCCESS;
}


/**@brief Function for the Device Manager initialization.
 */
static void device_manager_init()
{
    uint32_t               err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = false};
    dm_application_param_t  register_param;

    // Initialize peer device handle.
    err_code = dm_handle_initialize(&m_bonded_peer_handle);
    APP_ERROR_CHECK(err_code);

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing leds.
 */
static void leds_init()
{
    uint32_t err_code = bsp_init(BSP_INIT_LED,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

static void adc_init(adc_mode_t req_mode) {
    nrf_adc_config_t nrf_adc_config = {
        .resolution = NRF_ADC_CONFIG_RES_8BIT,
        .reference = NRF_ADC_CONFIG_REF_VBG
    };

    NVIC_DisableIRQ(ADC_IRQn);
    if (req_mode == ADC_MODE_KEYPAD) {
        nrf_adc_config.scaling = NRF_ADC_CONFIG_SCALING_INPUT_ONE_THIRD;
        nrf_adc_configure((nrf_adc_config_t *)&nrf_adc_config);
        nrf_adc_input_select(KEYPAD_ADC_PIN);
        adc_mode = ADC_MODE_KEYPAD;
    } else {
        nrf_adc_config.scaling = NRF_ADC_CONFIG_SCALING_SUPPLY_ONE_THIRD;
        nrf_adc_configure((nrf_adc_config_t *)&nrf_adc_config);
        nrf_adc_input_select(NRF_ADC_CONFIG_INPUT_DISABLED);
        adc_mode = ADC_MODE_BATTERY;
    }
    adc_sample = -1;
    nrf_adc_int_enable(ADC_INTENSET_END_Enabled << ADC_INTENSET_END_Pos);
    NVIC_SetPriority(ADC_IRQn, NRF_APP_PRIORITY_HIGH);
    NVIC_EnableIRQ(ADC_IRQn);
}

void ADC_IRQHandler(void) {
    nrf_adc_conversion_event_clean();

    adc_sample = nrf_adc_result_get();
}

static void adc_timeout_handler(void *p_context)
{
    uint8_t ble_battery_level;
    uint8_t key;
    uint32_t err_code;

    UNUSED_PARAMETER(p_context);

    app_trace_log("adc %d\r\n", (int)adc_sample);

    if (adc_mode == ADC_MODE_KEYPAD) {
        idle_counter = 0;
        key = adc_process_keypad_measurement(adc_sample);
        if (m_conn_handle != BLE_CONN_HANDLE_INVALID) {
            consumer_control_send(key);
        } else {
            keypad_combo_handler(key);
        }
    } else {
        idle_counter++;
        ble_battery_level = adc_process_battery_measurement(adc_sample);

        err_code = ble_bas_battery_level_update(&m_bas, ble_battery_level);
        if ((err_code != NRF_SUCCESS) &&
            (err_code != NRF_ERROR_INVALID_STATE) &&
            (err_code != BLE_ERROR_NO_TX_BUFFERS) &&
            (err_code != BLE_ERROR_GATTS_SYS_ATTR_MISSING)
            )
        {
            APP_ERROR_HANDLER(err_code);
        }

        adc_init(ADC_MODE_KEYPAD);
    }

    if (idle_counter >= MAX_IDLE_BATTERY_MEAS) {
        app_trace_log("idle for too long -- sleeping\n\r");
        sd_ble_gap_disconnect(m_conn_handle,
                              BLE_HCI_REMOTE_DEV_TERMINATION_DUE_TO_POWER_OFF);
        idle_counter = 0;
        sleep_mode_enter();
    }
}

static uint8_t adc_process_battery_measurement(int32_t sample)
{
    uint8_t ble_battery_level;

    battery_level = sample;
    battery_correction = ((float)ADC_BATTERY_3V6) / battery_level;

    /* Coin cells have a very shallow discharge curve, instead of reporting
     * accurate information we cop out here
     */
    ble_battery_level = ((battery_level * 100)/ADC_BATTERY_3V6);
    app_trace_log("battery level %d%%\r\n", (uint8_t)ble_battery_level);

    return ble_battery_level;
}

static uint8_t adc_process_keypad_measurement(int32_t sample)
{
    uint8_t i;

    adc_sample *= battery_correction;
    app_trace_log("adc corrected for battery level %d\r\n", (int)adc_sample);

    for (i = 0; i < KEYPAD_NUMKEYS; i++) {
        if ((keymark[i].low <= adc_sample) &&
            (keymark[i].high >= adc_sample)) {
            app_trace_log("cc key %d\r\n", keymark[i].key);
            return keymark[i].key;
        }
    }

    return RELEASE_KEY;
}

static void keypad_combo_action(keycombo_action_t action)
{
    uint32_t err_code;

    switch (action) {
    case ACTION_ERASE_BONDS:
        err_code = dm_device_delete_all(&m_app_handle);
        APP_ERROR_CHECK(err_code);

        err_code = ble_advertising_restart_without_whitelist();
        if (err_code != NRF_ERROR_INVALID_STATE)
        {
            APP_ERROR_CHECK(err_code);
        }
        break;

    default:
        break;
    }
}

static void keypad_combo_handler(uint8_t key)
{
    uint8_t i;

    if (key == RELEASE_KEY)
        return;

    for (i = 0; i < KEYPAD_COMBINATIONS; i++) {
        if (keycombo[i].keys[keycombo[i].cur] == key) {
            app_trace_log("keycombo %d match %d\n\r", i, keycombo[i].cur);
            keycombo[i].cur++;
            if (keycombo[i].cur == keycombo[i].len) {
                app_trace_log("keycombo %d matched", i);
                keypad_combo_action(keycombo[i].action);
                keycombo[i].cur = 0;
            }
            if (keycombo[i].cur > keycombo[i].len) {
                keycombo[i].cur = 0;
            }
        } else {
            keycombo[i].cur = 0;
        }
    }
}

/*
 * Keypad activity is detected using GPIOTE level changes on a sense pin.
 * This GPIOTE activity is also used to wake from app sleep.
 */
static void keypad_sensed_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    app_trace_log("key change sensed\r\n");
    if (adc_mode == ADC_MODE_KEYPAD) {
        nrf_adc_start();
        app_timer_start(m_adc_timer_id, ADC_MEAS_INTERVAL, NULL);
    }
}

static void keypad_keypress_detection_init(void)
{
    uint32_t err_code;
    nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_gpiote_in_init(KEYPAD_SENSE_PIN, &config, keypad_sensed_handler);
    APP_ERROR_CHECK(err_code);
    nrf_drv_gpiote_in_event_enable(KEYPAD_SENSE_PIN, false);
}

/*
 * At boot read adc for keypresses that indicate special application events.
 */
static void keypad_boot_buttons(void)
{
    adc_init(ADC_MODE_BATTERY);
    nrf_adc_start();
    while (adc_sample == -1);
    adc_process_battery_measurement(adc_sample);

    adc_init(ADC_MODE_KEYPAD);
    nrf_adc_start();
    while (adc_sample == -1);
    start_key = adc_process_keypad_measurement(adc_sample);
    app_trace_log("startup key %d\n\r", key);
}

/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t err_code;

    // Initialize.
    app_trace_init();
    keypad_boot_buttons();

    timers_init();
    leds_init();

    keypad_keypress_detection_init();

    ble_stack_init();
    scheduler_init();
    device_manager_init();
    gap_params_init();
    advertising_init();
    services_init();
    conn_params_init();

    // Start execution.
    timers_start();
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);

    // Enter main loop.
    for (;;)
    {
        app_sched_execute();
        power_manage();
    }
}

/**
 * @}
 */
