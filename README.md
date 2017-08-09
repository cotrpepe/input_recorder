# input_recorder: Record and Replay Your Operation on Linux

This is a linux command which records and replays your operation (keyboard, mouse and touch screen, etc.)

# Demo

## Recording

![Recording](./images/record.gif)

## Replaying

![Replaying](./images/replay.gif)

# Record your operation

## Start recording

```
$ ./record.sh
```

or

```
$ sudo input_recorder -r > tmp.txt
```

or

```
$ sudo DISPLAY=:0 input_recorder -r > tmp.txt // Executing via SSH
```

## Finish recording

Ctrl+C

# Replay your operation

```
$ ./replay.sh
```

or

```
$ sudo input_recorder -p tmp.txt
```

or

```
$ sudo DISPLAY=:0 input_recorder -p tmp.txt // Executing via SSH
```

or

```
$ sudo input_recorder -v -p rec.txt // Verbose logging
```

# How to compile

## Ubuntu 16.04

```
$ sudo apt-get install xorg-dev
$ make
```
