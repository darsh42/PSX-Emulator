#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "psx.h"
#include "error.h"
#include "memory.h"

#define STUB_PORT            1337
#define STUB_TYPE            IPPROTO_TCP
#define STUB_ADDRESS         "127.0.0.1"
#define STUB_MAX_CONNECTIONS 1

#define STUB_PACKET_SIZE 512

#define HASH_TABLE_SIZE 32
#define HASH_TABLE_HASH(address) ((address) % HASH_TABLE_SIZE)

#define print_stub_error(func, format, ...) print_error("cpu.c", func, format, __VA_ARGS__)

/** All possible main commands of the stub */
enum GDB_COMMANDS {
    GDB_PSX_EXTENDED_SUPPORT   = '!',
    GDB_PSX_EXCEPTION          = '?',
    GDB_PSX_CONTINUE           = 'c',
    GDB_PSX_REGISTER_READ_ALL  = 'g',
    GDB_PSX_REGISTER_WRITE_ALL = 'G',
    GDB_PSX_REGISTER_READ      = 'p',
    GDB_PSX_REGISTER_WRITE     = 'P',
    GDB_PSX_MEMORY_READ        = 'm',
    GDB_PSX_MEMORY_WRITE       = 'M',
    GDB_PSX_STEP_INSTRUCTION   = 's',
    GDB_PSX_BREAKPOINT_DELETE  = 'z',
    GDB_PSX_BREAKPOINT_SET     = 'Z',
    GDB_PSX_QUERY_GENERAL      = 'q',
    GDB_PSX_QUERY_SET          = 'Q',

    GDB_PSX_BREAKPOINT_LIST    = 'y',
    GDB_PSX_DEVICE_INFO        = '/',
    GDB_PSX_HELP               = '.',
    GDB_PSX_QUIT               = ',',
    GDB_PSX_STOP               = 'x',
};

struct gdb_packet {
    char   data[STUB_PACKET_SIZE]; /** main packet data */
    size_t size;                   /** maximum size of data buffer */
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
};

struct gdb_stub {
    struct PSX *psx;

    struct gdb_packet current;
    struct gdb_packet response;

    int protocol;
    int socket_fd;
    int listen_fd;
    int signal;
};

static struct gdb_stub stub;

static struct mp_entry hash_table[HASH_TABLE_SIZE];

/** gets command character from gdb packet */
char gdb_stub_get_command ( void )
{
    char c;
    int  i = 0;
    
    /** skip to start of command */
    while ( (c = stub.current.data[i]) != '$' && i < stub.current.size)
        i++;

    /** return the command character */
    return stub.current.data[++i];
}

/** verifies the checksum of the packet recieved */
bool gdb_stub_verify_checksum ( void )
{
    char c;
    int  i    =  0;
    int  sum  =  0;
    int check = -1;

    /** skip to command start */
    while ( (c = stub.current.data[i]) != '$' && i < STUB_PACKET_SIZE)
        i++;
    
    /** skip past '$' */
    i++;
    
    /** sum all chararcters */
    while ( (c = stub.current.data[i]) != '#' && i < STUB_PACKET_SIZE)
    {
        sum += c; // total
        i++;      // index
    }

    /** skip past '#' */
    i++;

    /** calculate expected checksum */
    sum %= 256;
    
    if ( i < STUB_PACKET_SIZE)
        /** convert checksum into int for comparison */
        check = strtol(&stub.current.data[i], NULL, 16);
    
    /** return comparison */
    return (sum == check);
}

static void gdb_stub_apply_checksum(char *data, char *destination, size_t size)
{
    /** sum all the chars in the data string */
    unsigned char c;
    int  i   = 0;
    int  sum = 0;
    
    /** skip to command */
    while ( (c = data[i]) != '$' && i < size )
        i++;
    
    /** skip past '$' */
    i++;
    
    /** sum chars to '#' */
    while ( (c = data[i]) != '#' && i < size )
    {
        sum += c;
        i++;
    }
    
    /** calculate checksum */
    sum %= 256;
    
    /** print checksum in hex to destination */
    sprintf(destination, "%02x", sum);
}

