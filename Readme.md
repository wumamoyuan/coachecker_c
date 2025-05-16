# CoAChecker

## Overview

This project is an implementation of the paper "CoAChecker: Safety Analysis on Administrative Policies of Cyber-Oriented Access Control".

CoAChecker is a tool designed to analyze the safety of ACoAC policies. It uses formal methods to check whether ACoAC policies meet the intended safety requirements.

## Installation

1. Clone the repository
2. Make sure you have the required dependencies:
   - NuSMV or nuXmv
   - C compiler (gcc recommended)
3. Download <a href="https://nuxmv.fbk.eu/download.html">nuXmv</a> 
   ```bash
   cd coachecker_c
   wget -O nuXmv.tar.xz https://nuxmv.fbk.eu/theme/download.php?file=nuXmv-2.1.0-linux64.tar.xz
   tar Jxvf nuXmv.tar.xz
   ``` 
4. Compile the project, the executable file will be in the bin directory
   ```bash
   cd src
   make
   ```

## Usage

To analyze an ACoAC policy file:

```bash
./coachecker -model_checker <nuXmv_path> -input <policy_file>
```

Example:
```bash
./coachecker -model_checker ../nuXmv/bin/nuXmv -input ../demo/test1.aabac
```

## Project Structure

- `src/` - Source code
- `bin/` - Compiled binaries
- `lib/` - Library files
- `demo/` - Demo files
- `logs/` - Log files

## License

[Add license information here]