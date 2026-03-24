For more extensive testing, you can generate your own traces. Making a memory
trace is done using the `valgrind` tool "lackey". The following command will
create a trace of the command `cat README.md`:

```
$ valgrind --tool=lackey --trace-mem=yes --log-file=mytrace.txt cat README.md
```