/** listens for a connection from gdb */
static void socket_connect ( const char *address, const int port )
{ 
    /** stores results of function for asserts */
    int result = 0;

    /** open the file descriptor of the socket */
    stub.socket_fd = socket(AF_INET, SOCK_STREAM, STUB_TYPE);
    
    /** quit and display message if socket issue */
    assert(stub.socket_fd != -1);
    
    /** create the socket address */
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr   = {inet_addr(address)},
        .sin_port   = htons(port)
    };

    /** bind the socket */
    result = bind(stub.socket_fd, (struct sockaddr *)&addr, sizeof(addr));

    /** quit and display if binding issue */
    assert(result != -1);

    /** listen for a connection to the socket */
    result = listen(stub.socket_fd, STUB_MAX_CONNECTIONS);

    /** quit and display if listening issue */
    assert(result != -1);

    /** block program execution and wait to accept the gdb connection */
    stub.listen_fd = accept(stub.socket_fd, NULL, 0);

    /** quit and display if accepting issue */
    assert(stub.listen_fd != -1);
}

/** reads from socket into gdb_packet */
static void socket_read ( char *buffer, size_t *used, size_t size )
{ 
    /** recieve data into buffer */
    *used = recv(stub.listen_fd, buffer, size, 0);
    
    /** quit and display message if recieving issue */
    assert(*used != -1);
}

/** writes back to gdb */
static void socket_write ( char *data, size_t size )
{ 
    /** check data pointer */
    assert(data != NULL);
    
    /** for asserts */
    int result = 0;
    
    /** send packet over */
    result = write(stub.listen_fd, (void *) data, size);
    
    /** check if successful */
    assert(result != -1);
}

/** closes the socket to gdb */
static void socket_close ( void )
{ 
    /** stores results of function for asserts */
    int result = 0;

    /** close the stub socket */
    result =  close(stub.socket_fd);

    /** quit and display message if close fails */
    assert(result != -1);
}

/** initialises the hash table for the matchpoints */
static void mp_hash_init ( void )
{ }

/** adds a new matchpoint to the hash table */
static void mp_hash_add ( enum MP_TYPE type, uint32_t address )
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
    struct mp_entry *new = malloc(sizeof(struct mp_entry));

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
static struct mp_entry * mp_hash_lookup ( enum MP_TYPE type, uint32_t address )
{
    /** calculate hash table entry */
    uint32_t index = HASH_TABLE_HASH(address);

    /** will store desired matchpoint */
    struct mp_entry *seek = NULL;

    /** find matchpoint */
    for (seek = &hash_table[index]; seek != NULL; seek = seek->next)
    {
        if (seek->mp_type > type && seek->address > address) { break; }
    }
    
    /** return matchpoint if found otherwise NULL */
    return (seek->mp_type == type && seek->address == address) ? seek: NULL;
}

/** finds the specified matchpoint NOTE: does delete */
static struct mp_entry * mp_hash_delete ( enum MP_TYPE type, uint32_t address )
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
static void mp_hash_deinit ( void )
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

/** gdb multiletter packet is not supported */
static void gdb_stub_default_response ( void )
{ 
    /** default response */
    strcpy(stub.response.data, "+$#00");
    stub.response.size = 5;
}


/** continues the operation of the emulator */
static void gdb_stub_continue ( void )
{ }

/** deletes a defined breakpoint */
static void gdb_stub_breakpoint_delete ( void )
{ }

/** lists all defined breakpoints */
static void gdb_stub_breakpoint_list ( void )
{ }

/** creates a new breakpoint */
static void gdb_stub_breakpoint_set ( void )
{ }

/** returns all device information */
static void gdb_stub_device_info ( void )
{ }

/** prints the help options */
static void gdb_stub_help ( void )
{ }

