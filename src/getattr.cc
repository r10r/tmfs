#include "tmfs.hh"

int tmfs_getattr(const char *path, struct stat *stbuf)
{
  return getattr_at("", path, stbuf);
}

int getattr_at(const fs::path & known_real_path, const std::string & relative_path, struct stat *stbuf)
{
  // get the real path
  fs::path real_path = get_real_path_at(known_real_path, relative_path);

  // and now just stat the real path
  memset(stbuf, 0, sizeof(struct stat));
  if (lstat(real_path.string().c_str(), stbuf))
    return -errno;
  return 0;
}