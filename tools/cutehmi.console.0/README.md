# Console

Interactive command line interface, which allows one to browse QML object model and evaluate expressions.

This tool allows one to execute QML expressions. One specifies an extension and the tool will make an attempt to import that
extension and instantiate `Console` component. If the extension does not provide `Console` component an alternative non-GUI
component may be specified with next positional argument. You can use `--help` command line argument to see a list of all possible
command line options.

For example following command imports [Console Example](../../extensions/CuteHMI/Examples/Console.0/) extension, which provides
`Console` component allowing for some basic interaction with the tool.
```
cutehmi.console.0 CuteHMI.Examples.Console.0
```

Component `Console` could be specified directly.
```
cutehmi.console.0 CuteHMI.Examples.Console.0 Console
```

After starting the program, a command prompt is shown. Command line interpreter basically works in two modes: console command mode
and QML expression mode. Any string that starts with `\` character is interpreted as console command; everything else is
interpreted as QML expression.

For example `\quit` string consists of `quit` command prepended by `\` (which by the way is also treated as a command - the one that
enables the console command mode), so it is interpreted as a console command, which quits the console.
```
# \quit
cutehmi.console.0: See you.
```

But the following are QML expressions with the results of evaluation printed below.
```
# 2+2
cutehmi.console.0: QVariant(int, 4)
# console.info("huhu")
qml: huhu
```

The motiviation behind this tool is to make it possible to conveniently set up or configure an extenision, in situations, when no
GUI is available. Creating a schema of a database is an example use case.

## Limitations

Except the obvious limitation like inability to represent GUI features or deficiencies of current version, Console may have
troubles with extensions that use threads. As it does not provide dedicated output window nor customized input, such extensions may
produce glitches in the command line.
