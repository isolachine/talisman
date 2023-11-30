
## Reading this document

The information flow analysis will compute a set of source sink pairs that violation
our integrity lattice. These violations are subsequently seperated into categories to help explain 
the cause of the gap. The categories will refer to an authorization step, such as looking up subject 
or object id's, creating id's, and/or resolving operations. 

Within each section a table will list all violations with relevant information regarding how and what should
be endorsed.

| Hook Function        | Gap Flow                                                    | Input                | Security ID         | Sink Line #'s                  |
|----------------------|-------------------------------------------------------------|----------------------|---------------------|--------------------------------| 
| selinux_netlink_send | {subject, dynamic, external} -> {subject, dynamic, monitor} | task_security_struct | cred->security->sid | /security/selinux/hooks.c 3976  | 

Take the above example. The hook function column shows which function the violation is related to. The gap flow states
an external, dynamic, value representing the subject flows to the subject argument at the sink. The sink expects a higher 
integrity than "external" for the location dimension of the lattice. The source column shows the specific source used as
the subject label. Each LSM has a function or a set of functions that are responsible for taking a request and producing 
an authorization result. We refer to these functions as "sink functions". The last column "Sink Line #'s" shows the line 
numbers of sink function calls that require endorsement showing where an endorser needs to be added. 


## SELinux




### Subject Lookup

Subject's in SELinux are looked up through a variety of helper functions, but ultimately come from the same location
within a subject's task_security_struct (an SELinux defined structure). Typically, a linux credential pointer is passed
as input or looked up via a kernel macro, which contains a pointer to a task_security_struct, from which an integer label
can be accessed. In order to store this value in a hashtable we require a unique id to key the subject label with. We rely 
on the thread id of the currently executing task, which can be looked up via the current macro.

~~~
/* Semaphore security operations */
static int selinux_sem_alloc_security(struct sem_array *sma)
{
	// Record subject id, keyed with the current task processor id
	__endorser__record_subject_id(current->task->pid, cred->security->sid);
	
	struct ipc_security_struct *isec;
	struct common_audit_data ad;
	u32 sid = current_sid();
	int rc;

	rc = ipc_alloc_security(current, &sma->sem_perm, SECCLASS_SEM);
	if (rc)
		return rc;

	isec = sma->sem_perm.security;

	ad.type = LSM_AUDIT_DATA_IPC;
	ad.u.ipc_id = sma->sem_perm.key;
    
    // Endorser validation will return 1 on successful check, 0 if the values do not match
	int __endorser__value = __endorser__validate_subject_id(current->task->pid, sid);
	if (__endorser__value <= 0)
	{
	     // Fail gracefully if enforcing, otherwise do nothing
	}
	
	rc = avc_has_perm(sid, isec->sid, SECCLASS_SEM,
			  SEM__CREATE, &ad);
	if (rc) {
		ipc_free_security(&sma->sem_perm);
		return rc;
	}
	return 0;
}
~~~


Above is an example of endorsing the subject id for the hook function selinux_sem_alloc_security, which governs semaphore
operations. On the first line we record the subject id, keyed with the processor id. Before the sink call to avc_has_perm
we validate that the subject id that will be passed to the sink matches what was recorded.

Below is a table outlining for each hook function (that was analyzed by our tool) the subject lookups that need to
be endorsed. Each entry contains the hook function name, the lattice violation, the location of the subject id, and a 
list of sink locations that the subject id is used.

The lattice violation for all subject lookups is largely the same, but differentiates between subjects that are passed
as input as opposed to looked up externally via kernel macros.

Similarly, the source location will be the same for each case, but was included for consistency. Sink locations represent
the locations that will require endorsement based on what was recorded earlier. 

In a number of cases there are multiple options for which sink call will be leveraged at runtime, so endorsements must be 
placed before each to cover all cases. Note that many of the sink calls are the same across different hook functions. 
Hook functions governing similar data types such as files, inodes, and dentry's will leverage similar helper functions 
leading to the same sink call. We only need a single endorser for each unique sink call location used by the hooks.

| Hook Function                     | Gap Flow                                                    | Input | Security ID            | Sink Line #'s                                                                                                                                              |
|-----------------------------------|-------------------------------------------------------------|-------|------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------| 
| selinux_netlink_send              | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3976                                                                                                                             |                
| selinux_file_ioctl                | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1579 /security/selinux/hooks.c:1684                                                                                              |
| selinux_inode_permission          | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:2865                                                                                                                             |
| selinux_socket_getpeername        | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3976                                                                                                                             |
| selinux_file_receive              | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1684 /security/selinux/hooks.c:1617                                                                                              | 
| selinux_inode_rename              | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1821 /security/selinux/hooks.c:1825 /security/selinux/hooks.c:1830 /security/selinux/hooks.c:1840 /security/selinux/hooks.c:1846 |
| selinux_shm_shmat                 | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:5136                                                                                                                             |
| selinux_task_getpgid              | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1544                                                                                                                             |
| selinux_kernel_act_as             | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3524                                                                                                                             |
| selinux_inode_symlink             | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1722 /security/selinux/hooks.c:1735                                                                                              |
| selinux_mmap_file                 | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1508 /security/selinux/hooks.c:1684 /security/selinux/hooks.c:1617                                                               |
| selinux_task_getsid               | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1544                                                                                                                             |
| selinux_sem_alloc_security        | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:5399                                                                                                                             |
| selinux_set_mnt_opts              | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:390 /security/selinux/hooks.c:395                                                                                                |
| selinux_shm_associate             | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:5332                                                                                                                             |
| selinux_file_lock                 | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1684 /security/selinux/hooks.c:1617                                                                                              |
| selinux_getprocattr               | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1544                                                                                                                             |
| selinux_file_fcntl                | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1684 /security/selinux/hooks.c:1617                                                                                              |
| selinux_msg_queue_associate       | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:5191                                                                                                                             |
| selinux_kernel_module_request     | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3568                                                                                                                             |
| selinux_inode_rmdir               | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1777 /security/selinux/hooks.c:1797                                                                                              |
| selinux_socket_create             | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3995                                                                                                                             |
| selinux_umount                    | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1886                                                                                                                             |
| selinux_socket_getsockname        | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3976                                                                                                                             |
| selinux_task_kill                 | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3666 /security/selinux/hooks.c:1544                                                                                              |
| selinux_inode_link                | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1777 /security/selinux/hooks.c:1797                                                                                              |
| selinux_task_setscheduler         | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1544                                                                                                                             |
| selinux_shm_shmctl                | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1594 /security/selinux/hooks.c:5136                                                                                              |
| selinux_inode_listxattr           | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1508                                                                                                                             |
| selinux_socket_bind               | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3976 /security/selinux/hooks.c:4082 /security/selinux/hooks.c:4122                                                               |
| selinux_task_setpgid              | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1544                                                                                                                             |
| selinux_inode_removexattr         | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1508                                                                                                                             |
| selinux_mount                     | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1617                                                                                                                             |
| selinux_task_setrlimit            | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1544                                                                                                                             |
| selinux_socket_recvmsg            | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3976                                                                                                                             |
| selinux_inode_mknod               | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1722 /security/selinux/hooks.c:1735                                                                                              |
| selinux_task_create               | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1544                                                                                                                             |
| selinux_sem_semop                 | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:5136                                                                                                                             |
| selinux_sb_kern_mount             | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1866                                                                                                                             |
| selinux_inode_getattr             | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1617                                                                                                                             |
| selinux_task_movememory           | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1544                                                                                                                             |
| selinux_socket_connect            | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3976 /security/selinux/hooks.c:4176                                                                                              |
| selinux_syslog                    | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1594                                                                                                                             |
| selinux_inode_getsecctx           | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1579                                                                                                                             |
| selinux_sem_semctl                | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1594                                                                                                                             |
| selinux_socket_setsockopt         | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3976                                                                                                                             |
| selinux_capget                    | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1544                                                                                                                             |
| selinux_file_permission           | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1684 /security/selinux/hooks.c:1617                                                                                              |
| selinux_quota_on                  | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1508                                                                                                                             |
| selinux_file_mprotect             | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1684 /security/selinux/hooks.c:1617                                                                                              |
| selinux_sem_associate             | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:5424                                                                                                                             |
| selinux_ipc_permission            | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:5136                                                                                                                             |
| selinux_inode_readlink            | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1508                                                                                                                             |
| selinux_socket_shutdown           | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3976                                                                                                                             |
| selinux_msg_queue_alloc_security  | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:5166                                                                                                                             |
| selinux_tun_dev_attach_queue      | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:4669                                                                                                                             |
| selinux_msg_queue_msgctl          | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1594 /security/selinux/hooks.c:5136                                                                                              |
| selinux_secmark_relabel_packet    | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:4613                                                                                                                             |
| selinux_shm_alloc_security        | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:5307                                                                                                                             |
| selinux_sb_statfs                 | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1866                                                                                                                             |
| selinux_inode_setxattr            | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:2993 /security/selinux/hooks.c:3003                                                                                              |
| selinux_inode_unlink              | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1777 /security/selinux/hooks.c:1797                                                                                              |
| selinux_tun_dev_open              | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:4697 /security/selinux/hooks.c:4701                                                                                              |
| selinux_kernel_create_files_as    | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3548                                                                                                                             |
| selinux_inode_getxattr            | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1508                                                                                                                             |
| selinux_socket_getsockopt         | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3976                                                                                                                             |
| selinux_ptrace_access_check       | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1544                                                                                                                             |
| selinux_socket_listen             | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3976                                                                                                                             |
| selinux_task_getscheduler         | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3647                                                                                                                             |
| selinux_quotactl                  | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1866                                                                                                                             |
| selinux_socket_sendmsg            | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3976                                                                                                                             |
| selinux_socket_accept             | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:3976                                                                                                                             |
| selinux_inode_setattr             | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1508                                                                                                                             |
| selinux_inode_getsecurity         | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1579                                                                                                                             |
| selinux_setprocattr               | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:5661 /security/selinux/hooks.c:5676                                                                                              |
| selinux_task_getioprio            | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1544                                                                                                                             |
| selinux_task_setioprio            | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1544                                                                                                                             |
| selinux_inode_follow_link         | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1508                                                                                                                             |
| selinux_inode_mkdir               | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1722 /security/selinux/hooks.c:1735                                                                                              |
| selinux_inode_create              | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1722 /security/selinux/hooks.c:1735                                                                                              |
| selinux_vm_enough_memory          | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1722 /security/selinux/hooks.c:1735                                                                                              |
| selinux_msg_queue_msgsnd          | {subject, dynamic, external} -> {subject, dynamic, monitor} | cred  | cred->security->sid    | /security/selinux/hooks.c:1579                                                                                                                             |

