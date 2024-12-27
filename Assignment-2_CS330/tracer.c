#include<context.h>
#include<memory.h>
#include<lib.h>
#include<entry.h>
#include<file.h>
#include<tracer.h>


///////////////////////////////////////////////////////////////////////////
//// 		Start of Trace buffer functionality 		      /////
///////////////////////////////////////////////////////////////////////////

int is_valid_mem_range(unsigned long buff, u32 count, int access_bit) 
{  struct exec_context *current = get_current_ctx(); // Get the current process's context
   int valid_range=0;
   if (buff >= current->mms[MM_SEG_CODE].start && buff + count <= current->mms[MM_SEG_CODE].next_free - 1) {
        if (access_bit == 0)  {
            valid_range = 1; // Valid for read
        } 
    } else if (buff >= current->mms[MM_SEG_RODATA].start && buff + count <= current->mms[MM_SEG_RODATA].next_free - 1) {
        if (access_bit == 1) {
            valid_range = 1; // Valid for read
        }
    } else if (buff >= current->mms[MM_SEG_DATA].start && buff + count <= current->mms[MM_SEG_DATA].next_free - 1) {
        valid_range=1;
        }
     else if (buff >= current->mms[MM_SEG_STACK].start && buff + count <= current->mms[MM_SEG_STACK].end - 1) {
        
            valid_range = 1; 
        
    }
    // Check the virtual memory areas (vm_area)
    struct vm_area *vma = current->vm_area;
    while (vma != NULL) {
        if (buff >= vma->vm_start && buff + count <= vma->vm_end-1) {
            // Check the access bit
            if ((vma->access_flags & access_bit) == access_bit) {
                valid_range= 1; // Valid memory range
            }
        }
        vma = vma->vm_next;
    }

    return valid_range; // Invalid memory range
	
}



long trace_buffer_close(struct file *filep)
{ if (filep == NULL) {
        return -EINVAL;
    }

    // Ensure the file type is TRACE_BUFFER
    if (filep->type != TRACE_BUFFER) {
        return -EINVAL;
    }

    // Deallocate memory used for fileops
    if (filep->fops != NULL) {
        os_free(filep->fops, sizeof(struct fileops));
    }

    // Deallocate memory used for the trace buffer info
    if (filep->trace_buffer != NULL) {
        os_page_free(USER_REG, filep->trace_buffer); // Free the 4KB page
    }

    // Deallocate memory used for the file
    os_free(filep, sizeof(struct file));

    // Set the file descriptor entry to NULL to mark it as closed
    // This step is not strictly necessary, but it's good practice
    // to prevent using a closed file descriptor.
    filep = NULL;

    return 0;
	
}



int trace_buffer_read(struct file *filep, char *buff, u32 count)
{if (filep == NULL || buff == NULL || count <= 0) {
        return 0;
    }

    // Ensure the file type is TRACE_BUFFER
    if (filep->type != TRACE_BUFFER) {
        return -EINVAL;
    }

    // Get the trace buffer from the file object
    struct trace_buffer_info *trace_buffer = filep->trace_buffer;

    if (trace_buffer == NULL) {
        return -EINVAL;
    }

    // Calculate the number of bytes available for reading from the trace buffer
    u32 bytes_available = trace_buffer->size;

    if (bytes_available == 0) {
        // The trace buffer is empty
        return 0;
    }

    // Calculate the read offset (taking into account circular buffer)
    u32 read_offset = trace_buffer->read ;
    // Calculate the number of bytes to read (minimum of available bytes, count, and circular buffer size)
    u32 bytes_to_read = (bytes_available < count) ? bytes_available : count;
    //u32 bytes_to_end = TRACE_BUFFER_MAX_SIZE - read_offset;
    //bytes_to_read = (bytes_to_read < bytes_to_end) ? bytes_to_read : bytes_to_end;
   // printk("%d,read\n",bytes_to_read);
    // Copy data from the trace buffer to the user buffer
    if (!is_valid_mem_range((unsigned long)buff, bytes_to_read, 0)) {
        return -EBADMEM;
    }
    
    char *src = (trace_buffer->data+read_offset);
    char *dest = buff;
    int f=0;
    for (u32 i = 0; i < bytes_to_read; i++) {
     if(i+read_offset<TRACE_BUFFER_MAX_SIZE) dest[i] = src[i];
      else{ dest[i]=*(trace_buffer->data+i+read_offset-TRACE_BUFFER_MAX_SIZE);

    }}

    // Update the file offset (taking into account circular buffer)
    trace_buffer->read += bytes_to_read;
    trace_buffer->read %= TRACE_BUFFER_MAX_SIZE;
    trace_buffer->size -= bytes_to_read;
    // printk("trace buffer size after read %d , offset %d\n",trace_buffer->size,trace_buffer->read);
    return bytes_to_read;
}


