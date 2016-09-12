# UnixShell

This is an interactive UNIX shell supporting common functionalities and file navigation.

##Common functions:   
  - Receive and run a command from users, such as "ls -l"
  - Handle commands with pipes, such as "ls|wc|wc"
  - I/O redirection: "ls>out.txt", "wc<out.txt"
  - Run a program on background: "gedit out.txt &"
  - Job control: "jobs", "fg", "bg", ctrl+z, ctrl+c
  - View history commands: "history"
  - View and add environmental variables: "echopath", "addpath"
  - Change work directory with the build-in command: "cd /usr"
  - Autocompleting：Press TAB to autocomplete command names and file names; press the up or down arrow to reuse history commands; other common used keys are also enabled, such as ctrl+b, ctrl+f, ctrl+u, ctrl+k, and so on

##The file navigation system

Enter the files navigation system if inputting command "cd" without any parameters. It is developed for a fast and convenient viewing of files and folders. You can simply use arrow keys to change directories very fast, as shown in the following graph.
    
![filenavigation](/filenavigation.png)
