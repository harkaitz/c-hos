#ifndef H32_H
#define H32_H

#include <stdint.h>
#include <stdbool.h>

typedef struct hos_Port              hos_Port;
typedef struct hos_SegmentDescriptor hos_SegmentDescriptor;
typedef struct hos_GateDescriptor    hos_GateDescriptor;

struct hos_Port {
    uint16_t PortNumber;
    uint8_t  PortBits;
    bool     IsSlow;
};

hos_SegmentDescriptor hos_SegmentDescriptor_create   (uint32_t base, uint32_t limit, uint8_t type);
uint32_t              hos_SegmentDescriptor_getBase  (hos_SegmentDescriptor *sd);
uint32_t              hos_SegmentDescriptor_getLimit (hos_SegmentDescriptor *sd);
uint16_t              hos_GDT_getCodeSegmentSelector();
uint16_t              hos_GDT_getDataSegmentSelector();
void                  hos_Port_write (const hos_Port *port, uint32_t data);
uint32_t              hos_Port_read  (const hos_Port *port);
hos_GateDescriptor    hos_GateDescriptor_create(void (*handler) (void), uint8_t privilegeLevel, uint8_t type);

extern hos_Port hos_PICMasterCommand, hos_PICMasterData;
extern hos_Port hos_PICSlaveCommand, hos_PICSlaveData;



#endif
