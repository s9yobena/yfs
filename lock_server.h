// this is the lock server
// the lock client has a similar interface

#ifndef lock_server_h
#define lock_server_h

#include <string>
#include "lock_protocol.h"
#include "lock_client.h"
#include "rpc.h"

class lock_t;

class lock_server {

protected:
	int nacquire;
	std::map<lock_protocol::lockid_t,lock_t*> locks_map;
	unsigned long locks_map_ar,locks_map_wr,locks_map_aw,locks_map_ww;
	pthread_mutex_t locks_map_m;
	pthread_cond_t locks_map_ok_read;
	pthread_cond_t locks_map_ok_write;
	void do_lock(lock_protocol::lockid_t lid);
	void do_unlock(lock_protocol::lockid_t lid);
	bool is_registered(lock_protocol::lockid_t lid);
	void do_register(lock_protocol::lockid_t lid);

public:
	lock_server();
	~lock_server() {};
	lock_protocol::status acquire(int clt, lock_protocol::lockid_t lid, int &);
	lock_protocol::status release(int clt, lock_protocol::lockid_t lid, int &);
	lock_protocol::status stat(int clt, lock_protocol::lockid_t lid, int &);

};

class lock_t {
       
	enum lock_state_t {FREE=0, LOCKED};

private:
	lock_protocol::lockid_t lid;
	enum lock_state_t lock_state; 
	// How many threads are waiting for the lock to become free.
	int lock_waiters;

	pthread_mutex_t lock_m;
	pthread_cond_t lock_free;

public:
	lock_t (lock_protocol::lockid_t _lid)
		:lid(_lid) 
	{
		lock_waiters = 0; // lock is free
		lock_state = FREE;
		assert(pthread_mutex_init(&lock_m, 0)==0);
		assert(pthread_cond_init(&lock_free, 0)==0);
	}

	~lock_t() {};

	void do_lock()
	{	
		ScopedLock lock_m_(&lock_m);
		while ( lock_state == LOCKED) {

			lock_waiters++;
			pthread_cond_wait(&lock_free, &lock_m);
			lock_waiters--;
		}
		lock_state = LOCKED;
	}
	
	void do_unlock() 
	{ 
		ScopedLock lock_m_(&lock_m);
			
		lock_state = FREE;
		if ( lock_waiters > 0) {
				
			pthread_cond_signal(&lock_free);
		}
	}

};
	

#endif 







