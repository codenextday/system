#ifndef UVC_CLIENT_API_H
#define UVC_CLIENT_API_H

#include <sys/msg.h>   // message queue


#define IMI_ON  1
#define IMI_OFF 0
#define IMI_WIDTH  640
#define IMI_HEIGHT 360
#define IMI_UVC_SHARED_MMAP (IMI_HEIGHT * IMI_WIDTH * 2)
#define IMI_UVC_DISPLAY_ENABLE 0
struct uvc_device {
	int shmid;
	int semid;
	int msqid;
	struct shared_buffer *share_buf_mmap;
};

//struct uvc_device *IMI_UVC_DEV;

// message queue
struct msg_form {
	long mtype;
	char mtext;
};

struct shared_buffer {
	char start[IMI_UVC_SHARED_MMAP];
	size_t length;
	int read_write_flag;
};

// union, semctl init
union semun {
	int val;    // for SETVAL
	struct semid_ds *buf;
	unsigned short *array;
};

int uvc_display_init();
int deinit_shared_memory();
int sem_p(int sem_id);
int sem_v(int sem_id);


#endif

