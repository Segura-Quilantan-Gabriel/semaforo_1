#include "  pico/stdlib.h"
#include <stdio.h>

#define V1_RED     2
#define V1_YELLOW  3
#define V1_GREEN   4
#define V2_RED     5
#define V2_YELLOW  6
#define V2_GREEN   7
#define P1_RED     8
#define P1_GREEN   9
#define P2_RED     10
#define P2_GREEN   11
#define BTN1       12
#define BTN2       13

int segmentos[7] = {14, 15, 16, 17, 18, 19, 20};

#define DISP1 21
#define DISP2 22

#define YELLOW_TIME_MS 3000
#define COUNTDOWN_TIME 10
#define GREEN_TIME_MS 15000

uint8_t digitos[10][7] = {
    {1,1,1,1,1,1,0}, // 0
    {0,1,1,0,0,0,0}, // 1
    {1,1,0,1,1,0,1}, // 2
    {1,1,1,1,0,0,1}, // 3
    {0,1,1,0,0,1,1}, // 4
    {1,0,1,1,0,1,1}, // 5
    {1,0,1,1,1,1,1}, // 6
    {1,1,1,0,0,0,0}, // 7
    {1,1,1,1,1,1,1}, // 8
    {1,1,1,1,0,1,1}  // 9
};

void init_gpio() {
    int pins[] = { V1_RED, V1_YELLOW, V1_GREEN, V2_RED, V2_YELLOW, V2_GREEN,
                   P1_RED, P1_GREEN, P2_RED, P2_GREEN, DISP1, DISP2 };
    for (int i = 0; i < 12; i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_OUT);
        gpio_put(pins[i], 0);
    }
    for (int i = 0; i < 7; i++) {
        gpio_init(segmentos[i]);
        gpio_set_dir(segmentos[i], GPIO_OUT);
        gpio_put(segmentos[i], 0);
    }
    gpio_init(BTN1);
    gpio_set_dir(BTN1, GPIO_IN);
    gpio_pull_up(BTN1);

    gpio_init(BTN2);
    gpio_set_dir(BTN2, GPIO_IN);
    gpio_pull_up(BTN2);
}

void set_vehicular(int red, int yellow, int green, int r_pin, int y_pin, int g_pin) {
    gpio_put(r_pin, red);
    gpio_put(y_pin, yellow);
    gpio_put(g_pin, green);
}

void set_peatonal(int red, int green, int r_pin, int g_pin) {
    gpio_put(r_pin, red);
    gpio_put(g_pin, green);
}

void mostrar_digito(int num, int display) {
    gpio_put(DISP1, display == 1);
    gpio_put(DISP2, display == 2);
    for (int i = 0; i < 7; i++) {
        gpio_put(segmentos[i], digitos[num][i]);
    }
    sleep_ms(5);
    gpio_put(DISP1, 0);
    gpio_put(DISP2, 0);
}

void countdown_display(int display) {
    for (int i = COUNTDOWN_TIME; i >= 0; i--) {
        for (int j = 0; j < 200; j++) {
            mostrar_digito(i, display);
        }
    }
}

void activar_cruce(int peatonal_red, int peatonal_green, int veh_red, int veh_yellow, int veh_green, int display, 
                   int otro_veh_red, int otro_veh_yellow, int otro_veh_green) {
    // Cambiar semáforo actual a amarillo
    set_vehicular(0, 1, 0, veh_red, veh_yellow, veh_green);
    sleep_ms(YELLOW_TIME_MS);

    // Poner en rojo el semáforo actual
    set_vehicular(1, 0, 0, veh_red, veh_yellow, veh_green);
    sleep_ms(500);

    // Activar peatonal
    set_peatonal(0, 1, peatonal_red, peatonal_green);
    countdown_display(display);
    set_peatonal(1, 0, peatonal_red, peatonal_green);

    // Otro semáforo permanece en verde el tiempo restante
    sleep_ms(GREEN_TIME_MS - YELLOW_TIME_MS - 500 - (COUNTDOWN_TIME * 1000));

    // Restaurar semáforo actual a verde
    set_vehicular(0, 0, 1, veh_red, veh_yellow, veh_green);
    set_vehicular(1, 0, 0, otro_veh_red, otro_veh_yellow, otro_veh_green);
}

int main() {
    stdio_init_all();
    init_gpio();

    set_vehicular(0, 0, 1, V1_RED, V1_YELLOW, V1_GREEN);
    set_vehicular(1, 0, 0, V2_RED, V2_YELLOW, V2_GREEN);
    set_peatonal(1, 0, P1_RED, P1_GREEN);
    set_peatonal(1, 0, P2_RED, P2_GREEN);

    absolute_time_t last_change = get_absolute_time();
    bool semaforo_estado = true; // true: V1 verde, false: V2 verde

    while (true) {
        if (semaforo_estado && !gpio_get(BTN1)) {
            activar_cruce(P1_RED, P1_GREEN, V1_RED, V1_YELLOW, V1_GREEN, 1, V2_RED, V2_YELLOW, V2_GREEN);
            last_change = get_absolute_time();
        } else if (!semaforo_estado && !gpio_get(BTN2)) {
            activar_cruce(P2_RED, P2_GREEN, V2_RED, V2_YELLOW, V2_GREEN, 2, V1_RED, V1_YELLOW, V1_GREEN);
            last_change = get_absolute_time();
        }

        if (absolute_time_diff_us(last_change, get_absolute_time()) >= GREEN_TIME_MS * 1000) {
            if (semaforo_estado) {
                set_vehicular(0, 1, 0, V1_RED, V1_YELLOW, V1_GREEN);
                sleep_ms(YELLOW_TIME_MS);
                set_vehicular(1, 0, 0, V1_RED, V1_YELLOW, V1_GREEN);
                set_vehicular(0, 0, 1, V2_RED, V2_YELLOW, V2_GREEN);
            } else {
                set_vehicular(0, 1, 0, V2_RED, V2_YELLOW, V2_GREEN);
                sleep_ms(YELLOW_TIME_MS);
                set_vehicular(1, 0, 0, V2_RED, V2_YELLOW, V2_GREEN);
                set_vehicular(0, 0, 1, V1_RED, V1_YELLOW, V1_GREEN);
            }
            semaforo_estado = !semaforo_estado;
            last_change = get_absolute_time();
        }
        sleep_ms(100);
    }
    return 0;
}