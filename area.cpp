#include "area.h"
#include <stdio.h>

extern int gw, gh, tcnt;
extern Area *g_areas;

pthread_mutex_t area_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifndef min
#define min(a,b) ((a) <= (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) >= (b) ? (a) : (b))
#endif

bool Area::lock_area(int sw, int sh){
    pthread_mutex_lock(&area_mutex);
    int ex = min(sw, gw - sx), ey = min(sh, gh - sy);
    locked_cnt = 0;

    for (int i = 0; i < ex; i++)
    {
    	/* code */
    	for (int j = 0; j < ey; j++)
    	{
    		/* code */
    		Area *area = getArea(sx + i, sy + j);
			if(area != NULL && pthread_mutex_trylock(&area->mut) != 0){
				pthread_mutex_unlock(&area_mutex);
				unlock_area(sw, sh);
				return false;
			}
			locked_cnt++;
    	}
    }
    pthread_mutex_unlock(&area_mutex);
    //printf("locked (%d, %d) - (%d,%d)..\n", sx,sy,sw,sh);
	return true;
}

bool Area::unlock_area(int sw, int sh){
    pthread_mutex_lock(&area_mutex);
    int ex = min(sw, gw - sx), ey = min(sh, gh - sy);

	for (int i = 0; i < ex; i++)
    {
    	/* code */
    	for (int j = 0; j < ey; j++)
    	{
    		/* code */
    		Area *area = getArea(sx + i, sy + j);
    		if(area) {
    			pthread_mutex_unlock(&area->mut);
    			locked_cnt--;
    			if(locked_cnt < 1) {
    				pthread_mutex_unlock(&area_mutex);
    				return true;
    			} 
    		}
    	}
    }
    pthread_mutex_unlock(&area_mutex);
    printf("unlocked..\n");
    locked_cnt = 0;
	return true;
}
Area* getArea(int _sx, int _sy) {
    return &g_areas[_sx * gw + _sy];
}