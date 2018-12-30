#ifndef __ONEWIRE_CONFIG_H
#define __ONEWIRE_CONFIG_H

/* Rename this file to "ow_config.h" for your application */

/* Enable operating system support */
#define OW_CFG_OS                           1

/* After user configuration, call default config to merge config together */
#include "ow/ow_config_default.h"

#endif /* __ONEWIRE_CONFIG_H */