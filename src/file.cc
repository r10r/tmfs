#include "tmfs.hh"

int tmfs_open(const char * path, struct fuse_file_info * fi)
{
  // get the real path
  std::string real_path = get_real_path(path);

  // open the file
  int fd = open(real_path.c_str(), fi->flags);
  if (fd < 0)
    return -errno;
  fi->fh = fd;
  return 0;
}

int tmfs_release(const char * path, struct fuse_file_info * fi)
{
  close(fi->fh);
  return 0;
}

int tmfs_read(const char * path, char * buf, size_t nbytes, off_t offset,
              struct fuse_file_info * fi)
{
  // read the data
  ssize_t bytes = pread(fi->fh, buf, nbytes, offset);
  if (bytes < 0)
    return -errno;
  return bytes;
}
