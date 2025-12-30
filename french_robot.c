#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "cyw43.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "constants.h"
#include "motor.h"
#include "wifi_config.h"

#ifndef WIFI_SSID
#error "Define WIFI_SSID in wifi_config.h"
#endif
#ifndef WIFI_PASSWORD
#error "Define WIFI_PASSWORD in wifi_config.h"
#endif

typedef struct {
    float left;
    float right;
} drive_command_t;

static volatile uint32_t last_command_ms = 0;
static volatile bool has_drive_command = false;
static drive_command_t last_drive = {0};

static void log_drive_command(float left, float right) {
    // Derive forward/backward (Y) and turn (X) intents from motor values.
    const float deadzone = 0.05f;
    float forward = 0.5f * (left + right);
    float turn = 0.5f * (left - right);

    const char *y_dir = "NeutralY";
    if (forward > deadzone) y_dir = "Forward(+Y)";
    else if (forward < -deadzone) y_dir = "Backward(-Y)";

    const char *x_dir = "NeutralX";
    if (turn > deadzone) x_dir = "Right(+X)";
    else if (turn < -deadzone) x_dir = "Left(-X)";

    printf("CMD DRIVE left=%.3f right=%.3f %s %s\n", left, right, y_dir, x_dir);
}

static void apply_drive(float left, float right) {
    // Clamp to safe range and drive both motors.
    if (left > 1.0f) left = 1.0f;
    if (left < -1.0f) left = -1.0f;
    if (right > 1.0f) right = 1.0f;
    if (right < -1.0f) right = -1.0f;

    motor_drive(MOTOR_A, left);
    motor_drive(MOTOR_B, right);

    log_drive_command(left, right);

    last_drive.left = left;
    last_drive.right = right;
    last_command_ms = to_ms_since_boot(get_absolute_time());
    has_drive_command = true;
}

static void handle_drive_text(const char *text) {
    // Expected formats:
    //   DRIVE <left> <right>
    //   STOP
    while (*text == ' ' || *text == '\t') {
        text++;
    }

    if (strncmp(text, "STOP", 4) == 0) {
        motor_coast_all();
        has_drive_command = false;
        printf("CMD STOP; coasting motors\n");
        return;
    }

    if (strncmp(text, "DRIVE", 5) != 0) {
        return;  // unknown command
    }

    text += 5;
    while (*text == ' ' || *text == '\t') {
        text++;
    }

    float left = 0.0f;
    float right = 0.0f;
    if (sscanf(text, "%f %f", &left, &right) == 2) {
        apply_drive(left, right);
    }
}

static void udp_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    (void)arg;
    (void)pcb;
    (void)addr;
    (void)port;

    if (!p) {
        return;
    }

    char buf[CONTROL_MESSAGE_MAX_LEN];
    u16_t len = p->tot_len;
    if (len >= sizeof(buf)) {
        len = sizeof(buf) - 1;
    }
    pbuf_copy_partial(p, buf, len, 0);
    buf[len] = '\0';
    pbuf_free(p);

    handle_drive_text(buf);
}

static struct udp_pcb *start_udp_listener(void) {
    struct udp_pcb *pcb = udp_new();
    if (!pcb) {
        printf("udp_new failed\n");
        return NULL;
    }

    err_t err = udp_bind(pcb, IP_ADDR_ANY, CONTROL_UDP_PORT);
    if (err != ERR_OK) {
        printf("udp_bind failed: %d\n", err);
        udp_remove(pcb);
        return NULL;
    }

    udp_recv(pcb, udp_recv_cb, NULL);
    printf("Listening for drive commands on UDP port %d\n", CONTROL_UDP_PORT);
    return pcb;
}

static const char *link_status_name(int status) {
    switch (status) {
        case CYW43_LINK_DOWN: return "DOWN";
        case CYW43_LINK_JOIN: return "JOIN";
        case CYW43_LINK_NOIP: return "NOIP";
        case CYW43_LINK_UP: return "UP";
        case CYW43_LINK_FAIL: return "FAIL";
        case CYW43_LINK_NONET: return "NONET";
        case CYW43_LINK_BADAUTH: return "BADAUTH";
        default: return "UNKNOWN";
    }
}

static bool wifi_connect(void) {
    cyw43_arch_enable_sta_mode();

    const int attempts = 5;
    for (int i = 1; i <= attempts; i++) {
        printf("Connecting to Wi-Fi SSID \"%s\" (attempt %d/%d)...\n", WIFI_SSID, i, attempts);
        int err = cyw43_arch_wifi_connect_timeout_ms(
                WIFI_SSID,
                WIFI_PASSWORD,
                CYW43_AUTH_WPA2_AES_PSK,
                WIFI_CONNECT_TIMEOUT_MS);
        int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);

        if (err == 0 && (status == CYW43_LINK_UP || status == CYW43_LINK_NOIP || status == CYW43_LINK_JOIN)) {
            struct netif *netif = netif_default;
            if (netif && netif_is_up(netif)) {
                printf("Wi-Fi connected, IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
            } else {
                printf("Wi-Fi connected (IP pending DHCP)\n");
            }
            return true;
        }

        printf("Wi-Fi connect failed: err=%d status=%s\n", err, link_status_name(status));
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(750);
    }

    printf("Wi-Fi connection failed after %d attempts. Motors will remain coasting.\n", attempts);
    return false;
}

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("Wi-Fi/LED init failed\n");
        return -1;
    }

    wifi_connect();  // continue even if it fails; logs status

    motor_init();
    motor_coast_all();
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);  // Status LED on

    start_udp_listener();

    printf("Robot ready for UDP control. Coasting until commands arrive.\n");
    sleep_ms(200);  // Give the USB serial a moment to connect

    uint32_t last_log_ms = to_ms_since_boot(get_absolute_time());
    while (true) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, !cyw43_arch_gpio_get(CYW43_WL_GPIO_LED_PIN));
        uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        uint32_t last_ms = last_command_ms;
        if (has_drive_command && (uint32_t)(now_ms - last_ms) > CONTROL_IDLE_TIMEOUT_MS) {
            motor_coast_all();
            has_drive_command = false;
            printf("No command for %u ms -> coasting motors\n", CONTROL_IDLE_TIMEOUT_MS);
        }
        // Throttle periodic logging to avoid USB/serial stalls.
        if ((uint32_t)(now_ms - last_log_ms) > 1000) {
            printf("Heartbeat: %u ms uptime\n", now_ms);
            last_log_ms = now_ms;
        }
        sleep_ms(10);
    }
}
