@page _2_7_VisualizethePlacementConvergenceProcedure Visualize the Placement Convergence Procedure
# Visualize the Placement Convergence Procedure

If users set the "DumpAllCoordTrace" parameter in the JSON configuration file, a series of "DumpAllCoordTrace-XXX.gz" files which record the location of the design elements will be dumped to the specified directory. Each dumped archive file is for a lower-bound placement iteration or a upper-bound placement iteration in the global placement procedure.

For evaluation and debugging, we provide Python script ("benchmarks/analysisScripts/paintPlacement.py") for users to visualize the placement trace with OpenGL. Please ensure that you have install OpenGL libs, if not you can install them by:

```
pip install PyOpenGL PyOpenGL_accelerate
```

Users can visualize the trace files by the following command:

```
usage example:
python paintPlacement.py -d xxxx/minimap2_allCellPinNet.zip -t xxxxx/dumpData_minimap_GENE -o xxxx/dumpData_minimap_GENE
```

1. -d indicate the design information archive, e.g.   benchmarks/VCU108/design/minimap2/minimap2_allCellPinNet.zip
2. -t indicate the path where the placement trace archives are dumped. (the trace files are required to be named as DumpAllCoordTrace-xxx.gz currently)
3. -o indicate the path where you want to store the output images (png) generated according to the trace files

Below is a screenshot showing the archive files and the generated images. With this images, you can easily generate a video of the convergence procedure with ffmpeg related commands/tools. The colors for the major types of elements are: 
<span style="background:#464646;color:#ff0000">LUT </span>, 
<span style="background:#464646;color:#00ff00">Flip-Flop </span>, 
<span style="background:#464646;color:#0000ff">CARRY </span>, 
<span style="background:#464646;color:#ff00ff">MUX </span>, 
<span style="background:#464646;color:#00ffff">DSP </span>, 
<span style="background:#464646;color:#ffff00">BRAM </span>, 
<span style="background:#464646;color:#ff7f7f">LUTRAM </span>, 
and <span style="background:#464646;color:#ffffff">Others(IO/Buffers...) </span>.

<center>
<img src="visProc.png" align="center" alt="Visualized Example" title="Visualized Example" width="500" /> 
</center>

It is easy to convert this series of images into video or GIF with ffmpeg to visualize the dynamic procedure, like:

```
ffmpeg -framerate 5 -i DumpAllCoordTrace-%d.png  Output.gif
```