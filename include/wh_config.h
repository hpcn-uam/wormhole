#ifndef __WH_CONF__H__
#define __WH_CONF__H__

//Basic defines for processing other defines
#define HBC_xstr(a) HBC_str(a)
#define HBC_str(a) #a

//The current version
#define wh_VERSION_trash 0.4-5-g8b982b7

#define wh_VERSION HBC_xstr(wh_VERSION_trash)

//Is debug on?
#define EINSTEIN_DEBUG
#define WORMS_DEBUG

#endif