/** reads the memory from address */
static void gdb_stub_memory_read ( void )
{ 
    char c;
    int  i = 0;
    
    /** skip to command */
    while ( (c = stub.current.data[i]) != '$' && i < STUB_PACKET_SIZE )
        i++;
    
    /** skip past '$' char */
    i++;
    /** skip past command char */
    i++;
    
    /** retrive address from string */
    int address = strtol(&stub.current.data[i], NULL, 16);
    
    /** skip to length */
    while ( (c = stub.current.data[i]) != ',' && i < STUB_PACKET_SIZE )
        i++;
    
    /** skip comma */
    i++;
    
    /** store length */
    int size = strtol(&stub.current.data[i], NULL, 16);

    /** compose reply */

    /** index into the response array */
    i = 0;

    /** set acknowledgement and start response */
    stub.response.data[i++] = '+';
    stub.response.data[i++] = '$';
    
    uint32_t b0, b1, b2, b3;
    
    /** read based on size */
    switch (size) {
        case 1: 
            memory_cpu_load_8bit(address, &b0); 
            sprintf(&stub.response.data[i], "%02x", b0);
            i += 2;
            break;

        case 2: 
            memory_cpu_load_8bit(address, &b0); 
            memory_cpu_load_8bit(address, &b1); 
            sprintf(&stub.response.data[i], "%02x%02x", b0, b1);
            i += 4;
            break;

        case 4: 
            memory_cpu_load_8bit(address, &b0);
            memory_cpu_load_8bit(address, &b1);
            memory_cpu_load_8bit(address, &b2);
            memory_cpu_load_8bit(address, &b3); 
            sprintf(&stub.response.data[i], "%02x%02x%02x%02x", b0, b1, b2, b3);
            i += 8;
            break;

        default: break;
    }
    
    /** end response */
    stub.response.data[i++] = '#';
    stub.response.size      = i;
    
    /** compute checksum and append to response */
    gdb_stub_apply_checksum(stub.response.data, &stub.response.data[i], i);

    stub.response.size++;
    stub.response.size++;
}

/** writes to memory at address */
static void gdb_stub_memory_write ( void )
{ }

/** quits the application */
static void gdb_stub_quit ( void )
{ }

/** reads a specified register */
static void gdb_stub_register_read_all ( void )
{ 
    /** index into the response array */
    int i = 0;

    /** set acknowledgement and start response */
    stub.response.data[i++] = '+';
    stub.response.data[i++] = '$';
    
    /** set response */
    char *registers = &stub.response.data[i];
    
    /** retrieve all arthmetic registers */
    for (int r = 0; r < 32; r++, i += 8, registers = &stub.response.data[i])
    {
        sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->R[r]));
    }
    
    /** retrieve COP0 SR register */
    sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->cop0.SR.value));    i+= 8;
    registers = &stub.response.data[i];

    /** retrieve Hi and Lo registers */
    sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->HI)); i+= 8;
    registers = &stub.response.data[i];
    sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->LO)); i+= 8;
    registers = &stub.response.data[i];

    /** retrieve COP0 CAUSE registers */
    sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->cop0.CAUSE.value)); i+= 8;
    registers = &stub.response.data[i];

    /** retrieve COP0 BADV registers */
    sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->cop0.BADV.value));  i+= 8;
    registers = &stub.response.data[i];

    /** retrieve Pc register */
    sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->PC)); i+= 8;
    registers = &stub.response.data[i];

    /** end response */
    stub.response.data[i++] = '#';
    stub.response.size      = i;
    
    /** compute checksum and append to response */
    gdb_stub_apply_checksum(stub.response.data, &stub.response.data[i], i);

    stub.response.size++;
    stub.response.size++;
}

/** writes to a specifed register */
static void gdb_stub_register_write_all ( void )
{ }

