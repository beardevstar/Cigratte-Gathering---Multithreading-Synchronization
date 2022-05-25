// CigBut.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "hw2_output.h"
#include "area.h"
#include "person.h"
#include "order.h"

#if _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for nanosleep
#else
#include <unistd.h> // for usleep
#endif

void sleep_ms(int milliseconds){ // cross-platform sleep function
#if _POSIX_C_SOURCE >= 199309L
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    if (milliseconds >= 1000)
      sleep(milliseconds / 1000);
    usleep((milliseconds % 1000) * 1000);
#endif
}

#define INPUT_FILE_PATH "input.txt"

int gw = 0, gh = 0, tcnt = 0;
int person_cnt = 0;
int order_cnt = 0;

Area* g_areas = NULL;
Person* persons = NULL;
Order* orders = NULL;

void ReadInputFile(const char* _path) {

    FILE* fp = fopen(_path, "r");
    fscanf(fp, "%d %d", &gh, &gw);
    if (gw < 1 || gh < 1) return;
    tcnt = gw * gh;

    g_areas = (Area*)calloc(tcnt, sizeof(Area));
    for (int i = 0; i < tcnt; i++) {
        Area* area = &g_areas[i];
        fscanf(fp, "%d", &area->buts);
        area->sx = i / gw; area->sy = i % gw;
    }

    fscanf(fp, "%d", &person_cnt);
    persons = (Person*)calloc( person_cnt, sizeof(Person));

    for (int i = 0; i < person_cnt; i++) {
        Person* person = persons + i;
        fscanf(fp, "%d %d %d %d %d", \
            &person->gid, &person->sw, &person->sh, &person->tg, &person->area_cnt \
        );
        if (person->area_cnt > 0) {
            person->areas = (Area**)calloc(person->area_cnt, sizeof(Area*));
            for (int j = 0; j < person->area_cnt ; j++)
            {
                int sx = 0, sy = 0;
                fscanf(fp, "%d %d", &sx, &sy);
                person->areas[j] = getArea(sx, sy);
            }
        }
    }

    fscanf(fp,"%d",&order_cnt);
    orders = (Order*)calloc(order_cnt, sizeof(Order));
    for (int i = 0; i < order_cnt ; ++i)
    {
    	/* code */
    	Order *ord = orders + i;
    	fscanf(fp,"%d %s",&ord->order_time, ord->com);
    }

    fclose(fp);
    return;

    printf("######%d * %d Grid!######\n", gw , gh);
    
    for (int i = 0; i < gh; i++)
    {
        for (int j = 0; j < gw; j++) {
            Area *area = getArea(i,j);
            printf("%d\t", area->buts );
        }
        printf("\n");
    }

    printf("######%d Person Exist!######\n",person_cnt);
    for (int i = 0; i < person_cnt; i++)
    {
        Person* person = persons + i;
        person->print();
    }

    printf("######%d Order Exist!######\n",order_cnt);
    for (int i = 0; i < order_cnt; ++i)
    {
    	/* code */
    	Order *ord = orders + i;
    	printf("time : %d, command : [%s]\n",ord->order_time, ord->com);
    }
    printf("\n\n");

}

void ReadFromStdin(){
    printf("Please input grid size(width, height) ");
    scanf("%d,%d", &gw, &gh);
    if (gw < 1 || gh < 1) return;
    tcnt = gw * gh;

    g_areas = (Area*)calloc(tcnt, sizeof(Area));
    for (int i = 0; i < tcnt; i++) {
        Area* area = &g_areas[i];
        area->sx = i / gw; area->sy = i % gw;
        printf("cell (%d,%d) : ", area->sx, area->sy);
        scanf("%d", &area->buts);
    }

    printf("Please input person count : ");
    scanf("%d", &person_cnt);
    persons = (Person*)calloc( person_cnt, sizeof(Person));
    printf("Please input person info (gid,si,sj,tg,ng) : \n");
    for (int i = 0; i < person_cnt; i++) {
        Person* person = persons + i;
        printf("%dth Person (gid,si,sj,tg,ng) : ",i+1);
        scanf("%d,%d,%d,%d,%d", \
            &person->gid, &person->sw, &person->sh, &person->tg, &person->area_cnt \
        );
        if (person->area_cnt > 0) {
            printf("Please input %d person's Area info: >\n",i+1);
            person->areas = (Area**)calloc(person->area_cnt, sizeof(Area*));
            for (int j = 0; j < person->area_cnt ; j++)
            {
                printf("\t%dth Area(si,sj) : ",j+1);
                int sx = 0, sy = 0;
                scanf("%d,%d", &sx, &sy);
                person->areas[j] = getArea(sx, sy);
            }
        }
    }

    printf("Please input order count : ");
    scanf("%d",&order_cnt);
    orders = (Order*)calloc(order_cnt, sizeof(Order));
    printf("Input Orders (time in milliseconds,command[break,continue,stop]) :\n");
    for (int i = 0; i < order_cnt ; ++i)
    {
        /* code */
        printf("%dth Order(ms,command : ",i+1);
        Order *ord = orders + i;
        scanf("%d,%s",&ord->order_time, ord->com);
    }
}

