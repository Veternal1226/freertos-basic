#include <stdlib.h>
#include <stddef.h>
#include "clib.h"
#include <string.h>
#include "fio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mouse.h"
/*
 * mouse escape !
 */
void mVisit(int maze[][MSIZE]){
    #if 1
    fio_printf(1,"mouse_go!\r\n");
    /* maximun of mouse path is (SIZE - 1) ^ 2 */
    MSTACK mstack[(MSIZE-1)*(MSIZE-1)];
    MSTACK mouse;
    int mptr = 0; /* stack pointer */

    //mShowMap(maze);
    mouse.x = 1;
    mouse.y = 1;
    mptr++;
    mouse.x++;
    while(maze[MSIZE - 2][MSIZE - 2] != 3){
        if(maze[mouse.x][mouse.y-1] == 0) // 上 
        {
            mstack[mptr].x = mouse.x;
            mstack[mptr].y = mouse.y;
            maze[mouse.x][mouse.y] = 2;
            mouse.y = mouse.y - 1;
            mptr++;
        }
        else if(maze[mouse.x][mouse.y+1] == 0) // 下 
        {
            mstack[mptr].x = mouse.x;
            mstack[mptr].y = mouse.y;
            maze[mouse.x][mouse.y] = 2;
            mouse.y = mouse.y + 1;
            mptr++;
        }
        else if(maze[mouse.x+1][mouse.y] == 0) // 右 
        {
            mstack[mptr].x = mouse.x;
            mstack[mptr].y = mouse.y;
            maze[mouse.x][mouse.y] = 2;
            mouse.x = mouse.x + 1;
            mptr++;
        }
        else if(maze[mouse.x-1][mouse.y] == 0) // 左 
        {
            mstack[mptr].x = mouse.x;
            mstack[mptr].y = mouse.y;
            maze[mouse.x][mouse.y] = 2;
            mouse.x = mouse.x - 1;
            mptr++;
        }
        else
        {
            maze[mouse.x][mouse.y] = 2;
            mouse.x = mstack[mptr - 1].x;
            mouse.y = mstack[mptr - 1].y;
            mptr--;
        }
        maze[mouse.x][mouse.y] = 3;
        mShowMap(maze);
    }
    #endif

}

/*
 * show mouse maze map
 */
void mShowMap(int maze[][MSIZE]){
    int i = 0 ,j = 0;
    for(i = 0; i < MSIZE; i++){
        fio_printf(1,"\r\n");
        for(j = 0; j < MSIZE; j++){
            fio_printf(1,"%d ",maze[i][j]);
        }
    }
    fio_printf(1,"\r\n");
}
