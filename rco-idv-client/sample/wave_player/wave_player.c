/*
 * This example reads standard from input and writes to
 * the default PCM device for 5 seconds of data.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <alsa/asoundlib.h>
#include <wave_player.h>

#define PLAYING_WITH_ALSA   1

struct list_head playing_file_list;
pthread_mutex_t list_mutex;
sem_t list_sem;

static inline void playing_list_init(void)
{
    INIT_LIST_HEAD(&playing_file_list);
    pthread_mutex_init(&list_mutex, NULL);
    sem_init(&list_sem, 0, 0);
    system("mkdir -p /root/.pcm/ && rm -rf /root/.pcm/*");
}
#if 0

static void dump_out_data(const void *buffer, size_t bytes)
{
    //printf("dump pcm file.");
    static FILE *fd = NULL;
    static int offset = 0;
    if(fd == NULL) {
        fd=fopen("/root/record.pcm", "wb+");
        if(fd == NULL) {
            printf("DEBUG open /root/record.pcm: %s", strerror(errno));
            offset = 0;
        }
    }
    fwrite(buffer, bytes, 1, fd);
    offset += bytes;
    fflush(fd);
#if 0
    if(offset >= (*size)*1024*1024) {
        *size = 0;
        fclose(fd);
        offset = 0;
        //system("setprop media.audio.record 0");
        printf("TEST playback pcmfile end\n");
    }
#endif
}
#endif

static void record_raw_data(char *buf, size_t size)
{
    int fd, rc;
    char pcm_file[PCM_FILENAME_SZ];
    playing_pcm_t *pcm_node;
    static int file_id = 0;
    static int count = 0;

    snprintf(pcm_file, PCM_FILENAME_SZ, "/root/.pcm/audio%d.pcm", file_id);
    fd = open(pcm_file, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    /* TODO */
    rc = write(fd, buf, size);
    close(fd);

    if (++count % PERIODS_PER_PCM == 0) {
        ++file_id;
        pcm_node = (playing_pcm_t *)malloc(sizeof(playing_pcm_t));
        /* TODO */
        strcpy(pcm_node->pcm_file, pcm_file);
        playing_list_lock();
        list_add_tail(&pcm_node->list, &playing_file_list);
        playing_list_unlock();
        playing_list_up();
    }
}

int main(int argc, char *argv[])
{
    int rc;
    int size;
#ifdef PLAYING_WITH_ALSA
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
#endif
    snd_pcm_uframes_t frames;
    char *buffer;
    int sd, client, ret;
    struct sockaddr_un serveraddr;
    size_t len;
    fd_set rfds;
    pthread_t playing_thread;

#ifdef PLAYING_WITH_ALSA
    handle = playing_prepare(&params, &frames);
    snd_pcm_close(handle);
#else
    frames = 256;
#endif

    size = frames * 4;
    buffer = (char *)malloc(size);
    /* TODO */

    sd = socket(AF_UNIX, SOCK_STREAM, 0);
    /* TODO */
    unlink("/tmp/.hdmi_audio_playing");
    memset(&serveraddr, 0, sizeof(struct sockaddr_un));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, "/tmp/.hdmi_audio_playing");
    len = offsetof(struct sockaddr_un, sun_path) + strlen(serveraddr.sun_path);
    ret = bind(sd, (struct sockaddr *)&serveraddr, len);
    /* TODO */
    ret = listen(sd, 1);
    /* TODO */

    client = -1;
    printf("waiting spice client to connect...\n");
    playing_list_init();
#ifdef PLAYING_WITH_ALSA
    pthread_create(&playing_thread, NULL, playing_raw_data, NULL);
#else
    pthread_create(&playing_thread, NULL, playing_raw_data_with_pulseaudio, NULL);
#endif /* PLAYING_WITH_ALSA */

    while (1) {
        int nready;
        int max_fd;

        FD_ZERO(&rfds);
        FD_SET(sd, &rfds);
        if (client > 0)
            FD_SET(client, &rfds);
        max_fd = (sd > client ? sd : client);
        nready = select(max_fd + 1, &rfds, NULL, NULL, NULL);
        if (nready < 0) {
            sleep(1);
            continue;
        }
        if (FD_ISSET(sd, &rfds)) {
            struct sockaddr_un cliaddr;
            socklen_t clilen = sizeof(cliaddr);

            printf("* spice client connected once.\n");
            client = accept(sd, (struct sockaddr *)&cliaddr, &clilen);
            /* TODO */
            continue;
        }
        if (FD_ISSET(client, &rfds)) {
            //printf("one period raw data coming...\n");
            rc = read(client, buffer, size);
            if (rc == 0) {
                close(client);
                continue;
            }
#if 0
            dump_out_data(buffer, size);
#endif
            record_raw_data(buffer, size);
        }
    }
    free(buffer);
    close(client);
    close(sd);
    return 0;
}