| Hook Function                     | Gap Flow                                                    | Input | Security ID           | Sink Line #'s                                                 |
|-----------------------------------|-------------------------------------------------------------|-------|-----------------------|---------------------------------------------------------------| 
| selinux_file_open                 |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:1617                                |
| selinux_file_send_sigiotask       |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:3408                                |
| selinux_key_permission            |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:5791                                |
| selinux_task_kill                 |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:3666 /security/selinux/hooks.c:1544 |
| selinux_socket_sock_rcv_skb       |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:4413 /security/selinux/hooks.c:4422 |
| selinux_xfrm_policy_lookup        |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/xfrm.c:169                                  |
| selinux_syslog                    |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:1594                                |
| selinux_task_wait                 |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:1594                                |
| selinux_xfrm_state_pol_flow_match |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:1528                                |
| selinux_msg_queue_msgrcv          |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:5282 /security/selinux/hooks.c:5285 |
| selinux_capable                   |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:1579                                |
| selinux_inode_getxattr            |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:1508                                |
| selinux_socket_unix_may_send      |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:4302                                |
| selinux_ptrace_traceme            |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:1528                                |
| selinux_socket_unix_stream_connect|  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:4271                                |
| selinux_capset                    |  {subject, dynamic, input} -> {subject, dynamic, monitor}   | cred  | cred->security->sid   | /security/selinux/hooks.c:1508                                |


### Object Lookup

SELinux authorizes operations for a variety of data types. Recording the security id of objects and validating that 
the correct id is being passed to the object argument at corresponding sink calls can be achieved similarly as subjects.
However, one challenge is identifying unique keys to use for each object. While a number of data types contain their own
id value, in some cases these values are not unique. We can see an example of this with inodes, as an inode id can be 
re-used. For file system objects related to inodes we leverage the pair (device id, inode id) as a unique key.

Below we provide code examples to endorse each data type our analysis observed in SELinux.

1. Inode
~~~
static int inode_has_perm(const struct cred *cred,
			  struct inode *inode,
			  u32 perms,
			  struct common_audit_data *adp)
{
	struct inode_security_struct *isec;
	u32 sid;

	validate_creds(cred);

	if (unlikely(IS_PRIVATE(inode)))
		return 0;

	sid = cred_sid(cred);
	isec = inode->i_security;

    // Endorser validation will return 1 on successful check, 0 if the values do not match
	int __endorser__value = __endorser__validate_object_id_string(inode->i_rdev, inode->i_ino, inode->i_security->sid);
	if (__endorser__value <= 0)
	{
	     // Fail gracefully if enforcing, otherwise do nothing
	}
	return avc_has_perm(sid, isec->sid, isec->sclass, perms, adp);
}
~~~

Similar endorsement strategy, except we use the inode number and device id concatenated as the key.
Note that inode numbers are not sufficient for a unique key.

2. Dentry
~~~
static int selinux_inode_follow_link(struct dentry *dentry, struct nameidata *nameidata)
{
    // Record object id, keyed with the device id for the superblock
	__endorser__record_object_id_string(dentry->d_inode->i_rdev, dentry->d_inode->i_ino, dentry->d_inode->i_security->sid);
	const struct cred *cred = current_cred();

	return dentry_has_perm(cred, dentry, FILE__READ);
}

static inline int dentry_has_perm(const struct cred *cred,
				  struct dentry *dentry,
				  u32 av)
{
	struct inode *inode = dentry->d_inode;
	struct common_audit_data ad;

	ad.type = LSM_AUDIT_DATA_DENTRY;
	ad.u.dentry = dentry;
	return inode_has_perm(cred, inode, av, &ad);
}
~~~

Dentrys are converted into inodes, so the endorsement process is the same, except the path to each of the 
relevant key/value pair start at a dentry as opposed an inode. In this example, the endorsement check at sink
within inode_has_perm will be the same, but the recording of the security id will come from the dentry.

3. Files

~~~
static int file_has_perm(const struct cred *cred, struct file *file, u32 av)
{
   // Record object id, keyed with the device id for the superblock
   __endorser__record_object_id_string(file->f_inode->i_rdev, file->f_inode->i_ino, file->f_security->sid);
   
   struct file_security_struct *fsec = file->f_security;
   struct inode *inode = file_inode(file);
   struct common_audit_data ad;
   u32 sid = cred_sid(cred);
   int rc;

   ad.type = LSM_AUDIT_DATA_PATH;
   ad.u.path = file->f_path;

   if (sid != fsec->sid) {
      // Endorser validation will return 1 on successful check, 0 if the values do not match
	  int __endorser__value = __endorser__validate_object_id_string(file->f_inode->i_rdev, file->f_inode->i_ino, file->f_security->sid);
	  if (__endorser__value <= 0)
	  {
	     // Fail gracefully if enforcing, otherwise do nothing
	  }
	  rc = avc_has_perm(sid, fsec->sid,
				  SECCLASS_FD,
				  FD__USE,
				  &ad);
	  if (rc)
		 goto out;
   }

   /* av is zero if only checking access to the descriptor. */
   rc = 0;
   if (av)
	  rc = inode_has_perm(cred, inode, av, &ad);

   out:
   return rc;
}
~~~



4. Superblock

~~~
static int selinux_quotactl(int cmds, int type, int id, struct super_block *sb)
{

	// Record object id, keyed with the device id for the superblock
	__endorser__record_object_id(sb->sdev, sb->s_security->sid);
	
	const struct cred *cred = current_cred();
	int rc = 0;

	if (!sb)
		return 0;

	switch (cmds) {
	case Q_SYNC:
	case Q_QUOTAON:
	case Q_QUOTAOFF:
	case Q_SETINFO:
	case Q_SETQUOTA:
		rc = superblock_has_perm(cred, sb, FILESYSTEM__QUOTAMOD, NULL);
		break;
	case Q_GETFMT:
	case Q_GETINFO:
	case Q_GETQUOTA:
		rc = superblock_has_perm(cred, sb, FILESYSTEM__QUOTAGET, NULL);
		break;
	default:
		rc = 0;  /* let the kernel handle invalid cmds */
		break;
	}
	return rc;
}

/* Check whether a task can perform a filesystem operation. */
static int superblock_has_perm(const struct cred *cred,
			       struct super_block *sb,
			       u32 perms,
			       struct common_audit_data *ad)
{
	struct superblock_security_struct *sbsec;
	u32 sid = cred_sid(cred);

	sbsec = sb->s_security;
	
	// Endorser validation will return 1 on successful check, 0 if the values do not match
	int __endorser__value = __endorser__validate_object_id(sb->sdev, sb->s_security->sid);
	if (__endorser__value <= 0)
	{
	     // Fail gracefully if enforcing, otherwise do nothing
	}
	return avc_has_perm(sid, sbsec->sid, SECCLASS_FILESYSTEM, perms, ad);
}
~~~

5. IPC

~~~
static int selinux_ipc_permission(struct kern_ipc_perm *ipcp, short flag)
{
	// Record object id, keyed with the ipc id
	__endorser__record_object_id(ipcp->id, ipcp->security->sid);
	
	u32 av = 0;

	av = 0;
	if (flag & S_IRUGO)
		av |= IPC__UNIX_READ;
	if (flag & S_IWUGO)
		av |= IPC__UNIX_WRITE;

	if (av == 0)
		return 0;

	return ipc_has_perm(ipcp, av);
}

