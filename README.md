# AlacrittyBackground

An app that hooks into the foreground window change event and minimizes other windows when you focus alacritty so that you can see your wallpaper (background).

I like to use alacritty in fullscreen like this:

```yml
window:
  startup_mode: Fullscreen
```

Set the alacritty terminal to be transparent to see the wallpaper:
```yml
window:
  opacity: 0.8 #Change to your opacity
```

## Usage

Simply run the *minimize.exe* file.

To stop it, run the *stop-minimize.bat*.

If you want to also hide the taskbar, use *minimizeTask.exe*

## Compilation

If you want to compile it yourself:

```bash
g++ minimize.cpp -o minimize.exe -Wl,-subsystem,windows
```
