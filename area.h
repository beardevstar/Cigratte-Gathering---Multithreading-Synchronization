#ifndef AREA_HEADER
#define AREA_HEADER

#include <pthread.h>

class Area {
	int locked_cnt;
public:
    int sx, sy;
    int buts;
    pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
public:
	bool lock_area(int sw, int sh);
	bool unlock_area(int sw, int sh);
};

Area* getArea(int _sx, int _sy);
#endif