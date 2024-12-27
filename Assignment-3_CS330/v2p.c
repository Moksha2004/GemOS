#include <types.h>
#include <mmap.h>
#include <fork.h>
#include <v2p.h>
#include <page.h>

/* 
 * You may define macros and other helper functions here
 * You must not declare and use any static/global variables 
 * */


/**
 * mprotect System call Implementation.
 */
long vm_area_mprotect(struct exec_context *current, u64 addr, int length, int prot)
{    
    length = ((length + 4095) / 4096) * 4096;
    struct vm_area*prev=NULL; 
    struct vm_area*curr=current->vm_area;
    struct vm_area*next=current->vm_area->vm_next; 
    while(curr){
        if(curr->vm_start <= addr && curr->vm_end>addr&& curr->vm_end > (addr + length)){
            u64 current_addr=addr,end_addr=addr+length;
while(current_addr<end_addr){
                u32 base = current->pgd; // CR3 reg contents
            int validbit = 0;
            int l = 0;
            u64 pfn_now = base; // Current pfn
            u64 *now;
            u64 of = 0;
                u64 addr1=current_addr;
            //printk("about to enter while loop\n");
            int f=0;
            while (l < 4) // Checking till where the valid bit holds
            {  // printk("addr =%x\n",addr);
                l++;
                //printk("entered l=%d, in loop1\n", l);
                of = (addr1 >> (36 - (l * 9) + 12)) & 0x1FF; // Corrected of calculation
                now = (u64 *)osmap(pfn_now);          // Conversion of real to va
                validbit = (now[of] & 1);        // Returns 1 if present.

                if (validbit == 0){
                    break;f=1;}

                pfn_now = now[of] >> 12; // Contains the pfn in the table entry.
            }
            if(l==4){
                if(prot==PROT_READ) now[of] &= ~(1 << 3);
                else now[of]|=0x8;
                //printk("now %x",now[of]);
                //validbit=0;
                //now[of]&=0;
                // put_pfn(pfn_now);
                // if(!get_pfn_refcount(pfn_now)) os_pfn_free(USER_REG, pfn_now);
                asm volatile("invlpg (%0)"::"r" (addr1));
            }
            current_addr+=4096;
            }
            if(curr->vm_start!=addr){
                 //printk("may2\n");
                struct vm_area* new_vm_area = os_alloc(sizeof(struct vm_area));
                 new_vm_area->vm_start =addr;
                 new_vm_area->vm_end = addr+length;
                 new_vm_area->access_flags = prot;
                 u64 t=curr->vm_end;
                 curr->vm_end=addr;
                 curr->vm_next=new_vm_area;
            struct vm_area* new_vm_area1 = os_alloc(sizeof(struct vm_area));    
                 new_vm_area1->vm_start =addr+length;
                 new_vm_area1->vm_end = t;
                 new_vm_area1->access_flags = curr->access_flags;
                 new_vm_area1->vm_next=next; 
                 new_vm_area->vm_next=new_vm_area1;
                 stats->num_vm_area+=2;}
            else{
                struct vm_area* new_vm_area = os_alloc(sizeof(struct vm_area));
                 new_vm_area->vm_start =addr;
                 new_vm_area->vm_end = addr+length;
                 new_vm_area->access_flags = curr->access_flags;
                 prev->vm_next=new_vm_area;
                 curr->vm_start=addr+length;
                 new_vm_area->vm_next=curr;
                 stats->num_vm_area++;
            }
        }
        else if(curr->vm_start < addr &&curr->vm_end>addr && curr->vm_end <= (addr + length)){
             u64 current_addr=addr,end_addr=addr+length;
while(current_addr<end_addr){
                u32 base = current->pgd; // CR3 reg contents
            int validbit = 0;
            int l = 0;
            u64 pfn_now = base; // Current pfn
            u64 *now;
            u64 of = 0;
                u64 addr1=current_addr;
            //printk("about to enter while loop\n");
            int f=0;
            while (l < 4) // Checking till where the valid bit holds
            {  // printk("addr =%x\n",addr);
                l++;
                //printk("entered l=%d, in loop1\n", l);
                of = (addr1 >> (36 - (l * 9) + 12)) & 0x1FF; // Corrected of calculation
                now = (u64 *)osmap(pfn_now);          // Conversion of real to va
                validbit = (now[of] & 1);        // Returns 1 if present.

                if (validbit == 0){
                    break;f=1;}

                pfn_now = now[of] >> 12; // Contains the pfn in the table entry.
            }
            if(l==4){
                if(prot==PROT_READ) now[of] &= ~(1 << 3);
                else now[of]|=0x8;
                //printk("now %x",now[of]);
                //validbit=0;
                //now[of]&=0;
                // put_pfn(pfn_now);
                // if(!get_pfn_refcount(pfn_now)) os_pfn_free(USER_REG, pfn_now);
                asm volatile("invlpg (%0)"::"r" (addr1));
            }
            current_addr+=4096;
            }
             //printk("may1\n");
            struct vm_area* new_vm_area = os_alloc(sizeof(struct vm_area));
                 new_vm_area->vm_start =addr;
                 new_vm_area->vm_end =curr->vm_end;
                 new_vm_area->access_flags = prot;
                 new_vm_area->vm_next=next;
                 curr->vm_end=addr;
                 curr->vm_next=new_vm_area;
                 stats->num_vm_area++;
        }
        
        else if(curr->vm_start >= addr && curr->vm_end <= (addr + length)){
             u64 current_addr=addr,end_addr=addr+length;
while(current_addr<end_addr){
                u32 base = current->pgd; // CR3 reg contents
            int validbit = 0;
            int l = 0;
            u64 pfn_now = base; // Current pfn
            u64 *now;
            u64 of = 0;
                u64 addr1=current_addr;
            //printk("about to enter while loop\n");
            int f=0;
            while (l < 4) // Checking till where the valid bit holds
            {  // printk("addr =%x\n",addr);
                l++;
                //printk("entered l=%d, in loop1\n", l);
                of = (addr1 >> (36 - (l * 9) + 12)) & 0x1FF; // Corrected of calculation
                now = (u64 *)osmap(pfn_now);          // Conversion of real to va
                validbit = (now[of] & 1);        // Returns 1 if present.

                if (validbit == 0){
                    break;f=1;}

                pfn_now = now[of] >> 12; // Contains the pfn in the table entry.
            }
            if(l==4){
                if(prot==PROT_READ) now[of] &= ~(1 << 3);
                else now[of]|=0x8;
                //printk("now %x",now[of]);
                //validbit=0;
                //now[of]&=0;
                // put_pfn(pfn_now);
                // if(!get_pfn_refcount(pfn_now)) os_pfn_free(USER_REG, pfn_now);
                asm volatile("invlpg (%0)"::"r" (addr1));
            }
            current_addr+=4096;
            }
            // printk("may3\n");
           curr->access_flags=prot;
        }
        else if(curr->vm_start > addr && curr->vm_end >(addr + length)){
             u64 current_addr=addr,end_addr=addr+length;
while(current_addr<end_addr){
                u32 base = current->pgd; // CR3 reg contents
            int validbit = 0;
            int l = 0;
            u64 pfn_now = base; // Current pfn
            u64 *now;
            u64 of = 0;
                u64 addr1=current_addr;
            //printk("about to enter while loop\n");
            int f=0;
            while (l < 4) // Checking till where the valid bit holds
            {  // printk("addr =%x\n",addr);
                l++;
                //printk("entered l=%d, in loop1\n", l);
                of = (addr1 >> (36 - (l * 9) + 12)) & 0x1FF; // Corrected of calculation
                now = (u64 *)osmap(pfn_now);          // Conversion of real to va
                validbit = (now[of] & 1);        // Returns 1 if present.

                if (validbit == 0){
                    break;f=1;}

                pfn_now = now[of] >> 12; // Contains the pfn in the table entry.
            }
            if(l==4){
                if(prot==PROT_READ) now[of] &= ~(1 << 3);
                else now[of]|=0x8;
                //printk("now %x",now[of]);
                //validbit=0;
                //now[of]&=0;
                // put_pfn(pfn_now);
                // if(!get_pfn_refcount(pfn_now)) os_pfn_free(USER_REG, pfn_now);
                asm volatile("invlpg (%0)"::"r" (addr1));
            }
            current_addr+=4096;
            }
             //printk("may4\n");
            struct vm_area* new_vm_area = os_alloc(sizeof(struct vm_area));
                 new_vm_area->vm_start =curr->vm_start;
                 new_vm_area->vm_end = addr+length;
                 new_vm_area->access_flags = prot;
                 curr->vm_start=addr+length;
                 new_vm_area->vm_next=curr;
                 prev->vm_next=new_vm_area;
                 stats->num_vm_area++;
        }
        prev=curr;
        curr=curr->vm_next;
        next=curr->vm_next;
    }
    prev=NULL;
    curr=current->vm_area;
    next=current->vm_area->vm_next;
    while(curr){
        if(next){
            // if(next){if(curr->vm_end==next->vm_start && curr->access_flags==curr->access_flags&& curr->access_flags==next->access_flags&&curr->vm_end==next->vm_start){
            //     stats->num_vm_area--;
            //     prev->vm_end=next->vm_end;
            //     prev->vm_next=next->vm_next;
            //     printk("may1\n");
            // }
            // }
             if(curr->vm_end==next->vm_start && curr->access_flags==next->access_flags){
                curr->vm_end=next->vm_end;
                curr->vm_next=next->vm_next;
                stats->num_vm_area--;
                // printk("may2\n");
            }

        }
        prev=curr;
        curr=curr->vm_next;
        next=curr->vm_next;
    }
    


    return 0;
}