/** reads a specified register */
static void gdb_stub_register_read ( void )
{ 
    /** get register to read */
    char c;
    int  i = 0;
    
    /** skip to command */
    while ( (c = stub.current.data[i]) != '$' && i < stub.current.size )
        i++;
    
    /** command index */
    i++;

    /** register */
    int reg = strtol(&stub.current.data[i], NULL, 16);
    
    /** compose reply */
    i = 0;

    /** set acknowledgement and start response */
    stub.response.data[i++] = '+';
    stub.response.data[i++] = '$';
    
    /** set response */
    char *registers = &stub.response.data[i];
    
    /** determine what register has been requested */
    if ( reg < 32  ) { sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->R[reg]));           i+= 8; } else /** retrieve artimetic register */ 
    if ( reg == 32 ) { sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->cop0.SR.value));    i+= 8; } else /** retrieve COP0 SR register */
    if ( reg == 33 ) { sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->HI));               i+= 8; } else /** retrieve Hi register */ 
    if ( reg == 34 ) { sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->LO));               i+= 8; } else /** retrieve Lo register */
    if ( reg == 35 ) { sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->cop0.CAUSE.value)); i+= 8; } else /** retrieve COP0 CAUSE register */
    if ( reg == 36 ) { sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->cop0.BADV.value));  i+= 8; } else /** retrieve COP0 BADV register */
    if ( reg == 37 ) { sprintf(registers, "%08x", __builtin_bswap32(stub.psx->cpu->PC));               i+= 8; }      /** retrieve Pc register */

    /** end response */
    stub.response.data[i++] = '#';
    stub.response.size      = i;
    
    /** compute checksum and append to response */
    gdb_stub_apply_checksum(stub.response.data, &stub.response.data[i], i);

    stub.response.size++;
    stub.response.size++;
}

/** writes to a specifed register */
static void gdb_stub_register_write ( void )
{ }

/** steps the emulator by one instruction */
static void gdb_stub_step_instruction ( void )
{ }

/** stops the execution of the emulator */
static void gdb_stub_stop ( void )
{ }

/** specifies if stub provides gdb extended support */
static void gdb_stub_extended_support ( void )
{ }

/** called if gdb sends an exception */
static void gdb_stub_exception ( void )
{ 
    /** index into the response array */
    int i = 0;

    /** set acknowledgement and start response */
    stub.response.data[i++] = '+';
    stub.response.data[i++] = '$';
    
    /** set response */
    strcpy(&stub.response.data[i], "S05"); i+= 3;
    
    /** end response */
    stub.response.data[i++] = '#';
    stub.response.size      = i + 2;
    
    /** compute checksum and append to response */
    gdb_stub_apply_checksum(stub.response.data, &stub.response.data[i], i);
}

/** gdb general query packet support */
static void gdb_stub_query_general ( void )
{ 
    /** find the type of general query requested */
    char c;
    int  i = 0, j = 0, k = 0;
    /** skip to command */
    while ( (c = stub.current.data[i]) != '$' && i < stub.current.size )
        i++;
    
    /** command index */
    i++;
    
    /** start of query type */
    char *type = &stub.current.data[++i];

    /** find length of type */
    while ( (c = stub.current.data[i]) != ':' && c != '#' && i < stub.current.size )
    {
        i++; // loop index
        j++; // lenght counter
    }

    /** compare with supported query types */
    if (!strncmp(type, "Search", j))    { } else 
    if (!strncmp(type, "Supported", j)) { 
        /** create start of response packet */
        stub.response.data[k++] = '+';
        stub.response.data[k++] = '$';

        /** while not at the end of the packet check each feature */
        while ( (c = stub.current.data[i]) != '#' && i < stub.current.size)
        {
            /** start of feature */
            char *feature = &stub.current.data[++i];

            /** set length counter to 0 */
            j = 0;

            /** find length of feature */
            while ( (c = stub.current.data[i]) != ';' && i < stub.current.size)
            {
                i++; // loop index
                j++; // length counter
            }

            // add supported features to response packet
            if (!strncmp(feature, "hwbreak+", j)) 
            { 
                strncpy(&stub.response.data[k], feature, j);
                k += j;
            }
        }

        /** calculate checksum */
        stub.response.data[k++] = '#';
        stub.response.size      = k;
        
        gdb_stub_apply_checksum(stub.response.data, &stub.response.data[k], k);

        stub.response.size++;
        stub.response.size++;
    } else 
    if (!strncmp(type, "Symbol", j))    { } else
    if (!strncmp(type, "Attached", j))  { 
        /** create start of response packet */
        stub.response.data[k++] = '+';
        stub.response.data[k++] = '$';
        
        /** add reply to packet */
        strcpy(&stub.response.data[k++], "0"); 

        /** calculate checksum */
        stub.response.data[k++] = '#';
        stub.response.size      = k;
        
        gdb_stub_apply_checksum(stub.response.data, &stub.response.data[k], k);

        stub.response.size++;
        stub.response.size++;
    } else 
    {
        gdb_stub_default_response();
    }
}

