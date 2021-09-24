# Get Started {#getStarted}

Here, we will go through some basic steps for users to build the placer and run a placement flow for a new design/device with a new placement configuration:
1. Build the Placer
2. Run An Example
3. Extract Design Information from Vivado
4. Extract Device Information from Vivado
5. Set the Placement Configuration in JSON file
6. (Optional) Load the Output Placement in Vivado
7. (Optional) Customize the Placement Flow

**1. Build the Placer**

We provide a script to build the placer and users can run the following command in the project root directory:

```
./build.sh
```

**2. Run An Example**

Below is a command that users can try to run. It will run the placement flow according to a given JSON configuration, where design/device/paramters are specified.

```
./AMFPlacer ../benchmarks/testConfig/OpenPiton.json
```