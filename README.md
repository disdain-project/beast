# BEAST - Binary Evolution And Sentience Toolkit

[![CircleCI badge](https://circleci.com/gh/dedicate-project/beast.svg?style=svg)](https://circleci.com/gh/dedicate-project/beast)
[![Coverage Status](https://coveralls.io/repos/github/dedicate-project/beast/badge.svg?branch=main)](https://coveralls.io/github/dedicate-project/beast?branch=main)

This project defines and implements a virtual machine with a custom instruction set. It operates on byte level and supports all common low-level machine operations, but functions in an entirely virtual environment. This project does not build an x86 interpreter or anything alike, but makes available a custom byte-level machine language that can be used to experiment with code transformations and custom low-level operators.


## Building

First, install the dependencies (assuming you're working on a Ubuntu system):
```bash
sudo apt install clang-tidy ccache
```

To build the project, check out the source code:
```bash
git clone https://github.com/dedicate-project/beast/
```

Then, inside the repository, perform the following actions to actually build the code:
```bash
mkdir build
cd build
cmake ..
make
```

To run all tests, afterwards run:
```bash
make test
```

If you want to create a coverage report, install these additional dependencies:
```bash
sudo apt install lcov npm
```

And run this:
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
make coverage
```


## Adding a new Operator

To add a new operator, you need to follow these steps:

1. Add a new opcode in `include/beast/opcodes.hpp`: Append the new opcode with the next higher hex value at the end of the enum class `OpCode`.
1. Add a signature for this opcode in `include/beast/program.hpp`: This way, client programs can add the operator to a program.
1. Add the program implementation into `src/program.cpp`: This is the implementation of the signature you added in the previous step. Follow the example of existing operators, but in general: Use `appendCode1(...)` with the opcode you added in the first step, and then add any bytes you deem necessary for your operator.
1. Extend the `switch` in `src/cpu_virtual_machine.cpp` in the function `step()`: Add a case for your new opcode, read all parameters (no need to read the opcode, that's done automatically), print a `debug` statement with the operator signature, and call a `VmSession` function that matches your operator.
1. If there is no suitable `VmSession` function, define its signature in `include/beast/vm_session.hpp`: This is a function that explicitly accepts the parameters for your operator. See the existing ones for how to define them.
1. Implement the `VmSession` function you just defined in `src/vm_session.cpp`: Perform the actual logic that should be executed if the VM encounters your operator. Again, see existing operators for how to do it.


## Defined Operators

| Operator                                          | Code | Prm # | Description                                                                                                                                                                                                                                                                                                                                                   | Impl |
|---------------------------------------------------|------|-------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|------|
| No Op                                             | 0x00 | 0     | ``noop()``: Performs no operation                                                                                                                                                                                                                                                                                                                             | y    |
| Declare Variable                                  | 0x01 | 2     | ``declare(v)``: Declare the numeric variable index ``v``.                                                                                                                                                                                                                                                                                                     | y    |
| Set Variable                                      | 0x02 | 3     | ``set_variable(v, f, c)``: Set the value of the numerical variable index ``v``. Follow variable links if ``f`` is ``true``, otherwise don't follow links. In the resolved variable, assign the value ``c``.                                                                                                                                                   | y    |
| Undeclare Variable                                | 0x03 | 1     | ``undeclare_variable(v)``: Undeclare the numerical variable index ``v``.                                                                                                                                                                                                                                                                                      | y    |
| Add Constant to Variable                          | 0x04 | 3     | ``add_constant_to_variable(v, f, c)``: Add the value `c` to the numerical variable index ``v``. Resolve ``v``'s links if ``f`` is ``true``, else don't resolve the links.                                                                                                                                                                                     | y    |
| Add Variable to Variable                          | 0x05 | 4     | ``add_variable_to_variable(vsrc, fsrc, vdest, fdest)``: Add the content of the variable ``vsrc`` (resolve if ``fsrc`` is ``true``) to the content of the variable ``vdest`` (resolve if ``fdest`` is ``true``).                                                                                                                                               | y    |
| Subtract Constant from Variable                   | 0x06 | 3     | ``subtract_constant_from_variable(v, f, c)``: Subtract the value ``c`` from the numerical variable index ``v``. Resolve ``v``'s links if ``f`` is ``true``, else don't resolve the links.                                                                                                                                                                     | y    |
| Subtract Variable from Variable                   | 0x07 | 4     | ``subtract_variable_from_variable(vsrc, fsrc, vdest, fdest)``: Subtract the content of the variable ``vsrc`` (resolve if ``fsrc`` is ``true``) from the content of the variable ``vdest`` (resolve if ``fdest`` is true).                                                                                                                                     | y    |
| Relative Jump to Variable Address if Variable > 0 | 0x08 | 4     | ``relative_var_jump_if_gt0(c, fc, v, fv)``: Perform a relative jump by the value in variable ``v`` if the condition variable ``c`` evaluates to a value > 0. Follow ``c``'s links only if ``fc`` is ``True``, and follow ``v``'s links only if ``fv`` is ``True``.                                                                                            | y    |
| Relative Jump to Variable Address if Variable < 0 | 0x09 | 4     | ``relative_var_jump_if_lt0(c, fc, v, fv)``: Perform a relative jump by the value in variable ``v`` if the condition variable ``c`` evaluates to a value < 0. Follow ``c``'s links only if ``fc`` is ``True``, and follow ``v``'s links only if ``fv`` is ``True``.                                                                                            | y    |
| Relative Jump to Variable Address if Variable = 0 | 0x0a | 4     | ``relative_var_jump_if_eq0(c, fc, v, fv)``: Perform a relative jump by the value in variable ``v`` if the condition variable ``c`` evaluates to a value == 0. Follow ``c``'s links only if ``fc`` is ``True``, and follow ``v``'s links only if ``fv`` is ``True``.                                                                                           | y    |
| Absolute Jump to Variable Address if Variable > 0 | 0x0b | 4     | ``absolute_var_jump_if_gt0(c, fc, v, fv)``: Perform an absolute jump to the value in variable ``v`` if the condition variable ``c`` evaluates to a value > 0. Follow ``c``'s links only if ``fc`` is ``True``, and follow ``v``'s links only if ``fv`` is ``True``.                                                                                           | y    |
| Absolute Jump to Variable Address if Variable < 0 | 0x0c | 4     | ``absolute_var_jump_if_lt0(c, fc, v, fv)``: Perform an absolute jump to the value in variable ``v`` if the condition variable ``c`` evaluates to a value < 0. Follow ``c``'s links only if ``fc`` is ``True``, and follow ``v``'s links only if ``fv`` is ``True``.                                                                                           | y    |
| Absolute Jump to Variable Address if Variable = 0 | 0x0d | 4     | ``absolute_var_jump_if_eq0(c, fc, v, fv)``: Perform an absolute jump to the value in variable ``v`` if the condition variable ``c`` evaluates to a value == 0. Follow ``c``'s links only if ``fc`` is ``True``, and follow ``v``'s links only if ``fv`` is ``True``.                                                                                          | y    |
| Relative Jump if Variable > 0                     | 0x0e | 3     | ``relative_jump_if_var_gt0(c, fc, a)``: Perform a relative jump by ``a`` if the condition variable ``c`` evaluates to a value > 0. Follow ``c``'s reference links only if ``fc`` is ``True``.                                                                                                                                                                 | y    |
| Relative Jump if Variable < 0                     | 0x0f | 3     | ``relative_jump_if_var_lt0(c, fc, a)``: Perform a relative jump by ``a`` if the condition variable ``c`` evaluates to a value < 0. Follow ``c``'s reference links only if ``fc`` is ``True``.                                                                                                                                                                 | y    |
| Relative Jump if Variable = 0                     | 0x10 | 3     | ``relative_jump_if_var_eq0(c, fc, a)``: Perform a relative jump by ``a`` if the condition variable ``c`` evaluates to a value == 0. Follow ``c``'s reference links only if ``fc`` is ``True``.                                                                                                                                                                | y    |
| Absolute Jump if Variable > 0                     | 0x11 | 3     | ``absolute_jump_if_var_gt0(c, fc, a)``: Perform an absolute jump to ``a`` if the condition variable ``c`` evaluates to a value > 0. Follow ``c``'s reference links only if ``fc`` is ``True``.                                                                                                                                                                | y    |
| Absolute Jump if Variable < 0                     | 0x12 | 3     | ``absolute_jump_if_var_lt0(c, fc, a)``: Perform an absolute jump to ``a`` if the condition variable ``c`` evaluates to a value < 0. Follow ``c``'s reference links only if ``fc`` is ``True``.                                                                                                                                                                | y    |
| Absolute Jump if Variable = 0                     | 0x13 | 3     | ``absolute_jump_if_var_eq0(c, fc, a)``: Perform an absolute jump to ``a`` if the condition variable ``c`` evaluates to a value == 0. Follow ``c``'s reference links only if ``fc`` is ``True``.                                                                                                                                                               | y    |
| Load Memory Size into Variable                    | 0x14 | 2     | ``load_memory_size_into_variable(v, f)``: Assign the number of possible variable slots into variable ``v``, resolve ``v``'s links only if ``f`` is ``True``.                                                                                                                                                                                                  | y    |
| Check if Variable is Input                        | 0x15 | 4     | ``check_if_variable_is_input(s, fs, d, fd)``: Determine if variable ``s`` is declared as an input variable (follow its links if ``fs`` is ``True``). If so, store the value ``0x1`` in the variable ``d`` (follow its links if ``fd`` is ``True``), otherwise store ``0x0``.                                                                                  | y    |
| Check if Variable is Output                       | 0x16 | 4     | ``check_if_variable_is_output(s, fs, d, fd)``: Determine if variable ``s`` is declared as an output variable (follow its links if ``fs`` is ``True``). If so, store the value ``0x1`` in the variable ``d`` (follow its links if ``fd`` is ``True``), otherwise store ``0x0``.                                                                                | y    |
| Load Input count into Variable                    | 0x17 |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Load Output count into Variable                   | 0x18 |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Load current address into Variable                | 0x19 |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Print Variable                                    | 0x1a | 3     | ``print_variable(v, fv, a)``: Print the content of the variable ``v`` (follow its links only if ``fv`` is ``True``). If ``a`` is ``True``, print the least significant byte if ``v``'s value as a string character (based on its ASCII value).                                                                                                                | y    |
| Set String Table Entry                            | 0x1b | 3     | ``set_string_table_entry(i, s)``: Set the string table entry at position ``i`` to the value stored in the string ``s``. ``s`` must be null-terminated. ``i`` must be within the string table's available number of entries, and ``s``'s length must be within the string table entries' maximum length. Both are defined by the VM executing the instruction. | y    |
| Print String from String Table                    | 0x1c | 1     | ``print_string_table_entry(i)``: Print the string table entry at position ``i`` to screen. The position must have been set before this instruction.                                                                                                                                                                                                           | y    |
| Load String Table Limit into Variable             | 0x1d |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Terminate                                         | 0x1e | 1     | ``terminate(c)``: Terminate the program immediately and set the return code to the value ``c``.                                                                                                                                                                                                                                                               | y    |
| Copy Variable                                     | 0x1f | 4     | ``copy_variable(s, fs, d, fd)``: Copy the contents of variable ``s`` (resolve links if ``fs`` is ``True``) into the variable ``d`` (resolve links if ``fd`` is ``True``). Only the value is copied, no other flags or characteristics.                                                                                                                                                                                                                                                                                                                                                             | y    |
| Load String Item Length into Variable             | 0x20 |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Load String Item into Variables                   | 0x21 |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Perform System Call                               | 0x22 |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Bit-shift Variable Left                           | 0x23 |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Bit-shift Variable Right                          | 0x24 |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Bit-wise Invert Variable                          | 0x25 |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Bit-wise AND two Variables                        | 0x26 |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Bit-wise OR two Variables                         | 0x27 |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Bit-wise XOR two Variables                        | 0x28 |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Load Random Value into Variable                   | 0x29 |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Modulo Variable by Constant                       | 0x2a |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Modulo Variable by Variable                       | 0x2b |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Rotate Variable Left                              | 0x2c |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Rotate Variable Right                             | 0x2d |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
| Unconditional Jump                                | 0x2e |       |                                                                                                                                                                                                                                                                                                                                                               | n    |