/** gdb set query packet support */
static void gdb_stub_query_set ( void )
{ }

/** initializes the stub context */
void gdb_stub_init ( int argc, char **argv )
{ 
    /** Parse arguments */
    socket_connect(STUB_ADDRESS, STUB_PORT);
    mp_hash_init();
    stub.psx = get_psx();
}

/** deinitializes the stub context cleaning up any resources */
void gdb_stub_deinit ( void )
{ 
    mp_hash_deinit();
    socket_close();
}

/** gdb stub processes main commands */
void gdb_stub_process ( void )
{ 
    /** clear buffer contents */
    memset(stub.current.data,  0, STUB_PACKET_SIZE);
    memset(stub.response.data, 0, STUB_PACKET_SIZE);

    while ( !gdb_stub_verify_checksum())
        socket_read( stub.current.data , &stub.current.size , sizeof( stub.current.data ) );

    // print_stub_error("gdb_stub_process", "current: %s", stub.current.data);
    switch ( gdb_stub_get_command() )
    {
        case ( GDB_PSX_EXTENDED_SUPPORT   ): gdb_stub_extended_support();   break;
        case ( GDB_PSX_EXCEPTION          ): gdb_stub_exception();          break;
        case ( GDB_PSX_CONTINUE           ): gdb_stub_continue();           break;
        case ( GDB_PSX_REGISTER_READ_ALL  ): gdb_stub_register_read_all();  break;
        case ( GDB_PSX_REGISTER_WRITE_ALL ): gdb_stub_register_write_all(); break;
        case ( GDB_PSX_REGISTER_READ      ): gdb_stub_register_read();      break;
        case ( GDB_PSX_REGISTER_WRITE     ): gdb_stub_register_write();     break;
        case ( GDB_PSX_MEMORY_READ        ): gdb_stub_memory_read ();       break;
        case ( GDB_PSX_MEMORY_WRITE       ): gdb_stub_memory_write();       break;
        case ( GDB_PSX_STEP_INSTRUCTION   ): gdb_stub_step_instruction();   break;
        case ( GDB_PSX_BREAKPOINT_DELETE  ): gdb_stub_breakpoint_delete();  break;
        case ( GDB_PSX_BREAKPOINT_SET     ): gdb_stub_breakpoint_set();     break;
        case ( GDB_PSX_QUERY_GENERAL      ): gdb_stub_query_general();      break;
        case ( GDB_PSX_QUERY_SET          ): gdb_stub_query_set();          break;

        case ( GDB_PSX_BREAKPOINT_LIST    ): gdb_stub_breakpoint_list();    break;
        case ( GDB_PSX_DEVICE_INFO        ): gdb_stub_device_info();        break;
        case ( GDB_PSX_HELP               ): gdb_stub_help();               break;
        case ( GDB_PSX_QUIT               ): gdb_stub_quit();               break;
        case ( GDB_PSX_STOP               ): gdb_stub_stop();               break;
        default:                             gdb_stub_default_response();   break;
    }
    
    // print_stub_error("gdb_stub_process", "response: %s", stub.response.data);
    socket_write( stub.response.data , stub.response.size );
}

