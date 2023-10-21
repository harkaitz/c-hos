#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#include "32.h"
#include "public.h"
#include "kernel.h"
#include "config.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#define IDT_INTERRUPT_GATE 0xE
#define IDT_DESC_PRESENT 0x80


/* From 32b.s */
extern void hos_32s_interruptionIgnorer(void);
extern void hos_32s_interruptionHandler0x20(void);
extern void hos_32s_interruptionHandler0x21(void);

struct hos_SegmentDescriptor {
    uint16_t limitLow;
    uint16_t baseLow;
    uint8_t  baseHigh;
    uint8_t  Type;
    uint8_t  flagsLimitHigh;
    uint8_t  baseVHigh;
} __attribute__((packed));

struct hos_GateDescriptor {
    uint16_t HandlerAddressLowBits;
    uint16_t GDTCodeSegmentSelector;
    uint8_t  Reserved;
    uint8_t  Access;
    uint16_t HandlerAddressHighBits;
} __attribute__((packed));

hos_Port hos_PICMasterCommand   = { 0x20, 8, true};
hos_Port hos_PICMasterData      = { 0x21, 8, true};
hos_Port hos_PICSlaveCommand    = { 0xA0, 8, true};
hos_Port hos_PICSlaveData       = { 0xA1, 8, true};


static hos_SegmentDescriptor gdt[4];
static hos_GateDescriptor    idt[256];

void
hos_32c_multibootEntry(void *_multiboot, unsigned int _magicNumber) {

    hos_Kernel_Register();
    
    /* Initialize GDT. */ {
        hos_log("Initializing GDT ...\n");
        gdt[0] = hos_SegmentDescriptor_create(0, 0, 0); /* null */
        gdt[1] = hos_SegmentDescriptor_create(0, 0, 0); /* unused */
        gdt[2] = hos_SegmentDescriptor_create(0, 64*1024*1024, 0x9A); /* code */
        gdt[3] = hos_SegmentDescriptor_create(0, 64*1024*1024, 0x92); /* data */
        static struct {
            uint16_t size;
            uint32_t addr;
        } __attribute__((packed)) gdtl;
        gdtl.addr = (uint32_t) gdt;
        gdtl.size = sizeof(gdt);
        asm volatile("lgdt (%0)": :"p" (&gdtl));
    }
    
    /* Initialize IDT. */ {
        hos_log("Initializing IDT ...\n");
        for (uint16_t i = 0; i<256; i++) {
            idt[i] = hos_GateDescriptor_create(hos_32s_interruptionIgnorer, 0, IDT_INTERRUPT_GATE);
        }
        idt[0x20] = hos_GateDescriptor_create(hos_32s_interruptionHandler0x20, 0, IDT_INTERRUPT_GATE);
        idt[0x21] = hos_GateDescriptor_create(hos_32s_interruptionHandler0x21, 0, IDT_INTERRUPT_GATE);
        static struct {
            uint16_t size;
            void    *Base;
        } __attribute__((packed)) idtp;
        idtp.size = sizeof(idt) - 1;
        idtp.Base = idt;
        asm volatile("lidt %0" : : "m" (idtp));
    }
    
    /* Initialize PIC. */ {
        hos_log("Initializing PIC ...\n");
        hos_Port_write(&hos_PICMasterCommand, 0x11);
        hos_Port_write(&hos_PICSlaveCommand,  0x11);
        hos_Port_write(&hos_PICMasterData,    0x20);
        hos_Port_write(&hos_PICSlaveData,     0x28);
        hos_Port_write(&hos_PICMasterData,    0x04);
        hos_Port_write(&hos_PICSlaveData,     0x02);
        hos_Port_write(&hos_PICMasterData,    0x01);
        hos_Port_write(&hos_PICSlaveData,     0x01);
        hos_Port_write(&hos_PICMasterData,    0x00);
        hos_Port_write(&hos_PICSlaveData,     0x00);
    }
    
    /* Initialize interruptions. */
    hos_kernel_run_preInterruptions();

    /* Activate interruptions. */
    asm volatile("sti");

    hos_shell();
    hos_log("Shell finished, shutting down ...\n");
}

uint32_t
hos_32c_interruptionEntry(uint8_t _interruptNumber, uint32_t _esp) {

    // Acknowledge the interrupt.
    if ((0x20 <= _interruptNumber) && (_interruptNumber < 0x28)) {
        hos_Port_write(&hos_PICMasterCommand, 0x20);
    } else if ((0x28 <= _interruptNumber) && (_interruptNumber < 0x30)) {
        hos_Port_write(&hos_PICMasterCommand, 0x20);
        hos_Port_write(&hos_PICSlaveCommand, 0x20);
    }
    
    // Handle interruption.
    hos_kernel_run_interruptions(_interruptNumber);
    
    return _esp;
}

