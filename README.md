# esp32-installer-example

A small library made to try [esp32-installer](https://github.com/nimaltd/esp32-installer) on, and a starting point for making your own library installable into an ESP-IDF project.

It does one useful thing. `example_due()` says yes once every `CONFIG_EXAMPLE_PERIOD_MS` milliseconds and never blocks, so a loop can blink an LED, read a sensor or print a line on a rhythm without sleeping. `example_hello()` prints `CONFIG_EXAMPLE_GREETING`. Both settings are Kconfig symbols, in `idf.py menuconfig`. Its `uptime` option adds `example_uptime()`, the time since start up in hours, minutes and seconds.

---

## Try the installer with it

Install the installer once:

```bash
pip install esp32-installer
```

Then, from your project's folder, the one holding `CMakeLists.txt` and `main/`:

```bash
esp32-installer nimaltd/esp32-installer-example
```

Press Enter at its questions, and the library goes into `components/example/` as an ESP-IDF component, with a `CMakeLists.txt` and a `Kconfig` the installer writes for it. Your own files are left as they are, unless your `main` lists the components it uses, or your project builds only what `main` names, as ESP-IDF's examples do. Then `example` is added to main's list.

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
Hello from example 1
Hello from example 2
Hello from example 3
```

### Things to try

**Change a setting.** The settings are Kconfig symbols. Set them in `idf.py menuconfig`, under **example**, and build again. Or set them in `sdkconfig.defaults`, which ESP-IDF reads when it makes a fresh `sdkconfig`:

```
CONFIG_EXAMPLE_PERIOD_MS=100
CONFIG_EXAMPLE_GREETING="Hi"
```

Run the install command again, and your settings are still there: the installer never touches `sdkconfig`, and writes only its own block in `sdkconfig.defaults`.

**Add a part, and take it out.** `--with uptime` adds `example_uptime.h` and `example_uptime.c` to the component. `--without uptime` takes them out again.

```bash
esp32-installer nimaltd/esp32-installer-example --with uptime
```

```c
example_uptime_t up = example_uptime();   /* up.hours, up.minutes, up.seconds */
```

**Uninstall it.** Whatever the installer added to your files goes, so they are back as they were, and the library's files go. A setting of its you set yourself in `sdkconfig.defaults` is named, for you to take out:

```bash
esp32-installer --uninstall example
```

**Without internet.** Download this repository as a zip, from the green **Code** button, **Download ZIP**, and give the installer the zip instead of the name:

```bash
esp32-installer D:/Downloads/esp32-installer-example-main.zip
```

---

## Make your own library installable

Copy [`installer.yml`](installer.yml) to the root of your library's repository, and change what is yours: its name, its description, its repository, its files and its settings. Every key the installer reads is in it, with a comment saying what it does; the ones this library has no use for are commented out. Only `name` and `files` are required.

Then anyone installs your library with:

```bash
esp32-installer yourname/yourlibrary
```

Each entry under `esp32.settings` becomes a Kconfig symbol in menuconfig, and your code reads it as `CONFIG_<name>` from `sdkconfig.h`. List the ESP-IDF components your code uses under `esp32.requires`, as this one lists `esp_timer`. The [esp32-installer README](https://github.com/nimaltd/esp32-installer#for-someone-writing-a-library) has the rest.

---

## API

| Function | What it does |
|---|---|
| `void example_init(example_t *handle)` | Start a period of `CONFIG_EXAMPLE_PERIOD_MS` from now |
| `bool example_due(example_t *handle)` | Whether the period has passed. Returns at once, and starts the next one when it has |
| `uint32_t example_count(const example_t *handle)` | How many times `example_due()` has said yes |
| `void example_hello(const example_t *handle)` | Print `CONFIG_EXAMPLE_GREETING`, and how many periods have passed |
| `example_uptime_t example_uptime(void)` | With the `uptime` option: hours, minutes and seconds since start up |

Declare one `example_t` for each thing that repeats on its own rhythm. A loop that comes back late is caught up, so the rhythm does not drift.

---

## License

Apache License 2.0. See [LICENSE.md](LICENSE.md).