int trace_buffer_write(struct file *filep, char *buff, u32 count)
{if (filep == NULL || buff == NULL || count <= 0) {
        return 0;
    }

    // Ensure the file type is TRACE_BUFFER
    if (filep->type != TRACE_BUFFER) {
        return -EINVAL;
    }

    // Get the trace buffer from the file object
    struct trace_buffer_info *trace_buffer = filep->trace_buffer;

    if (trace_buffer == NULL) {
        return -EINVAL;
    }

    // Calculate the space available in the trace buffer
    u32 space_available = TRACE_BUFFER_MAX_SIZE - trace_buffer->size;

    if (space_available == 0) {
        // The trace buffer is full
        return 0;
    }

    // Calculate the write offset (taking into account circular buffer)
    u32 write_offset = (trace_buffer->write) ;

    // Calculate the number of bytes to write (minimum of space available, count, and circular buffer size)
    u32 bytes_to_write = (space_available < count) ? space_available : count;
    //u32 bytes_to_end = TRACE_BUFFER_MAX_SIZE - write_offset;
    //bytes_to_write = (bytes_to_write < bytes_to_end) ? bytes_to_write : bytes_to_end;

    // Copy data from user buffer to the trace buffer
    if (!is_valid_mem_range((unsigned long)buff, bytes_to_write, 1)) {
        return -EBADMEM;
    }

    char *src = buff;
    char *dest = trace_buffer->data + write_offset;
    for (u32 i = 0; i < bytes_to_write; i++) {
        if(i+write_offset<TRACE_BUFFER_MAX_SIZE) dest[i] = src[i];
        else dest[i-TRACE_BUFFER_MAX_SIZE]=src[i];
    }

  
     trace_buffer->size += bytes_to_write;
    
     trace_buffer->write += bytes_to_write;
    trace_buffer->write %= TRACE_BUFFER_MAX_SIZE;
   // printk("trace buffer size after write %d , offset %d\n",trace_buffer->size,trace_buffer->write);
    return bytes_to_write;
    	
}

int sys_create_trace_buffer(struct exec_context *current, int mode)
{  // Ensure mode is valid
    if (mode != O_READ && mode != O_WRITE && mode != O_RDWR) {
        return -EINVAL;
    }

    // Find a free file descriptor
    int fd;
    for (fd = 0; fd < MAX_OPEN_FILES; fd++) {
        if (current->files[fd] == NULL) {
            break;
        }
    }

    if (fd >= MAX_OPEN_FILES) {
        // No free file descriptor available
        return -EINVAL;
    }

    // Allocate a file object
    struct file *filep = os_alloc(sizeof(struct file));
    if (filep == NULL) {
        return -ENOMEM;
    }
  
    // Initialize file object fields
    filep->type = TRACE_BUFFER;
    filep->mode = mode;
    filep->offp = 0;
    filep->ref_count = 1;
    filep->inode = NULL;
    

    // Allocate a 4KB page for the trace buffer
    struct trace_buffer_info *trace_buffer = (struct trace_buffer_info *)os_page_alloc(USER_REG);
    if (trace_buffer == NULL) {
        os_free(filep, sizeof(struct file));
        return -ENOMEM;
    }
    trace_buffer->read=0;
    trace_buffer->write=0;
    trace_buffer->size=0;
    trace_buffer -> data= (char*) os_page_alloc(USER_REG);
    // Initialize trace buffer fields
    // (Add your implementation-specific details here)

    // Allocate file pointers object
    struct fileops *fops = os_alloc(sizeof(struct fileops));
    if (fops == NULL) {
        os_free(filep, sizeof(struct file));
        os_page_free(USER_REG, trace_buffer); // Free the trace buffer page
        return -ENOMEM;
    }

    // Initialize file pointers functions (to be implemented)
    fops->read = trace_buffer_read;
    fops->write = trace_buffer_write;

    // Update the file object with the file pointers
    filep->fops = fops;

    // Update the file object with the trace buffer
    filep->trace_buffer = trace_buffer;

    // Assign the file object to the file descriptor
    current->files[fd] = filep;
   // printk("buffer created\n");
    return fd; // Return the allocated file descriptor

}