int prev_tm = 0;

void deal_suspend(Person *person){
    //printf("%d,%d",person->suspend_flag,person->prev_flag);
	if(person->suspend_flag == person->prev_flag) return;

	person->prev_flag = person->suspend_flag;
	if(person->suspend_flag == CONTINUE) {
		hw2_notify(PROPER_PRIVATE_CONTINUED, person->gid, 0, 0);
		return;
	}else if(person->suspend_flag == BREAK){
		hw2_notify(PROPER_PRIVATE_TOOK_BREAK, person->gid, 0, 0);
		while(person->suspend_flag == BREAK) {
            sleep_ms(10);
        }
	}else if(person->suspend_flag == STOP){
        hw2_notify(PROPER_PRIVATE_STOPPED, person->gid, 0, 0);
        pthread_exit(0);
    }
}

void* PersonThread(void* _arg) {
    Person* person = (Person*)_arg;

    int lock_w = person->sw,lock_h = person->sh;
    int slp_tg = person->tg;    
	hw2_notify(PROPER_PRIVATE_CREATED,person->gid,0,0);

	for (int i = 0; i < person->area_cnt; ++i)
	{
		/* code */
		Area *area = person->areas[i];
		while(area->lock_area(lock_w,lock_h) == false){//waiting...
			deal_suspend(person);
			sleep_ms(slp_tg);
		}

		hw2_notify(PROPER_PRIVATE_ARRIVED, person->gid, area->sx, area->sy);

		while(area->buts > 0 ){//gathering
			deal_suspend(person);
			sleep_ms(slp_tg);
			hw2_notify(PROPER_PRIVATE_GATHERED, person->gid, area->sx, area->sy);
			area->buts--;
		}

		hw2_notify(PROPER_PRIVATE_CLEARED, person->gid, area->sx, area->sy);
		area->unlock_area(lock_w,lock_h);
	}

	hw2_notify(PROPER_PRIVATE_EXITED, person->gid, 0, 0);
    return 0;
}

void setSuspendFlag(COM_TP _flag){
	for (int i = 0; i < person_cnt; i++)
    {
        Person* person = persons + i;
        person->suspend_flag = _flag ;
    }
}

void *OrderThread(void* _arg){
	while(true){
		int now_tm = get_timestamp()/1000;
		for (int i = 0; i < order_cnt; ++i)
		{
			/* code */
			Order *ord = orders + i;

			if(now_tm > ord->order_time &&  ord->order_time > prev_tm){
				//printf("%u:%s command match\n", ord->order_time, ord->com);

				if(memcmp(ord->com,"break",5) == 0 && strlen(ord->com) == 5){
					setSuspendFlag(BREAK);
					hw2_notify(ORDER_BREAK,0, 0, 0);
				}

				if(memcmp(ord->com,"continue",8) == 0 && strlen(ord->com) == 8){
					setSuspendFlag(CONTINUE);
					hw2_notify(ORDER_CONTINUE, 0, 0, 0);
				}

				if(memcmp(ord->com,"stop",4) == 0 && strlen(ord->com) == 4){
					setSuspendFlag(STOP);
                    hw2_notify(ORDER_STOP, 0, 0, 0);
					return 0;
				}
				prev_tm = now_tm;
			}
		}
		sleep_ms(10);
	}
	return 0;
}

void CreatePersonthreads() {
	pthread_t order_thread = 0;
	pthread_t* handles = (pthread_t*)malloc(person_cnt * sizeof(pthread_t));

	for (int i = 0; i < person_cnt; i++)
    {
    	Person* person = persons + i;
		pthread_create( handles + i , NULL ,  PersonThread , (void*) person);
    }
    pthread_create( &order_thread , NULL ,  OrderThread , 0);

    for (int i = 0; i < person_cnt; i++)
		pthread_join(handles[i],NULL);
}

int main()
{   
    //read from file
    //ReadInputFile(INPUT_FILE_PATH);
    //read from stdin
    ReadFromStdin();
    hw2_init_notifier();
    CreatePersonthreads();
}