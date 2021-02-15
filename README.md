# Sparta
Sparta is a virtualization-based endpoint security solution for Windows. It's an educational research project of mine, which tries to demonstrate the power of virtualization-based solutions.

Basically, Sparta traces all suspicious kernel mode code executions, kernel structure manipulations, kernel code modifications and sensitive process memory corruptions, in order to detect abnormal behavior which can indicate a malware infection. See the ["How It Works?"](#how-it-works) section for a deeper explanation.

## Usage
TBD

## Screenshots

### Basic Execution (Bootstrapping & CPUID "Spoofing")
![basic execution](https://github.com/omerk2511/Sparta/blob/master/Screenshots/1.png?raw=true)

### Invisible Syscall Hooking (TLB Splitting)
![shadow hooks](https://github.com/omerk2511/Sparta/blob/master/Screenshots/2.png?raw=true)

## How It Works?
TBD

## License
[MIT](https://choosealicense.com/licenses/mit/)

## Authors
- **Omer Katz** - [omerk2511](https://github.com/omerk2511)