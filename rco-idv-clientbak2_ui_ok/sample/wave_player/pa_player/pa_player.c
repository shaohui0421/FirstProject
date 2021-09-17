/*
 * This example reads standard from input and writes to
 * the default PCM device for 5 seconds of data.
 */

#include <sys/types.h>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <wave_player.h>

void *playing_raw_data_with_pulseaudio(void *arg)
{
    char *buffer;
    int rc;
    int error;
    pa_simple *s = NULL;
    char *pcm_file;
    playing_pcm_t *pcm_node;
    int fd;
    int playing_count = 0;
    int size = 1024;
    
    /* The Sample format to use */
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };
    
    pthread_detach(pthread_self());
    s = pa_simple_new(NULL, "wave_player", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error);
    if (s == NULL) {
        /* TODO */
        printf(__FILE__ ": pa_simple_new() failed: %s\n", pa_strerror(error));
        exit(1);
    }
    buffer = (char *)malloc(size);
    /* TODO */
    
start_playing:
    playing_list_down();
    playing_list_lock();
    pcm_node = list_entry(playing_file_list.next, playing_pcm_t, list);
    list_del(&pcm_node->list);
    playing_list_unlock();
    pcm_file = pcm_node->pcm_file;

    fd = open(pcm_file, O_RDONLY);
    /* TODO */

    while (1) {
        playing_count++;
        rc = read(fd, buffer, size);
        if (rc == 0)
            break;
        rc = pa_simple_write(s, buffer, rc, &error);
        if (rc < 0) {
            printf("pa_simple_write() failed: %s\n", pa_strerror(error));
        } else {
            //printf("frames playing OK\n");
        }
    }
    
    pa_simple_drain(s, &error);
    close(fd);
    unlink(pcm_file);
    free(pcm_node);

    goto start_playing;
    if (s)
        pa_simple_free(s);
    return NULL;
}