/**
 * mmap system call implementation.
 **/
long vm_area_map(struct exec_context* current, u64 addr, int length, int prot, int flags) {
    length = ((length + 4095) / 4096) * 4096;
    long ans=0;
    if( current->vm_area->vm_next==NULL){
      
    struct vm_area* dummy = os_alloc(sizeof(struct vm_area));
    dummy->vm_start = MMAP_AREA_START;
    dummy->vm_end = MMAP_AREA_START + 4096;
    dummy->access_flags = 0;
    stats->num_vm_area++;
    current->vm_area=dummy;
    current->vm_area->vm_next=NULL;
    }
    if(flags==MAP_FIXED){
        if(!addr) return -1;
        struct vm_area* prev=current->vm_area;
        struct vm_area* curr=current->vm_area->vm_next;
        while(curr){
             if (!(addr >= curr->vm_end || (addr + length) <= curr->vm_start)) return -1;
             curr=curr->vm_next;
        }
        struct vm_area* new_vm_area = os_alloc(sizeof(struct vm_area));
        new_vm_area->vm_start = addr;
        new_vm_area->vm_end = addr + length;
        new_vm_area->access_flags = prot;
        stats->num_vm_area++;
        prev=NULL;
        curr=current->vm_area;
        while(curr){
            if(new_vm_area->vm_start==prev->vm_end && prev->access_flags==prot &&new_vm_area->vm_end==curr->vm_start && curr->access_flags==prot){
                prev->vm_end=curr->vm_end;
                new_vm_area->vm_end=curr->vm_end;
                curr->vm_start=prev->vm_start;
                new_vm_area->vm_start=prev->vm_start;
                new_vm_area->vm_next=curr->vm_next;
                prev->vm_next=curr->vm_next;
            }
            else if(new_vm_area->vm_start==curr->vm_end && curr->access_flags==prot){
                curr->vm_end=new_vm_area->vm_end;
                curr->vm_next=new_vm_area->vm_next;
            }
            else if(new_vm_area->vm_end==curr->vm_start && curr->access_flags==prot) {
                curr->vm_start=new_vm_area->vm_end;
                new_vm_area->vm_next=curr->vm_next;
            }
            prev=curr;
            curr=curr->vm_next; 
        }
    }
    else if(flags==0){
        if(!addr){
                struct vm_area*prev=NULL;
                struct vm_area*curr=current->vm_area;
                u64 low = MMAP_AREA_START;
                ans=low;
                int f=0;
                while (curr)
                {  
                    if(low+length-1<=curr->vm_start) {//printk("hey\n");
                    ans=low;addr=low;f=1;break;}
                    low=curr->vm_end+1;
                    prev=curr;
                    curr=curr->vm_next;
                }
               addr=low-1;ans=addr;
                struct vm_area* new_vm_area = os_alloc(sizeof(struct vm_area));
              
                new_vm_area->vm_start = addr;
                new_vm_area->vm_end = addr + length;
                new_vm_area->access_flags = prot;
                stats->num_vm_area++;
                prev=NULL;
                curr=current->vm_area;
                //insert at beginning
                if (new_vm_area->vm_end < curr->vm_start) {
                    new_vm_area->vm_next = curr;
                    curr = new_vm_area;}
                while (curr && new_vm_area->vm_start >= curr->vm_end) {
                // Move to the next VMA
                    prev = curr;
                    curr = curr->vm_next;
                }

                // Insert the new VMA between prev and current
                if (prev) {
                    prev->vm_next = new_vm_area;
                }
                new_vm_area->vm_next = curr;
    
                prev=NULL;
                curr=current->vm_area;
            while(curr){
                //printk("came\n");
                if(curr->vm_start==prev->vm_end && prev->access_flags==curr->access_flags ){
                    
                    prev->vm_end=curr->vm_end;
                    prev->vm_next=curr->vm_next;
                    stats->num_vm_area--;
                }
                prev=curr;
                curr=curr->vm_next; 
            }
             
        }
        else{
            struct vm_area* prev=current->vm_area;
            struct vm_area* curr=current->vm_area;
            int search=0;
            while(curr){
                if (!(addr >= curr->vm_end || (addr + length) <= curr->vm_start)) search=1;
                curr=curr->vm_next;
            }
        if(!search){
             struct vm_area* new_vm_area = os_alloc(sizeof(struct vm_area));
              
                new_vm_area->vm_start = addr;
                new_vm_area->vm_end = addr + length;
                new_vm_area->access_flags = prot;
                stats->num_vm_area++;
                prev=NULL;
                curr=current->vm_area;
                //insert at beginning
                if (new_vm_area->vm_end < curr->vm_start) {
                    new_vm_area->vm_next = curr;
                    curr = new_vm_area;}
                while (curr && new_vm_area->vm_start >= curr->vm_end) {
                // Move to the next VMA
                    prev = curr;
                    curr = curr->vm_next;
                }

                // Insert the new VMA between prev and current
                if (prev) {
                    prev->vm_next = new_vm_area;
                }
                new_vm_area->vm_next = curr;
    
                prev=NULL;
                curr=current->vm_area;
            while(curr){
                //printk("came\n");
                if(curr->vm_start==prev->vm_end && prev->access_flags==curr->access_flags ){
                    
                    prev->vm_end=curr->vm_end;
                    prev->vm_next=curr->vm_next;
                    stats->num_vm_area--;
                }
                prev=curr;
                curr=curr->vm_next; 
            }
            }
            else{
                prev=NULL;
                curr=current->vm_area;
                u64 low = MMAP_AREA_START;
                while (curr)
                {  
                    if(low+length-1<=curr->vm_start) {//printk("hey\n");
                    ans=low;addr=low;break;}
                    low=curr->vm_end+1;
                    prev=curr;
                    curr=curr->vm_next;
                }
               addr=low-1;ans=addr;
                struct vm_area* new_vm_area = os_alloc(sizeof(struct vm_area));
              
                new_vm_area->vm_start = addr;
                new_vm_area->vm_end = addr + length;
                new_vm_area->access_flags = prot;
                stats->num_vm_area++;
                prev=NULL;
                curr=current->vm_area;
                //insert at beginning
                if (new_vm_area->vm_end < curr->vm_start) {
                    new_vm_area->vm_next = curr;
                    curr = new_vm_area;}
                while (curr && new_vm_area->vm_start >= curr->vm_end) {
                // Move to the next VMA
                    prev = curr;
                    curr = curr->vm_next;
                }

                // Insert the new VMA between prev and current
                if (prev) {
                    prev->vm_next = new_vm_area;
                }
                new_vm_area->vm_next = curr;
    
                prev=NULL;
                curr=current->vm_area;
            while(curr){
                //printk("came\n");
                if(curr->vm_start==prev->vm_end && prev->access_flags==curr->access_flags ){
                    
                    prev->vm_end=curr->vm_end;
                    prev->vm_next=curr->vm_next;
                    stats->num_vm_area--;
                }
                prev=curr;
                curr=curr->vm_next; 
            }
            }

            }
    }
  return ans;  

}





