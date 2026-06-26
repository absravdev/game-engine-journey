# Topic 0.2 — Modern C++ II: RAII, smart pointers, move semantics, rvalue references

**Exercise statements.** Designed for this topic, in increasing difficulty. Write your own code; the proof of understanding is compiling code in Git, not notes. Compile **and run** every exercise (`cl /EHsc /std:c++20 main.cpp`) and check both the compiler output and the program output before moving on. Solutions are **not** included here.

> These exercises build directly on **0.1 Exercise 5**: the manual `IntBuffer` whose shallow copy, double-free and dangling-pointer dangers you exposed on purpose. Here you fix them — twice.

## Exercise 1 — RAII and deterministic destruction (warm-up)

Goal: see a destructor run *by itself*, and in what order.

- Write a class `HeapInt` that **owns** a single heap `int`:
  - Constructor takes an `int v`, does `new int(v)`, and prints `acquire <v>`.
  - Destructor prints `release <v>` and does `delete`.
  - Add an `int value() const` getter.
- In `main`, create several `HeapInt` objects: some in the outer scope, some inside an inner `{ }` block.
- Run it and observe the order of the `release` prints.
- In the README, answer:
  - When exactly does each destructor run? (be precise about scope and the closing `}`)
  - In what order are objects destroyed relative to construction order, and why does that order make sense?
  - What single language guarantee makes RAII work at all?

Do **not** write `delete` anywhere in `main`. The whole point is that you never call it manually.

## Exercise 2 — `unique_ptr`: single ownership

- Create a `std::unique_ptr<int>` with `std::make_unique<int>(42)`. Print the value (`*p`) and the address it holds (`p.get()`).
- Try to write `auto q = p;` (a copy). **It will not compile.** Comment the line out, and in the README explain *why* the compiler rejects it, in terms of ownership.
- Transfer ownership with `std::move`: `auto q = std::move(p);`. Afterwards, test `p` in an `if (p)` and print whether `p` is now empty. Print `*q` to confirm `q` owns the value.
- Create a `std::unique_ptr<int[]>` of size 5 with `std::make_unique<int[]>(5)`, fill it with `i*i`, and print it.
- In the README: what is `sizeof(std::unique_ptr<int>)` on your machine, and how does it compare to `sizeof(int*)`? What does that tell you about the runtime cost of `unique_ptr`?

## Exercise 3 — Writing move by hand: copy vs move, instrumented

This is the core of the topic. You will *see* when a copy happens and when a move happens.

- Write a class `Resource` that owns a heap array `int* data_` plus a `size_t size_`. Instrument every special member with a print so you can trace what runs:
  - Constructor `explicit Resource(size_t n)`: `new int[n]`, fill with `i`, print `ctor n=<n>`.
  - Destructor: print `dtor (data_ null? yes/no)`, then `delete[]`.
  - **Copy constructor**: deep copy (allocate a new array, copy elements). Print `COPY`.
  - **Copy assignment**: print `COPY=`. Free the old array first, then deep copy.
  - **Move constructor**: steal the pointer, null the source. Print `MOVE`. Mark it `noexcept`.
  - **Move assignment**: print `MOVE=`. Free old, steal, null the source, with a self-assignment guard. Mark it `noexcept`.
- In `main`, trigger each path and read the prints:
  - `Resource a(5);`
  - `Resource b = a;` → which special member runs?
  - `Resource c = std::move(a);` → which one now? What is the state of `a` afterwards?
  - Assign into an already-constructed object both ways and observe `COPY=` vs `MOVE=`.
- In the README, explain for each line which special member was selected and why.

## Exercise 4 — lvalues, rvalues, and what `std::move` actually does

- Write two overloads:
  - `void inspect(const std::string& s)` → prints `lvalue (copy path)`.
  - `void inspect(std::string&& s)` → prints `rvalue (move path)`.
- Call `inspect` with:
  - a named variable (`std::string name = "hi"; inspect(name);`)
  - a temporary (`inspect(std::string("temp"));`)
  - a moved named variable (`inspect(std::move(name));`)
- Run it and observe which overload each call selects.
- In the README, answer precisely:
  - What is the difference between an lvalue and an rvalue? Give one example of each from your own code.
  - `std::move` performs no runtime work — what *is* it, mechanically? (one sentence)
  - Why is `return std::move(local);` from a function a **mistake**? (think about RVO)

## Exercise 5 — Integrator: rebuild your `IntBuffer` from 0.1, the safe way

Close the loop. Take the exact `IntBuffer` from **0.1 Exercise 5** and rebuild it twice, killing the three dangers (shallow copy, double-free, dangling pointer) by two different philosophies.

**Version (a) — Rule of Five (manual RAII).** A class `IntBuffer` owning a raw `int* data_`:
- Constructor `explicit IntBuffer(size_t n)`: `new int[n]`, fill with `i*i`.
- Destructor: `delete[]`.
- Copy constructor and copy assignment: **delete them** (`= delete`).
- Move constructor and move assignment: written by hand, `noexcept`, steal + null the source.
- Methods `sum()` and `size()` (same behaviour as the 0.1 free functions, now members).

**Version (b) — Rule of Zero.** Same public interface, but the single data member is a `std::vector<int>`. Write **none** of the five special members.

In `main`, demonstrate concretely that each 0.1 danger is gone:
- For (a): try `IntBuffer b2 = b;` and confirm it **fails to compile** (then comment it out). Then `IntBuffer b2 = std::move(b);` and confirm ownership transferred (the moved-from object is empty).
- For (b): copying is now *allowed and correct* — make a copy, mutate one, and show the other is unaffected (independent arrays = deep copy).

In the README, answer:
- For version (a), exactly why can the double-free from 0.1 no longer happen?
- The two versions choose different policies: (a) **forbids** copying, (b) allows a **deep** copy. For each, give one engine resource where that policy is the right one.
- **Challenge:** in version (a)'s move assignment, what specifically goes wrong if you omit the `if (this != &other)` self-assignment guard? Reason in writing about what `x = std::move(x);` would do without it.

---

**The payoff of 0.2:** the type from 0.1 that had *no ownership policy* now has one the compiler enforces for you. When you reach **Phase 4.4 (custom allocators)** you will write Rule-of-Five types like (a); everywhere else in the engine you will lean on Rule of Zero like (b).
