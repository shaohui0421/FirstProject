#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <libgen.h>
#include <sys/time.h>
#include <signal.h>
#include <dirent.h>

#define BUF_LEN		(2*1024*1024)
#define ROOT_DIR	"/run/mnt"
#define USB_MNT		"usb_"
#define	BASE_DIR	"vmbase"

long file_total_size = 0;
char dst_base_file[128];
char src_base_file[128];

typedef void (*sighandler_t)(int);

int safe_read(int fd, void *buf, int count)
{
    int n;

    if (fd < 0 || buf == NULL) {
        return -EINVAL;
    }
    do {
        n = read(fd, buf, count);
    } while (n < 0 && errno == EINTR);

    return n;
}

int safe_write(int fd, const void *buf, int count)
{
    int n;

    if (fd < 0 || buf == NULL) {
        return -EINVAL;
    }

    do {
        n = write(fd, buf, count);
    } while (n < 0 && errno == EINTR);

    return n;
}

int full_write(int fd, const void *buf, size_t len)
{
    int cc;
    int total;

    total = 0;
    if (fd < 0 || buf == NULL) {
        return -EINVAL;
    }

    while (len) {
        cc = safe_write(fd, buf, len);

        if (cc < 0) {
            if (total) {
                return total;
            }
            return cc;  
        }

        total += cc;
        buf = ((const char *)buf) + cc;
        len -= cc;
    }

    return total;
}


int exec_cmd(char *cmd, char *line_buf, int size)
{
    FILE *fp;
    char *last_char;
    
    fp = popen(cmd,"r");
    if(fp == NULL)
    {
        printf("cmd %s err(%s).\n", cmd, strerror(errno));
        return -1;
    }
    
    if (line_buf != NULL) {
    	memset(line_buf, 0, size);
    	if (fgets(line_buf, size-1, fp) == NULL) {
    		goto end;
    	}
    	if (strlen(line_buf) > 0) {
    		last_char = line_buf + strlen(line_buf) - 1;
    		if (*last_char == '\n' || *last_char == '\r') {
    			*last_char = '\0';
    		}
    	}
    }
end:
	pclose(fp);
	return 0;
}

int file_cp(char *src, char *dst)
{
	int ret;
	int src_fd = 0;
	int dst_fd = 0;
	char * buf = NULL;
	int rd_cnt;
	int wr_cnt;
	
	buf = calloc(BUF_LEN, 1); 
	if (buf == NULL) {
		printf("\n%s\n", strerror(errno));
		ret = -1;
		goto end;
	}
	
	src_fd = open(src, O_RDONLY);
	if (src_fd < 0) {
		ret = -1;
		goto end;
	}
	
	dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC , 0666);
	if (src_fd < 0) {
		ret = -1;
		goto end;
	}
	
	while (1) {
		rd_cnt = safe_read(src_fd, buf, BUF_LEN);
		if (rd_cnt < 0) {
			printf("\n%s\n", strerror(errno));
			ret = -1;
			goto end;
		}	
		if (rd_cnt == 0) {
			break;
		} 
		
		wr_cnt = full_write(dst_fd, buf, rd_cnt);
		if (wr_cnt != rd_cnt) {
			printf("\n%s\n", strerror(errno));
			ret = -1;
			goto end;			
		}
	}
end:
	if (buf) {
		free(buf);
	}
	if (src_fd > 0) {
		close(src_fd);
	}
	if (dst_fd > 0) {
		close(dst_fd);
	}
	return ret;
}

int is_file_exist(char * file)
{
	return (access(file, F_OK) == 0) ? 1 : 0;
}

int get_file_bytes(char * file, long * size)
{
	int ret = 0;
	struct stat st;
	ret = stat(file, &st);
	if (ret < 0) {
		printf("file %s: %s\n", file, strerror(errno));
		
	} else {
		*size = st.st_size;
	}
	
	return ret;
}

int get_dir_left_space(char *dir, long * size)
{
	char kblock[15];
	char cmd[64];
	int ret;
	memset(kblock, 0, sizeof(kblock));
	
	snprintf(cmd, sizeof(cmd)-1, "df %s | grep /dev/ | awk '{print $4}'", dir);
	ret = exec_cmd(cmd, kblock, sizeof(kblock));
	if (ret < 0) {
		return -1;
	}
	*size = atol(kblock) * 1024;
	return 0;
}

long b2g(long bytes)
{
	return	(bytes + 1024*1024 -  1) / (1024*1024);
}

void show_process(long total, long finish)
{
	int percent = 0;
	
	percent = (finish *100) / total;
	printf("\r\033 Files total %ld M Copied %ld M,(%d%%)    ", b2g(total), b2g(finish), percent);
	fflush(stdout);
	return;
}

void timer_handler(int signum)  
{  
	long size = 0;
	
	if (get_file_bytes(dst_base_file, &size) < 0) {
		return;
	}
	
	show_process(file_total_size, size);
	return;
}


void install_timer(size_t sec, sighandler_t handler_func)  
{  
    struct sigaction act;  
    struct itimerval tick;  
  
    if(sec > 0) {  
        act.sa_handler = handler_func;  
    }  
    else {  
        act.sa_handler = SIG_DFL;  
    }  
    sigemptyset(&act.sa_mask);  
    act.sa_flags = 0;  
    sigaction(SIGALRM, &act, 0);  
      
    memset(&tick, 0, sizeof(tick));  
    tick.it_value.tv_sec = sec;  
    tick.it_value.tv_usec = 0;  
    tick.it_interval.tv_sec = sec;  
    tick.it_interval.tv_usec = 0;  
  
    setitimer(ITIMER_REAL, &tick, 0); 
    return;
}  

