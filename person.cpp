#include <stdio.h>
#include "person.h"

extern int gw, gh, tcnt;

void Person::print() {
    printf("------Person %d------\nsw:%d,sh:%d\ttg:%d\tarea_count:%d\n", \
        gid,sw,sh,tg,area_cnt
    );
    printf("areas:\n");
    if (!areas) return;

    for (int i = 0; i < area_cnt; i++)
    {
        Area* area = areas[i];
        printf("(%d,%d)", area->sx, area->sy);
    }
    printf("\n\n");
}
