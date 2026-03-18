// hal.h
// ControllerOS Hardware Abstraction Layer
#ifndef _HAL_H_
#define _HAL_H_

// ========== 基础类型定义 ==========
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;
typedef void void_t;

// ========== 端口I/O操作 ==========
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outl(uint16_t port, uint32_t value) {
    __asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t value;
    __asm__ volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// ========== CPU控制 ==========
static inline void cli(void) {
    __asm__ volatile ("cli");
}

static inline void sti(void) {
    __asm__ volatile ("sti");
}

static inline void hlt(void) {
    __asm__ volatile ("hlt");
}

static inline void cpu_idle(void) {
    __asm__ volatile ("hlt");
}

static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static inline void cpuid(uint32_t code, uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d) {
    __asm__ volatile ("cpuid"
        : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
        : "a"(code), "c"(0));
}

// ========== GDT/IDT操作 ==========
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static inline void lgdt(struct gdt_ptr* gdt) {
    __asm__ volatile ("lgdt (%0)" : : "r"(gdt));
}

static inline void lidt(struct idt_ptr* idt) {
    __asm__ volatile ("lidt (%0)" : : "r"(idt));
}

static inline uint16_t sgdt(struct gdt_ptr* gdt) {
    __asm__ volatile ("sgdt (%0)" : : "r"(gdt));
    return gdt->limit;
}

static inline uint16_t sidt(struct idt_ptr* idt) {
    __asm__ volatile ("sidt (%0)" : : "r"(idt));
    return idt->limit;
}

// ========== 内存分页 ==========
static inline void invlpg(void* addr) {
    __asm__ volatile ("invlpg (%0)" : : "r"(addr) : "memory");
}

static inline void enable_paging(void) {
    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
}

static inline void disable_paging(void) {
    uint32_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~0x80000000;
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
}

// ========== 中断控制 ==========
static inline void set_intr_gate(uint32_t n, void* addr) {
    // 实际实现会在IDT中设置中断门
}

static inline void set_trap_gate(uint32_t n, void* addr) {
    // 实际实现会在IDT中设置陷阱门
}

static inline void set_system_gate(uint32_t n, void* addr) {
    // 实际实现会在IDT中设置系统门
}

// ========== HAL数据结构 ==========
typedef struct _HAL_DATA {
    uint32_t processor_count;
    uint32_t processor_features;
    uint32_t memory_size;
    uint32_t timer_frequency;
    uint32_t gdt_base;
    uint16_t gdt_limit;
    uint32_t idt_base;
    uint16_t idt_limit;
    uint32_t kernel_base;
    uint32_t kernel_size;
} hal_data_t;

// ========== HAL函数声明 ==========
void hal_initialize(void);
uint32_t hal_get_processor_count(void);
uint64_t hal_get_timer_count(void);
void hal_sleep(uint32_t milliseconds);
void hal_reboot(void);
void hal_poweroff(void);
void hal_print_char(char c);
void hal_print_string(const char* str);
void hal_dump_registers(void);

#endif // _HAL_H_
