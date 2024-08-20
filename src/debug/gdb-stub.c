#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <assert.h>
#include <string.h>
#include <socket.h>
#include "memory.h"

#define HASH_TABLE_SIZE 32
#define HASH_TABLE_HASH(address) ((address) % HASH_TABLE_SIZE)

/** All possible main commands of the stub */
enum GDB_COMMANDS {
    GDB_PSX_CONTINUE          = 'a',
    GDB_PSX_BREAKPOINT_DELETE = 'b',
    GDB_PSX_BREAKPOINT_LIST   = 'c',
    GDB_PSX_BREAKPOINT_SET    = 'd',
    GDB_PSX_DEVICE_INFO       = 'e',
    GDB_PSX_HELP              = 'f',
    GDB_PSX_MEMORY_READ       = 'g',
    GDB_PSX_MEMORY_WRITE      = 'h',
    GDB_PSX_QUIT              = 'i',
    GDB_PSX_REGISTER_READ     = 'j',
    GDB_PSX_REGISTER_WRITE    = 'k',
    GDB_PSX_STEP_INSTRUCTION  = 'l',
    GDB_PSX_STOP              = 'm',
};

struct gdb_packet {
    char  command;      /** single character command identifier */
    char *data;         /** contains the command specific data, like arguments or strings */
    char  checksum[3];  /** contains a '#' followed by a checksum, (sum of all chars in data, modulo 256) */

    size_t used;        /** used characters of data buffer */
    size_t size;        /** maximum size of data buffer */
};

/** All possible matchpoints (break/watch points) */
enum MP_TYPE {
    MP_BP_READ,
    MP_BP_WRITE,
    MP_WP_READ,
    MP_WP_WRITE,
    MP_WP_ACCESS
};

struct mp_entry {
    enum MP_TYPE mp_type;       /** type of matchpoint hit */
    uint32_t address;           /** address of matchpoint hit */
    uint32_t value;             /** value at the time of setting matchpoint */
    struct mp_entry *next;      /** next matchpoint in the list */
}

struct gdb_stub {
    struct gdb_packet current;

    int protocol;
    int socket_fd;
    int listen_fd;
    int signal;


    char response;
};

static gdb_stub stub;

static struct mp_entry hash_table[HASH_TABLE_SIZE];

/** listens for a connection from gdb */
static void
socket_connect
( struct connection *conn , const char *address , int port )
{ }

/** reads from socket into gdb_packet */
static void
socket_read
( char *data, size_t size )
{ }

/** writes back to gdb */
static void
socket_write
( char *data, size_t size )
{ }

/** closes the socket to gdb */
static void
socket_close
( void )
{ }

/** initialises the hash table for the matchpoints */
static void
mp_hash_init
( void )
{ }

/** adds a new matchpoint to the hash table */
static void
mp_hash_add
( enum MP_TYPE type, uint32_t address )
{ 
    /** calculate hash index  */
    uint32_t index = HASH_TABLE_HASH(address);

    /** walk to the breakpoint before the location of the new one */
    struct mp_entry *set;
    for (struct mp_entry *seek = &hash_table[index]; seek != NULL; set = seek, seek = seek->next)
    {
        if (seek->mp_type > type && seek->address > address) { break; }
    }

    /** allocate the new breakpoint */
    struct mp_entry *new = malloc(sizeof(mp_entry));

    /** get value from address */
    uint32_t value;
    memory_cpu_load_32bit(address, &value);
    
    /** populate the new breakpoint */
    new->mp_type = type;
    new->address = address;
    new->value   = value;
    new->next    = set->next;
    
    /** add it to list */
    set->next = new;
}

/** finds the specified matchpoint NOTE: does not delete */
static mp_entry *
mp_hash_lookup
( enum MP_TYPE type, uint32_t address )
{
    /** will store desired matchpoint */
    struct mp_entry *seek = NULL;

    /** find matchpoint */
    for (seek = &hash_table[index]; seek != NULL; set = seek, seek = seek->next)
    {
        if (seek->mp_type > type && seek->address > address) { break; }
    }
    
    /** return matchpoint if found otherwise NULL */
    return (seek->mp_type == type && seek->address == address) ? seek: NULL;
}