///////////////////////////////////////////////////////////////////////////
//// 		Start of strace functionality 		      	      /////
///////////////////////////////////////////////////////////////////////////

int os_trace_buffer_read(struct file *filep, char *buff, u32 count)
{if (filep == NULL || buff == NULL || count <= 0) {
//printk("1 read\n");
        return -EINVAL;
    }
/*if (filep->type != TRACE_BUFFER) {
printk("2 read\n");
        return -EINVAL;
    }*/
    

    // Get the trace buffer from the file object
    struct trace_buffer_info *trace_buffer = filep->trace_buffer;

    if (trace_buffer == NULL) {
    //printk("3 read\n");
        return -EINVAL;
    }

    // Calculate the number of bytes available for reading from the trace buffer
    u32 bytes_available = trace_buffer->size;

    if (bytes_available == 0) {
    //printk("4 read\n");
        // The trace buffer is empty
        return 0;
    }

    // Calculate the read offset (taking into account circular buffer)
    u32 read_offset = trace_buffer->read ;
    // Calculate the number of bytes to read (minimum of available bytes, count, and circular buffer size)
    u32 bytes_to_read = (bytes_available < count) ? bytes_available : count;
    //u32 bytes_to_end = TRACE_BUFFER_MAX_SIZE - read_offset;
    //bytes_to_read = (bytes_to_read < bytes_to_end) ? bytes_to_read : bytes_to_end;
  //  printk("%d,read\n",bytes_to_read);
   
    char *src = (trace_buffer->data+read_offset);
    char *dest = buff;
    int f=0;
    for (u32 i = 0; i < bytes_to_read; i++) {
     if(i+read_offset<TRACE_BUFFER_MAX_SIZE) dest[i] = src[i];
      else{ dest[i]=*(trace_buffer->data+i+read_offset-TRACE_BUFFER_MAX_SIZE);
        
      }
        
    }

    // Update the file offset (taking into account circular buffer)
    trace_buffer->read += bytes_to_read;
    trace_buffer->read %= TRACE_BUFFER_MAX_SIZE;
    trace_buffer->size -= bytes_to_read;
    // printk("trace buffer size after read %d , offset %d\n",trace_buffer->size,trace_buffer->read);
     //printk("5 read\n");
    return bytes_to_read;
}





