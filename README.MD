This is an implementation of cooperative threads using ucontext.h functions.

background:
I'm making use of a few different data structures. I have a queue that stores all running threads and a linked list of all threads which have terminated.

mypthread_create:
creates a context for the function passed in the arguments and then pushes it into the queue.
on initial execution it also creates a context for the currently running 'main' thread and pushes that onto the queue.

mypthread_yield:
finds the first available context on the queue and swaps context between the currently running thread and the new thread context.
handles requeueing the contexts

mypthread_exit:
pushes the calling thread onto the terminated thread list and setscontext to the next available thread.
note that the exiting thread context is no longer pushed back onto the thread queue because it will not be running ever again.

mypthread_join:
this call runs in a loop which checks whether or not the joining thread is in the terminated thread list.
if the targeted thread is not terminated yet then the current thread yields.
this behavior continues until the condition that the target thread has terminated.
once the target thread has terminated, the return value of the thread is coppied into the retval double void pointer and the function returns.
