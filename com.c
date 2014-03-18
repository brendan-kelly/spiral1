// Send audio list
// This function will wait for the android device to connect
// to the de2, once that happens it will send the list of
// files(songs) to the android

/* Playback
 *
 * When the android receives the song list, it will send back
 * 4 bytes containing the playback data to the de2.
 * ____ ________ _______ _____
 * Type	Function Track # Other
 * This function takes this 4 byte data and uses cases to decide:
 * Type: 0 - if its going to be a play/pause etc command
 * 		 1 - song data
 * Function: 0 - Select, play right away
 * 			 1 - Pause
 * 			 2 - Resume
 * 			 3 - Volume
 * 			 4 - Seek
 * 			 5 - Shuffle on
 * 			 6 - Shuffle off
 * 			 7 - Repeat on
 *			 8 - Repeat off
 * Track #: # for the track stored on the sd card
 * Other: Used for extra functionality in spiral 2
 * 	  	  Volume multiplier, seek multiplier
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "altera_up_avalon_rs232.h"
#include <altera_up_sd_card_avalon_interface.h>
#include <string.h>
#include "com.h"

					//'c'
#define PLAYBACK_CMD (int) 'c'
//#define PLAYBACK_CMD (int) 0
					//'t'
#define TRACK_NAME (int) 't'
//#define TRACK_NAME (int) 1
#define SONG_DATA_CMD 2

			//
#define SELECT (int) 's'
//#define SELECT (int) 0
#define PAUSE (int) 't'
//#define PAUSE (int) 1
#define RESUME 2
#define VOLUME 3
#define SEEK 4
#define SHUFFLE_ON 5
#define SHUFFLE_OFF 6
#define REPEAT_ON 7
#define REPEAT_OFF 8

#define NUMBER_OF_SONG_FILES 5

static char* FILE_NAME[NUMBER_OF_SONG_FILES] = {"test1.wav","test2.wav","test3.wav","test4.wav","test5.wav"};

void get_audio_info(alt_up_rs232_dev* uart, alt_up_audio_dev* audiohandle,unsigned int *AudioBufferArray[], short int filehandle) {
	int i;
	unsigned char data;
	unsigned char parity;



//	printf("Loading audio.\n");

	// Clear Buffer
	while(alt_up_rs232_get_used_space_in_read_FIFO(uart)){
		alt_up_rs232_read_data(uart, &data, &parity);
	}
//	printf("Clearing buffer.\n");

	// Now receive the message from the Middleman
	while(alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0)
	{
		 FillOldBuffer(AudioBufferArray, AUDIO_ARRAY_SIZE, filehandle);
	}

//	printf("Received.\n");

	// First byte is the number of characters in our message
	alt_up_rs232_read_data(uart, &data, &parity);
	int num_to_receive = (int)data;
	unsigned char info[num_to_receive];

	for(i = 0; i<num_to_receive; i++){
		while(alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0);
		alt_up_rs232_read_data(uart, &data, &parity);
		info[i]=data;
		printf("%c", data);
	} printf("\n");


	// Start with the number of bytes in our message
	unsigned char message[] = "Success!";
	alt_up_rs232_write_data(uart, (unsigned char) strlen(message));


	// Now send the actual message to the Middleman
	for(i = 0; i<strlen(message); i++){
		alt_up_rs232_write_data(uart, message[i]);
	}

	memset(message, 0, sizeof(char)*sizeof(message));

	//Playback section
/*
	for(i=0;i<num_to_receive;i++) {
			printf("%d", info[i]);
	}
	printf("\n");
*/
	int type = info[0];
	int function = info[1];

	switch(type) {
		case PLAYBACK_CMD:
			printf("Playback\n");
			switch(function) {
				case SELECT:
					printf("Play\n");
					//start playback
					StartAudio(audiohandle);
					break;
				case PAUSE:
					printf("Pause\n");
					StopAudio(audiohandle);
					break;
				case RESUME:
					printf("Resume\n");
					StartAudio(audiohandle);
					break;
				case VOLUME:
					printf("Volume\n");
					break;
				case SEEK:
					printf("Seek\n");
					break;
				case SHUFFLE_ON:
					printf("Shuffle On\n");
					break;
				case SHUFFLE_OFF:
					printf("Shuffle Off\n");
					break;
				case REPEAT_ON:
					printf("Repeat On\n");
					break;
				case REPEAT_OFF:
					printf("Repeat Off\n");
					break;
			}
			alt_up_rs232_write_data(uart, 'A');
			break;
		case TRACK_NAME:
			//stop playback
			StopAudio(audiohandle);
			//play(tracklist[0]);
	//		printf("debug %i\n",function-'0');
			//close any open file
			alt_up_sd_card_fclose(filehandle);
			//open file
			filehandle = open_file(FILE_NAME[function-'0']);
			//filehandle = open_file(FILE_NAME[function]);
			//read headder
			read_header(filehandle,FILE_NAME[function-'0']);
	//		read_header(filehandle,FILE_NAME[function]);
			//re-initalise buffers
			AudioBufferArray[0][0] = 0;
			//read into buffers
			read_into_buffer(AudioBufferArray,filehandle);
			//start playback
			StartAudio(audiohandle);
			break;
		case SONG_DATA_CMD:
			printf("Loading song data.\n");
			alt_up_rs232_write_data(uart, 'B');
			break;


	}


