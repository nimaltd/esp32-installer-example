# lib-installer-example-esp32

A small library made to try [lib-installer](https://github.com/nimaltd/lib-installer) on, and a starting point for making your own library installable into an ESP-IDF project.

It does one useful thing. `example_due()` says yes once every `CONFIG_EXAMPLE_PERIOD_MS` milliseconds and never blocks, so a loop can blink an LED, read a sensor or print a line on a rhythm without sleeping. `example_hello()` prints `CONFIG_EXAMPLE_GREETING`. Both settings are Kconfig symbols, in `idf.py menuconfig`. Its `uptime` option adds `example_uptime()`, the time since start up in hours, minutes and seconds.

It also uses what installer.yml gained in 2.0.0 and 2.1.0, so you can see each of them work:

- **A file made once.** Its settings come in `example.defaults`, made in your project's folder and yours from then on. No install writes over it.
- **Lines of CMake by place.** Under `before-project`, lines in your `CMakeLists.txt`, above `include(project.cmake)`, add `example.defaults` to `SDKCONFIG_DEFAULTS`. Under `component`, one puts your project's folder on the library's include path.
- **A step it runs.** `make-id` makes `example_id.h`, this device's own ID, a random number. The installer shows what it runs and asks first. Its script, `tools/make_id.py`, comes into your project's folder as the library's own.

---

## Try the installer with it

Install the installer once:

```bash
pip install lib-installer
```

It needs lib-installer 3.0.0 or newer. `pip install --upgrade lib-installer` brings an older one up to date.

Then, from your project's folder, the one holding `CMakeLists.txt` and `main/`:

```bash
lib-installer nimaltd/lib-installer-example-esp32
```

Press Enter at its questions, and the library goes into `components/example/` as an ESP-IDF component, with a `CMakeLists.txt` and a `Kconfig` the installer writes for it. Your `CMakeLists.txt` gets the block that adds `example.defaults`, and `example.defaults` is made. When your `main` lists the components it uses, or your project builds only what `main` names, as ESP-IDF's examples do, `example` is added to main's list too. Last, it asks about its step:

```
Run
  make-id      Make this device's own ID, example_id.h
               python tools/make_id.py example_id.h
               Run it? [Y/n]:
Made example_id.h: device C6E43235.
               done
```

Enter runs it, and `example_id.h` is in your project's folder, with a number of its own.

Then use it in `main/main.c`:

```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "example.h"

void app_main(void)
{
    example_t tick;

    example_init(&tick);

    while (1)
    {
        if (example_due(&tick))
        {
            example_hello(&tick);
        }

        vTaskDelay(1);
    }
}
```

Build it, flash it and watch it as always:

```bash
idf.py build flash monitor
```

```
Hello from example 1, device C6E43235
Hello from example 2, device C6E43235
Hello from example 3, device C6E43235
```

### Things to try

**Change a setting.** The settings are Kconfig symbols. Set them in `idf.py menuconfig`, under **example**, and build again. Or set them in `example.defaults`, the file the install made, which ESP-IDF reads after `sdkconfig.defaults` when it makes a fresh `sdkconfig`:

```
CONFIG_EXAMPLE_PERIOD_MS=100
CONFIG_EXAMPLE_GREETING="Hi"
```

Run the install command again, and your settings are still there: the output says `kept example.defaults  yours`, and the installer never touches `sdkconfig`.

**Its device ID.** Run the install again, and the step is not asked about: `make-id skipped, example_id.h is already there`, so the ID never changes under you. Delete `example_id.h`, and the next install asks again and makes a new one. Answer for it on the command line instead, with `--run make-id` to run it or `--no-run make-id` to leave it, and `--yes` takes every answer offered, for a script. Nobody to answer, and it is not run; then `example_id()` is 0.

**Look at what it wrote.** The lines of each place sit between two lines naming it, so you can find them, and the next install writes them again:

```cmake
# >>> lib-installer: example/before-project >>>
# From example's installer.yml, and written again on every install.
if(NOT SDKCONFIG_DEFAULTS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sdkconfig.defaults")
    set(SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_SOURCE_DIR}/sdkconfig.defaults")
endif()
list(APPEND SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_SOURCE_DIR}/example.defaults")
# <<< lib-installer: example/before-project <<<
```

Once `SDKCONFIG_DEFAULTS` is set, ESP-IDF reads only the files it names, which is why these lines name `sdkconfig.defaults` first. The component's, `example/component`, are in `components/example/CMakeLists.txt`, and puts your project's folder on the library's include path, which is how `example.c` finds `example_id.h`.

**Add a part, and take it out.** `--with uptime` adds `example_uptime.h` and `example_uptime.c` to the component. `--without uptime` takes them out again.

```bash
lib-installer nimaltd/lib-installer-example-esp32 --with uptime
```

```c
example_uptime_t up = example_uptime();   /* up.hours, up.minutes, up.seconds */
```

**Uninstall it.** Whatever the installer added to your files goes, so they are back as they were, and the library's files go, `tools/make_id.py` with them. `example.defaults` and `example_id.h` stay, since they are yours, and `example.defaults` is no longer read. A setting of its you set yourself in `sdkconfig.defaults` is named, for you to take out:

```bash
lib-installer --uninstall example
```

**Without internet.** Download this repository as a zip, from the green **Code** button, **Download ZIP**, and give the installer the zip instead of the name:

```bash
lib-installer D:/Downloads/lib-installer-example-esp32-main.zip
```

---

## Make your own library installable

Copy [`installer.yml`](installer.yml) to the root of your library's repository, and change what is yours: its name, its description, its repository, its files and its settings. Every key the installer reads is in it, with a comment saying what it does; the ones this library has no use for are commented out. Only `name` and `files` are required.

Then anyone installs your library with:

```bash
lib-installer yourname/yourlibrary
```

Each entry under `esp32.settings` becomes a Kconfig symbol in menuconfig, and your code reads it as `CONFIG_<name>` from `sdkconfig.h`. List the ESP-IDF components your code uses under `esp32.requires`, as this one lists `esp_timer`. `once` lists files for the project's own folder, `run` the steps run after the install, with the script in [`tools/make_id.py`](tools/make_id.py), and `esp32.cmake` the lines of CMake under the place each goes. The [lib-installer README](https://github.com/nimaltd/lib-installer#for-someone-writing-a-library) has the rest.

---

## API

| Function | What it does |
|---|---|
| `void example_init(example_t *handle)` | Start a period of `CONFIG_EXAMPLE_PERIOD_MS` from now |
| `bool example_due(example_t *handle)` | Whether the period has passed. Returns at once, and starts the next one when it has |
| `uint32_t example_count(const example_t *handle)` | How many times `example_due()` has said yes |
| `uint32_t example_id(void)` | This device's own ID, from `example_id.h`, or 0 when the step was not run |
| `void example_hello(const example_t *handle)` | Print `CONFIG_EXAMPLE_GREETING`, how many periods have passed, and the device's ID |
| `example_uptime_t example_uptime(void)` | With the `uptime` option: hours, minutes and seconds since start up |

Declare one `example_t` for each thing that repeats on its own rhythm. A loop that comes back late is caught up, so the rhythm does not drift.

---

## License

Apache License 2.0. See [LICENSE.md](LICENSE.md).
