/*
 * Copyright (C) 2008 Google, Inc.
 *
 * Based on, but no longer compatible with, the original
 * OpenBinder.org binder driver interface, which is:
 *
 * Copyright (c) 2005 Palmsource, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _UAPI_LINUX_BINDER_H
#define _UAPI_LINUX_BINDER_H

#include <linux/ioctl.h>

#define B_PACK_CHARS(c1, c2, c3, c4) \
	((((c1)<<24)) | (((c2)<<16)) | (((c3)<<8)) | (c4))
#define B_TYPE_LARGE 0x85

enum {
	BINDER_TYPE_BINDER	= B_PACK_CHARS('s', 'b', '*', B_TYPE_LARGE),
	BINDER_TYPE_WEAK_BINDER	= B_PACK_CHARS('w', 'b', '*', B_TYPE_LARGE),
	BINDER_TYPE_HANDLE	= B_PACK_CHARS('s', 'h', '*', B_TYPE_LARGE),
	BINDER_TYPE_WEAK_HANDLE	= B_PACK_CHARS('w', 'h', '*', B_TYPE_LARGE),
	BINDER_TYPE_FD		= B_PACK_CHARS('f', 'd', '*', B_TYPE_LARGE),
	BINDER_TYPE_FDA		= B_PACK_CHARS('f', 'd', 'a', B_TYPE_LARGE),
	BINDER_TYPE_PTR		= B_PACK_CHARS('p', 't', '*', B_TYPE_LARGE),
};

enum {
	FLAT_BINDER_FLAG_PRIORITY_MASK = 0xff,
	FLAT_BINDER_FLAG_ACCEPTS_FDS = 0x100,
};

/**
 * struct binder_object_header - header shared by all binder metadata objects
 * @type:	type of the object
 *
 * Every binder object starts with a 4-byte type field. The kernel reads
 * this field to determine the object type, then casts to the appropriate
 * struct. The legacy flat_binder_object begins with type+flags directly;
 * the SG-era structs (binder_fd_object, binder_buffer_object,
 * binder_fd_array_object) embed this header as their first member.
 */
struct binder_object_header {
	__u32	type;
};

#ifdef BINDER_IPC_32BIT
typedef __u32 binder_size_t;
typedef __u32 binder_uintptr_t;
#else
typedef __u64 binder_size_t;
typedef __u64 binder_uintptr_t;
#endif

/*
 * This is the flattened representation of a Binder object for transfer
 * between processes.  The 'offsets' supplied as part of a binder transaction
 * contains offsets into the data where these structures occur.  The Binder
 * driver takes care of re-writing the structure type and data as it moves
 * between processes.
 */
struct flat_binder_object {
	/* 8 bytes for large_flat_header. */
	__u32		type;
	__u32		flags;

	/* 8 bytes of data. */
	union {
		binder_uintptr_t	binder;	/* local object */
		__u32			handle;	/* remote object */
	};

	/* extra data associated with local object */
	binder_uintptr_t	cookie;
};

/**
 * struct binder_fd_object - describes a file descriptor to be transferred
 * @hdr:	binder object header
 * @pad_flags:	padding (must be zero)
 * @pad_binder:	union with @fd (padding, must be zero when @fd is used)
 * @fd:		file descriptor (in the @pad_binder union, low 32 bits)
 * @cookie:	optional cookie (unused for pure fd transfer)
 *
 * This is the SG-enabled form for BINDER_TYPE_FD. The legacy
 * flat_binder_object also carries fds via BINDER_TYPE_FD using its
 * handle/binder fields; the kernel accepts both forms.
 */
struct binder_fd_object {
	struct binder_object_header	hdr;
	__u32				pad_flags;
	union {
		binder_uintptr_t	pad_binder;
		__u32			fd;
	};
	binder_uintptr_t		cookie;
};