/** finds the specified matchpoint NOTE: does delete */
static mp_entry *
mp_hash_delete
( enum MP_TYPE type, uint32_t address )
{ 
    /** calculate the hash index */
    uint32_t index = HASH_TABLE_HASH(address);

    /** will store desired matchpoint */
    struct mp_entry *delete;
    struct mp_entry *previous;

    /** find matchpoint before the one to be deleted */
    for (delete = &hash_table[index]; delete != NULL; previous = delete, delete = delete->next)
    {
        if (delete->mp_type > type && delete->address > address) { break; }
    }
    
    /** if the next matchpoint is found remove from list */
    if (previous->next->mp_type == type && previous->next->address == address)
    {
        previous->next = delete->next;
    }
    
    /** return matchpoint if found otherwise NULL */
    return (delete->mp_type == type && delete->address == address) ? delete: NULL;
}

/** deinitialises the hash table for the matchpoints */
static void
mp_hash_deinit
( void )
{ 

    /** free all of the pointers */
    struct mp_entry *head;
    struct mp_entry *tail;
    for (uint32_t index = 0; index < HASH_TABLE_SIZE; index++) 
    {
        for (tail = &hash_table[index], head = tail->next; head != NULL; tail = head, head = head->next)
            free(tail);
    }
}

/** continues the operation of the emulator */
static void
gdb_stub_continue
( void )
{ }

/** deletes a defined breakpoint */
static void 
gdb_stub_breakpoint_delete
( void )
{ }

/** lists all defined breakpoints */
static void 
gdb_stub_breakpoint_list
( void )
{ }

/** creates a new breakpoint */
static void
gdb_stub_breakpoint_set
( void )
{ }

/** returns all device information */
static void
gdb_stub_device_info
( void )
{ }

/** prints the help options */
static void 
gdb_stub_help
( void )
{ }

/** reads the memory from address */
static void 
gdb_stub_memory_read
( void )
{ }

/** writes to memory at address */
static void 
gdb_stub_memory_write
( void )
{ }

/** quits the application */
static void 
gdb_stub_quit
( void )
{ }

/** reads a specified register */
static void 
gdb_stub_register_read
( void )
{ }

/** writes to a specifed register */
static void 
gdb_stub_register_write
( void )
{ }

/** steps the emulator by one instruction */
static void 
gdb_stub_step_instruction
( void )
{ }

/** stops the execution of the emulator */
static void 
gdb_stub_stop
( void )
{ }

/** initializes the stub context */
void
gdb_stub_init
( int argc, char **argv )
{ 
    socket_connect();
    mp_hash_init();
}

/** deinitializes the stub context cleaning up any resources */
void
gdb_stub_deinit
( void )
{ 
    mp_hash_deinit();
    socket_close();
}

/** gdb stub processes main commands */
void
gdb_stub_process
( void )
{ 
    socket_read( ( char * ) &stub.current, sizeof( stub.current ) );

    switch ( stub.current.command )
    {
        case ( GDB_PSX_CONTINUE          ): gdb_stub_continue();          stub.response = '+'; break;
        case ( GDB_PSX_BREAKPOINT_DELETE ): gdb_stub_breakpoint_delete(); stub.response = '+'; break;
        case ( GDB_PSX_BREAKPOINT_LIST   ): gdb_stub_breakpoint_list();   stub.response = '+'; break;
        case ( GDB_PSX_BREAKPOINT_SET    ): gdb_stub_breakpoint_set();    stub.response = '+'; break;
        case ( GDB_PSX_DEVICE_INFO       ): gdb_stub_device_info();       stub.response = '+'; break;
        case ( GDB_PSX_HELP              ): gdb_stub_help();              stub.response = '+'; break;
        case ( GDB_PSX_MEMORY_READ       ): gdb_stub_memory_read ();      stub.response = '+'; break;
        case ( GDB_PSX_MEMORY_WRITE      ): gdb_stub_memory_write();      stub.response = '+'; break;
        case ( GDB_PSX_QUIT              ): gdb_stub_quit();              stub.response = '+'; break;
        case ( GDB_PSX_REGISTER_READ     ): gdb_stub_register_read();     stub.response = '+'; break;
        case ( GDB_PSX_REGISTER_WRITE    ): gdb_stub_register_write();    stub.response = '+'; break;
        case ( GDB_PSX_STEP_INSTRUCTION  ): gdb_stub_step_instruction();  stub.response = '+'; break;
        case ( GDB_PSX_STOP              ): gdb_stub_stop();              stub.response = '+'; break;
        default:                                                          stub.response = '-'; break;
    }

    socket_write( stub.response, 1 );
}