/**
 * munmap system call implemenations
 */

long vm_area_unmap(struct exec_context *current, u64 addr, int length)
{   
   

    length = ((length + 4095) / 4096) * 4096;
    
    struct vm_area *prev = NULL;
    struct vm_area *curr = current->vm_area;

    while (curr) {
      
        if (curr->vm_start <= addr && curr->vm_end >= (addr + length)) {
           
            if(curr->vm_start>=addr&&curr->vm_end<=(addr+length)){
                                 u64 current_addr=addr,end_addr=addr+length;
                    while(current_addr<end_addr){
                        u32 base = current->pgd; // CR3 reg contents
                    int validbit = 0;
                    int l = 0;
                    u64 pfn_now = base; // Current pfn
                    u64 *now;
                    u64 of = 0;
                        u64 addr1=current_addr;
                    //printk("about to enter while loop\n");
                    int f=0;
                    while (l < 4) // Checking till where the valid bit holds
                    { //  printk("addr =%x\n",addr);
                        l++;
                    // printk("entered l=%d, in loop1\n", l);
                        of = (addr1 >> (36 - (l * 9) + 12)) & 0x1FF; // Corrected of calculation
                        now = (u64 *)osmap(pfn_now);          // Conversion of real to va
                        validbit = (now[of] & 1);        // Returns 1 if present.

                        if (validbit == 0){
                            break;f=1;}

                        pfn_now = now[of] >> 12; // Contains the pfn in the table entry.
                    }
                    if(l==4){
                        // if(prot==PROT_READ) now[of] &= ~(1 << 3);
                        // else now[of]|=0x8;
                        // printk("now %x",now[of]);
                        validbit=0;
                        now[of]&=0;
                        put_pfn(pfn_now);
                        if(!get_pfn_refcount(pfn_now)) os_pfn_free(USER_REG, pfn_now);
                        asm volatile("invlpg (%0)"::"r" (addr1));
                    }
                    current_addr+=4096;
                    } 
            
                prev->vm_next=curr->vm_next;
                os_free(curr,sizeof(struct vm_area));
                stats->num_vm_area--;
                return 0;
            }
            else if(curr->vm_start<addr&&curr->vm_end>(addr+length)){
                                 u64 current_addr=addr,end_addr=addr+length;
                    while(current_addr<end_addr){
                        u32 base = current->pgd; // CR3 reg contents
                    int validbit = 0;
                    int l = 0;
                    u64 pfn_now = base; // Current pfn
                    u64 *now;
                    u64 of = 0;
                        u64 addr1=current_addr;
                    //printk("about to enter while loop\n");
                    int f=0;
                    while (l < 4) // Checking till where the valid bit holds
                    { //  printk("addr =%x\n",addr);
                        l++;
                    // printk("entered l=%d, in loop1\n", l);
                        of = (addr1 >> (36 - (l * 9) + 12)) & 0x1FF; // Corrected of calculation
                        now = (u64 *)osmap(pfn_now);          // Conversion of real to va
                        validbit = (now[of] & 1);        // Returns 1 if present.

                        if (validbit == 0){
                            break;f=1;}

                        pfn_now = now[of] >> 12; // Contains the pfn in the table entry.
                    }
                    if(l==4){
                        // if(prot==PROT_READ) now[of] &= ~(1 << 3);
                        // else now[of]|=0x8;
                        // printk("now %x",now[of]);
                        validbit=0;
                        now[of]&=0;
                        put_pfn(pfn_now);
                        if(!get_pfn_refcount(pfn_now)) os_pfn_free(USER_REG, pfn_now);
                        asm volatile("invlpg (%0)"::"r" (addr1));
                    }
                    current_addr+=4096;
                    } 
                
                 struct vm_area* new_vm_area = os_alloc(sizeof(struct vm_area));
                 new_vm_area->vm_start = addr+length;
                 new_vm_area->vm_end = curr->vm_end;
                 new_vm_area->access_flags = curr->access_flags;
                 new_vm_area->vm_next=curr->vm_next;
                 stats->num_vm_area++;
                 struct vm_area* new_vm_area1 = os_alloc(sizeof(struct vm_area));
                 new_vm_area1->vm_start = curr->vm_start;
                 new_vm_area1->vm_end = addr;
                 new_vm_area1->access_flags = curr->access_flags;
                 new_vm_area1->vm_next=new_vm_area;
                 prev->vm_next=new_vm_area1;
                 os_free(curr,sizeof(struct vm_area));

            }
            else if(curr->vm_start<addr&& curr->vm_end==(addr+length)){
                                 u64 current_addr=addr,end_addr=addr+length;
                    while(current_addr<end_addr){
                        u32 base = current->pgd; // CR3 reg contents
                    int validbit = 0;
                    int l = 0;
                    u64 pfn_now = base; // Current pfn
                    u64 *now;
                    u64 of = 0;
                        u64 addr1=current_addr;
                    //printk("about to enter while loop\n");
                    int f=0;
                    while (l < 4) // Checking till where the valid bit holds
                    { //  printk("addr =%x\n",addr);
                        l++;
                    // printk("entered l=%d, in loop1\n", l);
                        of = (addr1 >> (36 - (l * 9) + 12)) & 0x1FF; // Corrected of calculation
                        now = (u64 *)osmap(pfn_now);          // Conversion of real to va
                        validbit = (now[of] & 1);        // Returns 1 if present.

                        if (validbit == 0){
                            break;f=1;}

                        pfn_now = now[of] >> 12; // Contains the pfn in the table entry.
                    }
                    if(l==4){
                        // if(prot==PROT_READ) now[of] &= ~(1 << 3);
                        // else now[of]|=0x8;
                        // printk("now %x",now[of]);
                        validbit=0;
                        now[of]&=0;
                        put_pfn(pfn_now);
                        if(!get_pfn_refcount(pfn_now)) os_pfn_free(USER_REG, pfn_now);
                        asm volatile("invlpg (%0)"::"r" (addr1));
                    }
                    current_addr+=4096;
                    } 
                struct vm_area* new_vm_area = os_alloc(sizeof(struct vm_area));
                new_vm_area->vm_start =curr->vm_start;
                new_vm_area->vm_end = addr;
                new_vm_area->access_flags = curr->access_flags;
                new_vm_area->vm_next=curr->vm_next;
                prev->vm_next=new_vm_area;
                curr->vm_start=addr;
                curr->vm_end=addr+length;
                os_free(curr,sizeof(struct vm_area));
            }
            else if(curr->vm_start>=addr&&curr->vm_end>(addr+length)){
                                 u64 current_addr=addr,end_addr=addr+length;
                    while(current_addr<end_addr){
                        u32 base = current->pgd; // CR3 reg contents
                    int validbit = 0;
                    int l = 0;
                    u64 pfn_now = base; // Current pfn
                    u64 *now;
                    u64 of = 0;
                        u64 addr1=current_addr;
                    //printk("about to enter while loop\n");
                    int f=0;
                    while (l < 4) // Checking till where the valid bit holds
                    { //  printk("addr =%x\n",addr);
                        l++;
                    // printk("entered l=%d, in loop1\n", l);
                        of = (addr1 >> (36 - (l * 9) + 12)) & 0x1FF; // Corrected of calculation
                        now = (u64 *)osmap(pfn_now);          // Conversion of real to va
                        validbit = (now[of] & 1);        // Returns 1 if present.

                        if (validbit == 0){
                            break;f=1;}

                        pfn_now = now[of] >> 12; // Contains the pfn in the table entry.
                    }
                    if(l==4){
                        // if(prot==PROT_READ) now[of] &= ~(1 << 3);
                        // else now[of]|=0x8;
                        // printk("now %x",now[of]);
                        validbit=0;
                        now[of]&=0;
                        put_pfn(pfn_now);
                        if(!get_pfn_refcount(pfn_now)) os_pfn_free(USER_REG, pfn_now);
                        asm volatile("invlpg (%0)"::"r" (addr1));
                    }
                    current_addr+=4096;
                    } 
                struct vm_area* new_vm_area = os_alloc(sizeof(struct vm_area));
                 new_vm_area->vm_start = addr+length;
                 new_vm_area->vm_end = curr->vm_end;
                 new_vm_area->access_flags = curr->access_flags;
                 new_vm_area->vm_next=curr->vm_next;
                 prev->vm_next=new_vm_area;
                 curr->vm_end=new_vm_area->vm_start;
                 os_free(curr,sizeof(struct vm_area));
                 return 0;
            }
            else if(curr->vm_start<addr &&curr->vm_end<=addr+length){
                                        u64 current_addr=addr,end_addr=addr+length;
                    while(current_addr<end_addr){
                        u32 base = current->pgd; // CR3 reg contents
                    int validbit = 0;
                    int l = 0;
                    u64 pfn_now = base; // Current pfn
                    u64 *now;
                    u64 of = 0;
                        u64 addr1=current_addr;
                    //printk("about to enter while loop\n");
                    int f=0;
                    while (l < 4) // Checking till where the valid bit holds
                    { //  printk("addr =%x\n",addr);
                        l++;
                    // printk("entered l=%d, in loop1\n", l);
                        of = (addr1 >> (36 - (l * 9) + 12)) & 0x1FF; // Corrected of calculation
                        now = (u64 *)osmap(pfn_now);          // Conversion of real to va
                        validbit = (now[of] & 1);        // Returns 1 if present.

                        if (validbit == 0){
                            break;f=1;}

                        pfn_now = now[of] >> 12; // Contains the pfn in the table entry.
                    }
                    if(l==4){
                        // if(prot==PROT_READ) now[of] &= ~(1 << 3);
                        // else now[of]|=0x8;
                        // printk("now %x",now[of]);
                        validbit=0;
                        now[of]&=0;
                        put_pfn(pfn_now);
                        if(!get_pfn_refcount(pfn_now)) os_pfn_free(USER_REG, pfn_now);
                        asm volatile("invlpg (%0)"::"r" (addr1));
                    }
                    current_addr+=4096;
                    }   
                struct vm_area*new_vm_area=os_alloc(sizeof(struct vm_area));
                new_vm_area->vm_start=curr->vm_start;
                new_vm_area->vm_end=addr;
                new_vm_area->access_flags=curr->access_flags;
                prev->vm_next=new_vm_area;
                new_vm_area->vm_next=curr->vm_next;
                os_free(curr,sizeof(struct vm_area));
                return 0;

            }
        }
        prev=curr;
        curr=curr->vm_next;
        }
 
        return 0;

}




