struct path {
  int ss;
};

struct tomoyo_path_info {
  const char *a;
  int b;
};

struct tomoyo_request_info {
  const struct tomoyo_path_info *filename;
  const struct tomoyo_path_info *filename2;
  int b;
};

int baz(struct tomoyo_request_info *r, struct tomoyo_path_info *buf);
char *tomoyo_realpath_from_path(struct path *path);

static int tomoyo_get_realpath(struct tomoyo_path_info *buf,
                               struct path *path) {
  buf->a = tomoyo_realpath_from_path(path);
  return 0;
}

int foo(struct path *key) {
  struct tomoyo_path_info buf;
  struct tomoyo_request_info r;

  // buf.a = tomoyo_realpath_from_path(key);
  tomoyo_get_realpath(&buf, key);

  r.filename = &buf;

  baz(&r, &buf);

  return 0;
}
