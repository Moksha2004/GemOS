#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include <unistd.h>


struct FreeChunk {
    size_t size;
    struct FreeChunk* next;
    struct FreeChunk* prev;
};
 struct FreeChunk*Free=NULL;//head
void* memalloc(unsigned long size) {
    size += 8;
    size = ((size + 7) / 8) * 8;
    size = (size < 24) ? 24 : size;
     if (Free == NULL) {
        size_t initial_size = 4 * 1024 * 1024;  // Adjust this as needed.
        void* initial_mem = mmap(NULL, initial_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (initial_mem == MAP_FAILED) {
            return NULL;
        }

        Free = (struct FreeChunk*)initial_mem;
        Free->size = initial_size;
        Free->prev = NULL;
        Free->next = NULL;
    }
    struct FreeChunk* current = Free;
    //printf("%d\n",size);
    struct FreeChunk* prev = NULL;  // Keep track of the previous chunk.

    while (current != NULL) {
   // printf("Moksha");
        if (current->size == size) {
            if (prev) {
                prev->next = current->next;
            } else {
                Free = current->next;
            }
            return (void*)current + 8;
        } else if (current->size > size) {
    //     printf("size=%lu\n",size);
    
            if (prev) {
                prev->next = (struct FreeChunk*)((void*)current + size);
            } else {
                Free = (struct FreeChunk*)((void*)current + size);
            }
            if(current->size-size>=24){
            Free->size = current->size - size;
            Free->prev = NULL;
            Free->next = current->next;}
            current->size=size;
            return (void*)current + 8;
        }
        prev = current;
        current = current->next;
    }

    size_t newsize = ((size + (4 * 1024 * 1024 - 1)) / (4 * 1024 * 1024)) * (4 * 1024 * 1024);
    void* new = mmap(NULL, newsize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (new == MAP_FAILED) {
        return NULL;
    }

    struct FreeChunk* m = (struct FreeChunk*)new;
    //unsigned long* m=(unsigned long*) new;
    m->size = size;
 //   printf("size=%lu\n",size);
    

     if (newsize > size) {
        struct FreeChunk* remaining = (struct FreeChunk*)((void*)m + size);
        remaining->size = newsize - size;
        remaining->prev = NULL;
        remaining->next = Free;
        Free = remaining;
    }

    return (void*)m +8;
}
int memfree(void*ptr){
if(!ptr) return -1;
void*left=NULL,*right=NULL;
void*start=ptr-8;
size_t size=*((unsigned long*)start);
struct FreeChunk*temp=Free;
while(temp){
size_t size_temp=*((unsigned long*)temp);
if(temp+size_temp+1==start) {left=temp;break;}
temp=temp->next;
}
temp=Free;
while(temp){
size_t size_temp=*((unsigned long*)temp);
if(temp==start+size+1) {right=temp;break;}
temp=temp->next;
}
if(!right&&!left){
struct FreeChunk*k=start;
k->next=Free;
k->prev=NULL;
Free=k;
}
else if(right&&!left){
struct FreeChunk*k=start,*right_free=right;
k->size=size+*((unsigned long*)right_free);
k->next=Free;
k->prev=NULL;
Free=k;
}
else if(!right&&left){
struct FreeChunk*k=start,*left_free=left;
k->size=size+*((unsigned long*)left_free);
k->next=Free;
k->prev=NULL;
Free=k;
}
else if(right&&left){
struct FreeChunk*k=start,*right_free=right,*left_free=left;
k->size=size+*((unsigned long*)right_free)+*((unsigned long*)left_free);
k->next=Free;
k->prev=NULL;
Free=k;
}
return 0;
}