static int ipc_has_perm(struct kern_ipc_perm *ipc_perms,
			u32 perms)
{
	struct ipc_security_struct *isec;
	struct common_audit_data ad;
	u32 sid = current_sid();

	isec = ipc_perms->security;

	ad.type = LSM_AUDIT_DATA_IPC;
	ad.u.ipc_id = ipc_perms->key;

	// Endorser validation will return 1 on successful check, 0 if the values do not match
	int __endorser__value = __endorser__validate_object_id(ipc_perms->id, ipc_perms->security->sid);
	if (__endorser__value <= 0)
	{
	     // Fail gracefully if enforcing, otherwise do nothing
	}
	return avc_has_perm(sid, isec->sid, isec->sclass, perms, &ad);
}
~~~
6. Keys

~~~
static int selinux_key_permission(key_ref_t key_ref,
				  const struct cred *cred,
				  unsigned perm)
{
	struct key *key;
	struct key_security_struct *ksec;
	u32 sid;

	/* if no specific permissions are requested, we skip the
	   permission check. No serious, additional covert channels
	   appear to be created. */
	if (perm == 0)
		return 0;

	sid = cred_sid(cred);

	key = key_ref_to_ptr(key_ref);
	
	// Record object id, keyed with the key serian number
	__endorser__record_object_id(key->serial, key->security->sid);
	
	ksec = key->security;

	// Endorser validation will return 1 on successful check, 0 if the values do not match
	int __endorser__value = __endorser__validate_object_id(key->serial, key->security->sid);
	if (__endorser__value <= 0)
	{
	     // Fail gracefully if enforcing, otherwise do nothing
	}
	return avc_has_perm(sid, ksec->sid, SECCLASS_KEY, perm, NULL);
}
~~~

7. Shared Memory

~~~
static int selinux_shm_alloc_security(struct shmid_kernel *shp)
{

	// Record object id, keyed with the shared memory id
	__endorser__record_object_id(shp->shm_perm->id, shp->shm_perm->security->id);
	
	struct ipc_security_struct *isec;
	struct common_audit_data ad;
	u32 sid = current_sid();
	int rc;

	rc = ipc_alloc_security(current, &shp->shm_perm, SECCLASS_SHM);
	if (rc)
		return rc;

	isec = shp->shm_perm.security;

	ad.type = LSM_AUDIT_DATA_IPC;
	ad.u.ipc_id = shp->shm_perm.key;

	// Endorser validation will return 1 on successful check, 0 if the values do not match
	int __endorser__value = __endorser__validate_object_id(shp->shm_perm->id, shp->shm_perm->security->id);
	if (__endorser__value <= 0)
	{
	     // Fail gracefully if enforcing, otherwise do nothing
	}
	rc = avc_has_perm(sid, isec->sid, SECCLASS_SHM,
			  SHM__CREATE, &ad);
	if (rc) {
		ipc_free_security(&shp->shm_perm);
		return rc;
	}
	return 0;
}
~~~

8. Semaphore

~~~
static int selinux_sem_semctl(struct sem_array *sma, int cmd)
{
	// Record object id, keyed with the semaphore id
	__endorser__record_object_id(sma->sem_perm->id, sma->sem_perm->security->id);

	int err;
	u32 perms;

	switch (cmd) {
	case IPC_INFO:
	case SEM_INFO:
		/* No specific object, just general system-wide information. */
		return task_has_system(current, SYSTEM__IPC_INFO);
	case GETPID:
	case GETNCNT:
	case GETZCNT:
		perms = SEM__GETATTR;
		break;
	case GETVAL:
	case GETALL:
		perms = SEM__READ;
		break;
	case SETVAL:
	case SETALL:
		perms = SEM__WRITE;
		break;
	case IPC_RMID:
		perms = SEM__DESTROY;
		break;
	case IPC_SET:
		perms = SEM__SETATTR;
		break;
	case IPC_STAT:
	case SEM_STAT:
		perms = SEM__GETATTR | SEM__ASSOCIATE;
		break;
	default:
		return 0;
	}

	err = ipc_has_perm(&sma->sem_perm, perms);
	return err;
}
~~~



| Hook Function                     | Gap Flow                                                    | Input         | Security ID                      | Sink Line #'s                    |
|-----------------------------------|-------------------------------------------------------------|---------------|----------------------------------|----------------------------------| 
| selinux_task_create               |  {object, dynamic, external} -> {object, dynamic, monitor}  | task_struct   | tsk->real_cred->security->sid    | /security/selinux/hooks.c:1544   |
| selinux_task_wait                 |  {object, dynamic, external} -> {object, dynamic, monitor}  | task_struct   | tsk->real_cred->security->sid    | /security/selinux/hooks.c:1528   |
| selinux_ptrace_traceme            |  {object, dynamic, external} -> {object, dynamic, monitor}  | task_struct   | tsk->real_cred->security->sid    | /security/selinux/hooks.c:1528   |

