//apt-get install gcc-multilib

#include "thread.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>


// thread metadata
struct thread {
	void *esp;
	struct thread *next;
	struct thread *prev;
	void *stack_add;
};

struct thread *ready_list = NULL;     // ready list
struct thread *cur_thread = NULL;     // current thread
struct thread *thread_exited = NULL;
struct lock *_lock=NULL;
// defined in context.s
void context_switch(struct thread *prev, struct thread *next);

// insert the input thread to the end of the ready list.
static void push_back(struct thread *t)
{
	t -> next == NULL;
	t -> prev == NULL;
	if ( ready_list == NULL )
		ready_list = t;
	else
	{
		struct thread* temp = ready_list;

		while ((temp -> next) != NULL)
		{
			temp = temp -> next ;
		}
		
		t -> prev = temp;
		temp -> next = t;
	}
}

// remove the first thread from the ready list and return to caller.
static struct thread *pop_front()
{
		struct thread* t = ready_list;
		ready_list = ready_list -> next;

		if( ready_list != NULL)
		ready_list -> prev = NULL;

		t->prev = t->next = NULL;
		return t;
}

// the next thread to schedule is the first thread in the ready list.
// obtain the next thread from the ready list and call context_switch.
static void schedule()
{
	struct thread *prev = cur_thread;
	struct thread *next =  pop_front(ready_list);
	cur_thread = next;
	context_switch(prev, next);
}

// push the cur_thread to the end of the ready list and call schedule
// if cur_thread is null, allocate struct thread for cur_thread
static void schedule1()
{
	if (cur_thread == NULL)
	{
		cur_thread = malloc(sizeof(struct thread));
		cur_thread -> next = NULL;
		cur_thread -> prev = NULL;
	}
	push_back(cur_thread);
	schedule();
}

// allocate stack and struct thread for new thread
// save the callee-saved registers and parameters on the stack
// set the return address to the target thread
// save the stack pointer in struct thread
// push the current thread to the end of the ready list
void create_thread(func_t func, void *param)
{
	struct thread *t = malloc(sizeof(struct thread));
	unsigned *stack = malloc(4096);
	t -> stack_add = stack;
	stack += 1024;
	t -> next = NULL;
	t -> prev = NULL;
	stack --;
	*stack = (unsigned)param;
	stack -= 2;
	*stack = (unsigned)func;
	stack -= 4;
	t -> esp = stack;
	push_back(t);
}

// call schedule1
void thread_yield()
{
	schedule1();
}

// call schedule
void thread_exit()
{
	if ( thread_exited != NULL )
	{
		free(thread_exited -> stack_add);
		free(thread_exited);
		thread_exited = NULL;
	}
	thread_exited = cur_thread;
	schedule();
}

// call schedule1 until ready_list is null
void wait_for_all()
{
	while ( ready_list != NULL )
	{
		thread_yield();
		if(ready_list==NULL && _lock!=NULL)
		{
			wakeup(_lock);
		}
	}
}

void sleep(struct lock *lock)
{

	if(_lock==NULL)
		_lock=lock;
	struct thread *temp = lock -> wait_list;
	if(temp==NULL)
		lock->wait_list = cur_thread;
	else
	{
		while ((temp -> next) != NULL)
		{
			temp = temp -> next ;
		}
		
		cur_thread -> prev = temp;
		temp -> next = cur_thread;
		cur_thread = NULL;
	}
	schedule();

}

void wakeup(struct lock *lock)
{
		if(_lock==NULL)
		_lock=lock;
		if(lock->wait_list==NULL)
			return;
		struct thread* t = lock -> wait_list;
		lock -> wait_list = ((struct thread*)(lock -> wait_list)) -> next;

		if( lock -> wait_list != NULL)
		((struct thread*)(lock -> wait_list)) -> prev = NULL;

		t->prev = t->next = NULL;
		push_back(t);
}
