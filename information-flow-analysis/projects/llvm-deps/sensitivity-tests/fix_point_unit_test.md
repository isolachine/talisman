# Changes in unit test

## 1. addr_test

**Configuration:** `pk` in `main` is tainted but its pointer is whitelisted.

**Code:**

```
16 if(ctx->pk_info == NULL) // address not tainted
17     ;
18 if(ctx->pk_b[1] == 3) // Vulnerable Branch reported
19     ;
20 if(ctx->pk_info->info == 0) // vulnerable branch
21     ;
22 if(ctx != NULL) // address not tainted
23     ;
```

**Resulst:**

|     Before     |      After     |
|----------------|----------------|
| main.c line 16 |                |
| main.c line 18 | main.c line 18 |
| main.c line 20 | main.c line 20 |
| main.c line 22 |                |

**Remark:**

Adding `pk` into `ptr_whitelist` removes line 16 and 22 since they are pointer comparison.

If `pk_info` is added into `source` section, line 16 will still be reported since that means `pk_info` is fully tainted.

## 2. context_sensitivity

**Configuration:** The first field of `key` in `main_function` is tainted (`key->a`) but its pointer is whitelisted. `foo` and `baz` are inlined while `bar` is not.

**Code:**

```
15  if (foo(key))
16    ;
17  if (foo(&nokey1))
18    ;
19
20  if (bar(key))
21    ;
22  if (bar(&nokey2))
23    ;
24
25  if (baz(key))
26    ;
27  if (baz(&nokey2))
28    ;
```

**Resulst:**

|     Before     |      After     |
|----------------|----------------|
| main.c line 15 | main.c line 15 |
| main.c line 20 | main.c line 20 |
| main.c line 22 | main.c line 22 |
| main.c line 25 | main.c line 25 |
| main.c line 27 |                |

**Remark:**

Adding `key->a` into `ptr_whitelist` removes line 27. Since DSA treats `key` and `nokey2` as the same piece of memory in `bar`, if `key` is not tainted, `nokey2` shouldn't be either.

## 3. global_undefined

**Configuration:** `GLOBAL` is tainted and it is not a pointer.

**Code:**

```
14  c = undefined_func(a, b);
15  if (a)
16    ;
17  if (b)
18    ;
19  if (c)
20    ;
21
22  y = undefined_func(GLOBAL, x);
23  if (x)
24    ;
25  if (y)
26    ;
```

**Resulst:**

|     Before     |      After     |
|----------------|----------------|
| main.c line 15 |                |
| main.c line 17 |                |
| main.c line 19 |                |
| main.c line 23 |                |
| main.c line 25 | main.c line 25 |

**Remark:**

The result shoudn't change for this one since no `ptr_whitelist` is configured.

## 4. if-O1

**Configuration:** `key1` and `key2` in `function` are tainted and they are not pointers.

**Code:**

```
11  u = key;
12  m = key2 + rand();
13
14  for (u = 1; u < m; u++)
15    PRINT;
```

**Resulst:**

|     Before     |      After     |
|----------------|----------------|
|                | main.c line 14 |


**Remark:**

`m` at line 14 is `key2` added by a random number which makes `m` random, thus not a secret anymore. But in terms of information flow, there always exists a flow from `key2` to `u < m`.

## 5. whitelist_partial

**Configuration:** `key` is tainted and it is not a pointer. `p.x` and `q->y` are whitelisted.

**Code:**

```
21  if (p.x == 4) // Vulnerable + Not reported due to whitelist
22    ;
23  if (p.y == 4) // Vulnerable + Reported
24    ;
25
26  if (q->x == 4) // Vulnerable + Reported
27    ;
28  if (q->y == 4) // Vulnerable + Not reported due to whitelist
29    ;
```

**Resulst:**

|      Before      |       After      |
|------------------|------------------|
|                  | main.cpp line 21 |
| main.cpp line 23 | main.cpp line 23 |
| main.cpp line 26 | main.cpp line 26 |


**Remark:**

Line 21 should not be reported since `p.x` is whitelisted.

## 5. whole_struct

**Configuration:** `key` and `d` are tainted. `d` is a whitelist pointer. `c->x` is  whitelisted.

**Code:**

```
26  if(c->x == 1) // Reported - Probably shouldn't be though
27    ;
28  if(c->y == 1)
29    ;
30  if(c->z == 1)
31    ;
32  if(d.x == 1) // Reported
33    ;
34  if(d.y == 1) // Reported
35    ;
36  if(d.z == 1) // Reported
37    ;
```

**Resulst:**

|      Before      |       After      |
|------------------|------------------|
| main.cpp line 26 |                  |
| main.cpp line 32 | main.cpp line 32 |
| main.cpp line 34 | main.cpp line 34 |
| main.cpp line 36 | main.cpp line 36 |


**Remark:**

Line 26 should not be reported since `c->x` is whitelisted. But if `c` is not a pointer, line 26 will still be reported.