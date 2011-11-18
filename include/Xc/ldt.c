
int init_new_context(struct task_struct *tsk, struct mm_struct *mm)
{
    struct mm_struct *old_mm;
	int retval = 0;

	mutex_init(&mm->context.lock);
	mm->context.size = 0;
	oldmm = current->mm;
	if (oldmm && oldmm->context.size > 0) {
        mutex_lock(&old->context.lock);
		retval = copy_ldt(&mm->context, &old_mm->context);
		mutex_unlock(&old_mm->context.lock);
	}
	return retval;
}
