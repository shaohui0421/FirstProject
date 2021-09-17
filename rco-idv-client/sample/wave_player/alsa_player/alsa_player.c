/*
 * This example reads standard from input and writes to
 * the default PCM device for 5 seconds of data.
 */
/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <sys/types.h>
#include <alsa/asoundlib.h>
#include <wave_player.h>

static void print_stack(void)
{

    size_t i;
    size_t size;
    char **strings;
    void *array[64] = {0, };

    size = backtrace (array, 64);
    strings = backtrace_symbols (array, size);

    if (!strings) {
        return;
    }

    for (i = 0; i < size; i++) {
        printf("%s\n", strings[i]);
    }

    free (strings);
}

static void dump_pcm_info(snd_pcm_t *handle, snd_pcm_hw_params_t *params, int dir)
{
    unsigned int val, val2;
    snd_pcm_uframes_t frames;

    /* Display information about the PCM interface */
    printf("PCM handle name = '%s'\n", snd_pcm_name(handle));

    printf("PCM state = %s\n", snd_pcm_state_name(snd_pcm_state(handle)));

    snd_pcm_hw_params_get_format(params, &val);
    printf("format = '%s'(%s)\n", snd_pcm_format_name((snd_pcm_format_t)val),
             snd_pcm_format_description((snd_pcm_format_t)val));

    snd_pcm_hw_params_get_subformat(params, (snd_pcm_subformat_t *)&val);
    printf("subformat = '%s' (%s)\n", snd_pcm_subformat_name((snd_pcm_subformat_t)val),
             snd_pcm_subformat_description((snd_pcm_subformat_t)val));

    snd_pcm_hw_params_get_channels(params, &val);
    printf("channels=%d\n", val);

    snd_pcm_hw_params_get_rate(params, &val, &dir);
    printf("rate = %d bps\n", val);

    snd_pcm_hw_params_get_period_time(params, &val, &dir);
    printf("period time = %d us\n", val);

    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    printf("period size = %d frames\n", (int)frames);

    snd_pcm_hw_params_get_buffer_time(params, &val, &dir);
    printf("buffer time = %d us\n", val);

    snd_pcm_hw_params_get_buffer_size(params, (snd_pcm_uframes_t *) &val);
    printf("buffer size = %d frames\n", val);

    snd_pcm_hw_params_get_periods(params, &val, &dir);
    printf("periods per buffer = %d frames\n", val);

    snd_pcm_hw_params_get_rate_numden(params, &val, &val2);
    printf("exact rate=%d/%d bps\n", val, val2);

    val = snd_pcm_hw_params_get_sbits(params);
    printf("significant bits = %d\n", val);

    snd_pcm_hw_params_get_tick_time(params, &val, &dir);
    printf("tick time = %d us\n", val);

    val = snd_pcm_hw_params_is_batch(params);
    printf("is batch = %d\n", val);

    val = snd_pcm_hw_params_is_block_transfer(params);
    printf("is block transfer = %d\n", val);

    val = snd_pcm_hw_params_is_double(params);
    printf("is double = %d\n", val);

    val = snd_pcm_hw_params_is_half_duplex(params);
    printf("is half duplex = %d\n", val);

    val = snd_pcm_hw_params_is_joint_duplex(params);
    printf("is joint duplex = %d\n", val);

    val = snd_pcm_hw_params_can_overrange(params);
    printf("can overrange = %d\n", val);

    val = snd_pcm_hw_params_can_mmap_sample_resolution(params);
    printf("can mmap = %d\n", val);

    val = snd_pcm_hw_params_can_pause(params);
    printf("can overrange = %d\n", val);

    val = snd_pcm_hw_params_can_sync_start(params);
    printf("can sync start = %d\n", val);
}

snd_pcm_t *playing_prepare(snd_pcm_hw_params_t **params, snd_pcm_uframes_t *frames)
{
    snd_pcm_t *handle;
    unsigned int val;
    int dir;
    int rc;

    /* Open PCM device for playback */
    rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        printf("unable to open pcm device: %s\n", snd_strerror(rc));
        exit(1);
    }

    snd_pcm_hw_params_alloca(params);
    snd_pcm_hw_params_any(handle, *params);
    snd_pcm_hw_params_set_access(handle, *params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, *params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle, *params, 2);

    val = 44100;
    snd_pcm_hw_params_set_rate_near(handle, *params, &val, &dir);

    *frames = 256;
    snd_pcm_hw_params_set_period_size_near(handle, *params, frames, &dir);

    rc = snd_pcm_hw_params(handle, *params);
    if (rc < 0) {
        printf("unable to set hw parameters: %s\n", snd_strerror(rc));
        //print_stack();
        dump_pcm_info(handle, *params, dir);
        exit(1);
    }

    snd_pcm_hw_params_get_period_size(*params, frames, &dir);
    return handle;
}

void *playing_raw_data(void *arg)
{
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
    char *buffer;
    int rc;
    int size;
    char *pcm_file;
    playing_pcm_t *pcm_node;
    int fd;
    int playing_count = 0;
    int underrun_count = 0;

    pthread_detach(pthread_self());

    handle = playing_prepare(&params, &frames);
    size = frames * 4;
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
        rc = snd_pcm_writei(handle, buffer, frames);
        if (rc == -EPIPE) {
            underrun_count++;
            printf("underrun occured: %d[%d]\n", underrun_count, playing_count);
        } else if (rc < 0) {
            printf("error from writei: %s\n", snd_strerror(rc));
        } else {
            //printf("frames playing OK\n");
        }
    }

    snd_pcm_drain(handle);
    snd_pcm_prepare(handle);
    close(fd);
    unlink(pcm_file);
    free(pcm_node);

    goto start_playing;
    free(buffer);
    snd_pcm_close(handle);
    return NULL;
}

