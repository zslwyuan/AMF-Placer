@page _2_2_RunAnExample Run An Example
# Run An Example

Below is a command that users can try to run in the "build" directory. It will run the placement flow according to a given JSON configuration, where design/device/paramters are specified.

```bash
./AMFPlacer ../benchmarks/testConfig/OpenPiton.json
```

Users may find that AMFPlacer prints somethings in the terminal and if you are interested in the details of those information, you can find the concrete explanation here: (@subpage _6_runtimeLog).

Users can also turn on the GUI interface to visualize the placement procedure:


```bash
./AMFPlacer ../benchmarks/testConfig/OpenPiton.json -gui
```

In the GUI window, you can zoom in by left click of your mouse, zoom out by right click of your mouse, and reset to orginal scale by the middle button of your mouse.
The critical path and various elements will be colored respectively.

