# CoAChecker

## Overview

This project is an implementation of the paper "CoAChecker: Safety Analysis on Administrative Policies of Cyber-Oriented Access Control".

CoAChecker is a tool designed to analyze the safety of ACoAC policies. It uses formal methods to check whether ACoAC policies meet the intended safety requirements.

## Installation

1. Clone the repository
2. Make sure you have the required dependencies:
   - gcc
   - cmake
3. Download [nuXmv](https://nuxmv.fbk.eu/download.html)
   ```bash
   cd coachecker_c
   wget -O nuXmv-2.1.0-linux64.tar.xz https://nuxmv.fbk.eu/theme/download.php?file=nuXmv-2.1.0-linux64.tar.xz
   tar Jxvf nuXmv-2.1.0-linux64.tar.xz
   mv nuXmv-2.1.0-linux64/ nuXmv
   ``` 
4. Build and compile the project, the executable file will be in the bin directory
   ```bash
   cd bin
   ./build.sh clean
   ./build.sh
   ```

## Usage

1. **To analyze an ACoAC policy file:**

   ```bash
   ./coachecker -model_checker <nuXmv_path> -input <policy_file> -timeout <timeout_threshold_in_seconds>
   ```

   Example:
   ```bash
   ./coachecker -model_checker ../nuXmv/bin/nuXmv -input ../demo/test1.aabac -timeout 60
   ```
   For more options, please see the help message:
   ```bash
   ./coachecker -h
   ```

2. **To evaluate the impact of instance scale on the performance of pruning, abstraction refinement, and bound estimation:**
   
   - Download the [dataset](https://drive.google.com/uc?id=1htKafYP5mJkMHnmPpzJuMaVNe6q27XE3&export=download)

      ```bash
      cd coachecker_c
      mkdir data && cd data
      
      ```
   
   - Run the following script and wait for it to complete

      ```bash
      cd coachecker_c/bin
      mkdir -p ../logs
      nohup ./exp_pruning.sh > ../logs/exp_pruning.log 2>&1 &
      ```
      

3. **To determine the individual contribution of each component (*pruning*, *abstraction refinement*, and *bound estimation*):**
   
   - Download the [dataset]()

      ```bash
      cd coachecker_c/data
      wget xxx
      ```

   - Run the script "bin/exp_ablation.sh" and wait for it to complete

      ```bash
      cd coachecker_c/bin
      nohup ./exp_ablation.sh > ../logs/exp_ablation.log 2>&1 &
      ```

      Check the running status of the script

      ```bash
      tail -f exp_ablation.log
      ```
   
   - After the script completes, analyze the output logs of coachecker in the folder "logs/exp_ablation"

      ```bash
      ./analyze_ablation.sh
      ```

      **Note.** The script `analyze_ablation.sh` may take a long time to complete because there are **200** instances in the dataset. For each instance, the script will run coachecker for **4** times with different configurations:  
      - all components are enabled
      - pruning is disabled
      - abstraction refinement is disabled
      - bound estimation is disabled
      
      For each run, the script sets the timeout threshold to **600** seconds. According to the test on a XX server, it requires about **10** hours to complete the experiments. To reduce the running time, it is recommended to set the timeout threshold to a smaller value.

## License

[Add license information here]