# idcCov

## Simple IDC Coverage Highlighter for IDA Free

This is a basic IDC script designed to provide simple code coverage visualization within the **IDA Free** environment.

## Motivation

IDA Free does not support IDAPython, preventing the use of popular and powerful coverage analysis plugins like Lighthouse. This script offers a minimalistic alternative using only the built-in IDC language.

## Features

*   Reads coverage data from a text file (`ModuleName+Offset` format, e.g., `MyModule+1a2b3` or `MyModule.dll+4c5d6`).
*   Highlights corresponding addresses in the currently loaded module.
*   Attempts to handle common extensions (`.dll`, `.exe`, `.sys`) in the coverage file by comparing against the base module name.
*   Uses a cycling color scheme for highlighting across multiple runs within the same IDA session.

## Usage

1.  Prepare your coverage file with one `ModuleName+Offset` entry per line.

![0](https://github.com/user-attachments/assets/3d4f8113-06da-4c64-b655-05f10a7628b4)

2.  Load your target binary in IDA Free.

3.  Run this script (File -> Script file...).

![1](https://github.com/user-attachments/assets/9686d51a-66be-4502-a80c-d9ad093f6895)

4.  Select your coverage file when prompted.

![2](https://github.com/user-attachments/assets/ee35e1ae-a0ce-47fc-991a-5682dcd83c51)

![5](https://github.com/user-attachments/assets/98ad2e23-c5d8-4ed5-970c-02e8e62bfef0)

![6](https://github.com/user-attachments/assets/498d3821-e6b6-4c9d-8cbc-a1918d8ee57a)


## Support

- [TinyInst](https://github.com/googleprojectzero/TinyInst)
- [What The Fuzz](https://github.com/0vercl0k/wtf)


## Important Note for IDA Pro Users

If you are using **IDA Pro**, you should use more advanced tools like **Lighthouse**: [https://github.com/gaasedelen/lighthouse](https://github.com/gaasedelen/lighthouse)


## Purpose & Contribution

This script was created primarily for educational purposes, demonstrating basic coverage visualization and IDC scripting for tasks like analyzing execution flow or specific code points. Due to IDC limitations (especially unreliable string functions), it uses workarounds (like fixed-length extension removal).

Feel free to fork, improve, and add features! Contributions are welcome.
