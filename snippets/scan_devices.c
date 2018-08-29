#include "scan_devices.h"
#include "ow/ow.h"
#include "stdio.h"

/**
 * \brief           Scan for 1-Wire devices on specific 1-Wire port
 * \param[in]       ow: 1-Wire handle
 * \param[in]       rom_ids: Points to array of 8-bytes entries for ROM addresses
 * \param[in]       rtf: Number of roms to find
 * \param[out]      rf: Number of roms found after scan
 */
owr_t
scan_onewire_devices(ow_t* ow, ow_rom_t* rom_ids, size_t rtf, size_t* rf) {
    owr_t res;
    size_t len = 0;

    ow_protect(ow, 1);
    res = ow_search_reset_raw(ow);
    while (res == owOK && (res = ow_search_raw(ow, &rom_ids[len])) == owOK) {
        len++;
        if (len == rtf) {                       /* Did we reach end of array? */
        	printf("ROM array is full! Stop scanning for more devices\r\n");
        	break;
        }
    }
    ow_unprotect(ow, 1);

    if (len > 0) {
        /* Print all devices */
        for (size_t i = 0; i < len; i++) {
            printf("Device ROM addr: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
                rom_ids[i].rom[0], rom_ids[i].rom[1], rom_ids[i].rom[2], rom_ids[i].rom[3],
                rom_ids[i].rom[4], rom_ids[i].rom[5], rom_ids[i].rom[6], rom_ids[i].rom[7]
            );
        }
    }
    if (rf != NULL) {
    	*rf = len;
    }
    if (res == owERRNODEV && len) {
    	return owOK;
    }
    return res;
}