| Hook Function                      | Gap Flow                                                  | Input                  | Security ID                                                                                                                    | Sink Line #'s                                                 |
|------------------------------------|-----------------------------------------------------------|------------------------|--------------------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------| 
| selinux_netlink_send               | {object, dynamic, input} -> {object, dynamic, monitor}    | sock                   | sk->sk_security->sid                                                                                                           | /security/selinux/hooks.c:3976                                                                                                                                |
| selinux_xfrm_policy_delete         | {object, dynamic, input} -> {object, dynamic, monitor}    | xfrm_sec_ctx           | ctx->ctx_sid                                                                                                                   | /security/selinux/xfrm.c:147                                                                                                                                  |
| selinux_file_ioctl                 | {object, dynamic, input} -> {object, dynamic, monitor}    | file                   | file->f_security->sid                                                                                                          | /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1617                                                                                                |
| selinux_inode_permission           | {object, dynamic, input} -> {object, dynamic, monitor}    | inode                  | inode->i_security->sid                                                                                                         | /security/selinux/hooks.c:2865                                                                                                                                |
| selinux_socket_getpeername         | {object, dynamic, input} -> {object, dynamic, monitor}    | sock                   | sk->sk_security->sid                                                                                                           | /security/selinux/hooks.c:3976                                                                                                                                |
| selinux_file_receive               | {object, dynamic, input} -> {object, dynamic, monitor}    | file                   | file->f_security->sid                                                                                                          | /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1617                                                                                                |
| selinux_inode_rename               | {object, dynamic, input} -> {object, dynamic, monitor}    | inode, dentry          | old_dir->i_security->sid, old_dentry->d_inode->i_security->sid, new_dir->i_security->sid, new_dentry->d_inode->i_security->sid | /security/selinux/hooks.c:1821, /security/selinux/hooks.c:1825, /security/selinux/hooks.c:1830, /security/selinux/hooks.c:1840, /security/selinux/hooks.c:184 |
| selinux_shm_shmat                  | {object, dynamic, input} -> {object, dynamic, monitor}    | shmid_kernel           | shp->shm_perm->security->sid                                                                                                   | /security/selinux/hooks.c:5136                                                                                                                                |
| selinux_task_getpgid               | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1544                                                                                                                                |
| selinux_task_setnice               | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1544                                                                                                                                |
| selinux_file_open                  | {object, dynamic, input} -> {object, dynamic, monitor}    | file                   | file->f_security->sid                                                                                                          | /security/selinux/hooks.c:1617                                                                                                                                |
| selinux_kernel_act_as              | {object, dynamic, input} -> {object, dynamic, monitor}    | int                    | secid                                                                                                                          | /security/selinux/hooks.c:3524                                                                                                                                |
| selinux_inode_symlink              | {object, dynamic, input} -> {object, dynamic, monitor}    | inode                  | dir->i_security->sid, dir->i_sb->s_security->sid                                                                               | /security/selinux/hooks.c:1722, /security/selinux/hooks.c:1735                                                                                                |
| selinux_mmap_file                  | {object, dynamic, input} -> {object, dynamic, monitor}    | cred                   | cred->security->sid, file->f_security->sid                                                                                     | /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1617                                                                                                |
| selinux_task_getsid                | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1544                                                                                                                                |
| selinux_sem_alloc_security         | {object, dynamic, input} -> {object, dynamic, monitor}    | sem_array              | sma->sem_perm.security->sid                                                                                                    | /security/selinux/hooks.c:5399                                                                                                                                |
| selinux_set_mnt_opts               | {object, dynamic, input} -> {object, dynamic, monitor}    | superblock             | sb->s_security->sid                                                                                                            | /security/selinux/hooks.c:390, /security/selinux/hooks.c:395                                                                                                  |
| selinux_file_lock                  | {object, dynamic, input} -> {object, dynamic, monitor}    | file                   | file->f_security->sid                                                                                                          | /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1617                                                                                                |
| selinux_getprocattr                | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1544                                                                                                                                |
| selinux_file_fcntl                 | {object, dynamic, input} -> {object, dynamic, monitor}    | file                   | file->f_security->sid                                                                                                          | /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1617                                                                                                |
| selinux_msg_queue_associate        | {object, dynamic, input} -> {object, dynamic, monitor}    | msg_queue              | msq->q_perm.security->sid                                                                                                      | /security/selinux/hooks.c:5191                                                                                                                                |
| selinux_inode_rmdir                | {object, dynamic, input} -> {object, dynamic, monitor}    | inode                  | dir->i_security, dentry->d_inode->i_security                                                                                   | /security/selinux/hooks.c:1777, /security/selinux/hooks.c:1797                                                                                                |
| selinux_file_send_sigiotask        | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:3408                                                                                                                                |
| selinux_umount                     | {object, dynamic, input} -> {object, dynamic, monitor}    | vfsmount               | mnt->mnt_sb->s_security->sid                                                                                                   | /security/selinux/hooks.c:1866                                                                                                                                |
| selinux_key_permission             | {object, dynamic, input} -> {object, dynamic, monitor}    | key                    | key->security->sid                                                                                                             | /security/selinux/hooks.c:5791                                                                                                                                |
| selinux_socket_getsockname         | {object, dynamic, input} -> {object, dynamic, monitor}    | socket                 | sock->sk->sk_security->sid                                                                                                     | /security/selinux/hooks.c:3976                                                                                                                                |
| selinux_task_setioprio             | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1544                                                                                                                                |
| selinux_task_kill                  | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:3666, /security/selinux/hooks.c:1544                                                                                                |
| selinux_socket_sock_rcv_skb        | {object, dynamic, input} -> {object, dynamic, monitor}    | sk_buff                | skb->secmark, nlbl->sid, xfrm->sid                                                                                             | /security/selinux/hooks.c:4413, /security/selinux/hooks.c:4422                                                                                                |
| selinux_inode_link                 | {object, dynamic, input} -> {object, dynamic, monitor}    | inode                  | dir->i_security, dentry->d_inode->i_security                                                                                   | /security/selinux/hooks.c:1777, /security/selinux/hooks.c:1797                                                                                                |
| selinux_xfrm_state_delete          | {object, dynamic, input} -> {object, dynamic, monitor}    | xfrm_sec_ctx           | ctx->ctx_sid                                                                                                                   | /security/selinux/xfrm.c:147                                                                                                                                  |
| selinux_task_setscheduler          | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1544                                                                                                                                |
| selinux_shm_shmctl                 | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1594                                                                                                                                |
| selinux_inode_listxattr            | {object, dynamic, input} -> {object, dynamic, monitor}    | dentry                 | dentry->d_inode->i_security->sid                                                                                               | /security/selinux/hooks.c:1617                                                                                                                                |
| selinux_socket_bind                | {object, dynamic, input} -> {object, dynamic, monitor}    | sock                   | sk->sk_security->sid                                                                                                           | /security/selinux/hooks.c:3976                                                                                                                                |
| selinux_task_setpgid               | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1544                                                                                                                                |
| selinux_inode_removexattr          | {object, dynamic, input} -> {object, dynamic, monitor}    | dentry                 | dentry->d_inode->i_security->sid                                                                                               | /security/selinux/hooks.c:1617                                                                                                                                |
| selinux_mount                      | {object, dynamic, input} -> {object, dynamic, monitor}    | path                   | path->dentry->d_sb->s_security->sid, path->dentry->d_inode->i_security->sid                                                    | /security/selinux/hooks.c:1866                                                                                                                                |
| selinux_task_setrlimit             | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1544                                                                                                                                |
| selinux_socket_recvmsg             | {object, dynamic, input} -> {object, dynamic, monitor}    | sock                   | sk->sk_security->sid                                                                                                           | /security/selinux/hooks.c:3976                                                                                                                                |
| selinux_inode_mknod                | {object, dynamic, input} -> {object, dynamic, monitor}    | inode                  | dir->i_security->sid, dir->i_sb->s_security->sid                                                                               | /security/selinux/hooks.c:1722, /security/selinux/hooks.c:1735                                                                                                |
| selinux_sem_semop                  | {object, dynamic, input} -> {object, dynamic, monitor}    | sem_array              | sma->sem_perm->security->sid                                                                                                   | /security/selinux/hooks.c:5136                                                                                                                                |
| selinux_sb_kern_mount              | {object, dynamic, input} -> {object, dynamic, monitor}    | superblock             | sb->s_security->sid                                                                                                            | /security/selinux/hooks.c:1866                                                                                                                                |
| selinux_xfrm_policy_lookup         | {object, dynamic, input} -> {object, dynamic, monitor}    | xfrm_sec_ctx           | ctx->ctx_sid                                                                                                                   | /security/selinux/xfrm.c:169                                                                                                                                  |
| selinux_inode_getattr              | {object, dynamic, input} -> {object, dynamic, monitor}    | path                   | path->dentry->d_inode->i_security->sid                                                                                         | /security/selinux/hooks.c:1647                                                                                                                                |
| selinux_task_movememory            | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1544                                                                                                                                |
| selinux_xfrm_state_alloc           | {object, dynamic, input} -> {object, dynamic, monitor}    | xfrm_sec_ctx           | ctx->ctx_sid                                                                                                                   | /security/selinux/xfrm.c:111                                                                                                                                  |
| selinux_socket_connect             | {object, dynamic, input} -> {object, dynamic, monitor}    | sock                   | sk->sk_security->sid, sk->sk_protocol->psec.sid                                                                                | /security/selinux/hooks.c:3976, /security/selinux/hooks.c:4176                                                                                                |
| selinux_sem_semctl                 | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid, sma->sem_perm->security->sid                                                                    | /security/selinux/hooks.c:5136                                                                                                                                |
| selinux_socket_setsockopt          | {object, dynamic, input} -> {object, dynamic, monitor}    | sock                   | sk->sk_security->sid                                                                                                           | /security/selinux/hooks.c:3976                                                                                                                                |
| selinux_capget                     | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1544                                                                                                                                |
| selinux_file_permission            | {object, dynamic, input} -> {object, dynamic, monitor}    | file                   | file->f_security->sid                                                                                                          | /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1617                                                                                                |
| selinux_xfrm_state_pol_flow_match  | {object, dynamic, input} -> {object, dynamic, monitor}    | xfrm_state             | x->security->ctx_sid                                                                                                           | /security/selinux/xfrm.c:208                                                                                                                                  |
| selinux_msg_queue_msgrcv           | {object, dynamic, input} -> {object, dynamic, monitor}    | msg_queue, msg_msg     | msq->q_perm.security->sid, msg->security->sid                                                                                  | /security/selinux/hooks.c:5282, /security/selinux/hooks.c:5285                                                                                                |
| selinux_quota_on                   | {object, dynamic, input} -> {object, dynamic, monitor}    | dentry                 | dentry->d_inode->i_security->sid                                                                                               | /security/selinux/hooks.c:1617                                                                                                                                |
| selinux_file_mprotect              | {object, dynamic, input} -> {object, dynamic, monitor}    | file                   | file->f_security->sid, tsk->real_cred->security->sid                                                                           | /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1508, /security/selinux/hooks.c:1544                                |
| selinux_sem_associate              | {object, dynamic, input} -> {object, dynamic, monitor}    | sem_array              | sma->sem_perm.security->sid                                                                                                    | /security/selinux/hooks.c:5424                                                                                                                                |
| selinux_ipc_permission             | {object, dynamic, input} -> {object, dynamic, monitor}    | kern_ipc_perm          | ipcp->security->sid                                                                                                            | /security/selinux/hooks.c:5496                                                                                                                                |
| selinux_inode_readlink             | {object, dynamic, input} -> {object, dynamic, monitor}    | dentry                 | dentry->d_inode->i_security->sid                                                                                               | /security/selinux/hooks.c:1617                                                                                                                                |
| selinux_socket_shutdown            | {object, dynamic, input} -> {object, dynamic, monitor}    | sock                   | sk->sk_security->sid                                                                                                           | /security/selinux/hooks.c:3976                                                                                                                                |
| selinux_msg_queue_alloc_security   | {object, dynamic, input} -> {object, dynamic, monitor}    | msg_queue              | msq->q_perm.security->sid                                                                                                      | /security/selinux/hooks.c:5166                                                                                                                                |
| selinux_tun_dev_attach_queue       | {object, dynamic, input} -> {object, dynamic, monitor}    | tun_security_struct    | security->sid                                                                                                                  | /security/selinux/hooks.c:4669                                                                                                                                |
| selinux_msg_queue_msgctl           | {object, dynamic, input} -> {object, dynamic, monitor}    | msg_queue, task_struct | msq->q_perm->security->sid, tsk->real_cred->security->sid                                                                      | /security/selinux/hooks.c:5136, /security/selinux/hooks.c:1594                                                                                                |
| selinux_secmark_relabel_packet     | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:4613                                                                                                                                |
| selinux_xfrm_policy_alloc          | {object, dynamic, input} -> {object, dynamic, monitor}    | xfrm_sec_ctx           | ctx->ctx_sid                                                                                                                   | /security/selinux/xfrm.c:111                                                                                                                                  |
| selinux_shm_alloc_security         | {object, dynamic, input} -> {object, dynamic, monitor}    | shmid_kernel           | shp->shm_perm.security->sid                                                                                                    | /security/selinux/hooks.c:5307                                                                                                                                |
| selinux_sb_statfs                  | {object, dynamic, input} -> {object, dynamic, monitor}    | dentry                 | dentry->d_sb->s_security->sid                                                                                                  | /security/selinux/hooks.c:1866                                                                                                                                |
| selinux_inode_setxattr             | {object, dynamic, input} -> {object, dynamic, monitor}    | inode                  | inode->i_security->sid, inode->i_sb->s_security->sid                                                                           | /security/selinux/hooks.c:2957, /security/selinux/hooks.c:2993, /security/selinux/hooks.c:3003                                                                |
| selinux_inode_unlink               | {object, dynamic, input} -> {object, dynamic, monitor}    | inode, dentry          | dir->i_security, dentry->d_inode->i_security                                                                                   | /security/selinux/hooks.c:1777, /security/selinux/hooks.c:1797                                                                                                |
| selinux_tun_dev_open               | {object, dynamic, input} -> {object, dynamic, monitor}    | tun_security_struct    | security->sid                                                                                                                  | /security/selinux/hooks.c:4697                                                                                                                                |
| selinux_kernel_create_files_as     | {object, dynamic, input} -> {object, dynamic, monitor}    | inode                  | inode->i_security->sid                                                                                                         | /security/selinux/hooks.c:3548                                                                                                                                |
| selinux_inode_getxattr             | {object, dynamic, input} -> {object, dynamic, monitor}    | dentry                 | dentry->d_sb->s_security->sid                                                                                                  | /security/selinux/hooks.c:1866                                                                                                                                |
| selinux_socket_unix_may_send       | {object, dynamic, input} -> {object, dynamic, monitor}    | sock                   | other->sk->sk_security->sid                                                                                                    | /security/selinux/hooks.c:4302                                                                                                                                |
| selinux_socket_getsockopt          | {object, dynamic, input} -> {object, dynamic, monitor}    | sock                   | sk->sk_security->sid                                                                                                           | /security/selinux/hooks.c:3976                                                                                                                                |
| selinux_ptrace_access_check        | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1948, /security/selinux/hooks.c:1544                                                                                                |
| selinux_socket_listen              | {object, dynamic, input} -> {object, dynamic, monitor}    | sock                   | sk->sk_security->sid                                                                                                           | /security/selinux/hooks.c:3976                                                                                                                                |
| selinux_socket_unix_stream_connect | {object, dynamic, input} -> {object, dynamic, monitor}    | sock                   | other->sk_security->sid                                                                                                        | /security/selinux/hooks.c:4271                                                                                                                                |
| selinux_task_getscheduler          | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1544                                                                                                                                |
| selinux_quotactl                   | {object, dynamic, input} -> {object, dynamic, monitor}    | superblock             | sb->s_security->sid                                                                                                            | /security/selinux/hooks.c:1866                                                                                                                                |
| selinux_socket_sendmsg             | {object, dynamic, input} -> {object, dynamic, monitor}    | socket                 | sock->sk->sk_security->sid                                                                                                     | /security/selinux/hooks.c:3976                                                                                                                                |
| selinux_socket_accept              | {object, dynamic, input} -> {object, dynamic, monitor}    | socket                 | sock->sk->sk_security->sid                                                                                                     | /security/selinux/hooks.c:3976                                                                                                                                |
| selinux_capset                     | {object, dynamic, input} -> {object, dynamic, monitor}    | cred                   | new->security->sid                                                                                                             | /security/selinux/hooks.c:1508                                                                                                                                |
| selinux_inode_setattr              | {object, dynamic, input} -> {object, dynamic, monitor}    | dentry                 | dentry->d_sb->s_security->sid                                                                                                  | /security/selinux/hooks.c:1866                                                                                                                                |
| selinux_setprocattr                | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid, sidtab->cur->sid                                                                                | /security/selinux/hooks.c:1544, /security/selinux/hooks.c:5661, /security/selinux/hooks.c:5676                                                                |
| selinux_task_getioprio             | {object, dynamic, input} -> {object, dynamic, monitor}    | task_struct            | tsk->real_cred->security->sid                                                                                                  | /security/selinux/hooks.c:1544                                                                                                                                |
| selinux_inode_follow_link          | {object, dynamic, input} -> {object, dynamic, monitor}    | dentry                 | dentry->d_sb->s_security->sid                                                                                                  | /security/selinux/hooks.c:1866                                                                                                                                |
| selinux_inode_mkdir                | {object, dynamic, input} -> {object, dynamic, monitor}    | inode                  | dir->i_security->sid, dir->i_sb->s_security->sid                                                                               | /security/selinux/hooks.c:1722, /security/selinux/hooks.c:1735                                                                                                |
| selinux_inode_create               | {object, dynamic, input} -> {object, dynamic, monitor}    | inode                  | dir->i_security->sid, dir->i_sb->s_security->sid                                                                               | /security/selinux/hooks.c:1722, /security/selinux/hooks.c:1735                                                                                                |
| selinux_msg_queue_msgsnd           | {object, dynamic, input} -> {object, dynamic, monitor}    | msg_queue, msg_msg     | msq->q_perm.security->sid, msg->security->sid                                                                                  | /security/selinux/hooks.c:5252, /security/selinux/hooks.c:5256, /security/selinux/hooks.c:5260                                                                |


