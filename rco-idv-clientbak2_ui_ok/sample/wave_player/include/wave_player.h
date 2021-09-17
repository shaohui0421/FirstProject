#ifndef _WAVE_PLAYER_H_
#define _WAVE_PLAYER_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <execinfo.h>
#include <list.h>
#include <alsa/asoundlib.h>

#define PCM_FILENAME_SZ 256
#define PERIODS_PER_PCM 80

typedef struct wave_playing_file_s {
    struct list_head list;
    char pcm_file[PCM_FILENAME_SZ];
} playing_pcm_t;

extern struct list_head playing_file_list;
extern pthread_mutex_t list_mutex;
extern sem_t list_sem;

static inline void playing_list_down(void)
{
    sem_wait(&list_sem);
}

static inline void playing_list_up(void)
{
    sem_post(&list_sem);
}

static inline void playing_list_lock(void)
{
    pthread_mutex_lock(&list_mutex);
}

static inline void playing_list_unlock(void)
{
    pthread_mutex_unlock(&list_mutex);
}

extern snd_pcm_t *playing_prepare(snd_pcm_hw_params_t **params, snd_pcm_uframes_t *frames);
extern void *playing_raw_data(void *arg);
extern void *playing_raw_data_with_pulseaudio(void *arg);

#endif /* _WAVE_PLAYER_H_ */