/**
 * Function will invoked whenever there is page fault for an address in the vm area region
 * created using mmap
 */

// long vm_area_pagefault(struct exec_context *current, u64 addr, int error_code)
// {    printk("addr=%x\n",addr);
//     struct vm_area*curr=current->vm_area;
//     printk("%d\n",error_code);
//     while(curr){
       
//         if(addr>=curr->vm_start&&addr<curr->vm_end){
//             printk("hey here\n");
//              if ((error_code == 0x6 && (curr->access_flags ==PROT_READ))) {
//                 printk("huha\n");
//                 return -1;
//             }
//             else if(error_code == (0x7)){

//             }
//             else if(error_code !=0x7){

//                u64 base=current->pgd;
//                u64 off=(addr>>39)&0x1FF;
//                u64*pgd=(u64*)osmap(base);
//                u64 pgd_t=pgd[off];
//                u64 p=pgd_t&1;
//                u64 new;
//                if(!p){
//                 printk("lvl1\n");
//                 pgd_t=pgd_t|1;
//                 if(curr->access_flags==PROT_READ)pgd_t=pgd_t& ~(1<<3);
//                 else pgd_t=pgd_t|1000;
//                 pgd_t=pgd_t|10000;
//                 new=os_pfn_alloc(OS_PT_REG);
//                 pgd_t = (pgd_t & 0x000000000FFF) | (new << 12);
//                 pgd[off]=pgd_t;
//                }
//                u64*pud=(u64*)osmap(pgd_t>>12);
//                off=(addr>>30)&0x1FF;
//                u64 pud_t=pud[off];
//                p=pud_t&1;
//                if(!p){
//                 printk("lvl2\n");
//                 pud_t=pud_t|1;
//                 if(curr->access_flags==PROT_READ)pud_t=pud_t& ~(1<<3);
//                 else pud_t=pud_t|1000;
//                 pud_t=pud_t|10000;
//                 new=os_pfn_alloc(OS_PT_REG);
//                 pud_t = (pud_t & 0x000000000FFF) | (new << 12);
//                 pud[off]=pud_t;
//                }
//                u64*pmd=(u64*)osmap(pud_t>>12);
//                off=(addr>>21)&0x1FF;
//                u64 pmd_t=pmd[off];
//                p=pmd_t&1;
//                if(!p){
//                 printk("lvl3\n");
//                 pmd_t=pmd_t|1;
//                 if(curr->access_flags==PROT_READ)pmd_t=pmd_t& ~(1<<3);
//                 else pmd_t=pmd_t|1000;
//                 pmd_t=pmd_t|10000;
//                 new=os_pfn_alloc(OS_PT_REG);
//                 pmd_t = (pmd_t & 0x000000000FFF) | (new << 12);
//                 pmd[off]=pmd_t;
//                }
//                u64*ptd=(u64*)osmap(pmd_t>>12);
//                off=(addr>>12)&0x1FF;
//                u64 ptd_t=ptd[off];
//                p=ptd_t&1;
//                if(!p){
//                 printk("lvl4\n");
//                 ptd_t=ptd_t|1;
//                 if(curr->access_flags==PROT_READ)ptd_t=ptd_t& ~(1<<3);
//                 else ptd_t=ptd_t|1000;
//                 ptd_t=ptd_t|10000;
//                 new=os_pfn_alloc(USER_REG);
//                 ptd_t = (ptd_t & 0x000000000FFF) | (new << 12);
//                 ptd[off]=ptd_t;
//                }
//                return 1;
//             }
           
