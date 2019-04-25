#ifndef _SERIAL_H_
#define _SERIAL_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

void UART2_printf(const char *fmt, ...);
void serial_setup();

#ifdef __cplusplus
}
#endif

#endif
