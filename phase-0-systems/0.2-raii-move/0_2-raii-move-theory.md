# 0.2 — Modern C++ II: RAII, smart pointers, move semantics, rvalue references

Theory notes (study reference).
Roadmap: Phase 0 (systems fundamentals). 0.2 follows 0.1 directly: it fixes the pain that 0.1 Exercise 5 exposed on purpose.

**Study order** (builds better than the title's order): `RAII → ownership → smart pointers → move semantics → rule of zero/five`.
0.1 Exercise 5 left a manual `IntBuffer` with three latent dangers — shallow copy, double-free, dangling pointer. Every idea in this topic exists to remove one of them.

## 1. RAII — Resource Acquisition Is Initialization

- The name is famously bad (Stroustrup himself regrets it). The real idea is simple: **tie the lifetime of a resource to the lifetime of an object.** The resource can be heap memory, a file handle, a mutex lock, a socket. The object lives on the stack.
- It works because of one language guarantee: when a stack object leaves its scope, its **destructor runs. Always.** Even if an exception is thrown. That destructor is the only 100% reliable hook C++ gives you. RAII hijacks it: the **constructor acquires** the resource, the **destructor releases** it.

```cpp
class Buffer {
public:
    explicit Buffer(int n) : size_(n), data_(new int[n]) {}  // acquire
    ~Buffer() { delete[] data_; }                            // release
private:
    int  size_;
    int* data_;
};

{
    Buffer b(10);   // constructor does new[]
    // ... use b ...
}                   // destructor does delete[] HERE, automatically
```

- The `}` is the magic. No `freeBuffer`, nothing to remember, nothing to call twice by mistake. Compare with 0.1's manual `freeBuffer(b)`.

> **The trap that leads to the rest of the topic.** The default copy is a **shallow copy**: it copies the *pointer*, not the array. `Buffer b = a;` gives two objects pointing at one array → both destructors call `delete[]` on it → **double-free (UB)**. This is exactly 0.1 Exercise 5. So RAII alone is not enough: the destructor is automatic, but the default copy is poison for a resource-owning type. It forces one question: **who is the owner?**

## 2. Ownership — the central concept of the topic

- This is the most important idea in 0.2, and it is **conceptual, not syntactic**. A resource needs **exactly one answer** to "who is responsible for freeing it?". That responsible party is the **owner**.
- Three ownership models:
  - **Unique ownership**: one owner at a time. When the owner dies, the resource dies. Ownership can be *transferred* (move) but not *duplicated* (copy). → `unique_ptr`.
  - **Shared ownership**: several owners; the resource lives until the *last* one dies. Needs a reference count. → `shared_ptr`.
  - **Non-owning / observing**: you look at the resource but don't control its lifetime. → raw pointer `T*`, reference `T&`, or `weak_ptr`.

> **Industry honesty (now, not later).** In real engines `shared_ptr` is viewed with suspicion. The refcount has a cost (atomic increments/decrements for thread safety) and, worse, shared ownership is often a symptom of confused design ("nobody knows who the owner is, so everyone is"). Gregory and most AAA codebases lean on **unique ownership + raw pointers as observers + custom allocators**, not `shared_ptr` everywhere. Ignore the usual tutorial advice of "use `shared_ptr` and forget about it". For an engine, the sane default is unique.

## 3. Smart pointers

**`std::unique_ptr<T>`** — the workhorse.
- Owns a heap object. Not copyable (copying = two owners → forbidden, *deleted* at compile time). Movable (transfers ownership).
- **Zero cost** vs a raw pointer: it is literally a pointer with a destructor that calls `delete`. `sizeof(unique_ptr<T>) == sizeof(T*)`. Nothing paid at runtime.
- Create with `std::make_unique<T>(args)`.

```cpp
auto data = std::make_unique<int[]>(10);   // owns the array
// delete[] automatic when data dies. No freeBuffer. No double-free.
// Cannot be copied by accident: the compiler refuses.
```

**`std::shared_ptr<T>`** — shared ownership with a reference count.
- Each copy raises the count; each destruction lowers it; at 0 the resource is freed. Cost: an extra allocation (the control block) and atomic operations. Use `std::make_shared<T>(args)` (one allocation for object + control block).

```
sp1 ──┐         ┌─ control block ─┐
      ├────────►│ refcount: 2     │──► [ the object ]
sp2 ──┘         └─────────────────┘
```

**`std::weak_ptr<T>`** — a non-owning observer of something managed by `shared_ptr`.
- Does not keep it alive. Exists mainly to break **cycles**: if A holds a `shared_ptr` to B and B one to A, neither refcount ever reaches 0 → leak. One side uses `weak_ptr`. You access it via `.lock()`, which returns a `shared_ptr` if the object is still alive. For now, just understand *why* it exists.

## 4. Move semantics and rvalue references (the deep part)

**The problem.** Returning a large `Buffer` from a function, or passing it around. With copy only, each transfer *duplicates the whole heap array*. For 10 ints, fine. For a 50 MB mesh or a 4K texture, catastrophic: you copy megabytes for nothing, because the source is about to die anyway.

**The idea.** Sometimes you are not copying, you are **moving**. The source is a temporary or is about to die. Instead of duplicating the resource, **steal its guts**: transfer the pointer and leave the source empty but valid, so its destructor won't free what you just stole.

> Analogy: copying is photocopying a document (expensive, now you have two). Moving is *handing over* the document with no copy kept (cheap). You don't photocopy a document you're about to throw away — you just hand it over.

**lvalue vs rvalue.** To express "move", the language needs to tell two kinds of value apart:
- **lvalue**: has identity, a name, a stable address you can take with `&`. Persists. In `int x = 5;`, `x` is an lvalue.
- **rvalue**: a temporary, no name, no stable address. `5`, `x + 1`, `makeBuffer(10)` (the returned temporary). About to expire.

```cpp
int x = 5;
int* p = &x;        // OK: x is an lvalue, has an address
int* q = &(x + 1);  // ERROR: x+1 is an rvalue, no address to take
```

**rvalue reference `T&&`.** A reference that binds to rvalues (temporaries). It is the tool that tells a function "what reached me is a temporary, I'm allowed to gut it":

```cpp
void f(Buffer&  b);   // lvalue ref: binds to named, persistent buffers
void f(Buffer&& b);   // rvalue ref: binds to temporaries
```

Overload resolution picks `&&` for temporaries and `&` for named lvalues. That is how the compiler routes a transfer toward "copy" or toward "move".

**Move constructor.** Takes a `T&&` and steals:

```cpp
Buffer(Buffer&& other) noexcept
    : size_(other.size_), data_(other.data_) {  // steal the pointer
    other.data_ = nullptr;   // null the source
    other.size_ = 0;
}
```

- The key line is `other.data_ = nullptr`. After stealing the pointer, you null the source. When `other`'s destructor runs (`delete[] data_`), it's `delete[] nullptr` — a guaranteed no-op. **Zero double-free.** Single owner at all times. This is *literally* the fix for 0.1's double-free, but instead of forbidding the transfer, you make it safe by nulling the source.

```
Before move:                       After move:
  other.data_ ──► [ array ]          other.data_ ──► nullptr    (emptied, safe to destroy)
  new.data_   ──► (unbuilt)          new.data_   ──► [ array ]  (single owner now)
```

- **`noexcept` is not decorative.** Mark moves `noexcept`. `std::vector`, when it grows, only uses your move constructor (instead of copying) if it is `noexcept`; otherwise it falls back to copy to keep its strong exception guarantee. A concrete, real reason that catches many people.

**`std::move` moves nothing.** Learn it this bluntly. It is a **cast**: a `static_cast<T&&>` that turns an lvalue into an rvalue reference. It tells the compiler "I'm done with this named object, treat it as a temporary". The actual movement is done by the move constructor that then gets selected.

```cpp
Buffer a(10);
Buffer b = std::move(a);  // std::move casts a to Buffer&&, selects the move ctor
// a is now in a valid-but-empty state (data_ == nullptr). Don't read its value.
```

- It should have been called `std::cast_to_rvalue`. It does *nothing* at runtime: a pure compile-time cast that changes overload resolution. This closes the loop with smart pointers: `unique_ptr` is non-copyable but movable, so you transfer ownership with `std::move`.

**Move on return (RVO).** When you write `return local;`, the compiler almost always does **RVO/NRVO**: it builds the object directly in its destination and elides both copy *and* move. When it can't elide, it moves (return values are treated as rvalues automatically). Practical conclusions:
- **Returning large objects by value is fine** in modern C++. Don't return raw pointers to "save copies".
- **Never write `return std::move(local);`** — it disables RVO and makes the code *worse*. Just `return local;`.

## 5. Synthesis — Rule of Zero (the moral)

If your class manages a resource (has a non-trivial destructor), the default copy/move operations are almost certainly wrong. Three levels of response:

- **Rule of Three** (C++98): if you define one of {destructor, copy ctor, copy assignment}, you probably need all three.
- **Rule of Five** (C++11): add move ctor and move assignment.
- **Rule of Zero**: **don't manage raw resources yourself.** Put the resource inside a type that already manages it (`unique_ptr`, `std::vector`), and then write **none** of the five — the compiler-generated ones do the right thing.

```cpp
// Rule of Zero — none of the five written by hand:
class Buffer {
    std::vector<int> data_;   // the member already does all the RAII work
public:
    explicit Buffer(int n) : data_(n) {}
    // destructor, copy, move: generated by the compiler and CORRECT
};
```

> **The moral of the whole topic:** in engine *application* code you aim for **Rule of Zero**. You write the five special members by hand only when *you* are building the low-level wrapper — a custom allocator, a wrapper over an OS handle. That happens for real in **Phase 4.4 (custom allocators)**. This is not academic: it is the foundation for writing those allocators correctly.

## Memory diagrams

**The three levels (don't confuse them).** A `unique_ptr` involves three distinct things:

```
STACK                                HEAP
┌────────────────────────────┐
│ data                       │
│  (IS a unique_ptr<int[]>)  │
│    internal ptr ───────────┼─────►  ┌───┬───┬───┬─ ... ─┐
│                            │        │ 0 │ 0 │ 0 │       │  (the resource)
└────────────────────────────┘        └───┴───┴───┴─ ... ─┘

- `data` is the variable; its type is unique_ptr<int[]>. `data` IS the smart pointer.
- the smart pointer is the OWNER of the array.
- the array is the resource owned. "owner" is always defined relative to the resource.
```

**Owner vs observer:**

```cpp
auto data = std::make_unique<int[]>(10);  // data IS the smart pointer = OWNER of the array
int* raw = data.get();                    // raw points at the same array but does NOT own it
// if data dies, raw dangles. One owner, any number of observers; only the owner frees.
```

## Method reminder

- These notes are **support**. The proof that you understand 0.2 is compiling code in Git (the exercises' `.cpp`), not these notes.
- Repo in English (code, comments, README, commits). The Spanish copy of these notes is personal study material and stays out of the repo.
- **Phase 0 milestone:** a CLI tool in C++ with a custom allocator, tests, CMake and CI. RAII and move semantics are direct bricks toward that allocator (4.4) — a good allocator is RAII through and through.
