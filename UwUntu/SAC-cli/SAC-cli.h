#ifndef SAC_CLI_H_
#define SAC_CLI_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "hablar-servidor.h"


void iniciar_logs();
void *sac_init(struct fuse_conn_info *conn);
int sac_mknod(const char *path, mode_t mode, dev_t dev);
int sac_utimens(const char *path, const struct timespec tv[2]);
static int sac_open(const char *path, struct fuse_file_info *fi);
int sac_write(const char *path, const char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi);
int sac_read(const char *path, char *buffer, size_t size, off_t offset,
		struct fuse_file_info *fi);
int sac_unlink(const char *path);
static int sac_getattr(const char *path, struct stat *statbuf);
//static int sac_getxattr(const char *path, const char *name, const char *value, struct stat *statbuf);
int sac_mkdir(const char *path, mode_t mode);
int sac_rmdir(const char *path);
int sac_opendir(const char *path, struct fuse_file_info *fi);
int sac_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi);


#endif /* SAC_CLI_H_ */
