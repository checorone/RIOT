#pragma once
#define STRING_SIZE (155U)

typedef void(* rx_cb_t) (void *arg, uint8_t data);

typedef struct {
    int lenght;
    char buf[STRING_SIZE];
} string;
