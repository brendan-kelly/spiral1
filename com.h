#ifndef COM_H_
#define COM_H_

#include <stdio.h>
#include <stdlib.h>
#include "altera_up_avalon_rs232.h"
#include <altera_up_sd_card_avalon_interface.h>
#include <string.h>
#include "altera_up_avalon_audio_and_video_config.h"
#include "altera_up_avalon_audio.h"
#include "audio.h"
#include "SD_Card.h"


alt_up_rs232_dev * SetupRS(void);

char ** send_playlist(alt_up_rs232_dev* uart);

void get_audio_info(alt_up_rs232_dev* uart, alt_up_audio_dev* audiohandle,unsigned int *AudioBufferArray[], short int filehandle);


#endif /* AUDIO_H_ */

