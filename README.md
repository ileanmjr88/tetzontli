# Tetzontli

**English** · [Español](README.es.md)

> *Tetzontli* — the volcanic foundation stone the Aztecs built on. A fitting
> name for a library of the foundational patterns you build firmware on.

A library of common embedded software patterns, each implemented in **C** (so it
drops straight into a bare-metal project) and **host-tested with GoogleTest**
(C++) using the same CMake + Ninja + sanitizer + CI tooling as a production
codebase.

Each pattern is a self-contained module under [`modules/`](modules/) — one
folder, one header, one implementation, one test suite — and backs one post in
the *Common Embedded Patterns* blog series.

## Why C implementations, C++ tests?

These patterns are meant to run on microcontrollers with no heap, no STL, and
often no FPU. Writing them in plain C keeps them honest and portable to real
hardware. The test harness, however, runs on your host machine, where C++ and
GoogleTest give a fast, expressive way to verify behavior — including edge cases
that are painful to reproduce on a target. C sources are wrapped in
`extern "C"` and linked into C++ test binaries.

## The patterns

| # | Module | Pattern | Status |
|---|--------|---------|--------|
| 1 | [`ring_buffer`](modules/ring_buffer) | Circular FIFO, SPSC, ISR-safe | ✅ built |
| 2 | [`memory_pool`](modules/memory_pool) | Fixed-block static allocator | 📋 planned |
| 3 | [`state_machine`](modules/state_machine) | Finite state machine | 📋 planned |
| 4 | [`command_dispatch`](modules/command_dispatch) | Command dispatch table | 📋 planned |
| 5 | [`event_queue`](modules/event_queue) | Event / message queue | 📋 planned |
| 6 | [`soft_timer`](modules/soft_timer) | Software timers / scheduler | 📋 planned |
| 7 | [`debounce`](modules/debounce) | Switch debouncing | 📋 planned |
| 8 | [`moving_average`](modules/moving_average) | Moving-average / low-pass filter | 📋 planned |
| 9 | [`frame_parser`](modules/frame_parser) | Framing / packet parser | 📋 planned |
| 10 | [`crc`](modules/crc) | CRC-8 / CRC-16 checksum | 📋 planned |
| 11 | [`fixed_point`](modules/fixed_point) | Q-format fixed-point math | 📋 planned |
| 12 | [`bit_ops`](modules/bit_ops) | Register abstraction / bitfields | 📋 planned |

The ordering is deliberate — later modules reuse earlier ones (the command
dispatcher consumes the ring buffer, the frame parser is a state machine that
calls the CRC module, the moving-average filter is backed by a ring buffer).

## Quick start

The toolchain is pinned with [Compendium](https://compendium.ilean.me) — the
same clang, CMake, and Ninja versions on every machine, no Docker, native speed.

```bash
# Install the pinned toolchain (clang, cmake, ninja, ccache)
compendium install

# Activate it for this shell
source <(compendium activate)

# Configure, build, and test using the managed toolchain
cmake --preset compendium
cmake --build build
ctest --test-dir build --output-on-failure
```

Don't have Compendium? Install it with
`curl -fsSL https://compendium.ilean.me/install.sh | sh`, or build with your own
clang/cmake/ninja the plain way: `cmake -B build -G Ninja && cmake --build build`.

### Build options

A second preset enables the sanitizers:

```bash
cmake --preset sanitizers   # AddressSanitizer + UBSan
cmake --build build
ctest --test-dir build --output-on-failure
```

Or pass options directly to any configure:

```bash
cmake --preset compendium \
  -DENABLE_COVERAGE=ON \     # coverage instrumentation
  -DENABLE_CLANG_TIDY=ON \   # clang-tidy on every C source
  -DENABLE_CPPCHECK=ON       # cppcheck on every C source
```

## Adding a module

1. Create `modules/<name>/` with `<name>.h`, `<name>.c`, `test_<name>.cpp`, and
   a `CMakeLists.txt` (copy `ring_buffer/CMakeLists.txt` and rename).
2. Uncomment the `add_subdirectory(modules/<name>)` line in the root
   `CMakeLists.txt`.
3. Build and test.

## Spanish translations

The Spanish docs (`README.es.md` in each module) are my own translations. I'm
**not looking for code contributions** here, but if you spot a translation error
or awkward phrasing, I'd genuinely appreciate a heads-up —
[open a translation issue](../../issues/new?template=spanish-translation.yml).

## License

MIT (see [LICENSE](LICENSE)).