// -------------------------------------------------------------------

hos_SegmentDescriptor
hos_SegmentDescriptor_create(uint32_t base, uint32_t limit, uint8_t type) {
    hos_SegmentDescriptor sd;
    uint8_t *p = (uint8_t*)&sd;
    
    if (limit > 65536) {
        p[6] = 0x40;
    } else {
        if ((limit & 0xFFF) != 0xFFF) {
            limit = (limit >> 12) - 1;
        } else {
            limit = limit >> 12;
        }
        p[6] = 0xC0;
    }

    p[0] = limit & 0xFF;
    p[1] = (limit >> 8) & 0xFF;
    p[6] |= (limit >> 16) & 0xF;

    p[2] = base & 0xFF;
    p[3] = (base >> 8) & 0xFF;
    p[4] = (base >> 16) & 0xFF;
    p[7] = (base >> 24) & 0xFF;

    p[5] = type;

    return sd;
}

uint32_t
hos_SegmentDescriptor_getBase(hos_SegmentDescriptor *sd) {
    uint8_t *p = (uint8_t*)sd;
    uint32_t base = p[7];
    base = (base << 8) + p[4];
    base = (base << 8) + p[3];
    base = (base << 8) + p[2];
    return base;
}

uint32_t
hos_SegmentDescriptor_getLimit(hos_SegmentDescriptor *sd) {
    uint8_t *p = (uint8_t*)sd;
    uint32_t limit = p[6] & 0xF;
    limit = (limit << 8) + p[1];
    limit = (limit << 8) + p[0];
    if ((p[6] & 0xC0) == 0xC0) {
        limit = (limit << 12) | 0xFFF;
    }
    return limit;
}

uint16_t
hos_GDT_getCodeSegmentSelector() {
    return ((uint8_t*)(&gdt[2])) - ((uint8_t*)gdt);
}

uint16_t
hos_GDT_getDataSegmentSelector() {
    return ((uint8_t*)&(gdt[3])) - ((uint8_t*)gdt);
}

void
hos_Port_write(const hos_Port *port, uint32_t data) {
    uint8_t  data8;
    uint16_t data16;
    
    switch (port->PortBits) {
    case 8:
        if (port->IsSlow) {
            __asm__ volatile("outb %0, %1" "\n"
                             "jmp 1f"      "\n"
                             "1: jmp 1f"   "\n"
                             "1:"
                             : : "a" (data8 = data), "Nd" (port->PortNumber));
        } else {
            __asm__ volatile("outb %0, %1" : : "a" (data8 = data), "Nd" (port->PortNumber));
        }
        break;
    case 16:
        __asm__ volatile("outw %0, %1" : : "a" (data16 = data), "Nd" (port->PortNumber));
        break;
    case 32:
        __asm__ volatile("outl %0, %1" : : "a" (data), "Nd" (port->PortNumber));
        break;
    }
}

uint32_t
hos_Port_read(const hos_Port *port) {
    uint8_t  data8;
    uint16_t data16;
    uint32_t data32 = 0;

    switch(port->PortBits) {
    case 8:
        __asm__ volatile("inb %1, %0" : "=a" (data8) : "Nd" (port->PortNumber));
        data32 = data8;
        break;
    case 16:
        __asm__ volatile("inw %1, %0" : "=a" (data16) : "Nd" (port->PortNumber));
        data32 = data16;
        break;
    case 32:
        __asm__ volatile("inl %1, %0" : "=a" (data32) : "Nd" (port->PortNumber));
        break;
    }

    return data32;
}

hos_GateDescriptor
hos_GateDescriptor_create(void (*handler) (void), uint8_t privilegeLevel, uint8_t type) {
    hos_GateDescriptor gd;
    uint32_t h = (uint32_t) handler;
    gd.HandlerAddressLowBits  = h & 0xFFFF;
    gd.GDTCodeSegmentSelector = hos_GDT_getCodeSegmentSelector();
    gd.Reserved               = 0;
    gd.Access                 = IDT_DESC_PRESENT | type | ((privilegeLevel&3) << 5);
    gd.HandlerAddressHighBits = (h >> 16) & 0xFFFF;
    return gd;
}
