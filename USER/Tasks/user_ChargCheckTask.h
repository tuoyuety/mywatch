#ifndef __USER_CHARGCHECKTASK_H__
#define __USER_CHARGCHECKTASK_H__

#ifdef __cplusplus
extern "C" {
#endif

void ChargPageEnterTask(void *argument);

/** 亮屏/从 Stop 唤醒等之后调用，强制按当前 PA2 与页面栈重新对齐充电页 */
void ChargeUi_NotifyResync(void);

#ifdef __cplusplus
}
#endif

#endif