//	printf("Done.\n");
}

char** send_playlist(alt_up_rs232_dev* uart) {
    alt_up_sd_card_dev *device_reference = NULL;
    int connected = 0;
    device_reference = alt_up_sd_card_open_dev("/dev/SD_Card_Interface");

    if (device_reference != NULL) {
        if ((connected == 0) && (alt_up_sd_card_is_Present())) {
            printf("Card connected.\n");
            if (alt_up_sd_card_is_FAT16()) {
          //      printf("Valid FAT16 file system detected.\n");
            } else {
                printf("Unknown file system.\n");
            }
            connected = 1;
        } else if ((connected == 1) && (alt_up_sd_card_is_Present() == false)) {
        //    printf("Card disconnected.\n");
            connected = 0;
        }
    }

    char first_file[32];
    char file[32];
    char** tracklist = malloc(20 * sizeof(char*));

    int i;
    for(i = 0; i<20; i++) {
    	tracklist[i]=malloc(32*sizeof(char));
    }

    int j=2;
    int k=0;
    int l;

  //  printf("%d\n",alt_up_sd_card_find_first(0, first_file));

    if(alt_up_sd_card_find_first(0, first_file) == 0);

    	//	printf("File 1:%s\n", first_file);
    	//	strcpy(tracklist[k], first_file);
    		k++;

        while(alt_up_sd_card_find_next(file) == 0){
    	//	strcpy(tracklist[k], file);
    	//	printf("File %d:%s\n", j, file);
    		j++;
    		k++;
        }
        //alt_up_rs232_write_data(uart, k);

        for(i = 0; i<NUMBER_OF_SONG_FILES; i++) {
        	for(l = 0; l<strlen(FILE_NAME[i]); l++) {
				alt_up_rs232_write_data(uart, FILE_NAME[i][l]);
            }
        	alt_up_rs232_write_data(uart, '~');
        }


    	return tracklist;
}

alt_up_rs232_dev* SetupRS(void)
{
	unsigned char data;
		unsigned char parity;
		char** tracklist;
		printf("starting\n");

		alt_up_rs232_dev* uart = alt_up_rs232_open_dev("/dev/rs232");

	//	printf("debug");
		// Clear Buffer
		printf("Clearing buffer.\n");
		while(alt_up_rs232_get_used_space_in_read_FIFO(uart)){
			alt_up_rs232_read_data(uart, &data, &parity);
		}

		printf("Waiting for data to come back from the Middleman\n");
		while (alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0)
		;
		// First byte is the number of characters in our message
		alt_up_rs232_read_data(uart, &data, &parity);
		int num_to_receive = (int)data;

		printf("waiting for prompt\n");
		printf("%c\n",data);

		while(data != 'A'){
			while (alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0)
			;
			alt_up_rs232_read_data(uart, &data, &parity);
			printf("%c", data);
		}

		printf("prompted\n");

		tracklist = send_playlist(uart);
		//printf(tracklist[0]);
		return uart;
}

int main()
{

	unsigned char data;
	unsigned char parity;
	char** tracklist;
	printf("starting\n");

	alt_up_rs232_dev* uart = alt_up_rs232_open_dev("/dev/rs232");

	// Clear Buffer
	while(alt_up_rs232_get_used_space_in_read_FIFO(uart)){
		alt_up_rs232_read_data(uart, &data, &parity);
	}
	printf("Clearing buffer.\n");

	printf("Waiting for data to come back from the Middleman\n");
	while (alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0)
	;
	// First byte is the number of characters in our message
	alt_up_rs232_read_data(uart, &data, &parity);
	int num_to_receive = (int)data;

	printf("waiting for prompt\n");
	printf("%c\n",data);

	while(data != 'A'){
		while (alt_up_rs232_get_used_space_in_read_FIFO(uart) == 0)
		;
		alt_up_rs232_read_data(uart, &data, &parity);
		printf("%c", data);
	}

	printf("prompted\n");

	tracklist = send_playlist(uart);
	//printf(tracklist[0]);
	while(1)
	{
		get_audio_info(uart);
	}
	return 0;
}