int search_base(char *dir, char * path, int size)
{
	struct dirent *dirp = NULL;
	DIR	*dp = NULL;
	struct stat st;
	char tmp_path[128];
	int find = 0;
	char * suffix;
	
	if (lstat(dir, &st) < 0) {
		goto end;
	}
	if (S_ISDIR(st.st_mode) == 0) {
		goto end;
	}
	
	if ((dp = opendir(dir)) == NULL) {
		goto end;
	}
	
	while ((dirp = readdir(dp)) != NULL) {
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0) {
			continue;
		}
		
		snprintf(tmp_path, sizeof(tmp_path), "%s/%s", dir, dirp->d_name);
		if (lstat(tmp_path, &st) < 0) {
			continue;
		}
		if (S_ISREG(st.st_mode) == 0) {
			continue;
		}
		
		suffix = strrchr(tmp_path, '.');
		if (suffix == NULL) {
			continue;
		}
		
		if (strcmp(suffix, ".base") == 0) {
			find = 1;
			strncpy(path, tmp_path, size - 1);
			break;
		}
	}
	
end:
	if (dp != NULL) {
		closedir(dp);
	}
	
	return find;	
}


int search_dir(char *root, char *base_file_path, int size) 
{
	struct dirent *dirp = NULL;
	DIR	*dp = NULL;
	char tmp_path[128];
	int find = 0;
	
	if (!is_file_exist(root)) {
		goto end;
	}

	if ((dp = opendir(root)) == NULL) {
		goto end;
	}

	while ((dirp = readdir(dp)) != NULL) {
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, ".") == 0) {
			continue;
		}
		
		if (memcmp(dirp->d_name, USB_MNT, strlen(USB_MNT)) != 0) {
			continue;
		}
	
		snprintf(tmp_path, sizeof(tmp_path) - 1, "%s/%s/%s", ROOT_DIR, dirp->d_name, BASE_DIR);
		
		if (is_file_exist(tmp_path)) {
			if (search_base(tmp_path, base_file_path, size) == 1) {
				find = 1;
				break;
			}
		}
	}

end:
	if (dp != NULL) {
		closedir(dp);
	}
	return find;
}

void clean_base(void)
{
	exec_cmd("rm -rf /opt/lessons/*.img", NULL, 0);
	exec_cmd("rm -rf /opt/lessons/*.base", NULL, 0);
	exec_cmd("rm -rf /opt/lessons/*.torrent", NULL, 0);
	exec_cmd("rm -rf /opt/lessons/RCC_Client/vm_image_info.ini", NULL, 0);
}

void mnt_fs(char *root)
{
	struct dirent *dirp = NULL;
	DIR	*dp = NULL;
	char cmd[256];
	char *dev_name;
	if (!is_file_exist(root)) {
		return;
	}

	if ((dp = opendir(root)) == NULL) {
		goto end;
	}

	while ((dirp = readdir(dp)) != NULL) {
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, ".") == 0) {
			continue;
		}

		if (memcmp(dirp->d_name, USB_MNT, strlen(USB_MNT)) != 0) {
			continue;
		}
		dev_name = dirp->d_name + strlen(USB_MNT);


		snprintf(cmd, sizeof(cmd), "mount /dev/%s %s/%s", dev_name, ROOT_DIR, dirp->d_name);

		exec_cmd(cmd, NULL, 0);
	}

end:
	if (dp != NULL) {
		closedir(dp);
	}
	return;

}

int main(void)
{
	char line[128];
	int ret;
	
	mnt_fs(ROOT_DIR);

	if (search_dir(ROOT_DIR, src_base_file, sizeof(src_base_file)) != 1) {
		return 0;
	}
	
	if (get_file_bytes(src_base_file, &file_total_size) < 0) {
		return 0;
	}
	
	snprintf(dst_base_file, sizeof(dst_base_file)-1, "/opt/lessons/%s", basename(src_base_file));
	
	exec_cmd("/bin/plymouth quit &", NULL, 0);
	
	while(1) {
		memset(line, 0, sizeof(line));
		printf("Do you want to install base file: %s [N/y]", basename(src_base_file));
		fflush(stdout);
		if (fgets(line, sizeof(line)-1, stdin) == NULL) {
			return 0;
		}
		
		if (strlen(line)>0 && (line[strlen(line)-1] == '\n' || line[strlen(line)-1] == '\r' )) {
			line[strlen(line)-1] = '\0';
		}
		
		if (line[0] == '\0' || strcasecmp(line, "n") == 0 || strcasecmp(line, "no") == 0) {
			return 0;
		}

		if (strcasecmp(line, "y") == 0 || strcasecmp(line, "yes") == 0) {
			break;
		}	
	}
	
	//printf("src %s dst %s\n", src_base_file, dst_base_file);
	clean_base();
	install_timer(1, timer_handler);
	
	ret = file_cp(src_base_file, dst_base_file);
	install_timer(0, NULL);
	sleep(1);

	if (!ret) {
		timer_handler(0);
		printf("\n file %s install successfully!\n", basename(src_base_file));
	} else {
		printf("\n file %s install failed!\n", basename(src_base_file));
	}
	sleep(2);
	return 0;
}

