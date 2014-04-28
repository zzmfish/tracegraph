#ifndef BACKTRACE_H
#define BACKTRACE_H

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((visibility("default"))) void backtrace();
__attribute__((visibility("default"))) void backtrace_init(const char *name, int max_frame_depth, int max_frame_size);
__attribute__((visibility("default"))) extern void backtrace_dump(const char *filepath);

#ifdef __cplusplus
}
#endif

#endif
