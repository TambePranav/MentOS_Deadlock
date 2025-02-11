///                MentOS, The Mentoring Operating system project
/// @file dirent.c
/// @brief Functions used to manage directories.
/// @copyright (c) 2019 This file is distributed under the MIT License.
/// See LICENSE.md for details.

#include "dirent.h"
#include "string.h"
#include "vfs.h"
#include "kheap.h"
#include "stdio.h"
#include "initrd.h"
#include "debug.h"
#include "syscall_types.h"
#include "assert.h"
#include "errno.h"

DIR *opendir(const char *path)
{
	char absolute_path[PATH_MAX];
	DIR *pdir = NULL;

	strcpy(absolute_path, path);
	// If the first character is not the '/' then get the absolute path.
	if (absolute_path[0] != '/') {
		if (!get_absolute_path(absolute_path)) {
			dbg_print("Cannot get the absolute path.\n");

			return NULL;
		}
	}

	// Get the mount point id.
	int32_t mp_id = get_mountpoint_id(absolute_path);
	if (mp_id < 0) {
		printf("opendir: cannot open directory '%s':"
			   "Cannot find mount-point\n",
			   absolute_path);

		return NULL;
	}

	if (mountpoint_list[mp_id].dir_op.opendir_f == NULL) {
		printf("opendir: cannot open directory '%s':"
			   "No opendir function\n",
			   absolute_path);

		return NULL;
	}

	pdir = mountpoint_list[mp_id].dir_op.opendir_f(absolute_path);
	// If the directiry is correctly open, set the handle.
	if (pdir != NULL) {
		pdir->fd = mp_id;
	}

	return pdir;
}

int closedir(DIR *dirp)
{
	if (dirp == NULL) {
		printf("closedir: cannot close directory :"
			   "Directory pointer is not valid\n");

		return -1;
	}
	if (mountpoint_list[dirp->fd].dir_op.closedir_f == NULL) {
		printf("closedir: cannot close directory '%s':"
			   "No closedir function\n",
			   dirp->path);

		return -1;
	}
	return mountpoint_list[dirp->fd].dir_op.closedir_f(dirp);
}

dirent_t *readdir(DIR *dirp)
{
	dirent_t *dent;
	// Call the readdir system call.
	DEFN_SYSCALL1(dent, __NR_readdir, dirp);
	return dent;
}