/**
 * struct binder_buffer_object - describes a userspace buffer to be transferred
 * @hdr:	binder object header
 * @flags:	one or more BINDER_BUFFER_FLAG_* values
 * @buffer:	userspace pointer to the buffer
 * @length:	size of the buffer in bytes
 * @parent:	index in the offset array pointing to the parent buffer
 * @parent_offset:offset within the parent buffer where this buffer's pointer
 *		should be fixed up
 *
 * Carries an arbitrary userspace buffer that the driver copies into the
 * transaction's extra (scatter-gather) buffer area, fixing up the pointer so
 * the recipient sees a valid address in its own address space. Parent/child
 * links allow nesting (e.g. an array of fds living inside a parent buffer).
 */
struct binder_buffer_object {
	struct binder_object_header	hdr;
	__u32				flags;
	binder_uintptr_t		buffer;
	binder_size_t			length;
	binder_size_t			parent;
	binder_size_t			parent_offset;
};

/**
 * struct binder_fd_array_object - an array of fds embedded in a parent buffer
 * @hdr:	binder object header
 * @pad:	padding (must be zero)
 * @num_fds:	number of file descriptors in the array
 * @parent:	index in the offset array of the parent binder_buffer_object
 * @parent_offset:offset within the parent buffer where the fd array begins
 *
 * The driver walks the array, translates each fd into the target process's
 * fd table, and writes the translated fds back into the (already-fixup'd)
 * parent buffer.
 */
struct binder_fd_array_object {
	struct binder_object_header	hdr;
	__u32				pad;
	binder_size_t			num_fds;
	binder_size_t			parent;
	binder_size_t			parent_offset;
};

enum {
	BINDER_BUFFER_FLAG_HAS_PARENT = 0x01,
};

/*
 * On 64-bit platforms where user code may run in 32-bits the driver must
 * translate the buffer (and local binder) addresses appropriately.
 */

struct binder_write_read {
	binder_size_t		write_size;	/* bytes to write */
	binder_size_t		write_consumed;	/* bytes consumed by driver */
	binder_uintptr_t	write_buffer;
	binder_size_t		read_size;	/* bytes to read */
	binder_size_t		read_consumed;	/* bytes consumed by driver */
	binder_uintptr_t	read_buffer;
};

/* Use with BINDER_VERSION, driver fills in fields. */
struct binder_version {
	/* driver protocol version -- increment with incompatible change */
	__s32       protocol_version;
};

/* This is the current protocol version. */
#ifdef BINDER_IPC_32BIT
#define BINDER_CURRENT_PROTOCOL_VERSION 7
#else
#define BINDER_CURRENT_PROTOCOL_VERSION 8
#endif

#define BINDER_WRITE_READ		_IOWR('b', 1, struct binder_write_read)
#define BINDER_SET_IDLE_TIMEOUT		_IOW('b', 3, __s64)
#define BINDER_SET_MAX_THREADS		_IOW('b', 5, __u32)
#define BINDER_SET_IDLE_PRIORITY	_IOW('b', 6, __s32)
#define BINDER_SET_CONTEXT_MGR		_IOW('b', 7, __s32)
#define BINDER_THREAD_EXIT		_IOW('b', 8, __s32)
#define BINDER_VERSION			_IOWR('b', 9, struct binder_version)

/*
 * NOTE: Two special error codes you should check for when calling
 * in to the driver are:
 *
 * EINTR -- The operation has been interupted.  This should be
 * handled by retrying the ioctl() until a different error code
 * is returned.
 *
 * ECONNREFUSED -- The driver is no longer accepting operations
 * from your process.  That is, the process is being destroyed.
 * You should handle this by exiting from your process.  Note
 * that once this error code is returned, all further calls to
 * the driver from any thread will return this same code.
 */

enum transaction_flags {
	TF_ONE_WAY	= 0x01,	/* this is a one-way call: async, no return */
	TF_ROOT_OBJECT	= 0x04,	/* contents are the component's root object */
	TF_STATUS_CODE	= 0x08,	/* contents are a 32-bit status code */
	TF_ACCEPT_FDS	= 0x10,	/* allow replies with file descriptors */
};

