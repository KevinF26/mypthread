#include"mypthread.h"
#include<malloc.h>
int tidcount=1;
struct Qnode * head=NULL;
struct Qnode * tail=NULL;
int Qsize=0;
int mainthread=0;
ucontext_t * currthread=NULL;
struct Wnode * whead=NULL;

mypthread * dequeue();

mypthread * wfind(mypthread_t thread)
{
	struct Wnode * curr=whead;
	while(curr!=NULL)
	{
		if(curr->thread->tid==thread)
		{
			return curr->thread;
		}
		curr=curr->next;
	}
	return NULL;
}

void setwaiting(ucontext_t * currthread,mypthread_t thread)
{
	struct Qnode * curr=head;
	while(curr!=NULL)
	{
		if(curr->thread->context==currthread)
		{
			curr->thread->waitingon=thread;
			return;
		}
		curr=curr->next;
	}
}

void wake()
{
	struct Qnode * curr=head;
	while(curr!=NULL)
	{
		if(curr->thread->context==currthread)
		{
			curr->thread->waitingon=0;
			return;
		}
		curr=curr->next;
	}
}

void winsert(mypthread * thread)
{
	struct Wnode * n=malloc(sizeof(struct Wnode));
	n->thread=thread;
	n->next=NULL;
	if(whead==NULL)
	{
		whead=n;
		return;
	}
	n->next=whead;
	whead=n;		
}

void removet(ucontext_t * target, void * retval)
{
	mypthread * curr=NULL;
        do
        {
                curr=dequeue();
		if(curr->context==currthread)
		{
			curr->retval=retval;
			winsert(curr);
			return;
		}
                enqueue(curr);
        }while(1);

}

void enqueue(mypthread * thread)
{
	struct Qnode * n=malloc(sizeof(struct Qnode));
	n->thread=thread;
	n->next=NULL;
	if(head==NULL && tail==NULL)
	{
		head=n;
		tail=n;
	}	
	else
	{
		tail->next=n;
		tail=n;
	}
	Qsize++;
}

mypthread * dequeue()
{
	if(head==NULL)
	{
		printf("error. error. error.\n");
	}
	mypthread * thread=head->thread;
	struct Qnode * temp=head;
	if(head==tail)
	{
		head=NULL;
		tail=NULL;
	}
	else
	{
		head=head->next;
	}
	Qsize--;
	free(temp);
	return thread;
}

void init()
{
	mypthread * thread;
	char * stack=malloc(SIGSTKSZ);

        thread=malloc(sizeof(mypthread));
        thread->context=malloc(sizeof(ucontext_t));
        thread->tid=tidcount;
        tidcount++;
        thread->ready=1;
        getcontext(thread->context);
	currthread=thread->context;
	enqueue(thread);
	
}

int mypthread_create(mypthread_t *thread, const mypthread_attr_t *attr, void *(*start_routine) (void *), void *arg)
{
	if(mainthread==0)
	{
		init();
		mainthread++;
	}
	char * stack=malloc(SIGSTKSZ);

	mypthread * threadstr=malloc(sizeof(mypthread));
	threadstr->context=malloc(sizeof(ucontext_t));
	threadstr->tid=tidcount;
	tidcount++;
	threadstr->ready=1;
	getcontext(threadstr->context);
	threadstr->context->uc_link=NULL;
	threadstr->context->uc_stack.ss_sp=stack;
	threadstr->context->uc_stack.ss_size=SIGSTKSZ;
	threadstr->context->uc_stack.ss_flags=0;
	makecontext(threadstr->context, (void (*)(void))start_routine, 1, arg);
	enqueue(threadstr);
	*thread=threadstr->tid;
	return 0;
}

void mypthread_exit(void *retval)
{
	removet(currthread, retval);
	mypthread * thread=dequeue();
	enqueue(thread);
	currthread=thread->context;
	setcontext(thread->context);
}

int mypthread_yield(void)
{
	int i=0;
	if(Qsize<2)
	{
		return 0;
	}
	mypthread * swp1=NULL;
	do
	{
		swp1=dequeue();
		enqueue(swp1);
	}while(swp1->context==currthread);	
	ucontext_t * temp=currthread;
	currthread=swp1->context;
	swapcontext(temp,swp1->context);
}

int mypthread_join(mypthread_t thread, void **retval)
{
	mypthread * data=NULL;
	while(data==NULL)
	{
		data=wfind(thread);
		if(data!=NULL)
		{
			if(data->retval!=NULL)
			{
				*retval=data->retval;
			}
			wake();
			return 0;
		}
		setwaiting(currthread,thread);
		mypthread_yield();
	}
}

