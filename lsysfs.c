/**
 * Less Simple, Yet Stupid Filesystem.
 * 
 * Mohammed Q. Hussain - http://www.maastaar.net
 *
 * This is an example of using FUSE to build a simple filesystem. It is a part of a tutorial in MQH Blog with the title "Writing Less Simple, Yet Stupid Filesystem Using FUSE in C": http://maastaar.net/fuse/linux/filesystem/c/2019/09/28/writing-less-simple-yet-stupid-filesystem-using-FUSE-in-C/
 *
 * License: GNU GPL
 */
 
#define FUSE_USE_VERSION 30
#define MAX_COUNT 256

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

// ... //

char dir_list[ MAX_COUNT ][ MAX_COUNT ];
int curr_dir_idx = -1;

char files_list[ MAX_COUNT ][ MAX_COUNT ];
int curr_file_idx = -1;

char files_content[ MAX_COUNT ][ MAX_COUNT ];
int curr_file_content_idx = -1;

struct time_list
{
	struct timespec atime;
	struct timespec ctime;
	struct timespec mtime;
};

struct time_list dir_time_list[MAX_COUNT];
struct time_list file_time_list[MAX_COUNT];

void add_dir( const char *dir_name )
{
	curr_dir_idx++;
	strcpy( dir_list[ curr_dir_idx ], dir_name );

	//set timestamp
	struct timespec now_time;
	timespec_get(&now_time, TIME_UTC);
	dir_time_list[curr_dir_idx].atime = now_time;
	dir_time_list[curr_dir_idx].ctime = now_time;
	dir_time_list[curr_dir_idx].mtime = now_time;
}

