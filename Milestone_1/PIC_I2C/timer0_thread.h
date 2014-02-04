typedef struct __timer0_thread_struct {
	int	data;
} timer0_thread_struct;

void init_timer0_lthread(timer0_thread_struct *);
int timer0_lthread(timer0_thread_struct *,int,int,unsigned char*);
