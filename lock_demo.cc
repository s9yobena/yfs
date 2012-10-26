//
// Lock demo
//

#include "lock_protocol.h"
#include "lock_client.h"
#include "rpc.h"
#include <arpa/inet.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>

std::string dst;
lock_client *lc;

void print_lock_state(int r, lock_protocol::lockid_t lock_id);

int
main(int argc, char *argv[])
{
	int r;

	if(argc != 2){
		fprintf(stderr, "Usage: %s [host:]port\n", argv[0]);
		exit(1);
	}

	dst = argv[1];
	lc = new lock_client(dst);
	// r = lc->stat(1);
	// printf ("stat returned %d\n", r);
	r = lc->acquire(05);
	print_lock_state(r,05);

	r = lc->acquire(05);
	print_lock_state(r,05);

	r = lc->acquire(06);
	print_lock_state(r,06);

	r = lc->acquire(07);
	print_lock_state(r,07);   

	r = lc->release(06);	
	// print_lock_state(r,06);
	
	r = lc->acquire(06);
	print_lock_state(r,06);

	r = lc->acquire(06);
	print_lock_state(r,06);

	
	
}

void print_lock_state(int r, lock_protocol::lockid_t lock_id) 
{
	if (r==0)
		printf ("lock %d is free and is now locked for us \n", lock_id);
	else
		printf ("lock %d is locked and we cannnot use it \n", lock_id);
}
