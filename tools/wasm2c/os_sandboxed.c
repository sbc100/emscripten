// Stubs for OS functions, for a sandboxed environment. Nothing is allowed
// exit the sandbox, calls to printf will fail, etc.

IMPORT_IMPL(void, Z_wasi_snapshot_preview1Z_proc_exitZ_vi, (u32 x), {
  abort_with_message("exit() called");
});

STUB_IMPORT_IMPL(u32, Z_envZ___sys_openZ_iiii, (u32 path, u32 flags, u32 varargs), -1);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_write, (u32 fd, u32 iov, u32 iovcnt, u32 pnum), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_read, (u32 fd, u32 iov, u32 iovcnt, u32 pnum), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_close, (u32 fd), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_sizes_get, (u32 pcount, u32 pbuf_size), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_environ_get, (u32 __environ, u32 environ_buf), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_fd_seek, (u32 fd, u64 offset, u32 whence, u32 new_offset), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_unlink, (u32 path), WASI_DEFAULT_ERROR);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_fstat64, (u32 fd, u32 buf), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_stat64, (u32 path, u32 buf), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_read, (u32 fd, u32 buf, u32 count), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_envZ___sys_access, (u32 pathname, u32 mode), EM_EACCES);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_clock_time_get, (u32 clock_id, u64 max_lag, u32 out), WASI_EINVAL);
STUB_IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_clock_res_get, (u32 clock_id, u32 out), WASI_EINVAL);
