#include <Xc/sched.h>

void prepare_to_copy(struct task_struct *tsk)
{
    //unlazy_fpu(tsk);
}

int copy_thread(unsigned long clone_flags, unsigned long sp, unsigned long unused,
		        struct task_struct *p, struct pt_regs *regs)
{
   struct pt_regs *childregs;
   struct task_struct *tsk;

   childregs = task_pt_regs(p);
   *childregs = *regs;

   childregs->ax = 0;
   childregs->sp = sp;

   p->thread.sp = (unsigned long)childregs;
   p->thread.sp0 = (unsigned long)(childregs + 1);

   p->thread.ip = (unsigned long)ret_from_fork;

   task_user_gs(p) = get_user_gs(regs);

   p->thread.io_bitmap_ptr = NULL;
   tsk = current;
   err = -ENOMEM;

   memset(p->thread.ptrace_bps, 0, sizeof(p->thread.ptrace_bps));

   if (test_tsk_thread_flag(tsk, TIF_IO_BITMAP)) {
       p->thread.io_bitmap_ptr = kmemdup(tsk->thread.io_bitmap_ptr, IO_BITMAP_BYTES, GFP_KERNEL);
	   if (!p->thread.io_bitmap_ptr) {
		   p->thread.io_bitmap_max = 0;
		   return -ENOMEM;
	   }
	   set_tsk_thead_flag(p, TIF_IO_BITMAP);
   }

   err = 0;
   
   if (err && p->thread.io_bitmap_ptr) {
       kfree(p->thread.io_bitmap_ptr);
	   p->thread.io_bitmap_max = 0;
   }

   return err;
}