int os_trace_buffer_write(struct file *filep, char *buff, u32 count)
{if (filep == NULL || buff == NULL || count <= 0) {
        return -EINVAL;
    }

    // Ensure the file type is TRACE_BUFFER
    if (filep->type != TRACE_BUFFER) {
        return -EINVAL;
    }

    // Get the trace buffer from the file object
    struct trace_buffer_info *trace_buffer = filep->trace_buffer;

    if (trace_buffer == NULL) {
        return -EINVAL;
    }

    // Calculate the space available in the trace buffer
    u32 space_available = TRACE_BUFFER_MAX_SIZE - trace_buffer->size;

    if (space_available == 0) {
        // The trace buffer is full
        return 0;
    }
    // printk("trace buffer size before write %d , offset %d\n",trace_buffer->size,trace_buffer->write);

    // Calculate the write offset (taking into account circular buffer)
    u32 write_offset = (trace_buffer->write) ;

    // Calculate the number of bytes to write (minimum of space available, count, and circular buffer size)
    u32 bytes_to_write = (space_available < count) ? space_available : count;
    //u32 bytes_to_end = TRACE_BUFFER_MAX_SIZE - write_offset;
    //bytes_to_write = (bytes_to_write < bytes_to_end) ? bytes_to_write : bytes_to_end;

    

    char *src = buff;
    char *dest = trace_buffer->data + write_offset;
    for (u32 i = 0; i < bytes_to_write; i++) {
        if(i+write_offset<TRACE_BUFFER_MAX_SIZE) dest[i] = src[i];
        else dest[i-TRACE_BUFFER_MAX_SIZE]=src[i];
        //printk("dest=%d\n",dest[i]);
    }
    u64 syscall_num=*((u64*)((dest-8)));
   //printk("checking written values %d\n",syscall_num);
     trace_buffer->size += bytes_to_write;
    
     trace_buffer->write += bytes_to_write;
    trace_buffer->write %= TRACE_BUFFER_MAX_SIZE;
   // printk("trace buffer size after write %d , offset %d\n",trace_buffer->size,trace_buffer->write);
    return bytes_to_write;
    	
}
//./run.sh /home/osws/Assignment2/gemOS/src/gemOS.kernel
//This is to just capture the information about the system call— into the trace buffer.
int perform_tracing(u64 syscall_num, u64 param1, u64 param2, u64 param3, u64 param4)
{ //printk("perform entered\n"); 
//printk("num =%d\n",syscall_num); 
if(syscall_num==1||syscall_num==37||syscall_num==38) return 0;

struct exec_context *current = get_current_ctx();
   //printk("mmm\nmoksha");
    if (current==0) {
   // printk("b");
        return 0;
    }
  // printk("b");
    // Get the current context's trace buffer file pointer
    struct file *fd ;
   if(current->st_md_base ==0)return 0;
    fd = current->files[current->st_md_base->strace_fd];
    if(current->st_md_base->is_traced!=1) return 0;
  // printk("c");
    // Ensure that the trace file is not NULL and its type is TRACE_BUFFER
    if (fd == 0 || fd->type != TRACE_BUFFER) {
        return 0;
    }
    //printk("d");
    struct strace_head*head=current->st_md_base;
struct strace_info sys;
//sys.syscall_num=syscall_num;
//sys.next=NULL;
if(current->st_md_base->tracing_mode==FILTERED_TRACING){
//printk("i have entered filtred tracing\n");
//printk("num =%d\n",syscall_num); 
struct strace_info*cu=head->next;
//printk("strace head number %d\n",cu->syscall_num);
int found=0;
while(cu){
if(cu->syscall_num==syscall_num){//printk("syscll_num=%d %d %d %d\n",syscall_num,param1,param2,param3);
found=1;break;}
cu=cu->next;
}
 if(found){        // printk("%d",syscall_num);
// printk("entered found\n");   
 if (syscall_num == SYSCALL_SLEEP || syscall_num == SYSCALL_DUMP_PTT || syscall_num == SYSCALL_PMAP || syscall_num == SYSCALL_DUP || syscall_num == SYSCALL_CLOSE){
			char buff[2* sizeof(u64)];
			((u64 *)buff)[0]=syscall_num;
			((u64 *)buff)[1]=param1;
			
			
			os_trace_buffer_write(fd, buff, 2*sizeof(u64));
			//printk("16");
		}
		else if (syscall_num == SYSCALL_CONFIGURE || syscall_num == SYSCALL_SIGNAL || syscall_num == SYSCALL_EXPAND || syscall_num == SYSCALL_CLONE || syscall_num == SYSCALL_MUNMAP || syscall_num == SYSCALL_OPEN || syscall_num == SYSCALL_DUP2 || syscall_num == SYSCALL_STRACE ){
		char buff[3* sizeof(u64)];
			((u64 *)buff)[0]=syscall_num;
			((u64 *)buff)[1]=param1;
			((u64 *)buff)[2]=param2;
			
			os_trace_buffer_write(fd, buff, 3*sizeof(u64));
			//printk("24\n");
		}
		else if (syscall_num == SYSCALL_MPROTECT || syscall_num == SYSCALL_WRITE || syscall_num == SYSCALL_READ || syscall_num == SYSCALL_LSEEK || syscall_num == SYSCALL_READ_STRACE || syscall_num == SYSCALL_READ_FTRACE ){
		char buff[4 * sizeof(u64)];
			((u64 *)buff)[0]=syscall_num;
			((u64 *)buff)[1]=param1;
			((u64 *)buff)[2]=param2;
			((u64 *)buff)[3]=param3;
			os_trace_buffer_write(fd, buff, 4*sizeof(u64));
		}
		else if (syscall_num == SYSCALL_MMAP || syscall_num == SYSCALL_FTRACE){
		char buff[5 * sizeof(u64)];
			((u64 *)buff)[0]=syscall_num;
			((u64 *)buff)[1]=param1;
			((u64 *)buff)[2]=param2;
			((u64 *)buff)[3]=param3;
			((u64 *)buff)[4]=param3;
			os_trace_buffer_write(fd, buff, 5*sizeof(u64));
		       // printk("40\n");
		}
		else{
		char buff[1 * sizeof(u64)];
			((u64 *)buff)[0]=syscall_num;
			//memcpy(buff, &syscall_num, sizeof(u64));
			os_trace_buffer_write(fd, buff, 1*sizeof(u64));
				  }
		
	}
	
	 }	
	else{
	//printk("I cam here\n");
	if (syscall_num == SYSCALL_SLEEP || syscall_num == SYSCALL_DUMP_PTT || syscall_num == SYSCALL_PMAP || syscall_num == SYSCALL_DUP || syscall_num == SYSCALL_CLOSE){
			char buff[2* sizeof(u64)];
			((u64 *)buff)[0]=syscall_num;
			((u64 *)buff)[1]=param1;
			
			
			os_trace_buffer_write(fd, buff, 2*sizeof(u64));
			//printk("16");
		}
		else if (syscall_num == SYSCALL_CONFIGURE || syscall_num == SYSCALL_SIGNAL || syscall_num == SYSCALL_EXPAND || syscall_num == SYSCALL_CLONE || syscall_num == SYSCALL_MUNMAP || syscall_num == SYSCALL_OPEN || syscall_num == SYSCALL_DUP2 || syscall_num == SYSCALL_STRACE ){
		char buff[3* sizeof(u64)];
			((u64 *)buff)[0]=syscall_num;
			((u64 *)buff)[1]=param1;
			((u64 *)buff)[2]=param2;
			
			os_trace_buffer_write(fd, buff, 3*sizeof(u64));
			//printk("24\n");
		}
		else if (syscall_num == SYSCALL_MPROTECT || syscall_num == SYSCALL_WRITE || syscall_num == SYSCALL_READ || syscall_num == SYSCALL_LSEEK || syscall_num == SYSCALL_READ_STRACE || syscall_num == SYSCALL_READ_FTRACE ){
		char buff[4 * sizeof(u64)];
			((u64 *)buff)[0]=syscall_num;
			((u64 *)buff)[1]=param1;
			((u64 *)buff)[2]=param2;
			((u64 *)buff)[3]=param3;
			os_trace_buffer_write(fd, buff, 4*sizeof(u64));
		}
		else if (syscall_num == SYSCALL_MMAP || syscall_num == SYSCALL_FTRACE){
		char buff[5 * sizeof(u64)];
			((u64 *)buff)[0]=syscall_num;
			((u64 *)buff)[1]=param1;
			((u64 *)buff)[2]=param2;
			((u64 *)buff)[3]=param3;
			((u64 *)buff)[4]=param3;
			os_trace_buffer_write(fd, buff, 5*sizeof(u64));
		       // printk("40\n");
		}
		else{
		char buff[1 * sizeof(u64)];
			((u64 *)buff)[0]=syscall_num;
			//memcpy(buff, &syscall_num, sizeof(u64));
			os_trace_buffer_write(fd, buff, 1*sizeof(u64));
				  }
	
	}
	
	//printk("perform exited\n"); 
	
return 0;
}