struct binder_transaction_data {
	/* The first two are only used for bcTRANSACTION and brTRANSACTION,
	 * identifying the target and contents of the transaction.
	 */
	union {
		/* target descriptor of command transaction */
		__u32	handle;
		/* target descriptor of return transaction */
		binder_uintptr_t ptr;
	} target;
	binder_uintptr_t	cookie;	/* target object cookie */
	__u32		code;		/* transaction command */

	/* General information about the transaction. */
	__u32	        flags;
	pid_t		sender_pid;
	uid_t		sender_euid;
	binder_size_t	data_size;	/* number of bytes of data */
	binder_size_t	offsets_size;	/* number of bytes of offsets */

	/* If this transaction is inline, the data immediately
	 * follows here; otherwise, it ends with a pointer to
	 * the data buffer.
	 */
	union {
		struct {
			/* transaction data */
			binder_uintptr_t	buffer;
			/* offsets from buffer to flat_binder_object structs */
			binder_uintptr_t	offsets;
		} ptr;
		__u8	buf[8];
	} data;
};

struct binder_ptr_cookie {
	binder_uintptr_t ptr;
	binder_uintptr_t cookie;
};

/**
 * struct binder_transaction_data_sg - transaction data with scatter-gather
 * @transaction_data:	the base transaction data (same layout as
 *			binder_transaction_data)
 * @buffers_size:	total size of the extra (scatter-gather) buffer area
 *			that follows the offsets array. Contains the sum of
 *			all BINDER_TYPE_PTR buffer lengths referenced from
 *			the offsets array.
 *
 * Used by BC_TRANSACTION_SG / BC_REPLY_SG. The kernel allocates
 * @buffers_size additional bytes in the target buffer (after the offsets
 * array) and copies each BINDER_TYPE_PTR buffer into it, fixing up pointers.
 */
struct binder_transaction_data_sg {
	struct binder_transaction_data transaction_data;
	binder_size_t buffers_size;
};

struct binder_handle_cookie {
	__u32 handle;
	binder_uintptr_t cookie;
} __packed;

struct binder_pri_desc {
	__s32 priority;
	__u32 desc;
};

struct binder_pri_ptr_cookie {
	__s32 priority;
	binder_uintptr_t ptr;
	binder_uintptr_t cookie;
};

enum binder_driver_return_protocol {
	BR_ERROR = _IOR('r', 0, __s32),
	/*
	 * int: error code
	 */

	BR_OK = _IO('r', 1),
	/* No parameters! */

	BR_TRANSACTION = _IOR('r', 2, struct binder_transaction_data),
	BR_REPLY = _IOR('r', 3, struct binder_transaction_data),
	/*
	 * binder_transaction_data: the received command.
	 */

	BR_ACQUIRE_RESULT = _IOR('r', 4, __s32),
	/*
	 * not currently supported
	 * int: 0 if the last bcATTEMPT_ACQUIRE was not successful.
	 * Else the remote object has acquired a primary reference.
	 */

	BR_DEAD_REPLY = _IO('r', 5),
	/*
	 * The target of the last transaction (either a bcTRANSACTION or
	 * a bcATTEMPT_ACQUIRE) is no longer with us.  No parameters.
	 */

	BR_TRANSACTION_COMPLETE = _IO('r', 6),
	/*
	 * No parameters... always refers to the last transaction requested
	 * (including replies).  Note that this will be sent even for
	 * asynchronous transactions.
	 */

	BR_INCREFS = _IOR('r', 7, struct binder_ptr_cookie),
	BR_ACQUIRE = _IOR('r', 8, struct binder_ptr_cookie),
	BR_RELEASE = _IOR('r', 9, struct binder_ptr_cookie),
	BR_DECREFS = _IOR('r', 10, struct binder_ptr_cookie),
	/*
	 * void *:	ptr to binder
	 * void *: cookie for binder
	 */

	BR_ATTEMPT_ACQUIRE = _IOR('r', 11, struct binder_pri_ptr_cookie),
	/*
	 * not currently supported
	 * int:	priority
	 * void *: ptr to binder
	 * void *: cookie for binder
	 */

