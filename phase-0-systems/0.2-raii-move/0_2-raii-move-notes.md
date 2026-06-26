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

---

_Exercises 2–5 will be added here when completed._