//         }
//         curr=curr->vm_next;
//     }
    
//     return 1;
// }

// long vm_area_pagefault(struct exec_context *current, u64 addr, int error_code)
// {
//     printk("entered page fault\n");
//     //first checking if addr is actually present in the allocated vm region.
//     struct vm_area * vmhead= current->vm_area;
//     struct vm_area *vmnow= vmhead;
//     int found=0;
//     u32 flag=0; //this is the access flag w.r.to vm area
//     while(!found&&vmnow)
//     {
//         // if(!vmnow->vm_next)
//         // {
//         //     if(vmnow->vm_start<=addr && vmnow->vm_end>addr)
//         //         found=1;
//         //     flag=vmnow->access_flags;
//         //     break;

//         // }
//         if(vmnow->vm_start<=addr && vmnow->vm_end>addr)
//         {
//             found=1;
//             flag=vmnow->access_flags;
//             break;
//         }     
//         vmnow=vmnow->vm_next;
//     }
//     if(found)
//     printk("found!\n");
//     if(!found || (found && flag==PROT_READ && (error_code==0x6)))
//     {
//         printk("about to return!\n");
//         return -1;
//     }
//     else if(error_code==0x7)
//     {
//          printk("into error code 0x7!\n");
//         if(flag==PROT_READ) return -1;
//         long temp=handle_cow_fault(current, addr, flag);
//         return 1;
//     }
//     u32 base= current->pgd; //this has the CR3 reg contents
//     int validbit=0;
//     int l=0;
//     u64 pfn_now=base; //current pfn
//     u64* now;
//     u64 of=0;
//     //u64* nowva;
//     printk("about to enter while loop");
//     while(l<4) //checking till where the valid bit holds
//     {
//         l++;
//         printk("entered l= %d, in loop1\n",l);
//         of= addr>>(36-(l*9)+12); //removes the part on the right
//         //of= (of & (0b111111111));
//         now= (u64*) osmap(pfn_now); //conversion of real to va
//         //nowva=now+of;
//         validbit= (now[of] & 1); //returns 1 if present.
//         if(validbit==0)
//         break;
//         pfn_now= (now[of]>>12); //this now contains the pfn in the tabe entry.      
//     }
//     //when the above loop breaks, of, pfn_now and now has the current page 
//     while(l<=4) //now creating all the upcoming pfns from where valid bit was 0.
//     {
//         printk("entered l= %d, in loop2\n",l);
//         u64 pfn_new;
//         if(l<4)
//          pfn_new= os_pfn_alloc(OS_PT_REG); //returns the new pfn.
//         else
//          pfn_new= os_pfn_alloc(USER_REG);
//         now[of]= ((now[of]) & (0b111111111111)); //retains the last 12 bits, everything else 0.
//         now[of]= ((now[of]) | (pfn_new<<12)); 
//         now[of]= ((now[of]) | 1); //makes the valid bit 1
//         now[of]= ((now[of]) | (0b1000)); 
//         if(flag==PROT_READ)
//         {
//             now[of]&=~(1<<3);
//         }
//         else
//         now[of]= ((now[of]) | (0b100)); //3rd bit=1, read and wite both allowed.
//         printk("entry i.e now[of]=%x\n",now[of]);
//         if(l==4)
//         break;
//         l++;
//         pfn_now=pfn_new;
//         now=(u64*) osmap(pfn_now);
//         of= addr>>(36-(l*9)+12);
//     }
//     //when this breaks, pfn_now and now,of has the info of 4th l.