### Creation
| Hook Function                     | Gap Flow                                                     | Input                  | Security ID          | Sink Line #'s                   | Comments                                 |
|-----------------------------------|--------------------------------------------------------------|------------------------|----------------------|---------------------------------|------------------------------------------|
|1. selinux_tun_dev_create          | {subject, dynamic, external} -> {object, dynamic, monitor}   | task_security_struct   | tsec->sid            | /security/selinux/hooks.c:4661  |                                          |
|2. selinux_tun_dev_open            | {subject, dynamic, external} -> {object, dynamic, monitor}   | task_security_struct   | tsec->sid            | /security/selinux/hooks.c:4701  |                                          |
|3. selinux_sem_alloc_security      | {subject, dynamic, external} -> {object, dynamic, monitor}   | task_security_struct   | tsec->sid            | /security/selinux/hooks.c:5399  | isec->sid assigned in ipc_alloc_security |
|4. selinux_shm_alloc_security      | {subject, dynamic, external} -> {object, dynamic, monitor}   | task_security_struct   | tsec->sid            | /security/selinux/hooks.c:5307  | isec->sid assigned in ipc_alloc_security |
|5. selinux_msg_queue_alloc_security| {subject, dynamic, external} -> {object, dynamic, monitor}   | task_security_struct   | tsec->sid            | /security/selinux/hooks.c:5166  | isec->sid assigned in ipc_alloc_security |
|6. selinux_task_kill               | {subject, dynamic, external} -> {object, dynamic, monitor}   | task_security_struct   | tsec->sid            | /security/selinux/hooks.c:3666  |                                          |
|7. selinux_task_create             | {subject, dynamic, external} -> {object, dynamic, monitor}   | task_security_struct   | tsec->sid            | /security/selinux/hooks.c:1544  |                                          |
|8. selinux_socket_create           | {subject, dynamic, external} -> {object, dynamic, monitor}   | task_security_struct   | tsec->sockcreate_sid | /security/selinux/hooks.c:3995  |                                          |

Cases 1-7 make calls to avc_has_perm in their respective hook functions. There is no helper function used between all functions. Endorsers need to be placed within these hook functions individually. The endorsement requires recording the subject security label that is stored in current's "task_security" structure in the "sid" field. Before respective calls to avc_has_perm are performed, check the equality of the recorded value with the object security label argument being passed to avc_has_perm. One endorser for each case.

Case #8 is the same as 1-7, except the endorser is recording the sockcreate_sid field of current's task_security structure, similarly checking the equality with the passed object security label to avc_has_perm. One endorser for this case.



| Hook Function                     | Gap Flow                                                      | Input                  | Security ID         | Sink Line #'s                    |
|-----------------------------------|---------------------------------------------------------------|------------------------|---------------------|----------------------------------|
|9. selinux_inode_create            | {subject, dynamic, external} -> {object, dynamic, monitor}    | task_security_struct   | tsec->create_sid    | /security/selinux/hooks.c:1735   |
|10. selinux_inode_symlink          | {subject, dynamic, external} -> {object, dynamic, monitor}    | task_security_struct   | tsec->create_sid    | /security/selinux/hooks.c:1735   |
|11. selinux_inode_mkdir            | {subject, dynamic, external} -> {object, dynamic, monitor}    | task_security_struct   | tsec->create_sid    | /security/selinux/hooks.c:1735   |
|12. selinux_inode_mknod            | {subject, dynamic, external} -> {object, dynamic, monitor}    | task_security_struct   | tsec->create_sid    | /security/selinux/hooks.c:1735   |

