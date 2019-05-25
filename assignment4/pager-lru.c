/*
 * File: pager-lru.c
 * Author:       Andy Sayler
 *               http://www.andysayler.com
 * Adopted From: Dr. Alva Couch
 *               http://www.cs.tufts.edu/~couch/
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2012/04/03
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#include "simulator.h"

/* Static vars */
static int initialized = 0;
static int tick = 1; // artificial time
static int timestamps[MAXPROCESSES][MAXPROCPAGES];

/*
 *  Returns:
 *   1 if unallocated page found
 *   0 if all pages are already allocated
 */
static int freePage(Pentry q[MAXPROCESSES], int proc) {
	int page;
	for(page = 0; page < MAXPROCPAGES; page++) {
        // unallocated so return true
		if(!q[proc].pages[page])
			return TRUE;
	}
	return FALSE;
}

static void lru_pageit(Pentry q[MAXPROCESSES]) {
	int proc;
    int page;
    int outPage;
    int pageTick;

	for(proc = 0; proc < MAXPROCESSES; proc++) {
        // skip itteration if process is inactive
		if(!q[proc].active){
            continue;
        }
        // set page according to the program counter
        page = q[proc].pc/PAGESIZE;
        // set timestamp for this page
		timestamps[proc][page] = tick;

		// skip itteration if page is already in memory
		if(q[proc].pages[page]){
            continue;
        }
        // continue if process is successfully swapped in
		if(pagein(proc, page)){
            continue;
        }
        // skip itteration if no free page
		if(!freePage(q, proc))
			continue;

        // set evict page to -1 in case no page is found
        outPage = -1;

    	for(page = 0; page < MAXPROCPAGES; page++) {
            // cant evict unallocated page
    		if(!q[proc].pages[page]){
                continue;
            }

    		if(timestamps[proc][page] < tick + 1) {
    			pageTick = timestamps[proc][page];
    			outPage = page;
                // this will be always be the most recently used page
    			if(pageTick <= 1)
    				break;
    		}
    	}
        // start swap-out so break loop
        if(pageout(proc, outPage)) {
            break;
	    }
	}
}

void pageit(Pentry q[MAXPROCESSES]) {

    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Local vars */
    int proctmp;
    int pagetmp;

    /* initialize static vars on first run */
    if(!initialized){
    	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
    	    for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
                timestamps[proctmp][pagetmp] = 0;
    	    }
    	}
        initialized = 1;
    }

    lru_pageit(q);

    // fprintf(stderr, "pager-lru not yet implemented. Exiting...\n");
    // exit(EXIT_FAILURE);

    /* advance time for next pageit iteration */
    tick++;
}
