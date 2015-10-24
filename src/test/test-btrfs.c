/*-*- Mode: C; c-basic-offset: 8; indent-tabs-mode: nil -*-*/

/***
  This file is part of systemd.

  Copyright 2014 Lennart Poettering

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#include <fcntl.h>

#include "log.h"
#include "fileio.h"
#include "util.h"
#include "btrfs-util.h"

int main(int argc, char *argv[]) {
        BtrfsQuotaInfo quota;
        int r, fd;

        fd = open("/", O_RDONLY|O_CLOEXEC|O_DIRECTORY);
        if (fd < 0)
                log_error_errno(errno, "Failed to open root directory: %m");
        else {
                char ts[FORMAT_TIMESTAMP_MAX], bs[FORMAT_BYTES_MAX];
                BtrfsSubvolInfo info;

                r = btrfs_subvol_get_info_fd(fd, 0, &info);
                if (r < 0)
                        log_error_errno(r, "Failed to get subvolume info: %m");
                else {
                        log_info("otime: %s", format_timestamp(ts, sizeof(ts), info.otime));
                        log_info("read-only (search): %s", yes_no(info.read_only));
                }

                r = btrfs_qgroup_get_quota_fd(fd, 0, &quota);
                if (r < 0)
                        log_error_errno(r, "Failed to get quota info: %m");
                else {
                        log_info("referenced: %s", strna(format_bytes(bs, sizeof(bs), quota.referenced)));
                        log_info("exclusive: %s", strna(format_bytes(bs, sizeof(bs), quota.exclusive)));
                        log_info("referenced_max: %s", strna(format_bytes(bs, sizeof(bs), quota.referenced_max)));
                        log_info("exclusive_max: %s", strna(format_bytes(bs, sizeof(bs), quota.exclusive_max)));
                }

                r = btrfs_subvol_get_read_only_fd(fd);
                if (r < 0)
                        log_error_errno(r, "Failed to get read only flag: %m");
                else
                        log_info("read-only (ioctl): %s", yes_no(r));

                safe_close(fd);
        }

        r = btrfs_subvol_make("/xxxtest");
        if (r < 0)
                log_error_errno(r, "Failed to make subvolume: %m");

        r = write_string_file("/xxxtest/afile", "ljsadhfljasdkfhlkjdsfha", WRITE_STRING_FILE_CREATE);
        if (r < 0)
                log_error_errno(r, "Failed to write file: %m");

        r = btrfs_subvol_snapshot("/xxxtest", "/xxxtest2", 0);
        if (r < 0)
                log_error_errno(r, "Failed to make snapshot: %m");

        r = btrfs_subvol_snapshot("/xxxtest", "/xxxtest3", BTRFS_SNAPSHOT_READ_ONLY);
        if (r < 0)
                log_error_errno(r, "Failed to make snapshot: %m");

        r = btrfs_subvol_remove("/xxxtest", BTRFS_REMOVE_QUOTA);
        if (r < 0)
                log_error_errno(r, "Failed to remove subvolume: %m");

        r = btrfs_subvol_remove("/xxxtest2", BTRFS_REMOVE_QUOTA);
        if (r < 0)
                log_error_errno(r, "Failed to remove subvolume: %m");

        r = btrfs_subvol_remove("/xxxtest3", BTRFS_REMOVE_QUOTA);
        if (r < 0)
                log_error_errno(r, "Failed to remove subvolume: %m");

        r = btrfs_subvol_snapshot("/etc", "/etc2", BTRFS_SNAPSHOT_READ_ONLY|BTRFS_SNAPSHOT_FALLBACK_COPY);
        if (r < 0)
                log_error_errno(r, "Failed to make snapshot: %m");

        r = btrfs_subvol_remove("/etc2", BTRFS_REMOVE_QUOTA);
        if (r < 0)
                log_error_errno(r, "Failed to remove subvolume: %m");

        r = btrfs_subvol_make("/xxxrectest");
        if (r < 0)
                log_error_errno(r, "Failed to make subvolume: %m");

        r = btrfs_subvol_make("/xxxrectest/xxxrectest2");
        if (r < 0)
                log_error_errno(r, "Failed to make subvolume: %m");

        r = btrfs_subvol_make("/xxxrectest/xxxrectest3");
        if (r < 0)
                log_error_errno(r, "Failed to make subvolume: %m");

        r = btrfs_subvol_make("/xxxrectest/xxxrectest3/sub");
        if (r < 0)
                log_error_errno(r, "Failed to make subvolume: %m");

        if (mkdir("/xxxrectest/dir", 0755) < 0)
                log_error_errno(errno, "Failed to make directory: %m");

        r = btrfs_subvol_make("/xxxrectest/dir/xxxrectest4");
        if (r < 0)
                log_error_errno(r, "Failed to make subvolume: %m");

        if (mkdir("/xxxrectest/dir/xxxrectest4/dir", 0755) < 0)
                log_error_errno(errno, "Failed to make directory: %m");

        r = btrfs_subvol_make("/xxxrectest/dir/xxxrectest4/dir/xxxrectest5");
        if (r < 0)
                log_error_errno(r, "Failed to make subvolume: %m");

        if (mkdir("/xxxrectest/mnt", 0755) < 0)
                log_error_errno(errno, "Failed to make directory: %m");

        r = btrfs_subvol_snapshot("/xxxrectest", "/xxxrectest2", BTRFS_SNAPSHOT_RECURSIVE);
        if (r < 0)
                log_error_errno(r, "Failed to snapshot subvolume: %m");

        r = btrfs_subvol_remove("/xxxrectest", BTRFS_REMOVE_QUOTA|BTRFS_REMOVE_RECURSIVE);
        if (r < 0)
                log_error_errno(r, "Failed to recursively remove subvolume: %m");

        r = btrfs_subvol_remove("/xxxrectest2", BTRFS_REMOVE_QUOTA|BTRFS_REMOVE_RECURSIVE);
        if (r < 0)
                log_error_errno(r, "Failed to recursively remove subvolume: %m");

        r = btrfs_subvol_make("/xxxquotatest");
        if (r < 0)
                log_error_errno(r, "Failed to make subvolume: %m");

        r = btrfs_subvol_auto_qgroup("/xxxquotatest", 0, true);
        if (r < 0)
                log_error_errno(r, "Failed to set up auto qgroup: %m");

        r = btrfs_subvol_make("/xxxquotatest/beneath");
        if (r < 0)
                log_error_errno(r, "Failed to make subvolume: %m");

        r = btrfs_subvol_auto_qgroup("/xxxquotatest/beneath", 0, false);
        if (r < 0)
                log_error_errno(r, "Failed to set up auto qgroup: %m");

        r = btrfs_qgroup_set_limit("/xxxquotatest/beneath", 0, 4ULL * 1024 * 1024 * 1024);
        if (r < 0)
                log_error_errno(r, "Failed to set up quota limit: %m");

        r = btrfs_subvol_set_subtree_quota_limit("/xxxquotatest", 0, 5ULL * 1024 * 1024 * 1024);
        if (r < 0)
                log_error_errno(r, "Failed to set up quota limit: %m");

        r = btrfs_subvol_snapshot("/xxxquotatest", "/xxxquotatest2", BTRFS_SNAPSHOT_RECURSIVE|BTRFS_SNAPSHOT_QUOTA);
        if (r < 0)
                log_error_errno(r, "Failed to setup snapshot: %m");

        r = btrfs_qgroup_get_quota("/xxxquotatest2/beneath", 0, &quota);
        if (r < 0)
                log_error_errno(r, "Failed to query quota: %m");

        assert_se(quota.referenced_max == 4ULL * 1024 * 1024 * 1024);

        r = btrfs_subvol_get_subtree_quota("/xxxquotatest2", 0, &quota);
        if (r < 0)
                log_error_errno(r, "Failed to query quota: %m");

        assert_se(quota.referenced_max == 5ULL * 1024 * 1024 * 1024);

        r = btrfs_subvol_remove("/xxxquotatest", BTRFS_REMOVE_QUOTA|BTRFS_REMOVE_RECURSIVE);
        if (r < 0)
                log_error_errno(r, "Failed remove subvolume: %m");

        r = btrfs_subvol_remove("/xxxquotatest2", BTRFS_REMOVE_QUOTA|BTRFS_REMOVE_RECURSIVE);
        if (r < 0)
                log_error_errno(r, "Failed remove subvolume: %m");

        return 0;
}