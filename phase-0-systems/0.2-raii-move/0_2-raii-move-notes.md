# 0.2 — Modern C++ II: RAII, smart pointers, move semantics, rvalue references

Working code and observations for topic 0.2.

## Exercise 1 — RAII and deterministic destruction

Source: `ex1.cpp`.

A class `HeapInt` that **owns** a single heap `int`. Constructor does `new int(v)` and
prints `acquire <v>` (acquire); destructor prints `release <v>` and does `delete`
(release). `value()` returns `*ptr_`. `main` creates objects across nested scopes to
observe *when* and *in what order* destructors run.

### 1.1 — What the output shows

```
acquire 1     <- a born (outer scope)
acquire 2     <- b born (inner { } block)
acquire 3     <- c born (inner { } block)
release 3     <- c dies  ┐ at the inner }, BEFORE the program continues
release 2     <- b dies  ┘
acquire 4     <- d born (outer scope, AFTER the block)
release 4     <- d dies  ┐ at main's }, reverse order
release 1     <- a dies  ┘
```

Three facts this proves:

1. **Objects die when their scope ends, not at the end of `main` (automatic
   lifetime).** `release 3` and `release 2` appear *before* `acquire 4` — `b` and `c`
   were already destroyed by the inner `}` before `d` was even created. The scope is
   the `{ }` block, not the whole function.

2. **Within a scope, objects are destroyed in reverse order of construction (LIFO).**
   Built `b` then `c` → destroyed `c` then `b`. A stack of plates: last in, first out.
   This is why the region is called the *stack*.

3. **No `delete` in `main`, yet every `acquire` has a matching `release`.** The
   **destructors** free the heap memory automatically when each scope ends (the
   constructor acquires with `new`, the destructor releases with `delete`). This is
   **RAII** — impossible to forget, unlike the manual `freeBuffer` of 0.1 ex5.

### 1.2 — Why reverse (LIFO) order makes sense

An object created later may depend on one created earlier (it could have been built
using it), but never the other way around — the earlier one existed first and knew
nothing of the later one. Destroying in reverse order therefore guarantees that when
an object is destroyed, nothing still alive depends on it. The order is not arbitrary;
it protects dependencies.

### 1.3 — The single language guarantee behind RAII

When a stack object leaves its scope, its destructor runs **always** — including when
an exception unwinds the stack. That guaranteed, automatic destructor call is the hook
RAII relies on. Everything else in this topic is built on it.

### 1.4 — Mistakes I made (the valuable part)

1. **Mixed manual `new` with `make_unique`, and put work in the class body.** First
   attempt was `new int* a = std::make_unique<int>;` inside the class body. Two errors:
   (a) the class body only **declares** members (`int* ptr_`); the *work* (allocating,
   assigning) goes inside the **constructor**, which is a function; (b) `make_unique` is
   a smart pointer — it belongs to ex2. Ex1 is manual `new`/`delete` on purpose, to feel
   the raw acquire/release before the abstraction.

2. **`value()` returned `ptr_` instead of `*ptr_`.** `value()` is declared to return
   `int`, but `ptr_` is an `int*` (an address) — a type mismatch the compiler rejects.
   The wanted value lives at that address; reach it with the dereference `*ptr_`. Same
   idea as 0.1 ex2/ex4: pointer is the address, `*pointer` is the pointee.

