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
The critical path and various elements will be colored respectively as shown below. You can press "W" and when the mouse moves to an elements, the name of it will be shown.
You can select which types of elements can be visible and you can also visualize the most critical paths by moving the slider on the top bar.

<center>
<img src="GUI.gif" alt="GUI" title="GUI"  height="500"/>   <img src="GUIdetailed.gif" alt="GUIdetailed" title="GUIdetailed" height="500" />  
</center>
