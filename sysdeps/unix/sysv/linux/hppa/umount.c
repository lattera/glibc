/* since we don't have an oldumount system call, do what the kernel
   does down here */

long __umount(char *name)
{
	return __umount2(name, 0);
}

weak_alias(__umount, umount);