int is_dir( const char *path )
{
	path++; // Eliminating "/" in the path
	
	for ( int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++ )
		if ( strcmp( path, dir_list[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

void add_file( const char *filename )
{
	curr_file_idx++;
	strcpy( files_list[ curr_file_idx ], filename );
	
	curr_file_content_idx++;
	strcpy( files_content[ curr_file_content_idx ], "" );
}

int is_file( const char *path )
{
	path++; // Eliminating "/" in the path
	
	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}

int get_file_index( const char *path )
{
	path++; // Eliminating "/" in the path
	
	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return curr_idx;
	
	return -1;
}

int get_dir_index(const char *path)
{
	path++; // Eliminating "/" in the path

	for (int curr_idx = 0; curr_idx < MAX_COUNT; curr_idx++ )
		if ( strcmp( path, dir_list[ curr_idx ] ) == 0 )
			return curr_idx;

	return -1;
}

void write_to_file( const char *path, const char *new_content )
{
	int file_idx = get_file_index( path );
	
	if ( file_idx == -1 ) // No such file
		return;
		
	strcpy( files_content[ file_idx ], new_content ); 

	//set timestamp
	struct timespec now_time;
	timespec_get(&now_time, TIME_UTC);
	file_time_list[file_idx].atime = now_time;
	file_time_list[file_idx].mtime = now_time;
}

int remove_dir( const char* path){

	int dir_index = get_dir_index(path);

	if (dir_index == -1) //No such dir
		return -1;
	
	for ( dir_index; dir_index < MAX_COUNT-1; dir_index++ ){
		strcpy(dir_list[dir_index], dir_list[dir_index+1]);
	}
	curr_dir_idx--;
	return 0;
}

int remove_file( const int file_idx){
	for ( int idx = file_idx; idx < MAX_COUNT-1; idx++ ){
		strcpy(files_list[idx], files_list[idx+1]);
		strcpy(files_content[idx], files_content[idx+1]);
	}
	curr_file_idx--;
	curr_file_content_idx--;
	return 0;
}

// ... //

static int do_getattr( const char *path, struct stat *st )
{
	st->st_uid = getuid(); // The owner of the file/directory is the user who mounted the filesystem
	st->st_gid = getgid(); // The group of the file/directory is the same as the group of the user who mounted the filesystem
	// st->st_atime = time( NULL ); // The last "a"ccess of the file/directory is right now
	// st->st_mtime = time( NULL ); // The last "m"odification of the file/directory is right now
	
	if ( strcmp( path, "/" ) == 0 || is_dir( path ) == 1 )
	{
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	
		//display timestamp
		int dir_index = get_dir_index(path);
		struct timespec now_time;
		timespec_get(&now_time, TIME_UTC);
		st->st_atime = dir_time_list[dir_index].atime.tv_sec;
		st->st_mtime = dir_time_list[dir_index].mtime.tv_sec;
		st->st_ctime = dir_time_list[dir_index].ctime.tv_sec;
	}
	else if ( is_file( path ) == 1 )
	{
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;

		//display timestamp
		int file_index = get_file_index(path);
		struct timespec now_time;
		timespec_get(&now_time, TIME_UTC);
		st->st_atime = file_time_list[file_index].atime.tv_sec;
		st->st_mtime = file_time_list[file_index].mtime.tv_sec;
		st->st_ctime = file_time_list[file_index].ctime.tv_sec;
	}
	else
	{
		return -ENOENT;
	}
	
	return 0;
}

static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	filler( buffer, ".", NULL, 0 ); // Current Directory
	filler( buffer, "..", NULL, 0 ); // Parent Directory
	
	if ( strcmp( path, "/" ) == 0 ) // If the user is trying to show the files/directories of the root directory show the following
	{
		for ( int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++ )
			filler( buffer, dir_list[ curr_idx ], NULL, 0 );
	
		for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
			filler( buffer, files_list[ curr_idx ], NULL, 0 );
	}
	
	return 0;
}

static int do_read( const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi )
{
	int file_idx = get_file_index( path );
	
	if ( file_idx == -1 )
		return -1;
	
	char *content = files_content[ file_idx ];
	
	memcpy( buffer, content + offset, size );

	//set timestamp
	struct timespec now_time;
	timespec_get(&now_time, TIME_UTC);
	file_time_list[file_idx].atime = now_time;
		
	return strlen( content ) - offset;
}

static int do_mkdir( const char *path, mode_t mode )
{
	path++;
	add_dir( path );
	
	return 0;
}

static int do_mknod( const char *path, mode_t mode, dev_t rdev )
{
	path++;
	add_file( path );
	
	return 0;
}

static int do_write( const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info )
{
	write_to_file( path, buffer );
	
	return size;
}

static int do_rmdir( const char *path)
{
	if(is_dir( path ))
		return remove_dir(path);
	return -1;
}

static int do_utimens( const char *path, const struct timespec tv[2])
{

	if(is_dir(path) == 1){
		int dir_index = get_dir_index(path);
	
		if (dir_index == -1) //No such dir
			return -1;

		//set timestamp
		struct timespec now_time;
		timespec_get(&now_time, TIME_UTC);
		dir_time_list[dir_index].atime = now_time;
		dir_time_list[dir_index].ctime = now_time;
		dir_time_list[dir_index].mtime = now_time;


	}else if (is_file(path) == 1){
		int file_idx = get_file_index( path );
	
		if ( file_idx == -1 ) // No such file
			return -1;

		//set timestamp
		struct timespec now_time;
		timespec_get(&now_time, TIME_UTC);
		file_time_list[file_idx].atime = now_time;
		file_time_list[file_idx].ctime = now_time;
		file_time_list[file_idx].mtime = now_time;
		
	}
	
	return 0;
}

static int do_unlink( const char *path)
{
	if(is_file( path ))
	{
		int file_idx = get_file_index( path );
		if ( file_idx == -1 ) // No such file
			return -1;
		return remove_file(file_idx);
	}
	return -1;
}

static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read		= do_read,
	.rmdir		= do_rmdir,
    .mkdir		= do_mkdir,
    .mknod		= do_mknod,
    .write		= do_write,
	.utimens	= do_utimens,
	.unlink		= do_unlink,
};

int main( int argc, char *argv[] )
{
	return fuse_main( argc, argv, &operations, NULL );
}