//     return 1;
// }
long vm_area_pagefault(struct exec_context *current, u64 addr, int error_code){

struct vm_area *vmhead = current->vm_area;
struct vm_area *vmnow = vmhead;
int found = 0;
u32 flag = 0; // Access flag w.r.to vm area

while (!found && vmnow)
{
    if (vmnow->vm_start <= addr && vmnow->vm_end > addr)
    {
        found = 1;
        flag = vmnow->access_flags;
        break;
    }
    vmnow = vmnow->vm_next;
}

if (!found || (found && flag == PROT_READ && (error_code == 0x6)))
{
    return -1;
}
else if (error_code == 0x7)
{  return -1;
    // printk("into error code 0x7!\n");
    // if (flag == PROT_READ)
    //     return -1;

    // long temp = handle_cow_fault(current, addr, flag);
    // return 1;
}

u32 base = current->pgd; // CR3 reg contents
int validbit = 0;
int l = 0;
u64 pfn_now = base; // Current pfn
u64 *now;
u64 of = 0;


while (l < 4) // Checking till where the valid bit holds
{
    l++;
    
    of = (addr >> (36 - (l * 9) + 12)) & 0x1FF; // Corrected of calculation
    now = (u64 *)osmap(pfn_now);          // Conversion of real to va
    validbit = (now[of] & 1);        // Returns 1 if present.

    if (validbit == 0)
        break;

    pfn_now = now[of] >> 12; // Contains the pfn in the table entry.
}

// When the above loop breaks, of, pfn_now, and now have the current page

while (l <= 4) // Creating all the upcoming pfns from where the valid bit was 0
{
   
    u64 pfn_new;

    of = (addr >> (36 - (l * 9) + 12)) & 0x1FF; // Corrected of calculation

    if (l < 4)
        pfn_new = os_pfn_alloc(OS_PT_REG); // Returns the new pfn
    else
        pfn_new = os_pfn_alloc(USER_REG);

    now[of] &= 0xFFF;          // Retains the last 12 bits, everything else 0
    now[of] |= (pfn_new << 12); // Sets the new pfn
    now[of] |= 1;               // Sets the valid bit
    now[of] |= 0x16;            // Sets the accessed and dirty bits

    if (flag == PROT_READ &&l==4)
    {
        now[of] &= ~(1 << 3); // Clears the writable bit
    }
    else
    {
        now[of] |= 0x8; // Sets the writable bit
    }


    if (l == 4)
    {
        return 1;
    }

    l++;
    pfn_now = pfn_new;
    now = (u64 *)osmap(pfn_now);
}

//When this breaks, pfn_now, and now, of have the info of the 4th l.

return -1;
}
/**
 * cfork system call implemenations
 * The parent returns the pid of child process. The return path of
 * the child process is handled separately through the calls at the 
 * end of this function (e.g., setup_child_context etc.)
 */

long do_cfork(){
    u32 pid;
    struct exec_context *new_ctx = get_new_ctx();
    struct exec_context *ctx = get_current_ctx();
     /* Do not modify above lines
     * 
     * */   
     /*--------------------- Your code [start]---------------*/
     

     /*--------------------- Your code [end] ----------------*/
    
     /*
     * The remaining part must not be changed
     */
    copy_os_pts(ctx->pgd, new_ctx->pgd);
    do_file_fork(new_ctx);
    setup_child_context(new_ctx);
    return pid;
}



/* Cow fault handling, for the entire user address space
 * For address belonging to memory segments (i.e., stack, data) 
 * it is called when there is a CoW violation in these areas. 
 *
 * For vm areas, your fault handler 'vm_area_pagefault'
 * should invoke this function
 * */

long handle_cow_fault(struct exec_context *current, u64 vaddr, int access_flags)
{
  return -1;
}