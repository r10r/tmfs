#include "tmfs.hh"

static fs::path _get_real_path(const fs::path & known_real_path, const std::string & relative_path)
{
  // use the relative path so that the real_path doesn't get replaced
  const auto clean_path = fs::path(relative_path).relative_path();
  auto it = clean_path.begin();

  fs::path real_path;
  if (known_real_path.empty()) {
    // Start from the root of the HFS
    real_path = fs::path(tmfs::instance().hfs_root());
    real_path /= "Backups.backupdb"; // ${hfs_root}/Backups.backupdb/

    // ok let's copy the 3 first part of the virtual path
    // (${comp_name}, ${date}, ${disk_name})
    for (int i = 0; i < 3 && it != clean_path.end(); ++i, ++it)
      real_path /= *it;
  } else {
    // Start from the known real path, which should already contain the reference to the correct
    // Backups.backupdb sub-directory in the HFS root.
    real_path = known_real_path;
  }

  fs::path private_data_dir = tmfs::instance().hfs_root() / ".HFS+ Private Directory Data\r";
  std::string dir_name_prefix = "dir_";

  // let's resolv all the parts of the path
  struct stat stbuf;
  for (; it != clean_path.end(); ++it)
  {
    real_path /= *it;
    // Does the file exists ?
    if (lstat(real_path.string().c_str(), &stbuf))
      return real_path;

    // Is the file a dir_id ?
    if (S_ISREG(stbuf.st_mode) && stbuf.st_size == 0 && stbuf.st_nlink > 0)
    {
      // build the real path
      std::string dir_name = dir_name_prefix + std::to_string(stbuf.st_nlink);
      fs::path dir_path = private_data_dir / dir_name;

      // check if it's really a ${dir_id}
      if (stat(dir_path.c_str(), &stbuf))
        continue; // it's not
      real_path = dir_path; // it is
    }
  }
  return real_path;
}

fs::path get_real_path(const std::string & str)
{
  auto result = _get_real_path("", str);
#ifndef NDEBUG
  std::cout << "get_real_path(\"" << str << "\") -> " << result << std::endl;
#endif
  return result;
}

fs::path get_real_path_at(const fs::path & known_real_path, const std::string & relative_path)
{
  auto result = _get_real_path(known_real_path, relative_path);
#ifndef NDEBUG
  std::cout << "get_real_path_at(" << known_real_path << ", \"" << relative_path << "\") -> " << result << std::endl;
#endif
  return result;
}