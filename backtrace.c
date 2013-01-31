#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <backtrace.h>

#define CPU_ARCH_X86      0
#define CPU_ARCH_X86_64   1
#define CPU_ARCH_ARM      2
#define CPU_ARCH          CPU_ARCH_ARM

unsigned long addr_start = 0, addr_end = 0;
int max_frame_depth = 0, max_frame_size = 0;

struct call_table_item {
  unsigned callee;
  struct call_table_item *next;
};

struct call_table_item **call_table = NULL;

void call_table_init(unsigned size)
{
  call_table = (struct call_table_item**) calloc(size, sizeof(struct call_table_item*));
}

void call_table_set(unsigned caller, unsigned callee)
{
  struct call_table_item *item = call_table[caller];
  while (item) {
    if (item->callee == callee)
      return;
    item = item->next;
  }
  struct call_table_item *new_item = malloc(sizeof(struct call_table_item));
  new_item->next = call_table[caller];
  new_item->callee = callee;
  call_table[caller] = new_item;
}

void backtrace()
{
  if (addr_end <= addr_start)
    return;
  void *bp = 0, *ip = 0, *sp = 0, *prev_bp = 0, *prev_ip = 0;

  #if CPU_ARCH == CPU_ARCH_X86
  __asm__("mov %%ebp, %0;" : "=r"(bp));
  __asm__("mov %%esp, %0;" : "=r"(sp));
  #elif CPU_ARCH == CPU_ARCH_X86_64
  __asm__("movq %%rbp, %0;" : "=r"(bp));
  __asm__("movq %%rsp, %0;" : "=r"(sp));
  #elif CPU_ARCH == CPU_ARCH_ARM
  __asm__("mov %0, fp" : "=r"(bp));
  __asm__("mov %0, sp" : "=r"(sp));
  //__asm__("mov %0, lr" : "=r"(ip));
  #else
  return;
  #endif
  int i = 0;
  while (bp >= sp) {
    #if CPU_ARCH == CPU_ARCH_X86 || CPU_ARCH == CPU_ARCH_X86_64
    prev_bp = *((void**)bp);
    prev_ip = *((void**)bp + 1);
    #else
    prev_bp = *((void**)bp - 3);
    prev_ip = *((void**)bp - 1);
    #endif
    if (prev_ip >= addr_start && prev_ip < addr_end
        && ip >= addr_start && ip < addr_end) {
      call_table_set((unsigned long)prev_ip - addr_start, (unsigned long)ip - addr_start);
    }
    if (abs(bp - prev_bp) > max_frame_size) //函数栈帧太大就认为出错
      break;
    i ++;
    if (i > max_frame_depth)
      break;
    bp = prev_bp;
    ip = prev_ip;
  }
}

void backtrace_dump(const char *filepath)
{
  if (!call_table) return;

  FILE *file = fopen(filepath, "w");
  unsigned caller = 0;
  for (; caller < addr_end - addr_start; caller ++) {
    struct call_table_item *item = call_table[caller];
    while (item) {
      fprintf(file, "%x %x\n", caller, item->callee);
      item = item->next;
    }
  }
  fclose(file);
}

void backtrace_init(const char *name, int _max_frame_depth, int _max_frame_size)
{
  static char inited = 0;
  if (inited) return;
  inited = 1;
  size_t name_len = strlen(name);
  char line[256];
  FILE *self = fopen("/proc/self/maps", "r");
  while (fgets(line, sizeof(line), self) != NULL) {
    size_t line_len = strlen(line);
    line_len --;
    line[line_len] = 0;
    if (name_len + 1 < line_len
        && strcmp(name, &line[line_len - name_len]) == 0
        && line[line_len - name_len - 1] == '/') {
      sscanf(line, "%lx-%lx", &addr_start, &addr_end);
      if (addr_end > addr_start) {
        printf("MAP: %s\n", line);
        call_table_init(addr_end - addr_start);
        break;
      }
      addr_start = addr_end = 0;
    }
  }
  fclose(self);
  max_frame_depth = _max_frame_depth;
  max_frame_size = _max_frame_size;
}