	BR_NOOP = _IO('r', 12),
	/*
	 * No parameters.  Do nothing and examine the next command.  It exists
	 * primarily so that we can replace it with a BR_SPAWN_LOOPER command.
	 */

	BR_SPAWN_LOOPER = _IO('r', 13),
	/*
	 * No parameters.  The driver has determined that a process has no
	 * threads waiting to service incoming transactions.  When a process
	 * receives this command, it must spawn a new service thread and
	 * register it via bcENTER_LOOPER.
	 */

	BR_FINISHED = _IO('r', 14),
	/*
	 * not currently supported
	 * stop threadpool thread
	 */

	BR_DEAD_BINDER = _IOR('r', 15, binder_uintptr_t),
	/*
	 * void *: cookie
	 */
	BR_CLEAR_DEATH_NOTIFICATION_DONE = _IOR('r', 16, binder_uintptr_t),
	/*
	 * void *: cookie
	 */

	BR_FAILED_REPLY = _IO('r', 17),
	/*
	 * The the last transaction (either a bcTRANSACTION or
	 * a bcATTEMPT_ACQUIRE) failed (e.g. out of memory).  No parameters.
	 */
};

enum binder_driver_command_protocol {
	BC_TRANSACTION = _IOW('c', 0, struct binder_transaction_data),
	BC_REPLY = _IOW('c', 1, struct binder_transaction_data),
	/*
	 * binder_transaction_data: the sent command.
	 */

	BC_ACQUIRE_RESULT = _IOW('c', 2, __s32),
	/*
	 * not currently supported
	 * int:  0 if the last BR_ATTEMPT_ACQUIRE was not successful.
	 * Else you have acquired a primary reference on the object.
	 */

	BC_FREE_BUFFER = _IOW('c', 3, binder_uintptr_t),
	/*
	 * void *: ptr to transaction data received on a read
	 */

	BC_INCREFS = _IOW('c', 4, __u32),
	BC_ACQUIRE = _IOW('c', 5, __u32),
	BC_RELEASE = _IOW('c', 6, __u32),
	BC_DECREFS = _IOW('c', 7, __u32),
	/*
	 * int:	descriptor
	 */

	BC_INCREFS_DONE = _IOW('c', 8, struct binder_ptr_cookie),
	BC_ACQUIRE_DONE = _IOW('c', 9, struct binder_ptr_cookie),
	/*
	 * void *: ptr to binder
	 * void *: cookie for binder
	 */

	BC_ATTEMPT_ACQUIRE = _IOW('c', 10, struct binder_pri_desc),
	/*
	 * not currently supported
	 * int: priority
	 * int: descriptor
	 */

	BC_REGISTER_LOOPER = _IO('c', 11),
	/*
	 * No parameters.
	 * Register a spawned looper thread with the device.
	 */

	BC_ENTER_LOOPER = _IO('c', 12),
	BC_EXIT_LOOPER = _IO('c', 13),
	/*
	 * No parameters.
	 * These two commands are sent as an application-level thread
	 * enters and exits the binder loop, respectively.  They are
	 * used so the binder can have an accurate count of the number
	 * of looping threads it has available.
	 */

	BC_REQUEST_DEATH_NOTIFICATION = _IOW('c', 14,
						struct binder_handle_cookie),
	/*
	 * int: handle
	 * void *: cookie
	 */

	BC_CLEAR_DEATH_NOTIFICATION = _IOW('c', 15,
						struct binder_handle_cookie),
	/*
	 * int: handle
	 * void *: cookie
	 */

	BC_DEAD_BINDER_DONE = _IOW('c', 16, binder_uintptr_t),
	/*
	 * void *: cookie
	 */

	BC_TRANSACTION_SG = _IOW('c', 17, struct binder_transaction_data_sg),
	BC_REPLY_SG = _IOW('c', 18, struct binder_transaction_data_sg),
	/*
	 * binder_transaction_data_sg: the sent command, plus the total size of
	 * the scatter-gather buffer area.
	 */
};

#endif /* _UAPI_LINUX_BINDER_H */