int sys_strace(struct exec_context *current, int syscall_num, int action)
{ //printk("I'm in strace\n");   
 if(current==NULL){
     return -EINVAL;
      }
      //initialize the strace_head
   if(action !=REMOVE_STRACE && action!=ADD_STRACE) {
   return -EINVAL;
   }
   
      if(!current->st_md_base){
       current->st_md_base = (struct strace_head *)os_alloc(sizeof(struct strace_head));
       current->st_md_base->count = 0;
         current->st_md_base->is_traced = 0; 
       current->st_md_base->next = NULL;
        current->st_md_base->last = NULL;
      }
      //for remove delete the node if present 
      if(action==REMOVE_STRACE){
      struct strace_head*head=current->st_md_base;
      struct strace_info* c = head->next;
    struct strace_info* previous = NULL;

    // If the head node contains the key, delete it and update the head
    if (c != NULL && c->syscall_num == syscall_num) {
        head->next = c->next;
        if(syscall_num== head->last->syscall_num)
        {
        	head->last=NULL;
        }
        return 0;
    }

    
    while (c != NULL && c->syscall_num != syscall_num) {
        previous = c;
        c = c->next;
    }
    
    if (c == NULL) {
        return -EINVAL;
    }
     if(c->syscall_num== head->last->syscall_num)
        {
        
        	previous->next=NULL;
        	head->last=previous;
        }
    previous->next = c->next;
    head->count=head->count-1;
    }
    else {struct strace_head*head=current->st_md_base;
      struct strace_info* c = head->next;
    struct strace_info* previous = NULL;
    int found=0;
    while(c != NULL ){
    if(c->syscall_num == syscall_num) {found=1;break;}
    previous=c;
    c=c->next;
  
    }
    if(found) return -EINVAL;
    if(head->next==NULL) {
    struct strace_info* sys=os_alloc(sizeof(struct strace_info));
    sys->syscall_num=syscall_num;
    sys->next=NULL;
    current->st_md_base->last =sys;
    current->st_md_base->next=sys;
    }
    else{
    struct strace_info* sys=os_alloc(sizeof(struct strace_info));
    sys->syscall_num=syscall_num;
    sys->next=NULL;
    
    current->st_md_base->last->next=sys;
    current->st_md_base->last =sys;
    }
    head->count=head->count+1;
    }
   
      //for add add if the node is not already there, else return -ENVAL
      
	return 0;
}