3. **Destructor printed `"acquire"` instead of `"release"`.** Compiles, but ruins the
   experiment: the constructor hook and the destructor hook must print *different* text,
   or the LIFO trace is unreadable (you can't tell creation from destruction).

4. **Tried to use `v` in the destructor, where it does not exist.** `v` was a parameter
   of the *constructor*; it died when the constructor returned. The destructor has no
   `v`. But the value still lives in the heap until the `delete`, so read it with `*ptr_`
   — and necessarily **before** the `delete` (reading after `delete` is use-after-free,
   the 0.1 footgun). The `cout`-before-`delete` order is mandatory once the destructor
   reads the value.

### 1.5 — Style / process notes
- The `value()` getter is correct but unused in `main`; it exists to practise the
  dereference. Optional: call `a.value()` once to see it in action.
- Strip teaching comments before committing (minimal comments; explanations live here,
  not in the code).

### New concepts

- **RAII (Resource Acquisition Is Initialization)** — tie the lifetime of a resource
  to the lifetime of an object: the constructor *acquires* the resource, the destructor
  *releases* it. The release is automatic and cannot be forgotten.
- **constructor vs destructor** — the constructor (`HeapInt(int v)`) runs when the
  object is created and acquires (`new`); the destructor (`~HeapInt()`) runs when the
  scope ends and releases (`delete`). They are the two opposite ends of the object's life.
- **destruction order (LIFO)** — within a scope, objects are destroyed in reverse order
  of construction, which safely respects any dependencies between them.
- **scope as a `{ }` block** — automatic lifetime is bound to the nearest enclosing
  block, not to the whole function; closing an inner `}` destroys that block's objects
  immediately.

## Exercise 2 — `unique_ptr`: single ownership

Source: `ex2.cpp`.

From `HeapInt` (the hand-made owner of ex1) to `std::unique_ptr`: the stdlib version
of the exact same RAII idea, generic and tested. The exercise walks single ownership
end to end — create, fail to copy, transfer with move, the array specialization, and
the runtime cost.

### 2.1 — What the output shows

```
p before move:
  address: 000001DD27D476D0   <- p owns the int (real heap address)
  value:   42
after move:
  p is empty (moved-from, holds nullptr)   <- ownership transferred to q
  q value: 42                              <- q is the new owner
0 1 4 9 16                                  <- arr filled with i*i
sizeof(unique_ptr<int>): 8
sizeof(int*):            8                  <- identical: zero overhead
```

The address goes from a real value (`p` owns the int) to `nullptr` (after the move),
and `q` now holds the `42`. Ownership transfer, visible on screen, not asserted.

### 2.2 — Why `auto q = p;` does not compile (the ownership reason)

`auto q = p;` asks the compiler to **copy** `p`, which needs the copy constructor.
For `unique_ptr` that constructor is `= delete`, so the copy is rejected at compile
time:

```
error C2280: 'std::unique_ptr<int,...>::unique_ptr(const std::unique_ptr<int,...> &)':
attempting to reference a deleted function
```

The signature it names takes `const unique_ptr&` — that *is* the copy constructor.
The reason is **single ownership**: copying would leave `p` and `q` both owning the
same heap `int`; when both die, each calls `delete` on it → **double-free** (UB). That
is exactly the 0.1 ex5 danger. The stdlib forbids the copy so the "one owner" invariant
cannot be violated by accident.

The jump from 0.1: there the shallow copy *compiled fine* and the double-free blew up
at **runtime** (silent, sometimes didn't even crash). Here the language **refuses to
compile**. The danger moved from runtime to compile-time — from a silent mine to a loud
error you see before running.

Copy is deleted, but **move is not**: the move constructor takes `unique_ptr&&` (an
rvalue reference) and is allowed. "Not copyable, but movable" made concrete in the
type's special members.

### 2.3 — Transfer with `std::move`; the moved-from state

`auto q = std::move(p);` transfers ownership. `std::move` itself moves nothing — it is
a cast (`static_cast<T&&>`) that turns the lvalue `p` into an rvalue reference, so the
**move constructor** is selected instead of the (deleted) copy. The move ctor steals
`p`'s internal pointer and nulls it.

After the move, `p` is **valid but empty**: for `unique_ptr` the standard guarantees
`p.get() == nullptr`. Observing a moved-from is safe (`p.get()`, `if (p)`, even
`cout << p`); dereferencing it is UB (see mistakes).

### 2.4 — `unique_ptr<int[]>`: owning a heap array

`make_unique<int[]>(n)` (note the `[]`) selects the **array specialization**, whose
destructor calls `delete[]`, not `delete`. It is the safe version of 0.1's raw
`new[]`/`delete[]`, managed automatically. Notes:
- it **value-initializes** — the five ints start at `0` (raw `new int[5]` would not
  guarantee that).
- access is by index, `arr[i]`; there is no `*arr` for the whole array. `arr.get()`
  returns an `int*` to the first element.

### 2.5 — Cost: `sizeof(unique_ptr<int>) == sizeof(int*)` (zero overhead)

Measured on x64: **8 == 8**. A `unique_ptr<int>` with the default deleter contains
exactly one pointer and nothing else — no refcount, no flag. The default deleter
(`std::default_delete`) is stateless and takes **0 bytes** (empty base optimization).

Runtime cost is **zero**:
- same size in memory, not one byte extra;
- `*p` is the same instruction as on a raw pointer — no added indirection;
- the `delete` in the destructor is **generated code, not stored data** — the same
  `delete` you'd write by hand in 0.1, but automatic and impossible to forget.

This is a **zero-overhead abstraction**: same cost as the raw pointer used correctly,
with the correctness enforced by the compiler. That is why `unique_ptr` is the sane
default in engines.

**Honesty / caveat:** "same size as a raw pointer" holds **with the default, stateless
deleter**. A *stateful custom deleter* (a function pointer, or a capturing lambda) makes
the `unique_ptr` grow to store it, and `sizeof` stops matching `int*`. True for the
common case (the 99%), not a universal law.

### 2.6 — Mistakes I made (the valuable part)

1. **use-after-move: dereferenced a moved-from `unique_ptr`.** After
   `auto q = std::move(p);`, `p` holds `nullptr`. I reused the three-print template from
   the "before" block in the "after" block, including `cout << *p`. `*p` on `nullptr`
   reads address 0 → **access violation** (`exited with code 0xC0000005`,
   `STATUS_ACCESS_VIOLATION`), a crash. The lesson: **observe** a moved-from (`if (p)`,
   `.get()`), never **dereference** it. Same sin as 0.1's use-after-free, different mask:
   using a resource after transferring its ownership. Fix: replaced `*p` with `if (p)`,
   which observes without dereferencing.

2. **dead loop: a counter set to 0 used as the loop bound.** Wrote
   `int count = 0; for (i = 0; i < count; ...)` — `count` was `0`, so `i < 0` is false
   from the first iteration and the body never ran. The array was never filled and never
   printed. It **compiles and runs clean** (exit 0); only the *missing output* reveals
   it. Diagnosis came from running it: had I read the program output instead of asking
   whether the code "looked bad", the empty array section pointed straight at the loop.
   Fix: a single named size `const std::size_t n`, used in `make_unique<int[]>(n)` and in
   both loop bounds — one source of truth, can't desync (duplicated sizes that drift are
   a real cause of buffer overruns).

**Process note:** I pasted code and asked "is this wrong?" *without running it*. The
compiler and the runtime are the source of truth, not eyeballing. Run first, then read
the output — the output is what diagnoses.

### 2.7 — Process / style notes

- The `cout << p` (without `.get()`) line was a deliberate experiment: it confirmed that
  in **C++20**, `cout << unique_ptr` prints the same address as `p.get()`. C++20 added an
  `operator<<` for `unique_ptr` that forwards to `os << get()`; in **C++17** the same line
  does **not** compile. Kept the finding here, dropped the redundant line from the final
  program — prefer the explicit `p.get()`, which states the non-owning *observe* and is
  portable to C++17.
- MSVC prints addresses as 16 hex digits, uppercase, no `0x` (e.g. `000001DD27D476D0`);
  g++ prints lowercase with `0x`. The textual representation is implementation-defined;
  the value is the same idea.

### New concepts

- **single ownership** — exactly one owner of a resource at a time (the `unique_ptr`
  model). Transfer (move) is allowed; duplicate (copy) is not.
- **copy = deleted** — `unique_ptr`'s copy constructor and copy assignment are `= delete`;
  any copy is a compile-time error (`C2280`), enforcing single ownership.
- **`std::move` is a cast** — `static_cast<T&&>` that turns an lvalue into an rvalue
  reference so the move ctor is selected. It moves nothing itself; pure compile-time.
- **moved-from = valid but empty** — after a move the source is valid (safe to destroy or
  reassign) but empty (`unique_ptr` holds `nullptr`). Observing is safe; dereferencing is
  UB.
- **`unique_ptr<int[]>`** — array specialization: destructor uses `delete[]`, access via
  `operator[]`, `make_unique` value-initializes.
- **zero-overhead abstraction** — `unique_ptr` (default deleter) costs the same as a raw
  pointer in size and speed, with correctness enforced by the compiler. Caveat: a stateful
  custom deleter adds size.
- **owner vs observer** — the owner (`unique_ptr`) is responsible for freeing and frees on
  death; an observer (raw `int*` from `.get()`) points without owning and never frees. The
  difference is behaviour plus a compile-time contract, **not bytes** — it does not appear
  in `sizeof`.

---

_Exercises 3–5 will be added here when completed._