Cases #9-12 represent hooks that handle creating file system objects. All hook functions above rely on a common function "may_create" which can serve as a location to place 1 endorser to cover all of these cases. We will want to record current's value of "tsec->create_sid" and check that the value passed as the object sink argument in avc_has_perm matches the recorded value.

Case #13 does not rely on the common function "may_create". The process of endorsement is the same as above except the source value being recorded is "cred->sid". A single endorser is necessary for this case.


| Hook Function                     | Gap Flow                                                      | Input  | Security ID   | Sink Line #'s                    |
|-----------------------------------|---------------------------------------------------------------|--------|---------------|----------------------------------|
|13. selinux_vm_enough_memory       | {subject, dynamic, external} -> {object, dynamic, monitor}    | cred   | cred->sid     | /security/selinux/hooks.c:1579   |  
|14. selinux_file_mprotect          | {subject, dynamic, external} -> {object, dynamic, monitor}    | cred   | cred->sid     | /security/selinux/hooks.c:1544   | 
|15. selinux_mmap_addr              | {subject, dynamic, external} -> {object, dynamic, monitor}    | cred   | tsec->sid     | /security/selinux/hooks.c:3282   | 
|16. selinux_mmap_file              | {subject, dynamic, external} -> {object, dynamic, monitor}    | cred   | tsec->sid     | /security/selinux/hooks.c:1508   | 

No common function between these cases. Cases 13-16 will have their own endorser to check that the subject label from either the credential or the task_security struct matches the value passed as the object label in calls to avc_has_perm.



| Hook Function                     | Gap Flow                                                      | Input                | Security ID                | Sink Line #'s |
|-----------------------------------|---------------------------------------------------------------|----------------------|----------------------------|---------------|
|17. selinux_socket_sock_rcv_skb    | {subject, dynamic, external} -> {object, dynamic, monitor}    | netlbl_lsm_secattr   | secattr->attr.secid        |               |
|17. selinux_socket_sock_rcv_skb    | {subject, dynamic, external} -> {object, dynamic, monitor}    | netlbl_lsm_secattr   | secattr->cache->data       |               |
|17. selinux_socket_sock_rcv_skb    | {subject, dynamic, external} -> {object, dynamic, monitor}    | sk_buff              | skb->sp->xvec[i]->security |               |


Case #17 is slightly more complicated as a security label can be pulled from 1 of 3 locations, so all three locations must be tracked. We can check that the security label matches one of these 3 locations at the sink.

> "Developers did not leverage the socket create sid within the task security struct as this is not a socket
   in the traditional sense. This socket is private and only accessible by the kernel"

<br></p>



### Use Subject Metadata

| Hook Function                     | Gap Flow                                                      | Input  | Security ID | Sink Line #'s                   |
|-----------------------------------|---------------------------------------------------------------|--------|-------------|---------------------------------|
|1. selinux_inode_getsecctx         | {subject, dynamic, external} -> {object, dynamic, monitor}    | cred   | cred->sid   | /security/selinux/hooks.c:1579  |  
|2. selinux_file_ioctl              | {subject, dynamic, external} -> {object, dynamic, monitor}    | cred   | cred->sid   | /security/selinux/hooks.c:1579  | 
|3. selinux_inode_getsecurity       | {subject, dynamic, external} -> {object, dynamic, monitor}    | cred   | cred->sid   | /security/selinux/hooks.c:1579  | 
|4. selinux_capable                 | {subject, dynamic, external} -> {object, dynamic, monitor}    | cred   | cred->sid   | /security/selinux/hooks.c:1579  | 
|5. selinux_capset                  | {subject, dynamic, external} -> {object, dynamic, monitor}    | cred   | cred->sid   | /security/selinux/hooks.c:1508  |

Cases #1-4 represent capability hooks. These hook functions will pass the subject label as an object label when checking if a subject has appropriate capabilities. All hook functions above call cred_has_capability, so a single endorser can be placed to handle all of these cases.

Case #5 is slightly different and will call avc_has_perm directly. So we need a single endorser for this case.

| Hook Function                     | Gap Flow                                                      | Input | Security ID | Sink Line #'s                  |
|-----------------------------------|---------------------------------------------------------------|-------|-------------|--------------------------------|
|6. selinux_sb_kern_mount           | {subject, dynamic, external} -> {object, dynamic, monitor}    | cred  | cred->sid   | /security/selinux/hooks.c:379  |
|7. selinux_set_mnt_opts            | {subject, dynamic, external} -> {object, dynamic, monitor}    | cred  | red->sid    | /security/selinux/hooks.c:379  |

Cases 6 and 7 represent mounting functions. Mounting functions have an option to relabel a superblock, which requires a check to see if the user has sufficient permissions. The hook will perform two checks, the first to see if the subject has permissions to relabel and a second check file associations. Hook "selinux_sb_kern_mount" calls "selinux_set_mnt_opts", so a single endorser can be placed to handle these cases.


### Operation Selection

