#ifndef __IOCpodule_h
#define __IOCpodule_h

#define PODULE_ADDRESS(speed,slot) \
  (0x03240000 + (PODULE_SPEED_##speed << 19) + ((slot) << 14))

#define PODULE_IDx(speed,slot) \
  ((PoduleID *)(PODULE_ADDRESS(speed,slot)))

#define PODULE_ID(slot) \
  PODULE_IDx(SYNC,slot)		/* recommended speed */

#endif /* NOT __IOCpodule_h */
