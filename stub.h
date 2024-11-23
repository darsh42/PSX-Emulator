#ifndef STUB_H_INCLUDED
#define STUB_H_INCLUDED

#define STUB_PORT            1337
#define STUB_TYPE            IPPROTO_TCP
#define STUB_ADDRESS        "127.0.0.1"
#define STUB_MAX_CONNECTIONS 1

#define STUB_PACKET_SIZE 512

#define HASH_TABLE_SIZE 32
#define HASH_TABLE_HASH(address) ((address) % HASH_TABLE_SIZE)

#define hextoint(c) (((c) <= '9') ? (c) - '0': ((c) <= 'Z') ? (c) - 'A': (c) - 'a')

extern uint32_t gdb_stub_pause;

extern void  wait_gdb_stub( void );
extern void *task_gdb_stub( void *ignore );

#endif // STUB_H_INCLUDED
