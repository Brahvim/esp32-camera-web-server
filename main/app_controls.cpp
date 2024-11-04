#include <string.h>
#include <esp_log.h>
#include <esp_http_server.h>

#include <Wire.h>
#include <Arduino.h>

#include "protocol_car_controls.hpp"
#include "protocol_android_controls.h"

using namespace NsControls;

static char const *TAG = __FILE__;

// Holds the state for each button:
static enum protocol_android_controls_button_event s_button_states[PROTOCOL_ANDROID_CONTROLS_BUTTON_ID_TOTAL_NUMBER_OF_IDS] = {

	PROTOCOL_ANDROID_CONTROLS_BUTTON_EVENT_NULL, // `NULL`,
	PROTOCOL_ANDROID_CONTROLS_BUTTON_EVENT_NULL, // `LEFT`,
	PROTOCOL_ANDROID_CONTROLS_BUTTON_EVENT_NULL, // `RIGHT`,
	PROTOCOL_ANDROID_CONTROLS_BUTTON_EVENT_NULL, // `FORWARD`,
	PROTOCOL_ANDROID_CONTROLS_BUTTON_EVENT_NULL, // `BACKWARD`,
	PROTOCOL_ANDROID_CONTROLS_BUTTON_EVENT_NULL, // `TOTAL_NUMBER_OF_IDS`.

};

static char const* wire_status_to_string(uint8_t const p_status) {
	/*

	`Wire.cpp` line `432`:
	"""
	https://www.arduino.cc/reference/en/language/functions/communication/wire/endtransmission/
	endTransmission() returns:
	0: success.
	1: data too long to fit in transmit buffer.
	2: received NACK on transmit of address.
	3: received NACK on transmit of data.
	4: other error.
	5: timeout
	"""

	PS The URL has changed to [ https://docs.arduino.cc/language-reference/en/functions/communication/Wire/endTransmission/ ].

	*/

	switch (p_status) {

		case 0: { return "Success"; } break;
		case 1: { return "Data written too large to fit in transmit buffer"; } break;
		case 2: { return "Received NACK when transmitting I2C address"; } break;
		case 3: { return "Received NACK when transmitting I2C data"; } break;
		case 4: { return "Other error"; } break;
		case 5: { return "I2C timeout"; } break;

		default: { return "Unknown error"; } break;

	}
}

static esp_err_t send_i2c_message(EspCamMessageType const p_command) {
	uint8_t resultMessageTypeData[sizeof(EspCamMessageType)];

	memcpy(resultMessageTypeData, &p_command, sizeof(EspCamMessageType));
	Wire.beginTransmission(I2C_ADDR_ARDUINO);
	size_t const bytes_sent = Wire.write(resultMessageTypeData, sizeof(EspCamMessageType));
	uint8_t const wire_status = Wire.endTransmission();

	if (bytes_sent != sizeof(EspCamMessageType)) {
		ESP_LOGE(TAG, "Arduino `Wire`/I2C transmission didn't send all bytes.");
		return ESP_FAIL;
	}

	if (!wire_status) {
		ESP_LOGE(TAG, "Arduino `Wire`/I2C transmission failure: \"%s\".", wire_status_to_string(wire_status));
		return ESP_FAIL;
	}

	return ESP_OK;
}

static void update_button_state(enum protocol_android_controls_button_id const p_button, enum protocol_android_controls_button_event const p_event) {
	s_button_states[p_button] = p_event;

	switch (p_button) {

		default: {

		} break;

		case PROTOCOL_ANDROID_CONTROLS_BUTTON_ID_LEFT: {

		} break;

		case PROTOCOL_ANDROID_CONTROLS_BUTTON_ID_RIGHT: {

		} break;

		case PROTOCOL_ANDROID_CONTROLS_BUTTON_ID_FORWARD: {

		} break;

		case PROTOCOL_ANDROID_CONTROLS_BUTTON_ID_BACKWARD: {

		} break;

		case PROTOCOL_ANDROID_CONTROLS_BUTTON_ID_EMERGENCY_STOP: {

		} break;

	}
}

static esp_err_t protocol_android_controls_handler(httpd_req_t *p_request) {
	char* str_url;
	esp_err_t to_ret = ESP_FAIL;
	size_t str_len = 1 + httpd_req_get_url_query_len(p_request);

	if (str_len > 1) {
		str_url = (char*) malloc(str_len);

		if (httpd_req_get_url_query_str(p_request, str_url, str_len) == ESP_OK) {
			enum protocol_android_controls_button_id button = PROTOCOL_ANDROID_CONTROLS_BUTTON_ID_NULL;
			enum protocol_android_controls_button_event event = PROTOCOL_ANDROID_CONTROLS_BUTTON_EVENT_NULL;

			char value_http_param_event[sizeof(enum protocol_android_controls_button_event)];
			char value_http_param_button[sizeof(enum protocol_android_controls_button_id)];

			if (
				httpd_query_key_value(
					str_url,
					PROTOCOL_ANDROID_CONTROLS_HTTP_PARAMETER_BUTTON,
					value_http_param_button,
					sizeof(value_http_param_button)
				) == ESP_OK) {
				button = (enum protocol_android_controls_button_id) atoi(value_http_param_button);
			}

			if (
				httpd_query_key_value(
					str_url,
					PROTOCOL_ANDROID_CONTROLS_HTTP_PARAMETER_EVENT,
					value_http_param_event,
					sizeof(value_http_param_event)
				) == ESP_OK) {
				event = (enum protocol_android_controls_button_event) atoi(value_http_param_event);
			}

			if (button && event) {
				to_ret &= httpd_resp_set_type(p_request, PROTOCOL_ANDROID_CONTROLS_HTTP_CONTENT_TYPE); // Not strictly required.
				// to_ret &= httpd_resp_set_status(p_request, "200 OK"); // Not strictly required.
				to_ret = httpd_resp_send(p_request, NULL, 0);
			} else {
				to_ret &= httpd_resp_set_status(p_request, "400 Bad Request");
				to_ret &= httpd_resp_send(p_request, NULL, 0);
			}
		}

		free(str_url);
	}

	return to_ret;
}

httpd_uri_t g_uri_controls = {

		.uri = "/controls",
		.method = HTTP_GET,
		.handler = protocol_android_controls_handler,
		.user_ctx = NULL,

	#ifdef CONFIG_HTTPD_WS_SUPPORT
		.is_websocket = true,
		.handle_ws_control_frames = false,
		.supported_subprotocol = NULL,
	#endif

};