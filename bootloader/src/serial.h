#ifndef _SERIAL_H_
#define _SERIAL_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#define TXBUFSIZE                    8192                                //Must be power of two because of some math magic
#define TXBUFHEADMASK                ( TXBUFSIZE - 1 )

void UART_printf(const char *fmt, ...);
void serial_setup();

#ifdef __cplusplus
}
#endif

#endif
