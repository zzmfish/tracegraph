#ifndef BACKTRACE_H
#define BACKTRACE_H

#ifdef __cplusplus
extern "C" {
#endif

void backtrace();
void backtrace_init(const char *name, int max_frame_depth, int max_frame_size);
void backtrace_dump(const char *filepath);

#ifdef __cplusplus
}
#endif

#endif
