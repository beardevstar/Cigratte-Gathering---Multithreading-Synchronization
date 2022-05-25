#ifndef PERSON_HEADER
#define PERSON_HEADER

#include "area.h"
enum COM_TP{
	BREAK = 0x10, CONTINUE, STOP
};

class Person {
public:
    int gid;
    int sw, sh;
    int tg;
    int area_cnt;
    Area** areas;


    COM_TP suspend_flag = CONTINUE, prev_flag = CONTINUE;

public:
    void print() ;
};

#endif