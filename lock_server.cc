// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

lock_server::lock_server():
	nacquire (0)
{
	assert(pthread_mutex_init(&locks_map_m, 0)==0);
	assert(pthread_cond_init(&locks_map_ok_read, 0)==0);
	assert(pthread_cond_init(&locks_map_ok_write, 0)==0);
	locks_map_wr = 0;
	locks_map_ar = 0;
	locks_map_ww = 0;
	locks_map_aw = 0;
}

lock_protocol::status 
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r) 
{	
	lock_protocol::status ret = lock_protocol::OK;

	if (is_registered(lid)) {

		do_lock(lid);
		r = 0;
	} else {
		do_register(lid);
		do_lock(lid);
		r = 0;
	}
	return ret;
}


lock_protocol::status 
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{
	
	lock_protocol::status ret = lock_protocol::OK;

	if (is_registered(lid)) {
	
		do_unlock(lid);
		r = 0;
	} else {
		printf("Warning: trying to release a lock that does not exist! \n");
		r = 1;
	}
	return ret;
	
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
	lock_protocol::status ret = lock_protocol::OK;
	printf("stat request from clt %d\n", clt);
	printf("clinet requisting status of lock: %llu\n", lid);
	r = nacquire;
	return ret;
}


void
lock_server::do_lock(lock_protocol::lockid_t lid)
{
	locks_map[lid]->do_lock();
}


void
lock_server::do_unlock(lock_protocol::lockid_t lid)
{
	locks_map[lid]->do_unlock();
}

bool
lock_server::is_registered(lock_protocol::lockid_t lid)
{

	bool ret;
	{
		ScopedLock locks_map_m_(&locks_map_m);
		while ( (locks_map_aw + locks_map_ww) > 0) {

			locks_map_wr++;
			pthread_cond_wait(&locks_map_ok_read, &locks_map_m);
			locks_map_wr--;
		}
		locks_map_ar++;
	}

	ret = locks_map.find(lid)!=locks_map.end()? true : false;

	{
		ScopedLock locks_map_m_(&locks_map_m);
		locks_map_ar--;
		if (locks_map_ar==0 && locks_map_ww >0) {
			pthread_cond_signal(&locks_map_ok_write);
		}
	}
	return ret;	
}

void
lock_server::do_register(lock_protocol::lockid_t lid)
{
	{
		ScopedLock locks_map_m_(&locks_map_m);
		while ( (locks_map_ar + locks_map_aw) > 0) {

			locks_map_ww++;
			pthread_cond_wait(&locks_map_ok_write, &locks_map_m);
			locks_map_ww--;
		}
		locks_map_aw++;
	}
	printf("creating new lock with id %llu \n",lid);
	lock_t *_lock = new lock_t(lid);
	locks_map.insert(std::pair<lock_protocol::lockid_t,lock_t*>(lid,_lock));

	{
		ScopedLock locks_map_m_(&locks_map_m);
		locks_map_aw--;
		if (locks_map_ww > 0) {
			
			pthread_cond_signal(&locks_map_ok_write);
		} else  {
			
			pthread_cond_broadcast(&locks_map_ok_read);
		}		
	}
		
}