int sys_read_strace(struct file *fd, char *buff, u64 count)
{int red=0;   
  //printk("started read\n");
   //printk("fd=%x\n",*fd);
    while(count){
    int k=os_trace_buffer_read(fd,buff+red,8);
    //printk("k=%d\n",k);
    if(!k) return red;
    red+=k;
	// 	int v=	trace_buffer_osread(filep, buff+bits_read , 8);
// 	if(v==0)return bits_read;
// 		bits_read+= v;
    u64 syscall_num=((u64*)(buff+red-8))[0];
    if(syscall_num==0) return red; 
    
  //  printk("sys=%d\n",(int)syscall_num);
     if (syscall_num == SYSCALL_SLEEP || syscall_num == SYSCALL_DUMP_PTT || syscall_num == SYSCALL_PMAP || syscall_num == SYSCALL_DUP || syscall_num == SYSCALL_CLOSE){
			red+=os_trace_buffer_read(fd, buff+red , 8);
			
		}
		else if (syscall_num == SYSCALL_CONFIGURE || syscall_num == SYSCALL_SIGNAL || syscall_num == SYSCALL_EXPAND || syscall_num == SYSCALL_CLONE || syscall_num == SYSCALL_MUNMAP || syscall_num == SYSCALL_OPEN || syscall_num == SYSCALL_DUP2 || syscall_num == SYSCALL_STRACE ){
			red+=os_trace_buffer_read(fd,buff+red, 16);
		}
		else if (syscall_num == SYSCALL_MPROTECT || syscall_num == SYSCALL_WRITE || syscall_num == SYSCALL_READ || syscall_num == SYSCALL_LSEEK || syscall_num == SYSCALL_READ_STRACE || syscall_num == SYSCALL_READ_FTRACE ){
			red+=os_trace_buffer_read(fd, buff+red , 24);
			//printk("redred=%d\n",*(buff+red));
		}
		else if (syscall_num == SYSCALL_MMAP || syscall_num == SYSCALL_FTRACE){

		        red+=os_trace_buffer_read(fd,buff+red , 32);
		}
		else red+=0;
		//printk("red=%d\n",red);
    count--;
    }
	return red;
}

