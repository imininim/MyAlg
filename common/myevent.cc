#include <vector>
#include <iostream>


#include <sys/types.h> 
#include <sys/socket.h> 
#include <stdio.h> 
#include <netinet/in.h> 
#include <sys/time.h> 
#include <sys/ioctl.h> 
#include <unistd.h> 
#include <stdlib.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

using namespace std;

namespace myevent {
#define EV_READ 1
#define EV_WRITE 2
#define EV_PERSIST 4

#define EV_WAIT 1
#define EV_ACTIVE 2


class base_event;
class eventop;
class selectop;
class event;
void event_del(event *ev);

class event {
public:
	event():fd(-1), ev_event(0), ev_flag(0), ev_base(nullptr) {}
//private:
	base_event *ev_base;
	int fd;
	int ev_event;	//事件类型 EV_READ|EV_WRITE
	int ev_flag;    //当前状态 EV_WAIT|EV_ACTIVE
	virtual void callback()=0;
};

class base_event {
public:
	base_event():event_count(0), event_count_active(0) {}
//private:
	eventop *op;
	int event_count;
	int event_count_active;
	std::vector<event*> active_queue;
	std::vector<event*> event_queue;
};

class eventop {
public:
	virtual void* init(base_event *base)=0;   //初始化
	virtual int add(event *ev)=0;      //新增事件
	virtual int del(event *ev)=0;      //删除事件
	virtual int recalc()=0;
	virtual int dispatch(base_event *base)=0;
	virtual int dealloc()=0;  //析构
};

class epollop:public eventop {
public:
	virtual void* init(base_event *base) {
		epfd=epoll_create(30);
		return nullptr;
	}
	virtual int add(event *ev) {
		ev_tmp.events = 0;
		int op = EPOLL_CTL_ADD;
		if (ev->ev_event & EV_READ) {
			ev_tmp.events |= EPOLLIN|EPOLLET;
			cerr <<"set read" << ev->fd << endl;
			if (fd_read_events[ev->fd] != NULL) {
				op = EPOLL_CTL_MOD;
			}
			fd_read_events[ev->fd] = ev;
		}
		if (ev->ev_event & EV_WRITE) {
			ev_tmp.events |= EPOLLOUT|EPOLLET;
			if (fd_write_events[ev->fd] != NULL) {
				op = EPOLL_CTL_MOD;
			}
			fd_write_events[ev->fd] = ev;
		}

		if (ev_tmp.events != 0) {
			ev_tmp.data.fd = ev->fd;
			epoll_ctl(epfd, op, ev->fd, &ev_tmp);
		}
		return 0;
	}
	virtual int del(event *ev) {
		ev_tmp.events = 0;
		if (ev->ev_event & EV_READ) {
			cerr <<"del read" << endl;
			ev_tmp.events |= EPOLLIN;
			fd_read_events[ev->fd] = NULL;
		}
		if (ev->ev_event & EV_WRITE) {
			ev_tmp.events |= EPOLLOUT;
			fd_write_events[ev->fd] = NULL;
		}

		if (ev_tmp.events != 0) {
			ev_tmp.data.fd = ev->fd;
			epoll_ctl(epfd, EPOLL_CTL_DEL, ev->fd, &ev_tmp);
		}

		return 0;
	}
	virtual int recalc() {
		return 0;
	}
	virtual int dispatch(base_event *base) {
		fd_set cpy_reads = reads;
		fd_set cpy_writes = writes;
		cerr << "epollwait" <<endl;
		int fd_num = epoll_wait(epfd, events, 30, 100);
		if (fd_num < 0) {
			cerr << "error" << endl;
			return -1;
		}
		if (fd_num == 0) {
			return 0;
		}
		cerr << "get data" << endl;
		for (int i = 0; i < fd_num; ++i) {
			int res = 0;
			if (events[i].events & EPOLLIN) {
				res |= EV_READ;
			}
			int fd = events[i].data.fd;
			if (events[i].events & EPOLLIN) {
				event *ev = fd_read_events[fd];
				if (ev->ev_event & EV_PERSIST) {
					event_del(ev);
				}
				cerr << "read "<< i << endl;
				base->active_queue.push_back(ev);
				base->event_count_active ++;
			}
			if (events[i].events & EPOLLOUT) {
				cerr << "write "<< i << endl;
				event *ev = fd_write_events[fd];
				if (ev->ev_event & EV_PERSIST) {
					event_del(ev);
				}
				base->active_queue.push_back(ev);
				base->event_count_active ++;
			}
		}
		return 0;
	}
	virtual int dealloc() {
		return 0;
	}