| Hook Function                    | Gap Flow                                       | Input               | Sink Line #'s                                                                                                             | Bitwise            | Comments                                                                 |
|----------------------------------|------------------------------------------------|----------------------|---------------------------------------------------------------------------------------------------------------------------|--------------------|--------------------------------------------------------------------------|
| selinux_key_permission           | {dynamic, input} -> {static, monitor}          | perm                 | /security/selinux/hooks.c:5791                                                                                            |                    |                                                                          | 
| selinux_sem_semctl               | {dynamic, input} -> {static, monitor}          | cmd                  | /security/selinux/hooks.c:1594, /security/selinux/hooks.c:5136                                                            |                    | flow x2 (2 sinks}. Implicit flows to sbj/obj, covered by op endorsement. |
| selinux_sem_alloc_security       | {external, op} -> {monitor, obj}               | GFP_ZERO             | /security/selinux/hooks.c:5399                                                                                            |                    |                                                                          |
| selinux_shm_shmctl               | {dynamic, input} -> {static, monitor}          | cmd                  | /security/selinux/hooks.c:1594, /security/selinux/hooks.c:5136                                                            |                    | flow x2 (2 sinks}. Implicit flows to sbj/obj, covered by op endorsement. |
| selinux_shm_alloc_security       | {external, op} -> {monitor, obj}               | GFP_ZERO             | /security/selinux/hooks.c:5399                                                                                            |                    |                                                                          |
| selinux_msg_queue_msgctl         | {dynamic, input} -> {static, monitor}          | cmd                  | /security/selinux/hooks.c:1594, /security/selinux/hooks.c:5136                                                            |                    | flow x2 (2 sinks}. Implicit flows to sbj/obj, covered by op endorsement. |
| selinux_msg_queue_alloc_security | {external, op} -> {monitor, obj}               | GFP_ZERO             | /security/selinux/hooks.c:5399                                                                                            |                    |                                                                          |
| selinux_ipc_permission           | {external} -> {monitor}                        | S_IRUGO              | /security/selinux/hooks.c:5136                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_ipc_permission           | {dynamic, input} -> {static, monitor}          | flag                 | /security/selinux/hooks.c:5136                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_ipc_permission           | {external} -> {monitor}                        | S_IWUGO              | /security/selinux/hooks.c:5136                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_task_kill                | {dynamic, input} -> {static, monitor}          | sig                  | /security/selinux/hooks.c:5366 /security/selinux/hooks.c:1544                                                             |                    |                                                                          |
| selinux_task_kill                | {dynamic, input} -> {static, monitor}          | secid                | /security/selinux/hooks.c:5366 /security/selinux/hooks.c:1544                                                             |                    |                                                                          |
| selinux_task_kill                | {dynamic, input} -> {static, monitor}          | perm                 | /security/selinux/hooks.c:5366 /security/selinux/hooks.c:1544                                                             |                    |                                                                          |
| selinux_file_open                | {dynamic, input} -> {static, monitor}          | file->f_mode         | /security/selinux/hooks.c:1617                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_open                | {dynamic, input} -> {static, monitor}          | file->f_flags        | /security/selinux/hooks.c:1617                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_open                | {external} -> {monitor}                        | FMODE_READ           | /security/selinux/hooks.c:1617                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_open                | {external} -> {monitor}                        | FMODE_WRITE          | /security/selinux/hooks.c:1617                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_open                | {external} -> {monitor}                        | O_APPEND             | /security/selinux/hooks.c:1617                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_receive             | {dynamic, input} -> {static, monitor}          | file->f_mode         | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_receive             | {dynamic, input, obj} -> {static, monitor, op} | file->inode          | /security/selinux/hooks.c:1617                                                                                            | :heavy_check_mark: | implicit check on av covered by other op endorsements                    |
| selinux_file_receive             | {external} -> {monitor}                        | FMODE_READ           | /security/selinux/hooks.c:1617                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_receive             | {external} -> {monitor}                        | FMODE_WRITE          | /security/selinux/hooks.c:1617                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_receive             | {external} -> {monitor}                        | O_APPEND             | /security/selinux/hooks.c:1617                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_send_sigiotask      | {dynamic, input} -> {static, monitor}          | signum               | /security/selinux/hooks.c:3408                                                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_SETFL              | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_SETOWN             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_SETSIG             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_GETFL              | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_GETOWN             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_GETSIG             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_GETOWNER_UIDS      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_GETLK              | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_SETLK              | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_SETLKW             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_OFD_GETLK          | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_OFD_SETLK          | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_OFD_SETLKW         | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_GETLK64            | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | F_SETLKW64           | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | O_APPEND             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | O_APPEND             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_fcntl               | {external} -> {monitor}                        | BITS_PER_LONG        | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_mmap_file                | {external} -> {monitor}                        | PROT_EXEC            | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: | accounts for 2 flows                                                     |
| selinux_mmap_file                | {external} -> {monitor}                        | PROT_WRITE           | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: | accounts for 2 flows                                                     |
| selinux_mmap_file                | {external} -> {monitor}                        | MAP_TYPE             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_mmap_file                | {external} -> {monitor}                        | MAP_SHARED           | /security/selinux/hooks.c:1617                                                                                            |                    |                                                                          |
| selinux_mmap_file                | {external} -> {monitor}                        | prot                 | /security/selinux/hooks.c:1617                                                                                            |                    |                                                                          |
| selinux_mmap_file                | {external} -> {monitor}                        | flags                | /security/selinux/hooks.c:1617                                                                                            |                    |                                                                          |
| selinux_mmap_file                | {external} -> {monitor}                        | reqprot              | /security/selinux/hooks.c:1617                                                                                            |                    |                                                                          |
| selinux_file_ioctl               | {external, op} -> {monitor, obj}               | cmd                  | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1579                            |                    | 5 contextual locations for sink call, caused this flow to appear 5 times |
| selinux_file_ioctl               | {external, op} -> {monitor, obj}               | cmd                  | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1579                            |                    |                                                                          |
| selinux_file_ioctl               | {input, op} -> {monitor, obj}                  | cmd                  | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            |                    |                                                                          |
| selinux_file_ioctl               | {external} -> {monitor}                        | SECURITY_CAP_AUDIT   | /security/selinux/hooks.c:1579                                                                                            |                    | flow x2                                                                  |
| selinux_file_ioctl               | {external} -> {monitor}                        | CAP_SYS_TTY_CONFIG   | /security/selinux/hooks.c:1579                                                                                            |                    | flow x2                                                                  |
| selinux_file_permission          | {external} -> {monitor}                        | MAY_READ             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_permission          | {dynamic, input} -> {static, monitor}          | mask                 | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_permission          | {input->mediator, dynamic->static, obj->op}    | file->f_flags        | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_permission          | {external} -> {monitor}                        | MAY_READ             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_permission          | {external} -> {monitor}                        | MAY_WRITE            | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_permission          | {external} -> {monitor}                        | MAY_WRITE            | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_permission          | {external} -> {monitor}                        | MAY_WRITE            | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_permission          | {external} -> {monitor}                        | MAY_APPEND           | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_permission          | {external} -> {monitor}                        | MAY_EXEC             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_permission          | {external} -> {monitor}                        | MAY_EXEC             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_permission          | {external} -> {monitor}                        | O_APPEND             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_permission          | {external} -> {monitor}                        | MAY_APPEND           | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_permission          | {external} -> {monitor}                        | MAY_READ             | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_permission          | {external} -> {monitor}                        | MAY_WRITE            | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_getsecurity        | {external} -> {monitor}                        | SECURITY_CAP_AUDIT   | /security/selinux/hooks.c:1579                                                                                            |                    | accounts for 2 flows                                                     |
| selinux_inode_getsecurity        | {external->mediator, op->obj}                  | SECURITY_CAP_AUDIT   | /security/selinux/hooks.c:1579                                                                                            |                    | accounts for 2 flows                                                     |
| selinux_inode_getsecurity        | {external->mediator, op->obj}                  | SECURITY_CAP_NOAUDIT | /security/selinux/hooks.c:1579                                                                                            |                    | accounts for 2 flows                                                     |
| selinux_inode_getsecurity        | {external} -> {monitor}                        | CAP_MAC_ADMIN        | /security/selinux/hooks.c:1579                                                                                            |                    |                                                                          |
| selinux_inode_getsecurity        | {external} -> {monitor}                        | XATTR_SELINUX_SUFFIX | /security/selinux/hooks.c:1579                                                                                            |                    | accounts for 2 flows                                                     |
| selinux_capable                  | {external} -> {monitor}                        | SECURITY_CAP_AUDIT   | /security/selinux/hooks.c:1579                                                                                            |                    | accounts for 2 flows                                                     |
| selinux_capable                  | {external->mediator, op->obj}                  | SECURITY_CAP_AUDIT   | /security/selinux/hooks.c:1579                                                                                            |                    | accounts for 2 flows                                                     |
| selinux_capable                  | {external->mediator, op->obj}                  | SECURITY_CAP_NOAUDIT | /security/selinux/hooks.c:1579                                                                                            |                    | accounts for 2 flows                                                     |
| selinux_capable                  | {input->mediator, dynamic->static}             | CAP_MAC_ADMIN        | /security/selinux/hooks.c:1579                                                                                            |                    |                                                                          |
| selinux_capable                  | {input->mediator, dynamic->static}             | XATTR_SELINUX_SUFFIX | /security/selinux/hooks.c:1579                                                                                            |                    | accounts for 2 flows                                                     |
| selinux_quotactl                 | {input->mediator,  op->sbj}                    | cmds                 | /security/selinux/hooks.c:1866                                                                                            |                    |                                                                          |
| selinux_quotactl                 | {input->mediator,  op->obj}                    | cmds                 | /security/selinux/hooks.c:1866                                                                                            |                    |                                                                          |
| selinux_syslog                   | {input->mediator,  op->sbj}                    | type                 | /security/selinux/hooks.c:1594                                                                                            |                    |                                                                          |
| selinux_netlink_send             | {dynamic->static}                              | perm                 | /security/selinux/hooks.c:3976                                                                                            |                    |                                                                          |
| selinux_vm_enough_memory         | {external->mediator}                           |                      | /security/selinux/hooks.c:1579                                                                                            |                    |                                                                          |
| selinux_vm_enough_memory         | {external->mediator}                           |                      | /security/selinux/hooks.c:1579                                                                                            |                    |                                                                          |
| selinux_vm_enough_memory         | {external->mediator}                           |                      | /security/selinux/hooks.c:1579                                                                                            |                    |                                                                          |
| selinux_vm_enough_memory         | {external->mediator}                           |                      | /security/selinux/hooks.c:1579                                                                                            |                    |                                                                          |
| selinux_vm_enough_memory         | {external->mediator}                           |                      | /security/selinux/hooks.c:1579                                                                                            |                    |                                                                          |
| selinux_vm_enough_memory         | {external->mediator}                           |                      | /security/selinux/hooks.c:1579                                                                                            |                    |                                                                          |
| selinux_vm_enough_memory         | {input->mediator,  op->sbj}                    |                      | /security/selinux/hooks.c:1579                                                                                            |                    |                                                                          |
| selinux_vm_enough_memory         | {input->mediator,  op->obj}                    |                      | /security/selinux/hooks.c:1579                                                                                            |                    |                                                                          |
| selinux_vm_enough_memory         | {input->mediator,  op->obj}                    |                      | /security/selinux/hooks.c:1579                                                                                            |                    | accounts for 4 flows                                                     |
| selinux_mount                    | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1866                                                            |                    | accounts for 3 flows                                                     |
| selinux_mount                    | {external->mediator,  op->sbj}                 |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1866                                                            |                    | accounts for 2 flows                                                     |
| selinux_mount                    | {external->mediator,  op->sbj}                 |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1866                                                            |                    |                                                                          |
| selinux_inode_rename             | {input->mediator, dynamic->static, obj->op}    | new_dentry->d_inode  | /security/selinux/hooks.c:1840, /security/selinux/hooks.c:1846                                                            |                    |                                                                          |
| selinux_inode_rename             | {input->mediator, dynamic->static, obj->op}    |                      | /security/selinux/hooks.c:1840, /security/selinux/hooks.c:1846                                                            |                    |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_READ             | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_WRITE            | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_EXEC             | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_APPEND           | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_ACCESS           | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_EXEC             | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_READ             | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_APPEND           | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_WRITE            | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_EXEC             | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_WRITE            | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_READ             | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_READ             | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator}                           | MAY_WRITE            | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {input->mediator, dynamic->static, obj->op}    | inode->i_mode        | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {input->mediator, dynamic->static, obj->op}    | inode->i_mode        | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {input->mediator, dynamic->static}             | mask                 | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {input->mediator, dynamic->static}             | inode->i_mode        | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator,  op->sbj}                 | inode->i_mode        | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_inode_permission         | {external->mediator,  op->obj}                 | inode->i_mode        | /security/selinux/hooks.c:2865                                                                                            | :heavy_check_mark: |                                                                          |
| selinux_file_mprotect            | {external->mediator,  op->sbj}                 |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator,  op->sbj}                 |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator,  op->sbj}                 |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator,  op->sbj}                 |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator,  op->sbj}                 |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator}                           |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator}                           |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator}                           |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator}                           |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator}                           |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator}                           |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {external->mediator}                           |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {input->mediator, dynamic->static}             |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {input->mediator, dynamic->static}             |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_file_mprotect            | {input->mediator, dynamic->static, obj->op}    |                      | /security/selinux/hooks.c:1617, /security/selinux/hooks.c:1684, /security/selinux/hooks.c:1508                            |                    |                                                                          |
| selinux_inode_setattr            | {external->mediator}                           |                      | /security/selinux/hooks.c:1617                                                                                            |                    |                                                                          |
| selinux_inode_setattr            | {external->mediator}                           |                      | /security/selinux/hooks.c:1617                                                                                            |                    |                                                                          |
| selinux_inode_setattr            | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:1617                                                                                            |                    |                                                                          |
| selinux_inode_setattr            | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:1617                                                                                            |                    |                                                                          |
| selinux_inode_setattr            | {external->mediator,  op->sbj}                 |                      | /security/selinux/hooks.c:1617                                                                                            |                    |                                                                          |
| selinux_inode_setattr            | {input->mediator,  op->obj}                    |                      | /security/selinux/hooks.c:1617                                                                                            |                    |                                                                          |
| selinux_inode_setattr            | {input->mediator,  op->obj}                    |                      | /security/selinux/hooks.c:1617                                                                                            |                    |                                                                          |
| selinux_inode_setattr            | {input->mediator,  op->sbj}                    |                      | /security/selinux/hooks.c:1617                                                                                            |                    |                                                                          |
| selinux_inode_setattr            | {input->mediator,  dynamic->static}            |                      | /security/selinux/hooks.c:1617                                                                                            |                    |                                                                          |
| selinux_inode_setxattr           | {external->mediator,  op->sbj}                 |                      | /security/selinux/hooks.c:2957, /security/selinux/hooks.c:2993, /security/selinux/hooks.c:3003                            |                    | accounts for 9 flows                                                     |
| selinux_inode_setxattr           | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:2957, /security/selinux/hooks.c:2993, /security/selinux/hooks.c:3003                            |                    | accounts for 9 flows                                                     |
| selinux_inode_setxattr           | {external->mediator,  op->sbj}                 |                      | /security/selinux/hooks.c:2957, /security/selinux/hooks.c:2993, /security/selinux/hooks.c:3003                            |                    | accounts for 9 flows                                                     |
| selinux_inode_setxattr           | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:2957, /security/selinux/hooks.c:2993, /security/selinux/hooks.c:3003                            |                    | accounts for 9 flows                                                     |
| selinux_inode_setxattr           | {external->mediator,  op->sbj}                 |                      | /security/selinux/hooks.c:2957, /security/selinux/hooks.c:2993, /security/selinux/hooks.c:3003                            |                    | accounts for 9 flows                                                     |
| selinux_inode_setxattr           | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:2957, /security/selinux/hooks.c:2993, /security/selinux/hooks.c:3003                            |                    | accounts for 9 flows                                                     |
| selinux_inode_setxattr           | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:2957, /security/selinux/hooks.c:2993, /security/selinux/hooks.c:3003                            |                    | accounts for 9 flows                                                     |
| selinux_inode_setxattr           | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:2957, /security/selinux/hooks.c:2993, /security/selinux/hooks.c:3003                            |                    | accounts for 9 flows                                                     |
| selinux_set_mnt_opts             | {external->mediator,  op->sbj}                 |                      | /security/selinux/hooks.c:2957, /security/selinux/hooks.c:2993, /security/selinux/hooks.c:3003                            |                    | accounts for 9 flows                                                     |
| selinux_set_mnt_opts             | {external->mediator,  op->obj}                 |                      | /security/selinux/hooks.c:2957, /security/selinux/hooks.c:2993, /security/selinux/hooks.c:3003                            |                    | accounts for 9 flows                                                     |
| selinux_setprocattr              | {external} -> {monitor}                        |                      | /security/selinux/hooks.c:5661, /security/selinux/hooks.c:5676                                                                  |                    | flow x2                                                                         |


