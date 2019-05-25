/*
 * File: pager-predict.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains a predictive pageit
 *      implmentation.
 */

#include <stdio.h>
#include <stdlib.h>

#include "simulator.h"
#define THRESHOLD 0.74


void pageit(Pentry q[MAXPROCESSES]) {
    /* Local vars */
    int pc, page, proc;

static double pagePredict[15][15] = {
        {0.0,0.5,0.0,0.0,0.0,0.0,0.5,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
        {0.0,0.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
        {0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
        {0.25,0.0,0.0,0.0,0.5,0.0,0.0,0.0,0.0,0.0,0.25,0.0,0.0,0.0,0.0},
        {0.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
        {0.0,0.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
        {0.0,0.0,0.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0},
        {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0,0.0,0.0},
        {0.5,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.5,0.0,0.0,0.0,0.0,0.0},
        {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0,0.0},
        {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0},
        {0.25,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.75,0.0,0.0},
        {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,1.0,0.0},
        {0.125,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.75,0.0,0.0,0.0,0.0,0.125},
        {1.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}
    };

    for(proc=0;proc<MAXPROCESSES;proc++){
        int pageBit[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        pc = q[proc].pc;
        page = pc/PAGESIZE;
        pageBit[page] = 1;
        // if probability greater than 0 set bit to minimize thrashing
        for(int i = 0; i < 15; i++){
            if(pagePredict[page][i] > 0){
                pageBit[i] = 1;
            }
        }
        // If first jump is low probability, don't swap in its neighbors
        for(int i = 0; i < 15; i++){
            if(pagePredict[page][i] > THRESHOLD){
                for(int j = 0; j < 15; j++){
                    if(pagePredict[i][j] > 0){
                        pageBit[j] = 1;
                    }
                }
            }
        }
        // swap out pages that were not marked
        for(int i = 0; i < 15; i++){
            if(pageBit[i] == 0){
                pageout(proc,i);
            }
        }
        // swap in pages that are marked
        for(int i = 0; i < 15; i++){
            if(pageBit[i] == 1){
                pagein(proc, i);
            }
        }
     }
}
