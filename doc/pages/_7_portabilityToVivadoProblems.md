# Existing Problems When Exporting To Vivado {#_7_portabilityToVivadoProblem}

Some users might want to use our AMF-Placer as the bridge between their proposed implementation and Vivado. Since the detailed implementation of Vivado is unclear for us and AMF-Placer is still under development for many practical demands, we want users to know our current problems/limitations without being trapped during their implementation/experiment.

Below are some problems/potential causes that we have found:

**1. The implementation of Vivado Shape Builder which generate macros is unclear for us**

1. We might sometime fail to handle some FF-based macros for clock domain crossing. These are some Xilinx Primitives. In this situation, several FFs will be required to be placed in near sites in specific BEL locations but we fail to figure out the implicit rules behind this situation;
2. We might fail to handle some small LUTRAMs macros since it seems that Vivado pre-implementation optimization might fix the order of LUTRAMs in one SLICEM CLB which we fail to disable.

The walk-around solution is the same as the next situation.

**2. The exact rules of Vivado FF packing is unclear for us:**

Alias nets are a bit confusing for our placer. In Vivado, nets might have their "Alias" names. It means that a physical net might have different names in different modules. This might cause problems in FF packing in CLB since in one FF control set, the CLK/Reset/Set/Preset signal should be the same. In our implementation, "the same" signal means one physical net, while in Vivado, it seems "the same" signal means that the name is the same. Below is an example, we try to packing the two FFs in benchmark "MiniMap2" into one control set but we fail. Their reset pins are connected to the same physical net but their pins require different alias names. We have submitted a report of this situation to Xilinx.

```
place_cell {
  design_1_i/xilinx_dma_pcie_ep_0/inst/xdma_0_i/inst/udma_wrapper/dma_top/dma_enable.vul_dma/WR/gen_rdwr_loop[0].gen_rdwr_eng.RDWR_INST/cfg_mrs_nn1_reg[13] SLICE_X107Y51/EFF
  design_1_i/xilinx_dma_pcie_ep_0/inst/xdma_0_i/inst/udma_wrapper/dma_top/dma_enable.vul_dma/WR/gen_rdwr_loop[0].gen_rdwr_eng.RDWR_INST/cfg_mrs_nn1_reg[9] SLICE_X107Y51/FFF
}

ERROR: [Vivado 12-1409] Cannot set loc and bel property of instance(s) 
	to bel FFF. SET/RESET Difference

```

<center>
<img src="aliasNet.png" alt="Alias Nets" title="Alias Nets" width="800" /> 
</center>


The problems above might stop your loading of the generated Tcl file from AMF-Placer. We provide a script "benchmarks/helperPythonScripts/removeFailurePartFromTcl.py" for user as walk-around solution. Please copy the last target BEL location like the one highlighted in the screenshot below.

<center>
<img src="tclErrorExample.png" alt="Packing Error Example" title="Packing Error Example" width="800" /> 
</center>

Then use the script to generate a new Tcl script which will continue the placement loading and bypassing the command cause the error. An example to use the script is shown below:

```
python removeFailurePartFromTcl.py  -i ./DumpCLBPacking-first-0.tcl -o ./new.tcl -e SLICE_X118Y51/FFF
```

**3. Fanout optimization of Vivado might cause commit failure during command 'place_design'**

We find that Vivado placer will duplicate some elements during checking our placed elements for the optimization of high fanout signals. Sometime this will cause commit failure as shown below.

<center>
<img src="commitError.png" alt="Commit Error Example" title="Commit Error Example" width="600" /> 
</center>

The problems above might stop your loading of the generated Tcl file from AMF-Placer. We provide a script "benchmarks/helperPythonScripts/unplaceFailureCells.py" for user as walk-around solution. Please copy the error message like those highlighed in red in the screenshot into a file. Then call the script to generate a  walk-around Tcl script which will unplace the involed cells and restart the P&R procedure.

```
python unplaceFailureCells.py -i ./errors -o ./new.tcl
```