	struct epoll_event events[30], ev_tmp;
	int epfd;
	fd_set reads;
	fd_set writes;
	event *fd_read_events[1024];
	event *fd_write_events[1024];
};


class selectop:public eventop {
public:
	virtual void* init(base_event *base) {
		FD_ZERO(&reads);
		FD_ZERO(&writes);
		return nullptr;
	}
	virtual int add(event *ev) {
		if (ev->ev_event & EV_READ) {
			cerr <<"set read" << ev->fd << endl;
			FD_SET(ev->fd, &reads);
			if (fd_max < ev->fd) {
				fd_max = ev->fd;
			}
			fd_read_events[ev->fd] = ev;
		}
		if (ev->ev_event & EV_WRITE) {
			FD_SET(ev->fd, &writes);
			if (fd_max < ev->fd) {
				fd_max = ev->fd;
			}
			cerr << "set write" << ev->fd << endl;
			fd_write_events[ev->fd] = ev;
		}
		return 0;
	}
	virtual int del(event *ev) {
		if (ev->ev_event & EV_READ) {
			cerr <<"del read" << endl;
			FD_CLR(ev->fd, &reads);
			fd_read_events[ev->fd] = NULL;
		}
		if (ev->ev_event & EV_WRITE) {
			FD_CLR(ev->fd, &writes);
			fd_write_events[ev->fd] = NULL;
		}
		return 0;
	}
	virtual int recalc() {
		return 0;
	}
	virtual int dispatch(base_event *base) {
		struct timeval timeout;
		timeout.tv_sec = 10;
		timeout.tv_usec = 0;
		fd_set cpy_reads = reads;
		fd_set cpy_writes = writes;
		cerr << "select" <<endl;
		int fd_num = select(fd_max+1, &cpy_reads, &cpy_writes, 0, &timeout);
		if (fd_num < 0) {
			cerr << "error" << endl;
			return -1;
		}
		if (fd_num == 0) {
			return 0;
		}
		cerr << "get data" << endl;
		for (int i = 0; i < fd_max+1; ++i) {
			int res = 0;
			if (FD_ISSET(i, &cpy_writes)) {
				res |= EV_WRITE;
			}
			if (FD_ISSET(i, &cpy_reads)) {
				res |= EV_READ;
			}
			if (res & EV_READ) {
				event *ev = fd_read_events[i];
				if (ev->ev_event & EV_PERSIST) {
					event_del(ev);
				}
				cerr << "read "<< i << endl;
				base->active_queue.push_back(ev);
				base->event_count_active ++;
			}
			if (res & EV_WRITE) {
				cerr << "write "<< i << endl;
				event *ev = fd_write_events[i];
				if (ev->ev_event & EV_PERSIST) {
					event_del(ev);
				}
				base->active_queue.push_back(ev);
				base->event_count_active ++;
			}
		}
		return 0;
	}
	virtual int dealloc() {
		return 0;
	}
public:
	fd_set reads;
	fd_set writes;
	int fd_max;
	event *fd_read_events[1024];
	event *fd_write_events[1024];
};


base_event* base_event_init() {
	base_event *base = new base_event();
	base->op = new selectop();
	//base->op = new epollop();
	base->op->init(base);
	return base;
}

int base_event_loop(base_event* base, int flag) {
	int done = 0;
	eventop *op = base->op;
	while (!done) {
		op->recalc();
		if (base->event_count <= 0) {
			std::cerr << "no event" << std::endl;
			return -1;
		}
		op->dispatch(base);
		for (int i = 0; i < base->event_count_active; ++i) {
			base->active_queue[i]->callback();
		}
		base->active_queue.clear();
		base->event_count_active = 0;
	}
	return 0;
}

void event_add(event *ev) {
	if (ev->ev_event & (EV_READ|EV_WRITE)) {
		ev->ev_base->event_queue.push_back(ev);
		ev->ev_base->event_count ++;
		ev->ev_base->op->add(ev);
	}
}

void event_del(event *ev) {
	if (ev->ev_flag & EV_WAIT) {
		for (int i = 0; i < ev->ev_base->event_queue.size(); ++i) {
			if (ev == ev->ev_base->event_queue[i]) {
				ev->ev_base->event_queue.erase(ev->ev_base->event_queue.begin()+i);
				ev->ev_base->event_count --;
				break;
			}
		}
	}
	if (ev->ev_flag & EV_ACTIVE) {
		for (int i = 0; i < ev->ev_base->active_queue.size(); ++i) {
			if (ev == ev->ev_base->active_queue[i]) {
				ev->ev_base->active_queue.erase(ev->ev_base->active_queue.begin()+i);
				ev->ev_base->event_count_active --;
				break;
			}
		}
	}
	ev->ev_base->op->del(ev);
}

}

class read_event:public myevent::event {
	virtual void callback() {
		char buf[1024];
		int str_len = read(fd, buf, 1024);
		buf[str_len] = 0;
		cerr << "fd: "<< fd << "len: " << str_len << " get: " << buf << endl;
		std::cerr << "read call back" << std::endl;
		sleep(1);
	}
};


class accept_event:public myevent::event {
	virtual void callback() {
		int new_fd = accept(fd, NULL, NULL); 
		read_event *ev = new read_event();
		ev->ev_event = EV_READ|EV_PERSIST;
		ev->ev_base = ev_base;
		ev->fd = new_fd;
		myevent::event_add(ev);
		std::cerr << "accept call back" << std::endl;
		sleep(1);
	}
};

int main() {
	int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);//建立服务器端socket 
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET; 
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(8888); 
	int server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
	listen(server_sockfd, 5);
	auto base = myevent::base_event_init();
	myevent::event *ev = new accept_event();
	ev->ev_event = EV_READ;
	ev->ev_base = base;
	ev->fd = server_sockfd;
	cerr << "ser fd" << server_sockfd << endl;
	myevent::event_add(ev);
	myevent::base_event_loop(base, 0);
	return 0;
}