int sys_start_strace(struct exec_context *current, int fd, int tracing_mode)
{ // Check if the tracing mode is valid
//printk("start_strace entered\n"); 
    if (tracing_mode != FULL_TRACING && tracing_mode != FILTERED_TRACING) {
        return -EINVAL;
    }

    // Find the file corresponding to the provided file descriptor (fd)
    struct file *filep = current->files[fd];

    // Check if the file exists and is of type TRACE_BUFFER
    if (filep == NULL || filep->type != TRACE_BUFFER) {
        return -EINVAL;
    }

    // Set the tracing mode in the file structure
    //filep->tracing_mode = tracing_mode;

    // Initialize data structures for system call tracing
    // Create a new strace_head structure for the process if it doesn't exist
    if (current->st_md_base == NULL) {
        current->st_md_base = (struct strace_head *)os_alloc(sizeof(struct strace_head));
        current->st_md_base->count = 0;
        current->st_md_base->is_traced = 1; // Set tracing as enabled
        current->st_md_base->strace_fd = fd; // Store the trace buffer file descriptor
        current->st_md_base->tracing_mode = tracing_mode;
        current->st_md_base->next = NULL;
        current->st_md_base->last = NULL;
    } else {
        // Update tracing mode if it's already enabled
        current->st_md_base->tracing_mode = tracing_mode;
        current->st_md_base->is_traced = 1;
        current->st_md_base->strace_fd = fd; 
    }
//printk("start_strace exited\n"); 
    // Return 0 if initialization was successful
    return 1;
}

int sys_end_strace(struct exec_context *current)
{ // Check if the system call tracing is enabled
//printk("end_stace entered\n"); 
    if (current==0||current->st_md_base == 0) {
        return -EINVAL; // Tracing is not enabled
    }
    
    // Clean up all meta-data structures related to system call tracing
     struct strace_head *current_head = current->st_md_base;
     
     struct strace_info *current_info = current_head->next;
     struct strace_info *next_info = current_info;
        while (current_info != 0) {
            next_info = current_info->next;
            os_free(current_info, sizeof(struct strace_info));
            current_info=next_info;
        }
       
 
        os_free(current->st_md_base, sizeof(struct strace_head));
        current->st_md_base = 0;
    
//printk("end_stace exited\n"); 

    // Return 0 indicating successful cleanup
    return 0;

}




///////////////////////////////////////////////////////////////////////////
//// 		Start of ftrace functionality 		      	      /////
///////////////////////////////////////////////////////////////////////////


long do_ftrace(struct exec_context *ctx, unsigned long faddr, long action, long nargs, int fd_trace_buffer)
{
	
    return 0;
}

//Fault handler
long handle_ftrace_fault(struct user_regs *user_regs)
{
	
	
        return 0;
}


int sys_read_ftrace(struct file *filep, char *buff, u64 count)
{
	
    return 0
	;
}
