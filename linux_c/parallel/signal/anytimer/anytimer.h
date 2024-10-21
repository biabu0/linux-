#ifndef ANYTIMER_H__
#define ANYTIMER_H__

#define JOB_MAX 1024
typedef void at_jobfunc_t(void *);

int at_addjob(int sec, at_jobfunc_t *jobp, void *arg);
/*
 *return >= 0       成功，返回任务ID
 *       == -EINVAL 失败，参数非法
 *       == -ENOSPC 失败，数组满
 *       == -ENOMEM 失败，内存空间不足
 **/


//周期性调用
int at_addjob_repeat(int ,at_jobfunc_t *, void *);
/*
 *          0
 *          -EINVAL     
 *          -ENOSPC
 *          -ENOMEM
 **/
int at_canceljob(int id);
/*
 *return == 0       成功
 *       == -EINVAL 失败，参数非法
 *       == -EBUSY  失败，指定任务已经完成
 *       == -ECANCELED  失败，指定任务重复取消
 */
int at_waitjob(int job);
/*收尸
 *return    == 0    成功释放
 *          ==  -EINVAL     参数非法             
 */
//at_pausejob();                //暂停
//at_resumejob();               //重启



#endif
