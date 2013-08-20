#ifndef __PROTO_HEADER_H__
#define __PROTO_HEADER_H__

extern void out_byte(uint16_t port, uint8_t value);
extern uint8_t in_byte(uint16_t port);
extern void display_str(char* s);
extern void display_color_str(char* s, int color);
extern void init_port(void);
extern void init_8259A(void);

#endif  /* __PROTO_HEADER_H__ */
