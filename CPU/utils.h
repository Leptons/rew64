#ifndef __UTILS_H__
#define __UTILS_H__

#define ROUND_UP(a, b) (((a)+(b)-1)/(b)*(b))
#define BETW(a, x, b) ((a)<=(x)&&(x)<(b))
#define min(a, b) ((a)<=(b)?a:b)
#define max(a, b) ((a)>=(b)?a:b)

int64 rev(int64 val, int n, int size);
void *xrealloc(void *mem, int size);
void *xmalloc(int size);
void *zmalloc(int size);

#endif // __UTILS_H__
