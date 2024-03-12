#include "tmfs.hh"

typedef struct dirhandle {
  DIR * dir;
  fs::path real_path;
} dirhandle;

int tmfs_opendir(const char * path, struct fuse_file_info * fi)
{
  // get the real path
  fs::path real_path = get_real_path(path);

  // checks if it's really a directory
  if (!fs::is_directory(real_path))
    return -ENOTDIR;

  dirhandle * dh = new dirhandle;
  dh->dir = opendir(real_path.c_str());
  if (dh->dir == 0)
    return -errno;

  dh->real_path = real_path;
  fi->fh = (intptr_t)dh;

  return 0;
}

int tmfs_releasedir(const char * path, struct fuse_file_info * fi)
{
  dirhandle * dh = (dirhandle *)fi->fh;
  int res = closedir(dh->dir);
  delete dh;
  return res;
}

int tmfs_readdir(const char * path, void * buf, fuse_fill_dir_t filler_cb, off_t offset,
                 struct fuse_file_info * fi)
{
  struct stat stbuf;

  dirhandle * dh = (dirhandle *)fi->fh;

  // XXX: Midnight Commander seems to be doing something weird, and calls readdir twice
  //      on the same directory. As we report everything in one go this means the second call
  //      produces an empty output.
  //      See also https://github.com/littlefs-project/littlefs-fuse/issues/43
  //      This is a workaround for that, and it's obviously quite expensive for big directories ...
  rewinddir(((dirhandle *)fi->fh)->dir);

  // report ./ and ../
  stbuf.st_mode = S_IFDIR | 0755;
  stbuf.st_nlink = 2;
  filler_cb(buf, ".", &stbuf, 0);
  filler_cb(buf, "..", &stbuf, 0);

  struct dirent * entry;
  while ((entry = readdir(dh->dir)))
  {
    // Skip '.' and '..', we already reported those.
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    // stat the file pointed by entry
    if (getattr_at(dh->real_path, entry->d_name, &stbuf))
      continue;
    stbuf.st_mode |= 0755;
    // report the entry
    filler_cb(buf, entry->d_name, &stbuf, 0);
  }
  return 0;
}