### Multiple Authorization


#### selinux_msg_queue_msgsnd

Original Hook

~~~
static int selinux_msg_queue_msgsnd(struct msg_queue *msq, struct msg_msg *msg, int msqflg)
{
	struct ipc_security_struct *isec;
	struct msg_security_struct *msec;
	struct common_audit_data ad;
	u32 sid = current_sid();
	int rc;

	isec = msq->q_perm.security;
	msec = msg->security;

	/*
	 * First time through, need to assign label to the message
	 */
	if (msec->sid == SECINITSID_UNLABELED) {
		/*
		 * Compute new sid based on current process and
		 * message queue this message will be stored in
		 */
		rc = security_transition_sid(sid, isec->sid, SECCLASS_MSG,
					     NULL, &msec->sid);
		if (rc)
			return rc;
	}

	ad.type = LSM_AUDIT_DATA_IPC;
	ad.u.ipc_id = msq->q_perm.key;

	/* Can this process write to the queue? */
	rc = avc_has_perm(sid, isec->sid, SECCLASS_MSGQ,
			  MSGQ__WRITE, &ad);
	if (!rc)
		/* Can this process send the message */
		rc = avc_has_perm(sid, msec->sid, SECCLASS_MSG,
				  MSG__SEND, &ad);
	if (!rc) // ←- This conditional would be moved to the callers of this hook
		/* Can the message be put in the queue? */
		rc = avc_has_perm(msec->sid, isec->sid, SECCLASS_MSGQ,
				  MSGQ__ENQUEUE, &ad);
		// Above avc_has_perm would be in separate function with appropriate setup

	return rc;
}
~~~

The last sink authorization function call to avc_has_perm checks whether a message can be placed into the 
message queue. This call will need to be placed into a different function in order to separate the hook
into two hooks.


#### New selinux_msg_queue_msgsnd w/out final check

~~~
static int selinux_msg_queue_msgsnd(struct msg_queue *msq, struct msg_msg *msg, int msqflg)
{
	struct ipc_security_struct *isec;
	struct msg_security_struct *msec;
	struct common_audit_data ad;
	u32 sid = current_sid();
	int rc;

	isec = msq->q_perm.security;
	msec = msg->security;

	/*
	 * First time through, need to assign label to the message
	 */
	if (msec->sid == SECINITSID_UNLABELED) {
		/*
		 * Compute new sid based on current process and
		 * message queue this message will be stored in
		 */
		rc = security_transition_sid(sid, isec->sid, SECCLASS_MSG,
					     NULL, &msec->sid);
		if (rc)
			return rc;
	}

	ad.type = LSM_AUDIT_DATA_IPC;
	ad.u.ipc_id = msq->q_perm.key;

	/* Can this process write to the queue? */
	rc = avc_has_perm(sid, isec->sid, SECCLASS_MSGQ,
			  MSGQ__WRITE, &ad);
	if (!rc)
		/* Can this process send the message */
		rc = avc_has_perm(sid, msec->sid, SECCLASS_MSG,
				  MSG__SEND, &ad);

	return rc;
}
~~~

#### Secondary hook with only message -> msgqueue check

~~~
static int selinux_msg_queue_msgsnd_msg_auth(struct msg_queue *msq, struct msg_msg *msg)
{
	int rc;

	struct ipc_security_struct *isec;
	struct msg_security_struct *msec;
	struct common_audit_data ad;
	
	isec = msq->q_perm.security;
	msec = msg->security;

	ad.type = LSM_AUDIT_DATA_IPC;
	ad.u.ipc_id = msq->q_perm.key;

	rc = avc_has_perm(msec->sid, isec->sid, SECCLASS_MSGQ,
				  MSGQ__ENQUEUE, &ad);

	return rc;
}
~~~

#### Change code in hook calling location within kernel to keep same control flow semantics

~~~
long do_msgsnd(int msqid, long mtype, void __user *mtext,
size_t msgsz, int msgflg)
{
int err2;
…

	// Line 651
	
	err = security_msg_queue_msgsnd(msq, msg, msgflg);
	if (err)
		goto out_unlock0;
	else{
		err2 = security_msg_queue_msgsnd_msg_auth(msq, msg)
		if (err2)
			goto out_unlock0;
	}

}
~~~

Finally, we need to add the newly created hook into the security options for both the LSM default hook, 
and into the security options table within SELinux. This way a default hook can be called, which will call into
SELinux, assuming SELinux is enabled on the system.