#include <master.h>

/* 
 * Function macros.
 */
#define PORT        4040
#define TABLE_PATH  "/device_table" 

#define INITIALIZE(master)  \
    initialize_master(&master, PORT/*, TABLE_PATH*/)


// #define RECEIVE      receive_master

// #define HANDLE       handle_master

#define RUN(master)         \
    run_master(&master)


int main(int argc, char *argv[])
{
    master_t    master;     // 마스터.

    // 마스터 초기화.
    INITIALIZE(master);

    // 마스터는 두가지 역할을 수행한다(스레드 구현).
    //  1. 클라이언트/슬레이브로부터 요청을 받아 큐에 적재.
    //RUN(master, RECEIVE);
    //  2. 적재된 요청을 선입선출하여, 전달 또는 처리.
    //RUN(master, HANDLE);
    RUN(master);

    // 대기(진행).
    while(1) sleep(1);

    return 0;
} 
